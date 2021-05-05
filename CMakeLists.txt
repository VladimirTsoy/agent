cmake_minimum_required(VERSION 3.5)
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

file(GLOB SOURCES
    ./src/*.cpp
    ./src/*.h

    ./src/NetApi/*.cpp
    ./src/NetApi/*.h
    ./src/NetApi/*.hpp

    ./src/Settings/*.cpp
    ./src/Settings/*.h
    ./src/Settings/*.hpp

    ./src/Deps/*.cpp
    ./src/Deps/*.h
    ./src/Deps/*.hpp

    ./src/SystemInfo/*.cpp
    ./src/SystemInfo/*.h
    ./src/SystemInfo/*.hpp

    ./src/Plugin/*.cpp
    ./src/Plugin/*.h
    ./src/Plugin/*.hpp

    ./src/Core/*.cpp
    ./src/Core/*.h

    ./src/DB/*.cpp
    ./src/DB/*.h
)

project(AthenaAgent LANGUAGES CXX VERSION 0.1.0)


include_directories(${CMAKE_BINARY_DIR})
configure_file("${CMAKE_SOURCE_DIR}/version.h.in" "${CMAKE_BINARY_DIR}/version.h" @ONLY)

find_package (SQLite3 REQUIRED)
if (SQLite3_FOUND)
  include_directories(${SQLite3_INCLUDE_DIRS})
  link_directories(${SQLite3_LIBRARIES})
  message("SQL OLALA ${SQLite3_LIBRARIES}") # ${SQLITE3_LIBRARIES})
endif ()

find_package (Threads)

#if(WIN32 AND "x${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}" STREQUAL "x")
#        message(WARNING "WIN32 compiler does not specify CMAKE_CXX_COMPILER_ARCHITECTURE_ID -- filling in manually")
#        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
#            set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "x64")
#        else()
#            set(CMAKE_CXX_COMPILER_ARCHITECTURE_ID "x86")
#        endif()
#        message(STATUS "Compiler architecture: ${CMAKE_CXX_COMPILER_ARCHITECTURE_ID}")
#endif()

#add_definitions( -DBOOST_ALL_NO_LIB )
set (Boost_USE_STATIC_LIBS OFF)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (Boost_NO_SYSTEM_PATHS FALSE)
#set (Boost_ARCHITECTURE "-x64")
find_package(Boost 1.72.0 REQUIRED COMPONENTS system filesystem thread coroutine )
set(HEADERS)
if (Boost_FOUND)
    message("Boost found (BOOST_USE_WINAPI_VERSION=${BOOST_USE_WINAPI_VERSION}). " ${BOOST_USE_WINAPI_VERSION})
    include_directories(${Boost_INCLUDE_DIR})
    include_directories(${BOOST_INCLUDE_DIRS})
    message("Boost_INCLUDE_DIR = " ${Boost_INCLUDE_DIR})
    message("PROJECT_BINARY_DIR = " ${PROJECT_BINARY_DIR})
    message("CMAKE_BINARY_DIR = " ${CMAKE_BINARY_DIR})
    message("PWD = " ${PWD})
    include_directories("${PROJECT_BINARY_DIR}/plugins/")
    link_directories(${Boost_LIBRARY_DIRS})
    add_definitions("-DHAS_BOOST")
    add_definitions(${Boost_DEFINITIONS})
    include_directories(${BOOST_ROOT})
endif()

set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL 1.1.0 REQUIRED COMPONENTS Crypto SSL)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
    #link_directories(${OPENSSL_LIBRARIES})
endif()


add_executable(AthenaAgent ${SOURCES})
target_include_directories(AthenaAgent PUBLIC "${PROJECT_BINARY_DIR}")

if (SQLite3_FOUND)
  message("link sqlite3")

  #  target_link_libraries(AthenaAgent SQLite::SQLite3)
  target_link_libraries(AthenaAgent sqlite3)
endif ()

target_link_libraries (AthenaAgent ${CMAKE_THREAD_LIBS_INIT})

if (UNIX AND NOT APPLE)
    target_link_libraries(AthenaAgent ${CMAKE_DL_LIBS})

    install(TARGETS AthenaAgent
        RUNTIME DESTINATION bin/
        LIBRARY DESTINATION lib/
        ARCHIVE DESTINATION lib/static/)

    target_compile_definitions(AthenaAgent PUBLIC SHARED_APP_DATA_PATH="${CMAKE_BINARY_DIR}/..")
endif()

if (Boost_FOUND)
    target_link_libraries(AthenaAgent Boost::filesystem Boost::system Boost::thread Boost::coroutine)
endif()

if(OPENSSL_FOUND)
    message("link openssl")
    target_link_libraries(AthenaAgent OpenSSL::SSL )
    target_link_libraries(AthenaAgent OpenSSL::Crypto)
endif()


if (NOT UNIX)
    message("link winlibs")
    target_link_libraries(AthenaAgent ws2_32)
    target_link_libraries(AthenaAgent wsock32)
    target_link_libraries(AthenaAgent wbemuuid)
    target_link_libraries(AthenaAgent -static-libgcc -static-libstdc++)

    target_compile_definitions(AthenaAgent PUBLIC SHARED_APP_DATA_PATH="C:/ProgramData")
endif()




