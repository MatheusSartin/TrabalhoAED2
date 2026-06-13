#include "projects.h"
#include <queue>     
#include <limits>    
#include <algorithm> 
#include <chrono>

using namespace std;

DijkstraResult Dijkstra(const Grafo& grafo, int origem, int destino) {
    auto start_time = chrono::high_resolution_clock::now();
    int nodes_explored = 0;

    int total_nos = grafo.size();
    vector<double> distancias(total_nos, numeric_limits<double>::infinity());
    vector<int> path(total_nos, -1);
    vector<bool> visitado(total_nos, false); 
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> fila;

    distancias[origem] = 0.0;
    fila.push(make_pair(0.0, origem));

    while (!fila.empty()) {
        int no_atual = fila.top().second;
        fila.pop();

        if (no_atual == destino) {
            nodes_explored++;
            break;
        }
        if (visitado[no_atual]) continue;
        
        visitado[no_atual] = true;
        nodes_explored++;

        for (const auto& vizinho : grafo[no_atual]) {
            int v = vizinho.first;        
            double peso = vizinho.second;
            double novaDistancia = distancias[no_atual] + peso;

            if (!visitado[v] && novaDistancia < distancias[v]) {
                distancias[v] = novaDistancia; 
                path[v] = no_atual;           
                fila.push(make_pair(novaDistancia, v)); 
            }
        }
    }
    
    auto end_time = chrono::high_resolution_clock::now();
    double timeMs = chrono::duration<double, milli>(end_time - start_time).count();

    DijkstraResult result;
    result.timeMs = timeMs;
    result.nodesExplored = nodes_explored;

    if (distancias[destino] == numeric_limits<double>::infinity()) {
        result.success = false;
        result.distance = 0;
        return result; 
    } 

    vector<int> caminho;
    for (int atual = destino; atual != -1; atual = path[atual]) {
        caminho.push_back(atual);
    }
    reverse(caminho.begin(), caminho.end());

    result.success = true;
    result.path = caminho;
    result.distance = distancias[destino];

    return result;
}