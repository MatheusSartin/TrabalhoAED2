#ifndef PROJECTS_H
#define PROJECTS_H

// Inclusão de bibliotecas
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <limits>

typedef std::vector<std::vector<std::pair<int, double>>> Grafo;

// Estrutura de dados do mapa
struct MapData {
    Grafo adjacencias;
    std::vector<std::pair<double, double>> coordenadas;
};

// Resultado do cálculo Dijkstra contendo rota, distâncias e estatísticas
struct DijkstraResult {
    bool encontrado = false;                   // true se existe rota válida
    std::vector<int> caminho;                  // sequência de nós da origem ao destino
    double distanciaTotal = 0.0;               // distância total em metros
    std::vector<double> distanciasTrecho;      // distâncias individuais por trecho
    double timeMs = 0.0;                       // tempo de execução em milissegundos
    int nodesExplored = 0;                     // quantidade de nós analisados
};

/**
 * @brief Calcula a distancia euclidiana entre duas coordenadas bidimensionais.
 * 
 * @param x1 Posicao X do primeiro ponto.
 * @param y1 Posicao Y do primeiro ponto.
 * @param x2 Posicao X do segundo ponto.
 * @param y2 Posicao Y do segundo ponto.
 * @return double Distancia calculada entre os dois pontos.
 */
double calcularDistancia(double x1, double y1, double x2, double y2);

/**
 * @brief Carrega os dados geograficos de nos e arestas de um arquivo de mapa.
 * 
 * @param nomeArquivo Caminho para o arquivo contendo a definicao do mapa (formato .poly).
 * @return MapData Estrutura contendo o grafo e as coordenadas de todos os nos.
 */
MapData carregarMapa(std::string nomeArquivo);

/**
 * @brief Executa o algoritmo de Dijkstra para encontrar o menor caminho.
 * 
 * @param grafo Estrutura do grafo contendo as listas de adjacencias e pesos.
 * @param origem ID do no de partida da rota.
 * @param destino ID do no de chegada da rota.
 * @return DijkstraResult Estrutura contendo o caminho, distancia total e estatisticas.
 */
DijkstraResult Dijkstra(const Grafo &grafo, int origem, int destino);

/**
 * @brief Imprime o resultado do calculo de menor caminho de forma formatada.
 * 
 * @param resultado Estrutura contendo o caminho calculado e as informacoes de distancia.
 * @param origem ID do no de origem da rota.
 * @param destino ID do no de destino da rota.
 */
void imprimirResultado(const DijkstraResult &resultado, int origem, int destino);

#endif
