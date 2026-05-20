#include "lute/runtime.h"

#include "lute/common.h"
#include "lute/ref.h"

#include "Luau/Compiler.h"

#include "lua.h"
#include "luacode.h"
#include "lualib.h"

#include "uv.h"

#include <assert.h>
#include <iostream>
#include <string>
#include <vector>

#include "../Uranium/Debug.h"
#include "../Uranium/Types.h"
#include "../Uranium/Table.h"


static void lua_close_checked(lua_State* L)
{
    if (L)
        lua_close(L);
}

Runtime::Runtime(LuteReporter& reporter)
    : reporter(reporter)
    , globalState(nullptr, lua_close_checked)
    , dataCopy(nullptr, lua_close_checked)
{

    stop.store(false);
    activeTokens.store(0);

    if (uv_loop_init(&eventLoop) < 0)
    {
        LUTE_ASSERT("Couldn't initialize runtime event loop");
    }
}

Runtime::~Runtime()
{
    {
        std::unique_lock lock(continuationMutex);

        stop.store(true);

        runLoopCv.notify_one();
    }

    if (runLoopThreadStarted)
    {
        uv_thread_join(&runLoopThread);
        runLoopThreadStarted = false;
    }
    // At this point, Runtime::hasWork will have returned false (i.e uv_loop_alive is false)
    // This means there are no outstanding handles, or file descriptors or work, to do, and we can exit
    uv_loop_close(&eventLoop);
}

bool Runtime::hasWork()
{
    // TODO: activeTokens and uv_loop_alive have a decent amount of overlap.
    // Unfortunately, we do currently have some places where we add/release
    // tokens that don't correspond to libuv activity, so for now we keep both.
    // uv_ref/unref could be used to patch tokens into the libuv loop itself.
    return hasContinuations() || hasThreads() || activeTokens.load() != 0 || uv_loop_alive(getEventLoop());
}

RuntimeStep Runtime::runOnce()
{
    uv_run_mode mode = (hasContinuations() || hasThreads()) ? UV_RUN_NOWAIT : UV_RUN_ONCE;
    uv_run(getEventLoop(), mode);

    // Complete all C++ continuations
    std::vector<std::function<void()>> copy;

    {
        std::unique_lock lock(continuationMutex);
        copy = std::move(continuations);
        continuations.clear();
    }

    for (auto&& continuation : copy)
        continuation();

    if (runningThreads.empty())
        return StepEmpty{};

    auto next = std::move(runningThreads.front());
    runningThreads.pop_front();

    next.ref->push(GL);
    lua_State* L = lua_tothread(GL, -1);

    if (L == nullptr)
    {
        reporter.reportError("Cannot resume a non-thread reference");
        return StepErr{L};
    }

    // We still have 'next' on stack to hold on to thread we are about to run
    lua_pop(GL, 1);

    int status = LUA_OK;

    // It's possible for a spawned task to be killed by a coroutine.close()
    // before it gets processed in the runningThreads queue. This leads to situations where a thread was scheduled to resume
    // but has already been killed.

    // One example:
    // 1) Main thread executes task.defer on a coroutine.create thread
    // 2) Code is queued up on the thread
    // 3) coroutine.cancel is invoked
    // 4) runtime evaluates callbacks
    // 5) runtime evaluates running threads <- UH OH, found a thread that was scheduled to resume but has already been killed
    // 6) We can just step over it, because
    // a) if it scheduled a resume, the corresponding pending token will have been cleared
    // b) the corresponding ref for the lua state will be freed at the end of Runtime::runOnce()
    int co_status = lua_costatus(GL, L);
    if (co_status == LUA_COFIN)
    {
        clearThreadCompletionHandler(L);
        return StepSuccess{L};
    }

    if (!next.success)
        status = lua_resumeerror(L, nullptr);
    else
        status = lua_resume(L, nullptr, next.argumentCount);

    if (status == LUA_YIELD)
    {
        return StepSuccess{L};
    }

    bool ranCompletionHandler = runThreadCompletionHandler(L, status);
    // Completion handlers are responsible for consuming/reporting terminal errors
    // for threads they own (for example, turning a failed HTTP handler into a 500).
    if (ranCompletionHandler && status != LUA_OK)
        return StepSuccess{L};

    if (status != LUA_OK)
    {
        return StepErr{L};
    };

    if (next.cont)
        next.cont();

    return StepSuccess{L};
}

bool Runtime::runToCompletion()
{
    while (hasWork())
    {
        auto step = runOnce();

        if (auto err = Luau::get_if<StepErr>(&step))
        {
            if (err->L == nullptr)
            {
                reporter.reportError("lua_State* L is nullptr");
                return false;
            }

            reportError(err->L);

            // ensure we exit the process with error code properly
            if (!hasWork())
                return false;
        }
        else
        {
            continue;
        }
    };

    return true;
}

void Runtime::reportError(lua_State* L)
{
    std::string error;

    if (const char* str = lua_tostring(L, -1))
        error = str;

    error += "\nstacktrace:\n";
    error += lua_debugtrace(L);

    
    reporter.reportError(error);
}

void Runtime::runContinuously()
{
    runLoopThreadStarted = uv_thread_create(
        &runLoopThread,
        [](void* arg)
        {
            Runtime* self = static_cast<Runtime*>(arg);
            while (!self->stop)
            {
                {
                    std::unique_lock lock(self->continuationMutex);

                    self->runLoopCv.wait(
                        lock,
                        [self]
                        {
                            return !self->continuations.empty() || self->stop;
                        }
                    );
                }

                self->runToCompletion();
            }
        },
        this
    ) == 0;

    if (!runLoopThreadStarted)
        LUTE_ASSERT("Failed to create runtime runloop thread");
}

bool Runtime::hasContinuations()
{
    std::unique_lock lock(continuationMutex);
    return !continuations.empty();
}

bool Runtime::hasThreads()
{
    return !runningThreads.empty();
}

void Runtime::addThreadCompletionHandler(lua_State* L, ThreadCompletionHandler completion)
{
    threadCompletionHandlers[L] = std::move(completion);
}

bool Runtime::runThreadCompletionHandler(lua_State* L, int status)
{
    auto it = threadCompletionHandlers.find(L);
    if (it == threadCompletionHandlers.end())
        return false;

    ThreadCompletionHandler completion = std::move(it->second);
    clearThreadCompletionHandler(L);

    if (completion.onFinish)
        completion.onFinish(L, status);

    return true;
}

void Runtime::clearThreadCompletionHandler(lua_State* L)
{
    threadCompletionHandlers.erase(L);
}

void Runtime::schedule(std::function<void()> f)
{
    std::unique_lock lock(continuationMutex);

    continuations.push_back(std::move(f));

    runLoopCv.notify_one();
}

void Runtime::scheduleLuauError(std::shared_ptr<Ref> ref, std::string error)
{
    std::unique_lock lock(continuationMutex);

    continuations.push_back(
        [this, ref, error = std::move(error)]() mutable
        {
            ref->push(GL);
            lua_State* L = lua_tothread(GL, -1);
            lua_pop(GL, 1);

            lua_pushlstring(L, error.data(), error.size());
            runningThreads.push_back({false, ref, lua_gettop(L)});
        }
    );

    runLoopCv.notify_one();
}

void Runtime::scheduleLuauResume(std::shared_ptr<Ref> ref, std::function<int(lua_State*)> cont)
{
    std::unique_lock lock(continuationMutex);

    continuations.push_back(
        [this, ref, cont = std::move(cont)]() mutable
        {
            ref->push(GL);
            lua_State* L = lua_tothread(GL, -1);
            lua_pop(GL, 1);

            bool isAlive = !lua_isthreadreset(L);
            int results = cont(L);
            if (isAlive)
            {
                runningThreads.push_back({true, ref, results});
            }
        }
    );

    runLoopCv.notify_one();
}

void Runtime::scheduleLuauCallback(std::shared_ptr<Ref> callbackRef, std::function<int(lua_State*)> argPusher)
{
    std::unique_lock lock(continuationMutex);

    continuations.push_back(
        [this, callbackRef = std::move(callbackRef), argPusher = std::move(argPusher)]() mutable
        {
            lua_State* newThread = lua_newthread(GL);
            std::shared_ptr<Ref> threadRef = getRefForThread(newThread);
            lua_pop(GL, 1);

            callbackRef->push(newThread);
            int nargs = argPusher(newThread);

            runningThreads.push_back({true, threadRef, nargs});
        }
    );

    runLoopCv.notify_one();
}

void Runtime::runInWorkQueue(std::function<void()> f)
{
    auto loop = getEventLoop();
    uv_work_t* work = new uv_work_t();
    work->data = new decltype(f)(std::move(f));

    uv_queue_work(
        loop,
        work,
        [](uv_work_t* req)
        {
            auto task = *(decltype(f)*)req->data;

            task();
        },
        [](uv_work_t* req, int status)
        {
            delete (decltype(f)*)req->data;
            delete req;
        }
    );
}

void Runtime::addPendingToken()
{
    activeTokens.fetch_add(1);
}

void Runtime::releasePendingToken()
{
    [[maybe_unused]] int before = activeTokens.fetch_sub(1);
    assert(before > 0);
}

uv_loop_t* Runtime::getEventLoop()
{
    return &eventLoop;
}

Runtime* getRuntime(lua_State* L)
{
    return static_cast<Runtime*>(lua_getthreaddata(lua_mainthread(L)));
}

uv_loop_t* getRuntimeLoop(lua_State* L)
{
    return getRuntime(L)->getEventLoop();
}

void ResumeTokenData::fail(std::string error)
{
    assert(!completed);
    completed = true;

    runtime->scheduleLuauError(ref, std::move(error));
    runtime->releasePendingToken();
}

void ResumeTokenData::complete(std::function<int(lua_State*)> cont)
{
    assert(!completed);
    completed = true;

    runtime->scheduleLuauResume(ref, std::move(cont));
    runtime->releasePendingToken();
}

ResumeToken getResumeToken(lua_State* L)
{
    ResumeToken token = std::make_shared<ResumeTokenData>();
    token->runtime = getRuntime(L);
    token->ref = getRefForThread(L);
    token->runtime->addPendingToken();

    return token;
}

bool Runtime::runBytecode(const std::string& bytecode, const std::string& chunkname, int argc, char** argv, std::function<void(lua_State*)> onLoaded)
{
    lua_State* L = lua_newthread(GL);

    luaL_sandboxthread(L);

    if (luau_load(L, chunkname.c_str(), bytecode.data(), bytecode.size(), 0) != 0)
    {
        reportError(L);
        lua_pop(GL, 1);
        return false;
    }

    if (onLoaded)
        onLoaded(L);

    if (argc > 0 && argv != nullptr)
    {
        if (!lua_checkstack(L, argc))
        {
            fprintf(stderr, "Failed to pass arguments to Luau\n");
            lua_pop(GL, 1);
            return false;
        }

        for (int i = 0; i < argc; ++i)
            lua_pushstring(L, argv[i]);
    }

    args.clear();
    for (int i = 0; i < argc; ++i)
        args.emplace_back(argv[i]);

    runningThreads.push_back({true, getRefForThread(L), argc});

    lua_pop(GL, 1);

    return runToCompletion();
}

bool Runtime::runSource(const std::string& source, const Luau::CompileOptions& compileOptions, const std::string& chunkname, int argc, char** argv)
{
    std::string bytecode = Luau::compile(source, compileOptions);
    return runBytecode(bytecode, chunkname, argc, argv);
}

// using func = int(lua_State* lua_state_ptr);

void push_debug_lib(lua_State* lua_state_ptr, const char* func_name_str, lua_CFunction func)
{
    lua_getglobal(lua_state_ptr, "debug");
    lua_pushcclosurek(lua_state_ptr, *func, func_name_str, 0, 0);
    lua_setfield(lua_state_ptr, -2, func_name_str);
    return;
}

void push_generic_global(lua_State* lua_state_ptr, const char* func_name_str, lua_CFunction func)
{
    lua_pushcclosurek(lua_state_ptr, func, func_name_str, 0, 0);
    lua_setglobal(lua_state_ptr, func_name_str);
    return;
}


void setup_custom_enviorment(lua_State* lua_state_ptr)
{
    luaL_openlibs(lua_state_ptr);
    
    push_debug_lib(lua_state_ptr, "getmetatable", debug_getmetatable);
    push_debug_lib(lua_state_ptr, "setmetatable", debug_setmetatable);
    push_debug_lib(lua_state_ptr, "getconstants", debug_getconstants);
    push_debug_lib(lua_state_ptr, "getupvalues", debug_getupvalues);
    // push_generic_global(lua_state_ptr, "getrawmetatable", debug_getmetatable);
    //push_generic_global(lua_state_ptr, "setrawmetatable", debug_setmetatable);
    std::vector<function_table_struct>function_table =
    {
    {debug_getmetatable, {"getrawmetatable", "debug_getmetatable", "getRawMetatable", "get_raw_metatable", "getRawMetatable", "setRawMT", "get_raw_mt", "getrawmt", "GetRawMetatable"}},
    {debug_setmetatable, {"setrawmetatable", "debug_setmetatable", "setRawMetatable", "setRawMT", "set_raw_mt", "setrawmt", "SetRawMetatable"}},
    {debug_getconstants, {"getconstants", "get_constants", "getConstants", "GetConstants"}},
    {debug_getupvalues, {"getupvalues", "get_upvalues", "getUpValues", "GetUpValues"}},
    {setreadonly, {"setreadonly", "set_readonly", "set_read_only", "setReadOnly", "SetReadOnly"}},
    {isreadonly, {"is_read_only", "isreadonly", "IsReadOnly", "isReadOnly"}},
    {getnamecallmethod, {"getnamecallmethod", "get_namecall_method", "get_namecall", "getnamecall", "getncm", "get_ncm", "GetNameCallMethod", "getNameCallMethod"}},
    {setnamecallmethod, {"setnamecallmethod", "set_namecall_method", "setncm", "setNameCallMethod", "set_ncm"}},
    {makewriteable, {"make_writeable", "makewriteable", "make_write_able", "makeWriteAble", "MakeWriteAble"}},
    {makereadonly, {"make_readonly", "makereadonly", "make_read_only", "MakeReadOnly"}},
    {iswriteable, {"iswriteable", "is_write_able", "IsWriteAble", "canwrite", "can_write", "CanWrite", "is_writable"}},
    {getfflag, {"getfflag", "get_fflag", "getFFlag", "GetFFlag"}}
};
    for (const function_table_struct& entry: function_table)
    {
    for (const char* name_str: entry.names)
    {
      push_generic_global(lua_state_ptr, name_str, entry.func);
    }
 }


    return;
}


lua_State* setupState(Runtime& runtime, std::function<void(lua_State*)> doBeforeSandbox)
{
    // Separate VM for data copies
    runtime.dataCopy.reset(luaL_newstate());

    runtime.globalState.reset(luaL_newstate());

    lua_State* L = runtime.globalState.get();

    runtime.GL = L;

    lua_setthreaddata(L, &runtime);

    // register the builtin tables
    luaL_openlibs(L);
    setup_custom_enviorment(L);

    // lua_pushnil(L);
    // lua_setglobal(L, "setfenv");

    // lua_pushnil(L);
    // lua_setglobal(L, "getfenv");

    if (doBeforeSandbox)
        doBeforeSandbox(L);

    luaL_sandbox(L);

    return L;
}
