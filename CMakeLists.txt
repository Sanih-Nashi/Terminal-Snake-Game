cmake_minimum_required(VERSION 3.16)

project(snake)

message(STATUS "Building ${PROJECT_NAME}")

# You may need to specify the path to your source files.
message(STATUS "File: ${CPP_FILE}")

# Set the C++ version to C++23 (recommended approach)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)  # Optional: Disable compiler-specific extensions like `gnu++`

# Define your target
add_executable(${PROJECT_NAME} src/sUtils.cpp src/rawMode.cpp src/main.cpp)

# Apply the compile options (target-specific)
target_compile_options(${PROJECT_NAME} PRIVATE 
    -fsanitize=address  # Enable AddressSanitizer
    -fdiagnostics-color=always  # Enable colored diagnostics (useful for readability)
    -O3  # Maximum optimization
)

# Optionally link the AddressSanitizer runtime (needed for AddressSanitizer to work)
target_link_libraries(${PROJECT_NAME} PRIVATE -fsanitize=address)
