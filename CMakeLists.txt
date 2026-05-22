add_library(Lute.Runtime)

target_sources(Lute.Runtime PRIVATE
    include/lute/ref.h
    include/lute/runtime.h
    include/lute/userdatas.h
    include/lute/uvrequest.h
    include/lute/uvstream.h
    include/lute/uvutils.h
    Uranium/Types.hpp
    src/ref.cpp
    src/runtime.cpp
    src/uvrequest.cpp
    src/uvutils.cpp
    Uranium/Debug.cpp
    Uranium/Table.cpp
    Uranium/Miscellaneous.cpp
    Uranium/Closure.cpp
)

target_compile_features(Lute.Runtime PUBLIC cxx_std_17)
target_include_directories(Lute.Runtime PUBLIC "include" ${LIBUV_INCLUDE_DIR} PRIVATE "${CMAKE_SOURCE_DIR}/extern/luau/VM/src")
target_link_libraries(Lute.Runtime PUBLIC Lute.Common)
target_link_libraries(Lute.Runtime PRIVATE Luau.Require Luau.Compiler Luau.VM uv_a)
target_compile_options(Lute.Runtime PRIVATE ${LUTE_OPTIONS})
