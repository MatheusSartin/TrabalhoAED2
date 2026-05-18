#include "parser.h"

using namespace std;

int main(int argc, char *argv[]) {

  // o nome do arquivo será passada como argumento da funcao
  std::string Mapa = argv[1];

  Grafo gr = carregarMapa(Mapa);

  // demonstração
  for (int i = 0; i < gr.size(); i++) {
    for (int j = 0; j < gr[i].size(); j++) {

      cout << i << " " << gr[i][j].first << " " << gr[i][j].second << "\n";
    }
  }

  return 0;
}
