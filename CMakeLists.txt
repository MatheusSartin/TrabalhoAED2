# TrabalhoAED2/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(TrabalhoAED2 VERSION 1.0 LANGUAGES C CXX)

# Require C++17 standard (good practice for modern Qt and C++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to process these two directories
add_subdirectory(core)
add_subdirectory(gui)
