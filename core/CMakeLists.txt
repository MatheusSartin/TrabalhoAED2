# Configuracao do CMake para a pasta core
 
# Alvo 1: Conversor (Programa em C)
# Compila o conversor de arquivos .osm para .poly
add_executable(osm_converter LeArqOSM_e_GeraArqPoly.c)
target_link_libraries(osm_converter PRIVATE m)
 
# Alvo 2: Biblioteca de Algoritmos (C++)
# Compila o parser e o Dijkstra em uma biblioteca estatica
add_library(aed2_core STATIC
    dijkstra.cpp
    parser.cpp
)
# Permite que outros alvos localizem o arquivo projects.h
target_include_directories(aed2_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
 
# Alvo 3: Aplicativo de Terminal (C++)
# Permite testes e execucoes diretamente no terminal
add_executable(cli_app cli_main.cpp)
 
# Vincula o aplicativo de terminal a biblioteca principal
target_link_libraries(cli_app PRIVATE aed2_core)
