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
#include <vector>

// Forward declaration para isolar o backend do compilador do Qt (MOC)
struct MapData;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

// Subclasse para suportar Zoom com scroll do mouse
class ZoomableGraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit ZoomableGraphicsView(QWidget *parent = nullptr) : QGraphicsView(parent) {}

protected:
    void wheelEvent(QWheelEvent *event) override {
        const double zoomFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            scale(zoomFactor, zoomFactor);
        } else {
            scale(1.0 / zoomFactor, 1.0 / zoomFactor);
        }
        event->accept();
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void showEvent(QShowEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_btnCarregarMapa_clicked();
    void on_btnCalcular_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene *scene;

    // Usar ponteiro para não exigir o include completo aqui
    MapData *currentMap;
    bool firstShow;
    double nodeRadius; // Raio calculado dos nos

    // Controle de alternancia de clique: 0 = origem, 1 = destino
    int nextClickTarget;
    int clickCount;

    // Label flutuante para exibir ID do no clicado
    QGraphicsItemGroup *nodeLabel;

    // Armazenar referencias para os itens graficos
    std::vector<QGraphicsEllipseItem*> nodeItems;
    std::vector<QGraphicsLineItem*> edgeItems;
    std::vector<QGraphicsLineItem*> pathEdgeItems;

    void drawMap();
    void clearPath();
    void drawPath(const std::vector<int>& path);
    void fitGraphInView();
    void zoomToRoute(const std::vector<int>& path);
    int findNearestNode(const QPointF &pos);
    void highlightSelectedNodes();
};
#endif // MAINWINDOW_H
