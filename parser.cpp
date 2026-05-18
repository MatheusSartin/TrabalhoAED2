#include "parser.h"

double calcularDistancia(double x1, double y1, double x2, double y2) {
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

// Carrega o mapa
Grafo carregarMapa(string nomeArquivo) {

  vector<pair<double, double>> coordenadas; // cada node i tera uma coordenada
  Grafo adjacencias; // cada node i tera um vector de arestas: A<destino, peso>

  ifstream arquivo(nomeArquivo);

  if (!arquivo.is_open()) {
    cout << "Erro ao abrir o arquivo";
    exit;
  }

  int totalNos, dimensoes, lixo1, lixo2;
  arquivo >> totalNos >> dimensoes >> lixo1 >> lixo2;

  coordenadas.resize(totalNos);
  adjacencias.resize(totalNos);

  // Popula os nodes
  for (int i = 0; i < totalNos; i++) {
    int id_no;
    double x, y;
    arquivo >> id_no >> x >> y;
    coordenadas[id_no] = make_pair(x, y);
  }

  int totalArestas, lixo3;
  arquivo >> totalArestas >> lixo3;

  // Popula as arestas
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
  return adjacencias;
}
