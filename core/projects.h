#ifndef PROJECTS_H
#define PROJECTS_H

// Bibliotecas
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

typedef std::vector<std::vector<std::pair<int, double>>> Grafo;

struct MapData {
    Grafo adjacencias;
    std::vector<std::pair<double, double>> coordenadas;
};

struct DijkstraResult {
    bool success;
    std::vector<int> path;
    double distance;
    double timeMs;
    int nodesExplored;
};

// Funcoes
double calcularDistancia(double x1, double y1, double x2, double y2);
MapData carregarMapa(std::string nomeArquivo);

DijkstraResult Dijkstra(const Grafo &grafo, int origem, int destino);

#endif
