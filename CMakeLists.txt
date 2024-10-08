CMAKE_MINIMUM_REQUIRED(VERSION 3.16)
PROJECT(ClienServerEcn)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
set(CONAN_DISABLE_CHECK_COMPILER True CACHE BOOL "" FORCE )
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

ADD_EXECUTABLE(Server src/server_main.cpp src/Server.cpp src/Server.h src/Common.hpp src/json.hpp src/Ticker.h)
TARGET_LINK_LIBRARIES(Server PRIVATE Threads::Threads CONAN_PKG::boost)

ADD_EXECUTABLE(Client src/client_main.cpp src/Client.cpp src/Client.h src/Common.hpp src/json.hpp)
TARGET_LINK_LIBRARIES(Client PRIVATE Threads::Threads CONAN_PKG::boost)

ADD_EXECUTABLE(BoostTest tests/boost_test_main.cpp src/Server.cpp src/Server.h src/Common.hpp src/json.hpp)
target_link_libraries(BoostTest PRIVATE Threads::Threads CONAN_PKG::boost)
