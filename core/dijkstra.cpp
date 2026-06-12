#include "projects.h"
#include <queue>     
#include <limits>    
#include <algorithm> 

using namespace std;

void Dijkstra(const Grafo& grafo, int origem, int destino ) {
    
    int total_nos = grafo.size();
    vector<double> distancias(total_nos, numeric_limits<double>::infinity());
    vector<int> path(total_nos, -1);
    vector<bool> visitado(total_nos, false); 
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>>   fila;

    distancias[origem] = 0.0;
    fila.push(make_pair(0.0, origem));

    while (!fila.empty()) {

        int no_atual = fila.top().second;
        fila.pop();

        if (no_atual == destino) break;
        if (visitado[no_atual] == true) continue;
        
        visitado[no_atual] = true;

        for (const auto& vizinho : grafo[no_atual]) {

            int v = vizinho.first;        
            double peso = vizinho.second;
            double novaDistancia = distancias[no_atual] + peso;

            if (visitado[v] == false && novaDistancia < distancias[v]) {
                distancias[v] = novaDistancia; 
                path[v] = no_atual;           
                fila.push(make_pair(novaDistancia, v)); 
            }
        }
    }

    
    vector<int> caminho;

    if (distancias[destino] == numeric_limits<double>::infinity()) {
        cout << "Aviso: Nao existe uma rota possivel entre o no " << origem << " e o no " << destino << "." << endl;
        return ; 
    } 
    for (int atual = destino; atual != -1; atual = path[atual]) {
        caminho.push_back(atual);
    }

    reverse(caminho.begin(), caminho.end());
    cout << "\nO menor caminho para " << destino << " = " << distancias[destino] << " m\n" << endl;

    for(size_t i = 0 ; i < caminho.size() - 1 ; i++){
        double dist_trecho = distancias[caminho[i+1]] - distancias[caminho[i]];
        cout << caminho[i] << " -> " << caminho[i+1] << " = " << dist_trecho << " m" << endl;
    }

    return ;
}