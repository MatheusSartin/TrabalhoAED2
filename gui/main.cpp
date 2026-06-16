#include <utility>
#include <type_traits>
#include "mainwindow.h"
#include <QApplication>
/**
 * @brief Ponto de entrada do aplicativo grafico (GUI).
 * 
 * @param argc Quantidade de argumentos passados na linha de comando.
 * @param argv Vetor de strings contendo os argumentos de inicializacao.
 * @return int Codigo de retorno da execucao do Qt Application.
 */
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
