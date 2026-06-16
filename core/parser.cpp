#include "projects.h"

using namespace std;

/**
 * @brief Calcula a distância euclidiana entre duas coordenadas bidimensionais.
 * 
 * @param x1 Posição X do primeiro ponto.
 * @param y1 Posição Y do primeiro ponto.
 * @param x2 Posição X do segundo ponto.
 * @param y2 Posição Y do segundo ponto.
 * @return double Distância calculada entre os dois pontos.
 */
double calcularDistancia(double x1, double y1, double x2, double y2) {
  return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

/**
 * @brief Carrega os dados geográficos de nós e arestas de um arquivo de mapa.
 * 
 * @param nomeArquivo Caminho para o arquivo contendo a definição do mapa (formato .poly).
 * @return MapData Estrutura contendo o grafo e as coordenadas de todos os nós.
 */
MapData carregarMapa(string nomeArquivo) {
  vector<pair<double, double>> coordenadas;
  Grafo adjacencias;

  ifstream arquivo(nomeArquivo);

  if (!arquivo.is_open()) {
    cout << "Erro ao abrir o arquivo";
    exit(1);
  }

  int totalNos, dimensoes, lixo1, lixo2;
  arquivo >> totalNos >> dimensoes >> lixo1 >> lixo2;

  coordenadas.resize(totalNos);
  adjacencias.resize(totalNos);

  for (int i = 0; i < totalNos; i++) {
    int idNo;
    double x, y;

    if (!(arquivo >> idNo >> x >> y))
      break;

    if (idNo < 0 || idNo >= totalNos) {
      cout << "ERRO FATAL: O mapa tentou criar um no com ID " << idNo
           << " mas o limite e " << (totalNos - 1) << endl;
      exit(1);
    }

    coordenadas[idNo] = make_pair(x, y);
  }

  int totalArestas, lixo3;
  arquivo >> totalArestas >> lixo3;

  for (int i = 0; i < totalArestas; i++) {
    int idAresta, origem, destino, direcao;

    if (!(arquivo >> idAresta >> origem >> destino >> direcao))
      break;

    if (origem < 0 || origem >= totalNos || destino < 0 || destino >= totalNos) {
      cout << "Aviso: Aresta " << idAresta
           << " ignorada! Tenta ligar nos invalidos (" << origem << " -> "
           << destino << ")" << endl;
      continue;
    }

    double xOrigem = coordenadas[origem].first;
    double yOrigem = coordenadas[origem].second;
    double xDestino = coordenadas[destino].first;
    double yDestino = coordenadas[destino].second;

    double peso = calcularDistancia(xOrigem, yOrigem, xDestino, yDestino);
    adjacencias[origem].push_back(make_pair(destino, peso));

    // Se a via for de mão dupla (direção == 0), adiciona a via de volta no grafo
    if (direcao == 0) {
      adjacencias[destino].push_back(make_pair(origem, peso));
    }
  }

  arquivo.close();
  return MapData{adjacencias, coordenadas};
}
