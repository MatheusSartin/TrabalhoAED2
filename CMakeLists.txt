# TrabalhoAED2/CMakeLists.txt

cmake_minimum_required(VERSION 3.16)
project(TrabalhoAED2 VERSION 1.0 LANGUAGES C CXX)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to process these two directories
add_subdirectory(core)
add_subdirectory(gui)
