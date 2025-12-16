cmake_minimum_required(VERSION 3.10)
project(MPCStreaming)

# Đặt C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tìm libcurl
find_package(CURL REQUIRED)

# Thêm executable
add_executable(mpc_streaming src/code_algorithm_mpc_libcurl.cpp)

# Link libcurl
target_link_libraries(mpc_streaming PRIVATE CURL::libcurl)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(mpc_streaming PRIVATE -Wall -Wextra -O2)
endif()

# Print thông tin
message(STATUS "CURL include dir: ${CURL_INCLUDE_DIRS}")
message(STATUS "CURL libraries: ${CURL_LIBRARIES}")