#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QGraphicsTextItem>
#include <QTimer>
#include <QShowEvent>
#include <QListWidgetItem>
#include <QString>
#include <QColor>
#include <QFont>
#include <QCoreApplication>
#include <QProcess>
#include <QFile>
#include <cmath>
#include <algorithm>
#include "../core/projects.h"

// Implementação de RouteLineItem

/**
 * @brief Construtor da classe RouteLineItem.
 * 
 * @param x1 Coordenada X de início.
 * @param y1 Coordenada Y de início.
 * @param x2 Coordenada X de fim.
 * @param y2 Coordenada Y de fim.
 * @param fromNode ID do nó inicial.
 * @param toNode ID do nó final.
 * @param legIndex Ãndice do trecho da rota.
 * @param mainWindow Ponteiro para a janela principal.
 * @param parent Item gráfico pai.
 */
RouteLineItem::RouteLineItem(qreal x1, qreal y1, qreal x2, qreal y2, int fromNode, int toNode, int legIndex, MainWindow *mainWindow, QGraphicsItem *parent)
    : QGraphicsLineItem(x1, y1, x2, y2, parent), 
      m_mainWindow(mainWindow), 
      m_fromNode(fromNode), 
      m_toNode(toNode), 
      m_legIndex(legIndex),
      m_dragHandle(nullptr),
      m_dragLine1(nullptr),
      m_dragLine2(nullptr),
      m_isDragging(false)
{
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
}

/**
 * @brief Destrutor da classe RouteLineItem. Remove itens de arraste criados temporariamente.
 */
RouteLineItem::~RouteLineItem()
{
    if (m_dragHandle) {
        if (scene()) scene()->removeItem(m_dragHandle);
        delete m_dragHandle;
    }
    if (m_dragLine1) {
        if (scene()) scene()->removeItem(m_dragLine1);
        delete m_dragLine1;
    }
    if (m_dragLine2) {
        if (scene()) scene()->removeItem(m_dragLine2);
        delete m_dragLine2;
    }
}

/**
 * @brief Retorna o retângulo delimitador estendido para detecção de cliques no segmento de reta.
 * 
 * @return QRectF Retângulo delimitador estendido.
 */
QRectF RouteLineItem::boundingRect() const
{
    QRectF baseRect = QGraphicsLineItem::boundingRect();
    qreal clickWidth = pen().widthF() * 3.5;
    if (clickWidth < 12.0) clickWidth = 12.0;
    qreal margin = clickWidth / 2.0;
    return baseRect.adjusted(-margin, -margin, margin, margin);
}

/**
 * @brief Retorna o caminho gráfico estendido (hitbox alargada) do segmento para cliques fáceis.
 * 
 * @return QPainterPath Forma gráfica sensível ao clique.
 */
QPainterPath RouteLineItem::shape() const
{
    QPainterPath path;
    path.moveTo(line().p1());
    path.lineTo(line().p2());
    
    QPainterPathStroker stroker;
    qreal clickWidth = pen().widthF() * 3.5;
    if (clickWidth < 12.0) clickWidth = 12.0;
    stroker.setWidth(clickWidth);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);
    
    return stroker.createStroke(path);
}

/**
 * @brief Captura o clique esquerdo do mouse para inicializar a interação de arrasto.
 * 
 * @param event Evento do mouse na cena.
 */
void RouteLineItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        setOpacity(0.2);

        QPointF p1 = line().p1();
        QPointF p2 = line().p2();
        QPointF currentPos = event->scenePos();
        
        qreal r = pen().widthF() * 1.5;
        if (r < 6.0) r = 6.0;
        
        m_dragHandle = new QGraphicsEllipseItem(currentPos.x() - r, currentPos.y() - r, r * 2, r * 2);
        m_dragHandle->setBrush(QBrush(QColor("#F97316")));
        m_dragHandle->setPen(QPen(QColor("#C2410C"), r * 0.2));
        m_dragHandle->setZValue(15);
        scene()->addItem(m_dragHandle);

        QPen dragPen(QColor("#F97316"), pen().widthF());
        dragPen.setStyle(Qt::DashLine);
        
        m_dragLine1 = new QGraphicsLineItem(p1.x(), p1.y(), currentPos.x(), currentPos.y());
        m_dragLine1->setPen(dragPen);
        m_dragLine1->setZValue(14);
        scene()->addItem(m_dragLine1);

        m_dragLine2 = new QGraphicsLineItem(currentPos.x(), currentPos.y(), p2.x(), p2.y());
        m_dragLine2->setPen(dragPen);
        m_dragLine2->setZValue(14);
        scene()->addItem(m_dragLine2);
        
        event->accept();
    } else {
        QGraphicsLineItem::mousePressEvent(event);
    }
}

/**
 * @brief Atualiza a pré-visualização das linhas de arraste conforme a movimentação do cursor.
 * 
 * @param event Evento do mouse na cena.
 */
void RouteLineItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging && m_dragHandle && m_dragLine1 && m_dragLine2) {
        QPointF currentPos = event->scenePos();
        
        qreal r = m_dragHandle->rect().width() / 2.0;
        m_dragHandle->setRect(currentPos.x() - r, currentPos.y() - r, r * 2, r * 2);
        
        QPointF p1 = line().p1();
        QPointF p2 = line().p2();
        m_dragLine1->setLine(QLineF(p1, currentPos));
        m_dragLine2->setLine(QLineF(currentPos, p2));
        
        event->accept();
    } else {
        QGraphicsLineItem::mouseMoveEvent(event);
    }
}

/**
 * @brief Finaliza o arrasto e inicia assincronamente a atualização da rota.
 * 
 * @param event Evento do mouse na cena.
 */
void RouteLineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging && event->button() == Qt::LeftButton) {
        m_isDragging = false;
        setOpacity(1.0);
        
        QPointF releasePos = event->scenePos();
        
        if (m_dragHandle) {
            scene()->removeItem(m_dragHandle);
            delete m_dragHandle;
            m_dragHandle = nullptr;
        }
        if (m_dragLine1) {
            scene()->removeItem(m_dragLine1);
            delete m_dragLine1;
            m_dragLine1 = nullptr;
        }
        if (m_dragLine2) {
            scene()->removeItem(m_dragLine2);
            delete m_dragLine2;
            m_dragLine2 = nullptr;
        }

        int index = m_legIndex;
        MainWindow *mw = m_mainWindow;

        QTimer::singleShot(0, mw, [mw, index, releasePos]() {
            mw->onRouteSegmentDragged(index, releasePos);
        });

        event->accept();
    } else {
        QGraphicsLineItem::mouseReleaseEvent(event);
    }
}

// Implementação de WaypointNodeItem

/**
 * @brief Construtor da classe WaypointNodeItem.
 * 
 * @param x Posição X central do ponto.
 * @param y Posição Y central do ponto.
 * @param r Raio do marcador visual.
 * @param nodeId ID do nó do grafo representado.
 * @param waypointIndex Ãndice deste ponto na sequência de paradas.
 * @param mainWindow Ponteiro para a janela principal.
 * @param parent Item gráfico pai.
 */
WaypointNodeItem::WaypointNodeItem(qreal x, qreal y, qreal r, int nodeId, int waypointIndex, MainWindow *mainWindow, QGraphicsItem *parent)
    : QGraphicsEllipseItem(x - r, y - r, r * 2, r * 2, parent),
      m_mainWindow(mainWindow),
      m_nodeId(nodeId),
      m_waypointIndex(waypointIndex),
      m_dragHandle(nullptr),
      m_isDragging(false)
{
    setAcceptHoverEvents(true);
    setCursor(Qt::PointingHandCursor);
    
    QBrush stopBrush(QColor("#F97316"));
    QPen stopPen(QColor("#C2410C"));
    stopPen.setWidthF(r * 0.3);
    
    setBrush(stopBrush);
    setPen(stopPen);
    setZValue(5);
    setToolTip(QString("Parada %1 (Nó %2)").arg(waypointIndex + 1).arg(nodeId));
}

/**
 * @brief Destrutor da classe WaypointNodeItem. Remove alças temporárias.
 */
WaypointNodeItem::~WaypointNodeItem()
{
    if (m_dragHandle) {
        if (scene()) scene()->removeItem(m_dragHandle);
        delete m_dragHandle;
    }
}

/**
 * @brief Retorna o retângulo delimitador estendido para detecção facilitada de cliques.
 * 
 * @return QRectF Retângulo delimitador estendido.
 */
QRectF WaypointNodeItem::boundingRect() const
{
    QRectF r = rect();
    qreal r_val = r.width() / 2.0;
    QPointF center = r.center();
    qreal clickRadius = r_val * 3.5;
    return QRectF(center.x() - clickRadius, center.y() - clickRadius, clickRadius * 2.0, clickRadius * 2.0);
}

/**
 * @brief Retorna a forma gráfica estendida para área de clique confortável (hitbox alargada).
 * 
 * @return QPainterPath Forma gráfica sensível a cliques.
 */
QPainterPath WaypointNodeItem::shape() const
{
    QPainterPath path;
    QRectF r = rect();
    qreal r_val = r.width() / 2.0;
    QPointF center = r.center();
    qreal clickRadius = r_val * 3.5;
    path.addEllipse(center, clickRadius, clickRadius);
    return path;
}

/**
 * @brief Captura o clique do mouse para iniciar o arrasto do ponto de parada.
 * 
 * @param event Evento do mouse na cena.
 */
void WaypointNodeItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_isDragging = true;
        setOpacity(0.2);

        QPointF currentPos = event->scenePos();
        qreal r = rect().width() / 2.0;
        
        m_dragHandle = new QGraphicsEllipseItem(currentPos.x() - r, currentPos.y() - r, r * 2, r * 2);
        m_dragHandle->setBrush(QBrush(QColor("#F97316")));
        m_dragHandle->setPen(QPen(QColor("#C2410C"), pen().widthF()));
        m_dragHandle->setZValue(15);
        scene()->addItem(m_dragHandle);
        
        event->accept();
    } else {
        QGraphicsEllipseItem::mousePressEvent(event);
    }
}

/**
 * @brief Atualiza a posição visual de arraste do ponto conforme o mouse se move.
 * 
 * @param event Evento do mouse na cena.
 */
void WaypointNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging && m_dragHandle) {
        QPointF currentPos = event->scenePos();
        qreal r = rect().width() / 2.0;
        m_dragHandle->setRect(currentPos.x() - r, currentPos.y() - r, r * 2, r * 2);
        event->accept();
    } else {
        QGraphicsEllipseItem::mouseMoveEvent(event);
    }
}

/**
 * @brief Finaliza o arrasto e dispara de forma assíncrona a atualização de posição da parada.
 * 
 * @param event Evento do mouse na cena.
 */
void WaypointNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_isDragging && event->button() == Qt::LeftButton) {
        m_isDragging = false;
        setOpacity(1.0);
        
        QPointF releasePos = event->scenePos();
        if (m_dragHandle) {
            scene()->removeItem(m_dragHandle);
            delete m_dragHandle;
            m_dragHandle = nullptr;
        }

        int index = m_waypointIndex;
        MainWindow *mw = m_mainWindow;

        QTimer::singleShot(0, mw, [mw, index, releasePos]() {
            mw->onWaypointDragged(index, releasePos);
        });

        event->accept();
    } else {
        QGraphicsEllipseItem::mouseReleaseEvent(event);
    }
}

/**
 * @brief Construtor da classe MainWindow. Inicializa componentes gráficos e de controle.
 * 
 * @param parent Widget pai (opcional).
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    aplicarEstilo();
    ui->stopsFrame->setVisible(false);
 
    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
 
    currentMap      = nullptr;
    firstShow       = true;
    nodeRadius      = 5.0;
    nodeLabel       = nullptr;
    nextClickTarget = 0;
    clickCount      = 0;
    isAddingNode    = false;
    isAddingEdge    = false;
    isRemovingEdge  = false;
    edgeNodeA       = -1;
  
    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->graphicsView->setContextMenuPolicy(Qt::NoContextMenu);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing);
    ui->graphicsView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->graphicsView->viewport()->installEventFilter(this);
 
    ui->spinOrigem->installEventFilter(this);
    ui->spinOrigem->setMinimum(-1);
    ui->spinOrigem->setValue(-1);
    ui->spinOrigem->setSpecialValueText(" ");

    ui->spinDestino->installEventFilter(this);
    ui->spinDestino->setMinimum(-1);
    ui->spinDestino->setValue(-1);
    ui->spinDestino->setSpecialValueText(" ");

    ui->spinParada->installEventFilter(this);
    ui->spinParada->setMinimum(-1);
    ui->spinParada->setValue(-1);
    ui->spinParada->setSpecialValueText(" ");

    QLabel *legendLabel = new QLabel(ui->graphicsView);
    legendLabel->setText("<b>Legenda:</b><br><br>"
                         "<span style='color:#3B82F6;'>■</span> Via de Mão Dupla (Mapa Geral)<br>"
                         "<span style='color:#F97316;'>■</span> Via de Mão Única (Mapa Geral)<br>"
                         "<span style='color:#EF4444;'>■</span> Trajeto de Interesse (Mão Dupla)<br>"
                         "<span style='color:#F97316;'>■</span> Trajeto de Interesse (Mão Única)");
    legendLabel->setStyleSheet("background-color: rgba(255, 255, 255, 0.9); border: 1px solid #D1D5DB; border-radius: 6px; padding: 10px;");
    
    QGridLayout *overlayLayout = new QGridLayout(ui->graphicsView);
    overlayLayout->setContentsMargins(15, 15, 15, 15);
    overlayLayout->addWidget(legendLabel, 0, 0, Qt::AlignTop | Qt::AlignLeft);
}
 
/**
 * @brief Destrutor da classe MainWindow. Libera os recursos alocados.
 */
MainWindow::~MainWindow()
{
    if (currentMap) delete currentMap;
    delete ui;
}
 
/**
 * @brief Filtra eventos recebidos pela interface para gerenciar zoom, pan e cliques no mapa.
 * 
 * @param obj Objeto de origem do evento.
 * @param event Evento a ser analisado.
 * @return true se o evento foi totalmente tratado, false caso contrário.
 */
bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn) {
        if (obj == ui->spinOrigem)   nextClickTarget = 0;
        else if (obj == ui->spinDestino) nextClickTarget = 1;
        else if (obj == ui->spinParada) nextClickTarget = 2;
    }
 
    if (obj == ui->graphicsView->viewport() && currentMap) {
        if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                QList<QGraphicsItem*> items = ui->graphicsView->items(mouseEvent->pos());
                bool clickedOnInteractiveItem = false;
                for (auto item : items) {
                    if (dynamic_cast<RouteLineItem*>(item) || dynamic_cast<WaypointNodeItem*>(item)) {
                        clickedOnInteractiveItem = true;
                        break;
                    }
                }
                if (clickedOnInteractiveItem) {
                    return false;
                }
 
                QPointF scenePos = ui->graphicsView->mapToScene(mouseEvent->pos());
                
                if (isRemovingEdge) {
                    QList<QGraphicsItem*> clickedItems = ui->graphicsView->items(mouseEvent->pos());
                    for (auto item : clickedItems) {
                        if (item->type() == QGraphicsLineItem::Type && item->zValue() == 0) {
                            int u = item->data(0).toInt();
                            int v = item->data(1).toInt();
                            
                            // Remove v de u
                            currentMap->adjacencias[u].erase(
                                std::remove_if(currentMap->adjacencias[u].begin(), currentMap->adjacencias[u].end(),
                                               [v](const std::pair<int, double>& p) { return p.first == v; }),
                                currentMap->adjacencias[u].end());
                                
                            // Remove u de v
                            currentMap->adjacencias[v].erase(
                                std::remove_if(currentMap->adjacencias[v].begin(), currentMap->adjacencias[v].end(),
                                               [u](const std::pair<int, double>& p) { return p.first == u; }),
                                currentMap->adjacencias[v].end());
                            
                            ui->lblStatus->setText(QString("Rua entre Nós %1 e %2 removida!").arg(u).arg(v));
                            isRemovingEdge = false;
                            ui->graphicsView->viewport()->setCursor(Qt::ArrowCursor);
                            drawMap();
                            highlightSelectedNodes();
                            return true;
                        }
                    }
                    return true;
                }

                if (isAddingNode) {
                    int novoId = currentMap->coordenadas.size();
                    currentMap->coordenadas.push_back({scenePos.x(), scenePos.y()});
                    currentMap->adjacencias.push_back({}); // Nova lista de adjacências vazia

                    QBrush nodeBrush(QColor("#3B82F6"));
                    QPen nodePen(QColor("#1E3A5F"));
                    nodePen.setWidthF(nodeRadius * 0.15);
                    
                    QGraphicsEllipseItem* el = scene->addEllipse(
                        scenePos.x() - nodeRadius,
                        scenePos.y() - nodeRadius,
                        nodeRadius * 2,
                        nodeRadius * 2,
                        nodePen,
                        nodeBrush
                    );
                    el->setZValue(1);
                    nodeItems.push_back(el);

                    ui->spinOrigem->setMaximum(novoId);
                    ui->spinDestino->setMaximum(novoId);
                    ui->spinParada->setMaximum(novoId);

                    ui->lblStatus->setText(QString("Nó %1 adicionado com sucesso!").arg(novoId));

                    isAddingNode = false;
                    ui->graphicsView->viewport()->setCursor(Qt::ArrowCursor);
                    return true;
                }

                int nearestNode  = findNearestNode(scenePos);
                if (nearestNode >= 0) {
                    if (isAddingEdge) {
                        if (edgeNodeA == -1) {
                            edgeNodeA = nearestNode;
                            ui->lblStatus->setText(QString("Rua: Nó %1 selecionado. Agora clique no SEGUNDO nó.").arg(edgeNodeA));
                            // Realce do nó A selecionado
                            nodeItems[edgeNodeA]->setBrush(QBrush(QColor("#F59E0B")));
                        } else {
                            if (nearestNode != edgeNodeA) {
                                int u = edgeNodeA;
                                int v = nearestNode;
                                double dx = currentMap->coordenadas[u].first - currentMap->coordenadas[v].first;
                                double dy = currentMap->coordenadas[u].second - currentMap->coordenadas[v].second;
                                double dist = std::sqrt(dx*dx + dy*dy);
                                
                                // Adiciona as arestas (ida e volta para mão dupla)
                                currentMap->adjacencias[u].push_back({v, dist});
                                currentMap->adjacencias[v].push_back({u, dist});
                                
                                ui->lblStatus->setText(QString("Trecho %1 ↔ %2 adicionado. Continue clicando para estender a rua, ou clique duas vezes no mesmo nó para soltar.").arg(u).arg(v));
                                edgeNodeA = v; // Transfere a ponta da rua para o novo nó
                                
                                drawMap();
                                highlightSelectedNodes();
                                nodeItems[edgeNodeA]->setBrush(QBrush(QColor("#F59E0B"))); // Mantém destacado
                            } else {
                                // Clicar no mesmo nó encerra a rua atual, permitindo começar outra
                                edgeNodeA = -1;
                                ui->lblStatus->setText("Rua finalizada. Clique em um novo nó inicial para criar outra rua.");
                                drawMap();
                                highlightSelectedNodes();
                            }
                        }
                        return true;
                    }

                    if (nextClickTarget == 0) {
                        ui->spinOrigem->setValue(nearestNode);
                        nextClickTarget = 1;
                    } else if (nextClickTarget == 1) {
                        ui->spinDestino->setValue(nearestNode);
                    } else if (nextClickTarget == 2) {
                        ui->spinParada->setValue(nearestNode);
                    }
                    clickCount++;
                    highlightSelectedNodes();
                     
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
 
/**
 * @brief Gerencia a exibição inicial da janela para enquadrar o mapa na tela.
 * 
 * @param event Evento de exibição.
 */
void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (firstShow && scene->items().count() > 0) {
        firstShow = false;
        QTimer::singleShot(100, this, &MainWindow::fitGraphInView);
    }
}
 
/**
 * @brief Ajusta o enquadramento do mapa de forma que todo o grafo caiba na área visível da tela.
 */
void MainWindow::fitGraphInView()
{
    if (scene->items().count() > 0) {
        QRectF bounds = scene->itemsBoundingRect();
        double marginX = bounds.width()  * 0.05;
        double marginY = bounds.height() * 0.05;
        bounds.adjust(-marginX, -marginY, marginX, marginY);
        ui->graphicsView->fitInView(bounds, Qt::KeepAspectRatio);
    }
}
 
/**
 * @brief Encontra o nó do mapa mais próximo de uma coordenada informada, dentro de um raio limite.
 * 
 * @param pos Posição a ser analisada na cena.
 * @return int ID do nó mais próximo encontrado, ou -1 caso nenhum esteja no limite estabelecido.
 */
int MainWindow::findNearestNode(const QPointF &pos)
{
    if (!currentMap || currentMap->coordenadas.empty()) return -1;
 
    int    nearest = -1;
    double minDist = nodeRadius * 3.0;
 
    for (size_t i = 0; i < currentMap->coordenadas.size(); ++i) {
        double nx   = currentMap->coordenadas[i].first;
        double ny   = currentMap->coordenadas[i].second;
        double dx   = pos.x() - nx;
        double dy   = pos.y() - ny;
        double dist = std::sqrt(dx * dx + dy * dy);
        if (dist < minDist) {
            minDist = dist;
            nearest = static_cast<int>(i);
        }
    }
    return nearest;
}
 
/**
 * @brief Destaca visualmente no mapa os nós selecionados de origem e destino com cores correspondentes.
 */
void MainWindow::highlightSelectedNodes()
{
    if (!currentMap) return;
 
    QBrush defaultBrush(QColor("#3B82F6"));
    QPen   defaultPen(QColor("#1E3A5F"));
    defaultPen.setWidthF(nodeRadius * 0.15);
 
    for (auto node : nodeItems) {
        node->setBrush(defaultBrush);
        node->setPen(defaultPen);
        node->setZValue(1);
        node->setVisible(false);
    }
 
    int origem = ui->spinOrigem->value();
    if (origem >= 0 && origem < static_cast<int>(nodeItems.size())) {
        nodeItems[origem]->setVisible(true);
        nodeItems[origem]->setBrush(QBrush(QColor("#EAB308")));
        QPen originPen(QColor("#92400E"));
        originPen.setWidthF(nodeRadius * 0.3);
        nodeItems[origem]->setPen(originPen);
        nodeItems[origem]->setZValue(5);
    }
 
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

/**
 * @brief Slot acionado para carregar um arquivo de mapa (.poly).
 */
void MainWindow::on_btnCarregarMapa_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Abrir Mapa", "", "Map Files (*.poly *.osm);;Polygon Files (*.poly);;OSM Files (*.osm);;Todos Arquivos (*)");
    if (fileName.isEmpty()) return;

    if (fileName.endsWith(".osm", Qt::CaseInsensitive)) {
        ui->lblStatus->setText("Convertendo arquivo .osm para .poly...");
        QCoreApplication::processEvents();

        QString appDir = QCoreApplication::applicationDirPath();
        QString converterPath = appDir + "/../core/osm_converter.exe";
        if (!QFile::exists(converterPath)) {
            converterPath = appDir + "/osm_converter.exe";
        }
        if (!QFile::exists(converterPath)) {
            converterPath = "osm_converter.exe";
        }

        QProcess process;
        process.start(converterPath, QStringList() << fileName);
        if (!process.waitForFinished(60000)) {
            QMessageBox::critical(this, "Erro", "Tempo limite excedido na conversão do arquivo OSM.");
            ui->lblStatus->setText("Falha na conversão.");
            return;
        }
        if (process.exitCode() != 0) {
            QMessageBox::critical(this, "Erro", "Falha na conversão do arquivo OSM. O conversor retornou código: " + QString::number(process.exitCode()));
            ui->lblStatus->setText("Falha na conversão.");
            return;
        }

        QString newFileName = fileName.left(fileName.length() - 4) + ".poly";
        if (!QFile::exists(newFileName)) {
            QMessageBox::critical(this, "Erro", "Arquivo .poly não foi encontrado após a conversão.");
            ui->lblStatus->setText("Falha na conversão.");
            return;
        }
        fileName = newFileName;
    }

    try {
        if (currentMap) delete currentMap;
        currentMap = new MapData(carregarMapa(fileName.toStdString()));

        int totalNos = static_cast<int>(currentMap->adjacencias.size());
        ui->spinOrigem->setMaximum(totalNos - 1);
        ui->spinDestino->setMaximum(totalNos - 1);
        ui->spinParada->setMaximum(totalNos - 1);

        paradas.clear();
        ui->listWidgetParadas->clear();

        int totalArestas = 0;
        for (const auto& adj : currentMap->adjacencias)
            totalArestas += adj.size();

        ui->lblStatus->setText(QString("Mapa carregado: %1 nós, %2 arestas.").arg(totalNos).arg(totalArestas));

        nextClickTarget = 0;
        clickCount      = 0;

        drawMap();
        QTimer::singleShot(50, this, &MainWindow::fitGraphInView);

    } catch (...) {
        QMessageBox::critical(this, "Erro", "Falha ao carregar o mapa.");
    }
}

/**
 * @brief Desenha todo o grafo (vértices e arestas) do mapa carregado na cena gráfica.
 */
void MainWindow::drawMap()
{
    scene->clear();
    nodeItems.clear();
    edgeItems.clear();
    pathEdgeItems.clear();
    nodeLabel = nullptr;

    if (!currentMap || currentMap->adjacencias.empty()) return;

    int totalNos = static_cast<int>(currentMap->coordenadas.size());

    double minX = currentMap->coordenadas[0].first,  maxX = minX;
    double minY = currentMap->coordenadas[0].second, maxY = minY;

    for (const auto& coord : currentMap->coordenadas) {
        if (coord.first  < minX) minX = coord.first;
        if (coord.first  > maxX) maxX = coord.first;
        if (coord.second < minY) minY = coord.second;
        if (coord.second > maxY) maxY = coord.second;
    }

    double maxRange = std::max(maxX - minX, maxY - minY);
    if (maxRange < 1.0) maxRange = 1.0;

    nodeRadius = maxRange * 0.004;
    if (nodeRadius < 1.0) nodeRadius = 1.0;

    QPen edgePen(QColor("#3B82F6")); // Azul para vias de mão dupla gerais
    edgePen.setWidthF(nodeRadius * 0.3);
    edgePen.setCosmetic(false);
    edgePen.setJoinStyle(Qt::RoundJoin);
    edgePen.setCapStyle(Qt::RoundCap);

    QPen onewayPen = edgePen;
    onewayPen.setStyle(Qt::DashLine);
    onewayPen.setColor(QColor("#F97316")); // Laranja

    QBrush nodeBrush(QColor("#3B82F6"));
    QPen   nodePen(QColor("#1E3A5F"));
    nodePen.setWidthF(nodeRadius * 0.15);

    for (size_t u = 0; u < currentMap->adjacencias.size(); ++u) {
        double x1 = currentMap->coordenadas[u].first;
        double y1 = currentMap->coordenadas[u].second;
        for (const auto& aresta : currentMap->adjacencias[u]) {
            int v = aresta.first;
            
            bool temVparaU = false;
            for (const auto& revAresta : currentMap->adjacencias[v]) {
                if (revAresta.first == static_cast<int>(u)) {
                    temVparaU = true;
                    break;
                }
            }

            if (temVparaU) {
                if (static_cast<int>(u) < v) {
                    double x2 = currentMap->coordenadas[v].first;
                    double y2 = currentMap->coordenadas[v].second;
                    QGraphicsLineItem* line = scene->addLine(x1, y1, x2, y2, edgePen);
                    line->setZValue(0);
                    line->setData(0, static_cast<int>(u));
                    line->setData(1, v);
                    edgeItems.push_back(line);
                }
            } else {
                double x2 = currentMap->coordenadas[v].first;
                double y2 = currentMap->coordenadas[v].second;
                QGraphicsLineItem* line = scene->addLine(x1, y1, x2, y2, onewayPen);
                line->setZValue(0);
                line->setData(0, static_cast<int>(u));
                line->setData(1, v);
                edgeItems.push_back(line);
            }
        }
    }

    for (int i = 0; i < totalNos; ++i) {
        double x = currentMap->coordenadas[i].first;
        double y = currentMap->coordenadas[i].second;
        QGraphicsEllipseItem* ellipse = scene->addEllipse(
            x - nodeRadius, y - nodeRadius,
            nodeRadius * 2, nodeRadius * 2,
            nodePen, nodeBrush);
        ellipse->setZValue(1);
        ellipse->setToolTip(QString("Nó %1\n(%2, %3)").arg(i).arg(x, 0, 'f', 1).arg(y, 0, 'f', 1));
        ellipse->setVisible(false);
        nodeItems.push_back(ellipse);
    }

    ui->graphicsView->resetTransform();
}

/**
 * @brief Slot acionado para calcular o menor caminho entre origem e destino, passando por eventuais paradas.
 */
void MainWindow::on_btnCalcular_clicked()
{
    if (!currentMap || currentMap->adjacencias.empty()) {
        QMessageBox::warning(this, "Aviso", "Carregue um mapa primeiro.");
        return;
    }

    int origem  = ui->spinOrigem->value();
    int destino = ui->spinDestino->value();
    int totalNos = static_cast<int>(currentMap->adjacencias.size());

    if (origem < 0 || origem >= totalNos || destino < 0 || destino >= totalNos) {
        QMessageBox::warning(this, "Aviso", "Nó de origem ou destino inválido.");
        return;
    }

    for (int p : paradas) {
        if (p < 0 || p >= totalNos) {
            QMessageBox::warning(this, "Aviso", "Há uma parada intermediária com ID de nó inválido.");
            return;
        }
    }

    clearPath();
    ui->lblStatus->setText("Calculando...");
    QCoreApplication::processEvents();

    std::vector<int> stops = {origem};
    stops.insert(stops.end(), paradas.begin(), paradas.end());
    stops.push_back(destino);

    DijkstraResult finalRes;
    finalRes.encontrado = true;
    finalRes.distanciaTotal = 0.0;
    finalRes.timeMs = 0.0;
    finalRes.nodesExplored = 0;

    std::vector<std::vector<int>> subPaths;

    for (size_t i = 0; i < stops.size() - 1; ++i) {
        int u = stops[i];
        int v = stops[i + 1];
        DijkstraResult part = Dijkstra(currentMap->adjacencias, u, v);
        finalRes.timeMs += part.timeMs;
        finalRes.nodesExplored += part.nodesExplored;

        if (!part.encontrado) {
            finalRes.encontrado = false;
            finalRes.caminho.clear();
            finalRes.distanciasTrecho.clear();
            finalRes.distanciaTotal = 0.0;
            subPaths.clear();
            break;
        }

        finalRes.distanciaTotal += part.distanciaTotal;

        if (finalRes.caminho.empty()) {
            finalRes.caminho = part.caminho;
        } else {
            finalRes.caminho.insert(finalRes.caminho.end(), part.caminho.begin() + 1, part.caminho.end());
        }

        finalRes.distanciasTrecho.insert(finalRes.distanciasTrecho.end(), part.distanciasTrecho.begin(), part.distanciasTrecho.end());
        subPaths.push_back(part.caminho);
    }

    exibirCaminho(finalRes, origem, destino);

    if (finalRes.encontrado) {
        drawPath(subPaths);
        ui->lblStatus->setText(QString("Caminho encontrado! %1 nós na rota.").arg(finalRes.caminho.size()));
        ui->lblCost->setText(QString("Custo: %1 m").arg(finalRes.distanciaTotal, 0, 'f', 2));
        ui->lblTime->setText(QString("Tempo: %1 ms").arg(finalRes.timeMs, 0, 'f', 2));
        ui->lblNodes->setText(QString("Nós Explorados: %1").arg(finalRes.nodesExplored));
        QTimer::singleShot(50, this, [this, finalRes]() {
            zoomToRoute(finalRes.caminho);
        });
    } else {
        ui->lblStatus->setText("Nenhum caminho encontrado.");
        ui->lblCost->setText("Custo: -");
        ui->lblTime->setText(QString("Tempo: %1 ms").arg(finalRes.timeMs, 0, 'f', 2));
        ui->lblNodes->setText(QString("Nós Explorados: %1").arg(finalRes.nodesExplored));
        
        ui->listWidgetCaminho->clear();
        auto *item = new QListWidgetItem("  âš ï¸ Sem rota possível com a sequência de paradas fornecida.");
        item->setForeground(QColor("#FF6B6B"));
        ui->listWidgetCaminho->addItem(item);

        QMessageBox::information(this, "Resultado", "Não há caminho entre os nós selecionados com a sequência de paradas fornecida.");
    }
}

/**
 * @brief Aplica zoom focado na área delimitada pelo traçado da rota calculada.
 * 
 * @param path Vetor de IDs dos nós que formam a rota.
 */
void MainWindow::zoomToRoute(const std::vector<int>& path)
{
    if (!currentMap || path.empty()) return;

    double minX = currentMap->coordenadas[path[0]].first,  maxX = minX;
    double minY = currentMap->coordenadas[path[0]].second, maxY = minY;

    for (int nodeId : path) {
        double x = currentMap->coordenadas[nodeId].first;
        double y = currentMap->coordenadas[nodeId].second;
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;
    }

    double marginX = (maxX - minX) * 0.15 + nodeRadius * 5;
    double marginY = (maxY - minY) * 0.15 + nodeRadius * 5;

    QRectF routeRect(minX - marginX, minY - marginY,
                     (maxX - minX) + 2 * marginX,
                     (maxY - minY) + 2 * marginY);

    ui->graphicsView->fitInView(routeRect, Qt::KeepAspectRatio);
}

/**
 * @brief Limpa todos os elementos visuais da rota ativa na cena gráfica.
 */
void MainWindow::clearPath()
{
    for (auto line : pathEdgeItems) {
        scene->removeItem(line);
        delete line;
    }
    pathEdgeItems.clear();
    ui->listWidgetCaminho->clear();
    highlightSelectedNodes();
}

/**
 * @brief Desenha a rota calculada na cena gráfica, incluindo retas interativas e paradas.
 * 
 * @param subPaths Vetor de vetores contendo os caminhos de cada trecho da rota.
 */
void MainWindow::drawPath(const std::vector<std::vector<int>>& subPaths)
{
    if (!currentMap || subPaths.empty()) return;

    double minX = currentMap->coordenadas[0].first,  maxX = minX;
    double minY = currentMap->coordenadas[0].second, maxY = minY;
    for (const auto& coord : currentMap->coordenadas) {
        if (coord.first  < minX) minX = coord.first;
        if (coord.first  > maxX) maxX = coord.first;
        if (coord.second < minY) minY = coord.second;
        if (coord.second > maxY) maxY = coord.second;
    }
    double maxRange = std::max(maxX - minX, maxY - minY);
    if (maxRange < 1.0) maxRange = 1.0;

    QPen pathPen(QColor("#EF4444"));
    pathPen.setWidthF(maxRange * 0.006);
    pathPen.setCosmetic(false);
    pathPen.setJoinStyle(Qt::RoundJoin);
    pathPen.setCapStyle(Qt::RoundCap);

    for (size_t legIndex = 0; legIndex < subPaths.size(); ++legIndex) {
        const auto& path = subPaths[legIndex];
        if (path.size() < 2) continue;

        for (size_t i = 0; i < path.size() - 1; ++i) {
            int u = path[i], v = path[i + 1];
            double x1 = currentMap->coordenadas[u].first,  y1 = currentMap->coordenadas[u].second;
            double x2 = currentMap->coordenadas[v].first,  y2 = currentMap->coordenadas[v].second;
            
            bool isOneWay = true;
            for (const auto& revAresta : currentMap->adjacencias[v]) {
                if (revAresta.first == u) {
                    isOneWay = false;
                    break;
                }
            }

            QPen currentPen = pathPen;
            if (isOneWay) {
                currentPen.setColor(QColor("#F97316")); // Laranja
            }

            // Desenha trechos interativos da rota
            RouteLineItem* line = new RouteLineItem(x1, y1, x2, y2, u, v, static_cast<int>(legIndex), this);
            line->setPen(currentPen);
            line->setZValue(2);
            scene->addItem(line);
            pathEdgeItems.push_back(line);
        }
    }

    // Desenha marcadores de parada interativos
    for (size_t i = 0; i < paradas.size(); ++i) {
        int p = paradas[i];
        if (p >= 0 && p < static_cast<int>(nodeItems.size())) {
            double px = currentMap->coordenadas[p].first;
            double py = currentMap->coordenadas[p].second;
            
            WaypointNodeItem* stopItem = new WaypointNodeItem(px, py, nodeRadius, p, static_cast<int>(i), this);
            scene->addItem(stopItem);
            pathEdgeItems.push_back(stopItem);
        }
    }

    int o = ui->spinOrigem->value();
    int d = ui->spinDestino->value();
    
    // Realce e rótulo do ponto de partida
    if (o >= 0 && o < static_cast<int>(nodeItems.size())) {
        nodeItems[o]->setVisible(true);
        nodeItems[o]->setBrush(QBrush(QColor("#EAB308")));
        QPen originPen(QColor("#92400E")); originPen.setWidthF(nodeRadius * 0.3);
        nodeItems[o]->setPen(originPen);   nodeItems[o]->setZValue(3);

        double ox = currentMap->coordenadas[o].first;
        double oy = currentMap->coordenadas[o].second;
        
        QGraphicsTextItem *oText = new QGraphicsTextItem();
        oText->setHtml(QString("<div style='background:#EAB308; color:#0F0F23; padding:2px 6px; border-radius:3px; font-weight:bold; font-size:11px;'>"
                               "Partida (Nó %1)</div>").arg(o));
        QRectF rect = oText->boundingRect();
        oText->setPos(ox - rect.width() / 2, oy - nodeRadius * 4 - rect.height());
        oText->setZValue(10);
        scene->addItem(oText);
        pathEdgeItems.push_back(oText);
    }
    
    // Realce e rótulo do ponto de chegada
    if (d >= 0 && d < static_cast<int>(nodeItems.size())) {
        nodeItems[d]->setVisible(true);
        nodeItems[d]->setBrush(QBrush(QColor("#EF4444")));
        QPen destPen(QColor("#7F1D1D"));   destPen.setWidthF(nodeRadius * 0.3);
        nodeItems[d]->setPen(destPen);     nodeItems[d]->setZValue(3);
 
        double dx = currentMap->coordenadas[d].first;
        double dy = currentMap->coordenadas[d].second;
        
        QGraphicsTextItem *dText = new QGraphicsTextItem();
        dText->setHtml(QString("<div style='background:#EF4444; color:white; padding:2px 6px; border-radius:3px; font-weight:bold; font-size:11px;'>"
                               "Chegada (Nó %1)</div>").arg(d));
        QRectF rect = dText->boundingRect();
        dText->setPos(dx - rect.width() / 2, dy - nodeRadius * 4 - rect.height());
        dText->setZValue(10);
        scene->addItem(dText);
        pathEdgeItems.push_back(dText);
    }
}
 
/**
 * @brief Exibe o caminho detalhado passo a passo na tabela lateral (listWidgetCaminho).
 * 
 * @param resultado Estrutura com os resultados do menor caminho calculado.
 * @param origem ID do nó de partida.
 * @param destino ID do nó de chegada.
 */
void MainWindow::exibirCaminho(const DijkstraResult &resultado, int origem, int destino)
{
    ui->listWidgetCaminho->clear();
 
    if (!resultado.encontrado) {
        auto *item = new QListWidgetItem(QString("Sem rota entre o nó %1 e o nó %2.").arg(origem).arg(destino));
        item->setForeground(QColor("#EF4444"));
        ui->listWidgetCaminho->addItem(item);
        return;
    }
 
    auto *header = new QListWidgetItem("  Passo       Nó O.  â†’  Nó D.       Distância");
    header->setBackground(QColor("#E5E7EB"));
    header->setForeground(QColor("#111827"));
    QFont fHeader = header->font(); fHeader.setBold(true); header->setFont(fHeader);
    ui->listWidgetCaminho->addItem(header);
    ui->listWidgetCaminho->addItem(new QListWidgetItem(QString(60, QChar('-'))));
 
    const auto &caminho = resultado.caminho;
    for (size_t i = 0; i < caminho.size() - 1; i++) {
        double distTrecho = resultado.distanciasTrecho[i];
        QString linha = QString("  %1         %2  â†’  %3           %4 m")
                            .arg(i + 1, 4)
                            .arg(caminho[i], 6)
                            .arg(caminho[i + 1], 6)
                            .arg(distTrecho, 10, 'f', 2);
 
        auto *item = new QListWidgetItem(linha);
        item->setBackground(i % 2 == 0 ? QColor("#F9FAFB") : QColor("#F3F4F6"));
 
        if (i == 0)
            item->setForeground(QColor("#16A34A")); // Verde (Partida)
        else if (i == caminho.size() - 2)
            item->setForeground(QColor("#EA580C")); // Laranja (Chegada)
        else
            item->setForeground(QColor("#1F2937"));
 
        ui->listWidgetCaminho->addItem(item);
    }
 
    // Rodapé com resumo estatístico
    ui->listWidgetCaminho->addItem(new QListWidgetItem(QString(60, QChar('-'))));
    auto *rodape = new QListWidgetItem(
        QString("  Total: %1 trechos  |  Distância: %2 m")
            .arg(caminho.size() - 1)
            .arg(resultado.distanciaTotal, 0, 'f', 2));
    rodape->setBackground(QColor("#E5E7EB"));
    rodape->setForeground(QColor("#111827"));
    QFont fRodape = rodape->font(); fRodape.setBold(true); rodape->setFont(fRodape);
    ui->listWidgetCaminho->addItem(rodape);
}
 
/**
 * @brief Define as folhas de estilo CSS da aplicação (modo light/claro).
 */
void MainWindow::aplicarEstilo()
{
    setStyleSheet(R"(
        QMainWindow {
            background-color: #F3F4F6;
        }
        QWidget {
            background-color: #F3F4F6;
            color: #1F2937;
            font-family: "Monospace";
            font-size: 13px;
        }
        QFrame#sidebarFrame {
            background-color: #FFFFFF;
            border-right: 1px solid #E5E7EB;
        }
        QFrame#statsFrame, QFrame#stopsFrame {
            background-color: #F9FAFB;
            border: 1px solid #E5E7EB;
            border-radius: 6px;
            padding: 4px;
        }
        QLabel#labelTitulo, QLabel#lblTitle {
            color: #2563EB;
            font-size: 18px;
            font-weight: bold;
            padding: 12px 0px;
            border-bottom: 1px solid #E5E7EB;
            margin-bottom: 8px;
        }
        QLabel#lblSubtitle {
            font-size: 11px;
            color: #6B7280;
        }
        QLabel {
            color: #374151;
            background: transparent;
        }
        QLineEdit, QSpinBox, QComboBox {
            background-color: #FFFFFF;
            color: #1F2937;
            border: 1px solid #D1D5DB;
            border-radius: 4px;
            padding: 4px 8px;
            min-height: 25px;
        }
        QSpinBox {
            min-width: 80px;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border-color: #3B82F6;
        }
        QPushButton {
            background-color: #E5E7EB;
            color: #1F2937;
            border: 1px solid #D1D5DB;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
        }
        QPushButton:hover { background-color: #D1D5DB; }
        QPushButton:pressed { background-color: #9CA3AF; }

        QPushButton#btnCarregarMapa { background-color: #2563EB; color: white; border: none; }
        QPushButton#btnCarregarMapa:hover { background-color: #1D4ED8; }
        
        QPushButton#btnAdicionarNo { background-color: #F59E0B; color: white; border: none; }
        QPushButton#btnAdicionarNo:hover { background-color: #D97706; }
        
        QPushButton#btnAdicionarAresta { background-color: #F59E0B; color: white; border: none; }
        QPushButton#btnAdicionarAresta:hover { background-color: #D97706; }
        
        QPushButton#btnRemoverAresta { background-color: #EF4444; color: white; border: none; }
        QPushButton#btnRemoverAresta:hover { background-color: #DC2626; }
        
        QPushButton#btnCalcular { background-color: #10B981; color: white; border: none; }
        QPushButton#btnCalcular:hover { background-color: #059669; }
        
        QPushButton#btnLimpar { background-color: #EF4444; color: white; border: none; }
        QPushButton#btnLimpar:hover { background-color: #DC2626; }

        QPushButton#btnCopiarImagem { background-color: #6B7280; color: white; border: none; }
        QPushButton#btnCopiarImagem:hover { background-color: #4B5563; }

        QListWidget {
            background-color: #FFFFFF;
            color: #1F2937;
            border: 1px solid #E5E7EB;
            border-radius: 6px;
            font-family: "Monospace";
            font-size: 12px;
        }
        QListWidget::item {
            padding: 4px 6px;
            border-bottom: 1px solid #F3F4F6;
        }
        QListWidget::item:selected {
            background-color: #DBEAFE;
            color: #1E3A8A;
        }
        QStatusBar, QMenuBar {
            background-color: #FFFFFF;
            color: #374151;
            border-top: 1px solid #E5E7EB;
        }
        QGraphicsView {
            background-color: #F9FAFB;
            border: none;
        }
        QCheckBox {
            padding: 4px;
            margin-top: 5px;
            margin-bottom: 5px;
        }
    )");
}
 
#include <QClipboard>
 
/**
 * @brief Captura a exibição da cena gráfica do mapa e copia como imagem para a área de transferência.
 */
void MainWindow::on_btnCopiarImagem_clicked()
{
    if (!currentMap) {
        QMessageBox::warning(this, "Aviso", "Nenhum mapa carregado para copiar.");
        return;
    }
    QPixmap pixmap = ui->graphicsView->grab();
    QGuiApplication::clipboard()->setPixmap(pixmap);
    
    QMessageBox::information(this, "Sucesso", "Imagem do mapa copiada para a área de transferência!");
}
 
/**
 * @brief Slot acionado para adicionar manualmente uma parada intermediária baseada no valor do spinbox.
 */
void MainWindow::on_btnAdicionarParada_clicked()
{
    if (!currentMap) {
        QMessageBox::warning(this, "Aviso", "Carregue um mapa primeiro.");
        return;
    }
    int parada = ui->spinParada->value();
    int totalNos = static_cast<int>(currentMap->coordenadas.size());
    if (parada < 0 || parada >= totalNos) {
        QMessageBox::warning(this, "Aviso", "ID da parada inválido.");
        return;
    }
 
    paradas.push_back(parada);
    ui->listWidgetParadas->addItem(QString("Parada %1: Nó %2").arg(paradas.size()).arg(parada));
    
    highlightSelectedNodes();
}
 
/**
 * @brief Slot acionado para remover a parada selecionada ou a última parada da lista.
 */
void MainWindow::on_btnRemoverParada_clicked()
{
    int row = ui->listWidgetParadas->currentRow();
    if (row < 0) {
        if (!paradas.empty()) {
            paradas.pop_back();
            delete ui->listWidgetParadas->takeItem(ui->listWidgetParadas->count() - 1);
        }
    } else {
        paradas.erase(paradas.begin() + row);
        delete ui->listWidgetParadas->takeItem(row);
        
        ui->listWidgetParadas->clear();
        for (size_t i = 0; i < paradas.size(); ++i) {
            ui->listWidgetParadas->addItem(QString("Parada %1: Nó %2").arg(i + 1).arg(paradas[i]));
        }
    }
    highlightSelectedNodes();
}
 
/**
 * @brief Callback disparado quando uma linha de rota é arrastada, inserindo um novo ponto de parada.
 * 
 * @param legIndex Ãndice do trecho da rota que foi arrastado.
 * @param releasePos Posição da soltura do mouse.
 */
void MainWindow::onRouteSegmentDragged(int legIndex, const QPointF& releasePos)
{
    if (!currentMap) return;
    
    int nearest = findNearestNode(releasePos);
    int totalNos = static_cast<int>(currentMap->coordenadas.size());
    if (nearest >= 0 && nearest < totalNos) {
        int startNode = (legIndex == 0) ? ui->spinOrigem->value() : paradas[legIndex - 1];
        int endNode = (legIndex == static_cast<int>(paradas.size())) ? ui->spinDestino->value() : paradas[legIndex];
        
        if (nearest != startNode && nearest != endNode) {
            paradas.insert(paradas.begin() + legIndex, nearest);
            
            ui->listWidgetParadas->clear();
            for (size_t i = 0; i < paradas.size(); ++i) {
                ui->listWidgetParadas->addItem(QString("Parada %1: Nó %2").arg(i + 1).arg(paradas[i]));
            }
            
            highlightSelectedNodes();
            on_btnCalcular_clicked();
        }
    }
}
 
/**
 * @brief Slot acionado para limpar o menor caminho atual e reiniciar todos os parâmetros.
 */
void MainWindow::on_btnLimpar_clicked()
{
    if (nodeLabel) {
        scene->removeItem(nodeLabel);
        delete nodeLabel;
        nodeLabel = nullptr;
    }
    ui->spinOrigem->setValue(-1);
    ui->spinDestino->setValue(-1);
    ui->spinParada->setValue(-1);
    paradas.clear();
    nextClickTarget = 0;
    ui->listWidgetParadas->clear();
    clearPath();
    highlightSelectedNodes();
    
    ui->lblStatus->setText("Busca limpa.");
    ui->lblCost->setText("Custo: -");
    ui->lblTime->setText("Tempo: -");
    ui->lblNodes->setText("Nós Explorados: -");
}
 
/**
 * @brief Slot acionado quando a origem é modificada, limpando a rota ativa.
 * 
 * @param arg1 Novo valor do spinbox.
 */
void MainWindow::on_spinOrigem_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    paradas.clear();
    ui->listWidgetParadas->clear();
    if (nodeLabel) {
        scene->removeItem(nodeLabel);
        delete nodeLabel;
        nodeLabel = nullptr;
    }
    clearPath();
    highlightSelectedNodes();
}
 
/**
 * @brief Slot acionado quando o destino é modificado, limpando a rota ativa.
 * 
 * @param arg1 Novo valor do spinbox.
 */
void MainWindow::on_spinDestino_valueChanged(int arg1)
{
    Q_UNUSED(arg1);
    paradas.clear();
    ui->listWidgetParadas->clear();
    if (nodeLabel) {
        scene->removeItem(nodeLabel);
        delete nodeLabel;
        nodeLabel = nullptr;
    }
    clearPath();
    highlightSelectedNodes();
}
 
/**
 * @brief Callback disparado quando uma parada intermediária é arrastada, alterando sua posição ou excluindo-a.
 * 
 * @param waypointIndex Índice da parada intermediária arrastada.
 * @param releasePos Posição da soltura do mouse.
 */
void MainWindow::onWaypointDragged(int waypointIndex, const QPointF& releasePos)
{
    if (!currentMap) return;
    
    int nearest = findNearestNode(releasePos);
    int totalNos = static_cast<int>(currentMap->coordenadas.size());
    if (nearest >= 0 && nearest < totalNos) {
        int startNode = (waypointIndex == 0) ? ui->spinOrigem->value() : paradas[waypointIndex - 1];
        int endNode = (waypointIndex == static_cast<int>(paradas.size()) - 1) ? ui->spinDestino->value() : paradas[waypointIndex + 1];
        
        if (nearest != startNode && nearest != endNode) {
            paradas[waypointIndex] = nearest;
            
            ui->listWidgetParadas->clear();
            for (size_t i = 0; i < paradas.size(); ++i) {
                ui->listWidgetParadas->addItem(QString("Parada %1: Nó %2").arg(i + 1).arg(paradas[i]));
            }
            
            highlightSelectedNodes();
            on_btnCalcular_clicked();
        } else {
            paradas.erase(paradas.begin() + waypointIndex);
            
            ui->listWidgetParadas->clear();
            for (size_t i = 0; i < paradas.size(); ++i) {
                ui->listWidgetParadas->addItem(QString("Parada %1: Nó %2").arg(i + 1).arg(paradas[i]));
            }
            
            highlightSelectedNodes();
            on_btnCalcular_clicked();
        }
    }
}
void MainWindow::on_btnAdicionarNo_clicked()
{
    if (!currentMap) {
        QMessageBox::warning(this, "Aviso", "Carregue um mapa primeiro antes de adicionar nós.");
        return;
    }
    isAddingNode = true;
    isAddingEdge = false;
    isRemovingEdge = false;
    ui->graphicsView->viewport()->setCursor(Qt::CrossCursor);
    ui->lblStatus->setText("Modo Adição de Nó: Clique no mapa onde deseja criar o novo nó.");
}

void MainWindow::on_btnAdicionarAresta_clicked()
{
    if (!currentMap) {
        QMessageBox::warning(this, "Aviso", "Carregue um mapa primeiro antes de adicionar ruas.");
        return;
    }
    isAddingNode = false;
    isRemovingEdge = false;
    isAddingEdge = true;
    edgeNodeA = -1;
    ui->graphicsView->viewport()->setCursor(Qt::CrossCursor);
    ui->lblStatus->setText("Modo Adição de Rua: Clique no PRIMEIRO nó da rua.");
}

void MainWindow::on_btnRemoverAresta_clicked()
{
    if (!currentMap) {
        QMessageBox::warning(this, "Aviso", "Carregue um mapa primeiro antes de remover ruas.");
        return;
    }
    isAddingNode = false;
    isAddingEdge = false;
    isRemovingEdge = true;
    ui->graphicsView->viewport()->setCursor(Qt::CrossCursor);
    ui->lblStatus->setText("Modo Remoção de Rua: Clique em uma rua existente no mapa para apagá-la.");
}
