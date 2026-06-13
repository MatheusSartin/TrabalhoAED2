#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPen>
#include <QBrush>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QShowEvent>
#include <cmath>
#include "../core/projects.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    currentMap = nullptr;
    firstShow = true;
    nodeRadius = 5.0;
    nodeLabel = nullptr;
    nextClickTarget = 0; // Primeiro clique = origem
    clickCount = 0;

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);

    // Enable dragging (pan) in QGraphicsView
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);

    // Permitir transformacoes de zoom suaves
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    // Conectar clique do mouse na cena para selecionar nos
    ui->graphicsView->setContextMenuPolicy(Qt::NoContextMenu);
    ui->graphicsView->viewport()->installEventFilter(this);

    // Detectar quando o usuario clica nos spinboxes para redirecionar o proximo clique
    ui->spinOrigem->installEventFilter(this);
    ui->spinDestino->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    if (currentMap) delete currentMap;
    delete ui;
}

// Interceptar cliques do mouse no viewport e foco nos spinboxes
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    // Detectar quando o usuario clica/foca nos spinboxes
    if (event->type() == QEvent::FocusIn) {
        if (obj == ui->spinOrigem) {
            nextClickTarget = 0; // Proximo clique no grafo = origem
        } else if (obj == ui->spinDestino) {
            nextClickTarget = 1; // Proximo clique no grafo = destino
        }
    }

    // Detectar cliques no grafo
    if (obj == ui->graphicsView->viewport() && currentMap) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                int nearestNode = findNearestNode(scenePos);
                if (nearestNode >= 0) {
                    // Logica de alternancia
                    if (nextClickTarget == 0) {
                        ui->spinOrigem->setValue(nearestNode);
                        // Apos definir origem, proximo clique = destino
                        nextClickTarget = 1;
                    } else {
                        ui->spinDestino->setValue(nearestNode);
                        // Manter em destino para cliques subsequentes
                    }
                    clickCount++;
                    highlightSelectedNodes();

                    // Mostrar label flutuante com o ID do no
                    if (nodeLabel) {
                        scene->removeItem(nodeLabel);
                        delete nodeLabel;
                        nodeLabel = nullptr;
                    }

                    double nx = currentMap->coordenadas[nearestNode].first;
                    double ny = currentMap->coordenadas[nearestNode].second;

                    nodeLabel = new QGraphicsItemGroup();

                    QGraphicsTextItem *text = new QGraphicsTextItem();
                    text->setHtml(QString("<div style='background:#1F2937; color:white; padding:4px 8px; border-radius:4px; font-size:12px;'>"
                                          "<b>Nó %1</b></div>").arg(nearestNode));

                    QRectF textRect = text->boundingRect();
                    text->setPos(nx - textRect.width() / 2, ny - nodeRadius * 3 - textRect.height());

                    nodeLabel->addToGroup(text);
                    nodeLabel->setZValue(10);
                    scene->addItem(nodeLabel);
                }
            }
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (firstShow && scene->items().count() > 0) {
        firstShow = false;
        QTimer::singleShot(100, this, &MainWindow::fitGraphInView);
    }
}

void MainWindow::fitGraphInView()
{
    if (scene->items().count() > 0) {
        QRectF bounds = scene->itemsBoundingRect();
        double marginX = bounds.width() * 0.05;
        double marginY = bounds.height() * 0.05;
        bounds.adjust(-marginX, -marginY, marginX, marginY);
        ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
    }
}

int MainWindow::findNearestNode(const QPointF &pos)
{
    if (!currentMap || currentMap->coordenadas.empty()) return -1;

    int nearest = -1;
    double minDist = nodeRadius * 3.0; // Distancia maxima de clique (3x o raio)

    for (size_t i = 0; i < currentMap->coordenadas.size(); ++i) {
        double nx = currentMap->coordenadas[i].first;
        double ny = currentMap->coordenadas[i].second;
        double dx = pos.x() - nx;
        double dy = pos.y() - ny;
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist) {
            minDist = dist;
            nearest = static_cast<int>(i);
        }
    }
    return nearest;
}

void MainWindow::highlightSelectedNodes()
{
    if (!currentMap) return;

    // Esconder todos os nos
    QBrush defaultBrush(QColor("#3B82F6"));
    QPen defaultPen(QColor("#1E3A5F"));
    defaultPen.setWidthF(nodeRadius * 0.15);

    for (auto node : nodeItems) {
        node->setBrush(defaultBrush);
        node->setPen(defaultPen);
        node->setZValue(1);
        node->setVisible(false);
    }

    // Mostrar e destacar origem (amarelo)
    int origem = ui->spinOrigem->value();
    if (origem >= 0 && origem < static_cast<int>(nodeItems.size())) {
        nodeItems[origem]->setVisible(true);
        nodeItems[origem]->setBrush(QBrush(QColor("#EAB308")));
        QPen originPen(QColor("#92400E"));
        originPen.setWidthF(nodeRadius * 0.3);
        nodeItems[origem]->setPen(originPen);
        nodeItems[origem]->setZValue(5);
    }

    // Mostrar e destacar destino (vermelho)
    int destino = ui->spinDestino->value();
    if (destino >= 0 && destino < static_cast<int>(nodeItems.size())) {
        nodeItems[destino]->setVisible(true);
        nodeItems[destino]->setBrush(QBrush(QColor("#EF4444")));
        QPen destPen(QColor("#7F1D1D"));
        destPen.setWidthF(nodeRadius * 0.3);
        nodeItems[destino]->setPen(destPen);
        nodeItems[destino]->setZValue(5);
    }
}

void MainWindow::on_btnCarregarMapa_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Mapa", "", "Polygon Files (*.poly);;Todos Arquivos (*)");
    if (fileName.isEmpty())
        return;

    try {
        if (currentMap) delete currentMap;
        currentMap = new MapData(carregarMapa(fileName.toStdString()));

        int totalNos = static_cast<int>(currentMap->adjacencias.size());

        ui->spinOrigem->setMaximum(totalNos - 1);
        ui->spinDestino->setMaximum(totalNos - 1);

        // Contar arestas
        int totalArestas = 0;
        for (const auto& adj : currentMap->adjacencias) {
            totalArestas += adj.size();
        }

        ui->lblStatus->setText(QString("Mapa carregado: %1 nós, %2 arestas.").arg(totalNos).arg(totalArestas));

        // Resetar alternancia de clique
        nextClickTarget = 0;
        clickCount = 0;

        drawMap();

        // Usar QTimer para dar tempo ao layout ser computado antes de ajustar a view
        QTimer::singleShot(50, this, &MainWindow::fitGraphInView);

    } catch (...) {
        QMessageBox::critical(this, "Erro", "Falha ao carregar o mapa.");
    }
}

void MainWindow::drawMap()
{
    scene->clear();
    nodeItems.clear();
    edgeItems.clear();
    pathEdgeItems.clear();
    nodeLabel = nullptr; // scene->clear() ja deletou

    if (!currentMap || currentMap->adjacencias.empty()) return;

    int totalNos = static_cast<int>(currentMap->coordenadas.size());

    // Encontrar bounding box das coordenadas
    double minX = currentMap->coordenadas[0].first;
    double maxX = minX;
    double minY = currentMap->coordenadas[0].second;
    double maxY = minY;

    for (const auto& coord : currentMap->coordenadas) {
        if (coord.first < minX) minX = coord.first;
        if (coord.first > maxX) maxX = coord.first;
        if (coord.second < minY) minY = coord.second;
        if (coord.second > maxY) maxY = coord.second;
    }

    double rangeX = maxX - minX;
    double rangeY = maxY - minY;
    double maxRange = std::max(rangeX, rangeY);
    if (maxRange < 1.0) maxRange = 1.0;

    // Calcular raio do no proporcional ao tamanho do grafo
    nodeRadius = maxRange * 0.004;
    if (nodeRadius < 1.0) nodeRadius = 1.0;

    // Pens: escalam junto com o grafo
    QPen edgePen(QColor("#94A3B8"));
    edgePen.setWidthF(nodeRadius * 0.3);
    edgePen.setCosmetic(false);

    QBrush nodeBrush(QColor("#3B82F6")); // azul vibrante
    QPen nodePen(QColor("#1E3A5F"));
    nodePen.setWidthF(nodeRadius * 0.15);

    // Desenhar Arestas primeiro (ficam por baixo dos nos)
    for (size_t u = 0; u < currentMap->adjacencias.size(); ++u) {
        double x1 = currentMap->coordenadas[u].first;
        double y1 = currentMap->coordenadas[u].second;

        for (const auto& aresta : currentMap->adjacencias[u]) {
            int v = aresta.first;
            if (static_cast<int>(u) < v) {
                double x2 = currentMap->coordenadas[v].first;
                double y2 = currentMap->coordenadas[v].second;

                QGraphicsLineItem* line = scene->addLine(x1, y1, x2, y2, edgePen);
                line->setZValue(0);
                edgeItems.push_back(line);
            }
        }
    }

    // Desenhar Nos (ocultos por padrao - aparecem apenas ao selecionar)
    for (int i = 0; i < totalNos; ++i) {
        double x = currentMap->coordenadas[i].first;
        double y = currentMap->coordenadas[i].second;

        QGraphicsEllipseItem* ellipse = scene->addEllipse(
            x - nodeRadius, y - nodeRadius,
            nodeRadius * 2, nodeRadius * 2,
            nodePen, nodeBrush
        );
        ellipse->setZValue(1);
        ellipse->setToolTip(QString("Nó %1\n(%2, %3)").arg(i).arg(x, 0, 'f', 1).arg(y, 0, 'f', 1));
        ellipse->setVisible(false); // Oculto por padrao
        nodeItems.push_back(ellipse);
    }

    // Resetar zoom e reposicionar
    ui->graphicsView->resetTransform();
}

void MainWindow::on_btnCalcular_clicked()
{
    if (!currentMap || currentMap->adjacencias.empty()) {
        QMessageBox::warning(this, "Aviso", "Carregue um mapa primeiro.");
        return;
    }

    int origem = ui->spinOrigem->value();
    int destino = ui->spinDestino->value();
    int totalNos = static_cast<int>(currentMap->adjacencias.size());

    if (origem < 0 || origem >= totalNos ||
        destino < 0 || destino >= totalNos) {
        QMessageBox::warning(this, "Aviso", "Nó de origem ou destino inválido.");
        return;
    }

    clearPath();

    ui->lblStatus->setText("Calculando...");
    QCoreApplication::processEvents();

    DijkstraResult res = Dijkstra(currentMap->adjacencias, origem, destino);

    if (res.success) {
        drawPath(res.path);
        ui->lblStatus->setText(QString("Caminho encontrado! %1 nós na rota.").arg(res.path.size()));
        ui->lblCost->setText(QString("Custo: %1 m").arg(res.distance, 0, 'f', 2));
        ui->lblTime->setText(QString("Tempo: %1 ms").arg(res.timeMs, 0, 'f', 2));
        ui->lblNodes->setText(QString("Nós Explorados: %1").arg(res.nodesExplored));

        // Zoom automatico na rota
        QTimer::singleShot(50, this, [this, res]() {
            zoomToRoute(res.path);
        });
    } else {
        ui->lblStatus->setText("Nenhum caminho encontrado.");
        ui->lblCost->setText("Custo: -");
        ui->lblTime->setText(QString("Tempo: %1 ms").arg(res.timeMs, 0, 'f', 2));
        ui->lblNodes->setText(QString("Nós Explorados: %1").arg(res.nodesExplored));
        QMessageBox::information(this, "Resultado", "Não há caminho entre os nós selecionados.");
    }
}

void MainWindow::zoomToRoute(const std::vector<int>& path)
{
    if (!currentMap || path.empty()) return;

    // Calcular bounding box da rota
    double minX = currentMap->coordenadas[path[0]].first;
    double maxX = minX;
    double minY = currentMap->coordenadas[path[0]].second;
    double maxY = minY;

    for (int nodeId : path) {
        double x = currentMap->coordenadas[nodeId].first;
        double y = currentMap->coordenadas[nodeId].second;
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }

    // Adicionar margem de 15% ao redor da rota
    double marginX = (maxX - minX) * 0.15 + nodeRadius * 5;
    double marginY = (maxY - minY) * 0.15 + nodeRadius * 5;

    QRectF routeRect(minX - marginX, minY - marginY,
                     (maxX - minX) + 2 * marginX,
                     (maxY - minY) + 2 * marginY);

    ui->graphicsView->fitInView(routeRect, Qt::KeepAspectRatio);
}

void MainWindow::clearPath()
{
    for (auto line : pathEdgeItems) {
        scene->removeItem(line);
        delete line;
    }
    pathEdgeItems.clear();

    // Restaurar cor padrao dos nos
    highlightSelectedNodes();
}

void MainWindow::drawPath(const std::vector<int>& path)
{
    if (!currentMap || path.size() < 2) return;

    // Calcular largura proporcional para a rota
    double minX = currentMap->coordenadas[0].first;
    double maxX = minX;
    double minY = currentMap->coordenadas[0].second;
    double maxY = minY;
    for (const auto& coord : currentMap->coordenadas) {
        if (coord.first < minX) minX = coord.first;
        if (coord.first > maxX) maxX = coord.first;
        if (coord.second < minY) minY = coord.second;
        if (coord.second > maxY) maxY = coord.second;
    }
    double maxRange = std::max(maxX - minX, maxY - minY);
    if (maxRange < 1.0) maxRange = 1.0;

    QPen pathPen(QColor("#EF4444")); // Vermelho vibrante
    pathPen.setWidthF(maxRange * 0.006);
    pathPen.setCosmetic(false);

    for (size_t i = 0; i < path.size() - 1; ++i) {
        int u = path[i];
        int v = path[i+1];

        double x1 = currentMap->coordenadas[u].first;
        double y1 = currentMap->coordenadas[u].second;
        double x2 = currentMap->coordenadas[v].first;
        double y2 = currentMap->coordenadas[v].second;

        QGraphicsLineItem* line = scene->addLine(x1, y1, x2, y2, pathPen);
        line->setZValue(2);
        pathEdgeItems.push_back(line);
    }

    // Destacar Origem (Amarelo) e Destino (Vermelho) na rota
    if (!path.empty()) {
        int o = path.front();
        int d = path.back();

        nodeItems[o]->setBrush(QBrush(QColor("#EAB308")));
        QPen originPen(QColor("#92400E"));
        originPen.setWidthF(nodeRadius * 0.3);
        nodeItems[o]->setPen(originPen);
        nodeItems[o]->setZValue(3);

        nodeItems[d]->setBrush(QBrush(QColor("#EF4444")));
        QPen destPen(QColor("#7F1D1D"));
        destPen.setWidthF(nodeRadius * 0.3);
        nodeItems[d]->setPen(destPen);
        nodeItems[d]->setZValue(3);
    }
}
