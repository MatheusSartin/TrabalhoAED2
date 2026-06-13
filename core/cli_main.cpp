#include "projects.h"

using namespace std;

int main(int argc, char *argv[]) {

  if (argc < 2) {
      cout << "Erro: Voce esqueceu de passar o nome do mapa!" << endl;
      cout << "Uso correto: .\\programa.exe map.poly" << endl;
      return 1; 
  }
  string Mapa = argv[1];


  MapData mapData = carregarMapa(Mapa);
  DijkstraResult res = Dijkstra(mapData.adjacencias, 40, 570);

  if (res.success) {
      cout << "Custo: " << res.distance << " m, Tempo: " << res.timeMs << " ms" << endl;
  }

  
  return 0;
}
