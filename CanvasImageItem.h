#ifndef CANVASIMAGEITEM_H
#define CANVASIMAGEITEM_H

#include <QGraphicsPixmapItem>
#include <QPainter>
#include <QImage>
#include <QList>
#include <QLine>
#include <QString>

class CanvasImageItem : public QGraphicsPixmapItem {
public:
    struct State {
        QImage originalImage;
        int brightness;
        int contrast;
        QList<QLine> drawnLines;
    };

    explicit CanvasImageItem(const QString &fileName, QGraphicsItem *parent = nullptr);

    QString getFileName() const { return sourceFileName; }

    bool loadSuccess() const;
    void pushUndoState();
    bool undo();

    void applyIntensity();
    
    void setBrightness(int value);
    void setContrast(int value);
    int getBrightness() const { return currentBrightness; }
    int getContrast() const { return currentContrast; }

    void addLine(const QLine& line);
    void crop(const QRect& rect);
    void resetProcessing();

    QImage getFinalImage() const;

    QImage originalImage;
    QImage processedImage;
    QList<QLine> drawnLines;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

private:
    QString sourceFileName;
    bool loaded;
    int currentBrightness;
    int currentContrast;
    QList<State> undoStack;
};

#endif // CANVASIMAGEITEM_H
