cmake_minimum_required(VERSION 3.14)
project(axbox-manager CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(Threads REQUIRED)

# openai-api.cpp：提供统一网关 openai_api_gateway 和上游透传
set(OPENAI_API_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPENAI_API_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(OPENAI_API_BUILD_SERVER OFF CACHE BOOL "" FORCE)
set(OPENAI_API_BUILD_GATEWAY ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/openai-api.cpp)

# 网页文件编进程序
file(GLOB_RECURSE WEB_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/web/*")
set(ASSETS_CPP "${CMAKE_CURRENT_BINARY_DIR}/assets.cpp")
add_custom_command(
    OUTPUT "${ASSETS_CPP}"
    COMMAND ${CMAKE_COMMAND} -DWEB_DIR=${CMAKE_CURRENT_SOURCE_DIR}/web -DOUT=${ASSETS_CPP}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_web.cmake
    DEPENDS ${WEB_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/cmake/embed_web.cmake
    COMMENT "Embedding web assets")

add_executable(axbox-manager
    src/main.cpp
    src/auth.cpp
    src/config.cpp
    src/jobs.cpp
    src/model_repo.cpp
    src/supervisor.cpp
    src/sysinfo.cpp
    src/util.cpp
    "${ASSETS_CPP}")
target_include_directories(axbox-manager PRIVATE src)
target_link_libraries(axbox-manager PRIVATE openai_api Threads::Threads ${CMAKE_DL_LIBS})
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9)
    target_link_libraries(axbox-manager PRIVATE stdc++fs)
endif()

install(TARGETS axbox-manager openai_api_gateway RUNTIME DESTINATION bin)
