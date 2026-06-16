#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsView>
#include <QGraphicsItemGroup>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsSceneMouseEvent>
#include <QListWidget>
#include <QLabel>
#include <QSpinBox>
#include <QLineEdit>
#include <QPushButton>
#include <QString>
#include <vector>
#include <QScrollBar>

#include "../core/projects.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

/**
 * @class ZoomableGraphicsView
 * @brief Visualizacao grafica com zoom e arrastar (pan) pelo botao direito.
 */
class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    /**
     * @brief Construtor da classe ZoomableGraphicsView.
     * 
     * @param parent Widget pai (opcional).
     */
    explicit ZoomableGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent) {
        m_isPanning = false;
    }

protected:
    /**
     * @brief Trata o evento da roda do mouse para efetuar zoom na visualizacao.
     * 
     * @param event O evento contendo dados de movimento e angulo da roda do mouse.
     */
    void wheelEvent(QWheelEvent *event) override {
        const double zoomFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            scale(zoomFactor, zoomFactor);
        } else {
            scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        }
        event->accept();
    }

    /**
     * @brief Trata o evento de clique do mouse para iniciar o deslocamento da tela (pan) com o botao direito.
     * 
     * @param event O evento do mouse contendo informacoes de clique e coordenadas.
     */
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::RightButton) {
            m_lastMousePos = event->pos();
            m_isPanning = true;
            setCursor(Qt::ClosedHandCursor);
            event->accept();
        } else {
            QGraphicsView::mousePressEvent(event);
        }
    }

    /**
     * @brief Trata o evento de movimento do mouse para deslocar a visualizacao caso o botao direito esteja pressionado.
     * 
     * @param event O evento do mouse com a nova posicao do ponteiro.
     */
    void mouseMoveEvent(QMouseEvent *event) override {
        if (m_isPanning && (event->buttons() & Qt::RightButton)) {
            QPoint delta = event->pos() - m_lastMousePos;
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x());
            verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y());
            m_lastMousePos = event->pos();
            event->accept();
        } else {
            QGraphicsView::mouseMoveEvent(event);
        }
    }

    /**
     * @brief Trata o evento de soltura do botao do mouse para desativar o deslocamento da tela.
     * 
     * @param event O evento do mouse com a soltura do botao.
     */
    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::RightButton) {
            m_isPanning = false;
            setCursor(Qt::ArrowCursor);
            event->accept();
        } else {
            QGraphicsView::mouseReleaseEvent(event);
        }
    }

private:
    bool m_isPanning;
    QPoint m_lastMousePos;
};

class MainWindow;

#include <QPainterPath>
#include <QPainterPathStroker>

class RouteLineItem : public QGraphicsLineItem
{
public:
    RouteLineItem(qreal x1, qreal y1, qreal x2, qreal y2, int fromNode, int toNode, int legIndex, MainWindow *mainWindow, QGraphicsItem *parent = nullptr);
    ~RouteLineItem() override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    MainWindow *m_mainWindow;
    int m_fromNode;
    int m_toNode;
    int m_legIndex;
    
    QGraphicsEllipseItem *m_dragHandle;
    QGraphicsLineItem *m_dragLine1;
    QGraphicsLineItem *m_dragLine2;
    bool m_isDragging;
};

class WaypointNodeItem : public QGraphicsEllipseItem
{
public:
    WaypointNodeItem(qreal x, qreal y, qreal r, int nodeId, int waypointIndex, MainWindow *mainWindow, QGraphicsItem *parent = nullptr);
    ~WaypointNodeItem() override;
    QRectF boundingRect() const override;
    QPainterPath shape() const override;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    MainWindow *m_mainWindow;
    int m_nodeId;
    int m_waypointIndex;
    
    QGraphicsEllipseItem *m_dragHandle;
    bool m_isDragging;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void onRouteSegmentDragged(int legIndex, const QPointF& releasePos);
    void onWaypointDragged(int waypointIndex, const QPointF& releasePos);

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // Ações da interface gráfica
    void on_btnCarregarMapa_clicked();
    void on_btnAdicionarNo_clicked();
    void on_btnAdicionarAresta_clicked();
    void on_btnRemoverAresta_clicked();
    void on_btnCalcular_clicked();
    void on_btnLimpar_clicked();
    void on_btnCopiarImagem_clicked();
    void on_btnAdicionarParada_clicked();
    void on_btnRemoverParada_clicked();
    void on_spinOrigem_valueChanged(int arg1);
    void on_spinDestino_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    MapData *currentMap;
    bool firstShow;
    double nodeRadius;

    // Controle do destino de cliques no mapa
    int nextClickTarget;
    int clickCount;

    // Controle de edição interativa
    bool isAddingNode;
    bool isAddingEdge;
    bool isRemovingEdge;
    std::vector<int> pendingStreetNodes;

    // Lista de paradas intermediárias
    std::vector<int> paradas;

    // Rótulo dinâmico do nó sob foco
    QGraphicsItemGroup *nodeLabel;

    // Itens visuais adicionados à cena
    std::vector<QGraphicsEllipseItem*> nodeItems;
    std::vector<QGraphicsLineItem*>    edgeItems;
    std::vector<QGraphicsItem*>        pathEdgeItems;

    // Métodos de controle do mapa
    void drawMap();
    void clearPath();
    void drawPath(const std::vector<std::vector<int>>& subPaths);
    void fitGraphInView();
    void zoomToRoute(const std::vector<int>& path);
    int  findNearestNode(const QPointF& pos);
    void highlightSelectedNodes();

    // Métodos de exibição e estilização
    void aplicarEstilo();
    void exibirCaminho(const DijkstraResult& resultado, int origem, int destino);
};

#endif // MAINWINDOW_H
