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

// Funcoes
double calcularDistancia(double x1, double y1, double x2, double y2);
Grafo carregarMapa(std::string nomeArquivo);

void Dijkstra(const Grafo &grafo, int origem, int destino);

#endif
