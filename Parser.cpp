#include <fstream>
#include <vector>
#include <utility> 
#include <cmath>
#include <string>

using namespace std;

vector<pair<double, double>> coordenadas;
vector<vector<pair<int, double>>> adjacencias;

double calcularDistancia(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

bool carregarMapa(string nomeArquivo) {
    ifstream arquivo(nomeArquivo);

    if (!arquivo.is_open()) {
        return false; 
    }

    int totalNos, dimensoes, lixo1, lixo2;
    arquivo >> totalNos >> dimensoes >> lixo1 >> lixo2;

    coordenadas.resize(totalNos);
    adjacencias.resize(totalNos);

    for (int i = 0; i < totalNos; i++) {
        int id_no;
        double x, y;
        arquivo >> id_no >> x >> y;
        coordenadas[id_no] = make_pair(x, y);
    }

    int totalArestas, lixo3;
    arquivo >> totalArestas >> lixo3;

    for (int i = 0; i < totalArestas; i++) {
        int id_aresta, origem, destino, direcao;
        arquivo >> id_aresta >> origem >> destino >> direcao;

        double x_origem = coordenadas[origem].first;
        double y_origem = coordenadas[origem].second;
        double x_destino = coordenadas[destino].first;
        double y_destino = coordenadas[destino].second;

        double peso = calcularDistancia(x_origem, y_origem, x_destino, y_destino);

        adjacencias[origem].push_back(make_pair(destino, peso));

        if (direcao == 0) {
            adjacencias[destino].push_back(make_pair(origem, peso));
        }
    }
    
    arquivo.close();
    return true; 
}