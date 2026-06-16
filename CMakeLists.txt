# Arquivo de configuracao principal do CMake
 
cmake_minimum_required(VERSION 3.16)
project(TrabalhoAED2 VERSION 1.0 LANGUAGES C CXX)
 
# Requer padrao C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
 
# Processa os subdiretorios do projeto
add_subdirectory(core)
add_subdirectory(gui)
