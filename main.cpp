#include "projects.h"

using namespace std;

int main(int argc, char *argv[]) {

  if (argc < 2) {
      cout << "Erro: Voce esqueceu de passar o nome do mapa!" << endl;
      cout << "Uso correto: .\\programa.exe map.poly" << endl;
      return 1; 
  }
  string Mapa = argv[1];


  Grafo gr = carregarMapa(Mapa);
  Dijkstra(gr , 40 , 570);
  
  return 0;
}
