# Calculador de Rotas com Algoritmo de Dijkstra

Este projeto consiste em um sistema para cálculo de rotas em mapas reais. O sistema foi desenvolvido como parte da disciplina de Algoritmos e Estruturas de Dados 2 (AED 2). A aplicação possui uma interface gráfica moderna desenvolvida em Qt6 e uma interface de terminal (CLI) focada em desempenho.

## Funcionalidades Principais

* **Importação de Mapas Reais:** Conversão e leitura de mapas obtidos do OpenStreetMap para a estrutura de grafos.
* **Cálculo de Menor Caminho:** Implementação eficiente do algoritmo de Dijkstra com fila de prioridades (Min-Heap).
* **Paradas Intermediárias:** Suporte para adicionar múltiplos pontos de passagem entre a origem e o destino.
* **Interação Direta no Mapa:** Possibilidade de arrastar a linha da rota vermelha para criar novos pontos, ou arrastar os círculos laranja existentes para editar suas posições.
* **Exclusão Rápida:** Remover pontos de parada arrastando-os de volta para o ponto de partida, ponto de chegada ou paradas vizinhas.
* **Visualização de Vias:** Linhas contínuas indicam vias de mão dupla, enquanto linhas tracejadas indicam vias de mão única.
* **Painel de Estatísticas:** Apresentação imediata do custo da rota (em metros), tempo gasto no cálculo e quantidade de nós explorados.
* **Cópia da Área Gráfica:** Botão para capturar a imagem do mapa atual e copiá-la diretamente para a área de transferência do sistema.

## Estrutura do Repositório

O código está estruturado em duas partes principais:

* **core:** Contém a representação do grafo por listas de adjacências, o leitor de arquivos de mapas (parser) e o algoritmo de Dijkstra. Também abriga o conversor para ler dados XML do OpenStreetMap.
* **gui:** Contém a interface visual baseada em Qt6, com o mapa interativo em um cenário de desenho (`QGraphicsScene`).

## Requisitos de Sistema

Para compilar e executar o projeto, você precisará dos seguintes componentes:

1. Compilador C++ com suporte ao padrão C++17 ou superior.
2. CMake (versão 3.16 ou superior).
3. Qt6 SDK (módulos Core, Gui e Widgets).
4. Bibliotecas de desenvolvimento adicionais para suporte gráfico no Linux (se aplicável).

## Como Compilar o Projeto

Siga os passos abaixo para configurar e compilar o projeto a partir do terminal:

```bash
# 1. Crie um diretório para build e entre nele
mkdir build
cd build

# 2. Configure o projeto com o CMake
cmake ..

# 3. Compile os executáveis
cmake --build .
```

Após o término da compilação, os executáveis da interface gráfica (`aed2_gui`) e da interface de terminal (`cli_app`) estarão disponíveis no diretório de build.

## Como Utilizar a Interface Gráfica

1. Execute o aplicativo `aed2_gui`.
2. Clique no botão **Carregar Mapa** e selecione um arquivo no formato `.poly` compatível.
3. Insira os IDs nos campos de **Origem** e **Destino** (ou clique nos nós do mapa para selecioná-los).
4. Clique em **Calcular** para visualizar o menor caminho traçado na cor vermelha.
5. Para adicionar uma nova parada intermediária, clique com o botão esquerdo do mouse sobre a linha vermelha da rota e arraste-a para a região desejada.
6. Para modificar a posição de uma parada laranja existente, clique diretamente sobre ela e arraste-a para o novo local.
7. Para limpar as rotas e começar uma nova busca, clique no botão **Limpar Rota**.
