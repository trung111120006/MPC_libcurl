cmake_minimum_required(VERSION 3.10)
project(mpc_streaming)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Find required libraries
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(TINYXML2 REQUIRED tinyxml2)

# Collect all source files
file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Create executable
add_executable(mpc_streaming ${SOURCES})

# Link libraries
target_link_libraries(mpc_streaming 
    CURL::libcurl
    ${TINYXML2_LIBRARIES}
)

# Include tinyxml2 directories
target_include_directories(mpc_streaming PRIVATE ${TINYXML2_INCLUDE_DIRS})

# Create necessary directories at build time
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/obj)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/download)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/decoded_output)

# Custom target for running the application
add_custom_target(run
    COMMAND ${CMAKE_SOURCE_DIR}/bin/mpc_streaming
    DEPENDS mpc_streaming
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running mpc_streaming..."
)

# Print build completion message
add_custom_command(TARGET mpc_streaming POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Build complete: ${CMAKE_SOURCE_DIR}/bin/mpc_streaming"
)