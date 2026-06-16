#include "projects.h" // Cabeçalho do projeto 
#include <queue>      // Necessário para usar a Fila de Prioridade (priority_queue)
#include <limits>     // Necessário para usar o conceito de "Infinito" (numeric_limits)
#include <algorithm>  // Necessário para funções de utilidade como std::reverse
#include <chrono>     // Necessário para medir o tempo de execução do algoritmo

using namespace std;

// Função principal que executa o Algoritmo de Dijkstra
// Recebe o grafo (por referência para não copiar tudo), o nó de origem e o nó de destino
DijkstraResult Dijkstra(const Grafo& grafo, int origem, int destino) {
    
    // Inicia o cronômetro para medir o tempo de execução
    auto start_time = chrono::high_resolution_clock::now();
    int nodes_explored = 0; // Contador de quantos nós foram analisados , para métricas

    int total_nos = grafo.size(); // Descobre o número total de nós (vértices) no grafo

    // FASE DE INICIALIZAÇÃO

    // distancias: Guarda a menor distância conhecida da origem até cada nó. 
    // Começamos assumindo que todos estão a uma distância "Infinita".
    vector<double> distancias(total_nos, numeric_limits<double>::infinity());
    
    // path: Guarda o "pai" de cada nó para podermos reconstruir o caminho no final. 
    // Inicializado com -1 (indicando que não tem pai ainda).
    vector<int> path(total_nos, -1);
    
    // visitado: Marca quais nós já tiveram todos os seus vizinhos analisados.
    vector<bool> visitado(total_nos, false); 
    
    // fila: Fila de prioridade (Min-Heap) que sempre nos entrega o nó mais próximo primeiro.
    // Ela guarda pares no formato: <distância_acumulada, numero_do_no>
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> fila;

    // A distância da origem para ela mesma é sempre 0
    distancias[origem] = 0.0;
    // Colocamos o nó de origem na fila para dar o pontapé inicial
    fila.push(make_pair(0.0, origem));

    // LOOP PRINCIPAL DO ALGORITMO 
    while (!fila.empty()) {
        // Pega o nó que está no topo da fila (o que tem a menor distância atual)
        int no_atual = fila.top().second;
        fila.pop(); // Remove este nó da fila

        // CONDIÇÃO DE PARADA ANTECIPADA (Early Exit)
        // Se o nó que tiramos da fila é o nosso destino, não precisamos continuar procurando!
        if (no_atual == destino) {
            nodes_explored++;
            break;
        }

        // Se o nó já foi processado antes (por outro caminho que era mais longo), nós o ignoramos
        if (visitado[no_atual]) continue;
        
        // Marca o nó como visitado (processado) e aumenta o contador
        visitado[no_atual] = true;
        nodes_explored++;

        //  ANÁLISE DOS VIZINHOS
        // Percorre todos os vizinhos do nó atual
        for (const auto& vizinho : grafo[no_atual]) {
            int v = vizinho.first;         // O identificador do nó vizinho
            double peso = vizinho.second;  // O peso (distância/custo) da aresta até ele
            
            // Calcula qual seria a distância total até o vizinho se passássemos pelo no_atual
            double novaDistancia = distancias[no_atual] + peso;

            // RELAXAMENTO DE ARESTA:
            // Se o vizinho ainda não foi visitado E encontramos um caminho mais curto para ele:
            if (!visitado[v] && novaDistancia < distancias[v]) {
                distancias[v] = novaDistancia;          // Atualiza a nova menor distância
                path[v] = no_atual;                     // Dizemos que para chegar no vizinho 'v', viemos do 'no_atual'
                fila.push(make_pair(novaDistancia, v)); // Colocamos o vizinho na fila com a nova distância
            }
        }
    }

    // FIM DA BUSCA
    
    // Para o cronômetro e calcula quanto tempo o algoritmo levou em milissegundos
    auto end_time = chrono::high_resolution_clock::now();
    double timeMs = chrono::duration<double, milli>(end_time - start_time).count();

    // Prepara a estrutura de resposta com as métricas coletadas
    DijkstraResult result;
    result.timeMs = timeMs;
    result.nodesExplored = nodes_explored;

    // VERIFICAÇÃO DE SUCESSO 
    // Se a distância do destino ainda é infinita, significa que não existe caminho até ele
    if (distancias[destino] == numeric_limits<double>::infinity()) {
        result.success = false;
        result.distance = 0;
        return result; 
    } 

    // RECONSTRUÇÃO DO CAMINHO 
    // Se chegamos aqui, o caminho existe! Agora vamos fazer o "caminho de volta"
    vector<int> caminho;
    // Começa do destino e vai voltando pelo vetor 'path' (que guardou os pais) até chegar a -1 (a origem)
    for (int atual = destino; atual != -1; atual = path[atual]) {
        caminho.push_back(atual);
    }
    // Como fizemos o caminho de trás para frente (Destino -> Origem), precisamos inverter a ordem da lista
    reverse(caminho.begin(), caminho.end());

    // Preenche o resultado final com sucesso, o caminho correto e a distância total
    result.success = true;
    result.path = caminho;
    result.distance = distancias[destino];

    return result;
}