#include "projects.h"

using namespace std;

/**
 * @brief Ponto de entrada da interface de linha de comando (CLI).
 * 
 * @param argc Quantidade de argumentos passados na linha de comando.
 * @param argv Vetor de strings contendo os argumentos passados.
 * @return int Codigo de retorno da execucao (0 para sucesso, 1 para erro).
 */
int main(int argc, char *argv[]) {

    if (argc < 2) {
        cout << "Erro: Voce esqueceu de passar o nome do mapa!" << endl;
        cout << "Uso correto: .\\programa.exe map.poly" << endl;
        return 1;
    }

    string Mapa = argv[1];

    MapData mapData = carregarMapa(Mapa);
    DijkstraResult resultado = Dijkstra(mapData.adjacencias, 40, 570);
    imprimirResultado(resultado, 40, 570);

    return 0;
}
