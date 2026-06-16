#include "projects.h"
#include <queue>
#include <limits>
#include <algorithm>
#include <chrono>

using namespace std;

/**
 * @brief Executa o algoritmo de Dijkstra para encontrar o menor caminho.
 * 
 * @param grafo Estrutura do grafo contendo as listas de adjacências e pesos.
 * @param origem ID do nó de partida da rota.
 * @param destino ID do nó de chegada da rota.
 * @return DijkstraResult Estrutura contendo o caminho, distância total e estatísticas.
 */
DijkstraResult Dijkstra(const Grafo& grafo, int origem, int destino) {
    auto startTime = chrono::high_resolution_clock::now();
    int nodesExplored = 0;
    int totalNos = grafo.size();

    vector<double> distancias(totalNos, numeric_limits<double>::infinity());
    vector<int> path(totalNos, -1);
    vector<bool> visitado(totalNos, false);
    
    // Fila de prioridades organizada por menor distância (Min-Heap)
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> fila;

    distancias[origem] = 0.0;
    fila.push(make_pair(0.0, origem));

    while (!fila.empty()) {
        int noAtual = fila.top().second;
        fila.pop();

        if (noAtual == destino) {
            nodesExplored++;
            break;
        }

        if (visitado[noAtual]) continue;
        visitado[noAtual] = true;
        nodesExplored++;

        for (const auto& vizinho : grafo[noAtual]) {
            int v = vizinho.first;
            double peso = vizinho.second;
            double novaDist = distancias[noAtual] + peso;

            // Relaxamento de aresta se um caminho mais curto for encontrado
            if (!visitado[v] && novaDist < distancias[v]) {
                distancias[v] = novaDist;
                path[v] = noAtual;
                fila.push(make_pair(novaDist, v));
            }
        }
    }

    auto endTime = chrono::high_resolution_clock::now();
    double timeMs = chrono::duration<double, milli>(endTime - startTime).count();

    DijkstraResult resultado;
    resultado.timeMs = timeMs;
    resultado.nodesExplored = nodesExplored;

    if (distancias[destino] == numeric_limits<double>::infinity()) {
        resultado.encontrado = false;
        return resultado;
    }

    // Reconstrói a rota retrocedendo a partir do destino
    vector<int> caminho;
    for (int atual = destino; atual != -1; atual = path[atual]) {
        caminho.push_back(atual);
    }
    reverse(caminho.begin(), caminho.end());

    resultado.encontrado = true;
    resultado.caminho = caminho;
    resultado.distanciaTotal = distancias[destino];

    for (size_t i = 0; i < caminho.size() - 1; i++) {
        double distTrecho = distancias[caminho[i + 1]] - distancias[caminho[i]];
        resultado.distanciasTrecho.push_back(distTrecho);
    }

    return resultado;
}

/**
 * @brief Imprime o resultado do cálculo de menor caminho de forma formatada.
 * 
 * @param resultado Estrutura contendo o caminho calculado e as informações de distância.
 * @param origem ID do nó de origem da rota.
 * @param destino ID do nó de destino da rota.
 */
void imprimirResultado(const DijkstraResult& resultado, int origem, int destino) {
    if (!resultado.encontrado) {
        cout << "Aviso: Nao existe uma rota possivel entre o no " << origem
             << " e o no " << destino << "." << endl;
        return;
    }

    cout << "\nO menor caminho para " << destino
         << " = " << resultado.distanciaTotal << " m\n" << endl;

    const auto& caminho = resultado.caminho;
    for (size_t i = 0; i < caminho.size() - 1; i++) {
        cout << caminho[i] << " -> " << caminho[i + 1]
             << " = " << resultado.distanciasTrecho[i] << " m" << endl;
    }
}