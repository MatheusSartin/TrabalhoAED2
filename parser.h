#ifndef PARSER_H
#define PARSER_H

// Bibliotecas
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
typedef vector<vector<pair<int, double>>> Grafo;

// Funcoes
double calcularDistancia(double x1, double y1, double x2, double y2);
Grafo carregarMapa(string nomeArquivo);

#endif
