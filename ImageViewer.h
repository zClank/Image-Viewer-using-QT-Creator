#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QRubberBand>
#include <QPoint>
#include <QList>
#include <QKeyEvent>
#include "CanvasImageItem.h"

class ImageViewer : public QGraphicsView {
    Q_OBJECT

public:
    enum Mode { PanZoom, AdjustIntensity, Crop, Draw };

    explicit ImageViewer(QWidget *parent = nullptr);

    bool loadImage(const QString &fileName);
    bool saveCurrent(const QString &fileName);
    bool hasImage() const;

    void setMode(Mode mode);
    void setBrightness(int value);
    void setContrast(int value);
    
    void setCanvasMode(bool canvasMode);
    bool undoLastAction();
    
signals:
    void brightnessChanged(int value);
    void contrastChanged(int value);
    void selectionChanged();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onSceneSelectionChanged();

private:
    void finalizeCrop();
    CanvasImageItem* getActiveItem() const;
    void updatePlaceholder();
    
    QGraphicsScene *scene;
    QList<CanvasImageItem*> canvasItems;
    QGraphicsTextItem *placeholderText;

    Mode currentMode;
    bool isCanvasMode;

    // Interaction state
    QPoint lastMousePos;
    QPoint rubberBandOrigin;
    QRubberBand *rubberBand;
    bool isDragging;
    bool stateSnapshotTaken; // Ensure we only push state once per drag
};

#endif // IMAGEVIEWER_H
