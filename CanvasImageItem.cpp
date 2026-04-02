#include "CanvasImageItem.h"
#include "ImageProcessor.h"
#include <QStyleOptionGraphicsItem>

/**
 * @brief Constructs a new Canvas Item, tracking its own image state.
 * @param fileName Absolute or relative path to the image loaded.
 * @param parent Optional Qt graphics parent.
 */
CanvasImageItem::CanvasImageItem(const QString &fileName, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent), loaded(false), currentBrightness(0),
      currentContrast(0) {
  // Enable ability for users to select this image and move it around the scene
  setFlags(QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemIsMovable);

  if (originalImage.load(fileName)) {
    loaded = true;
    processedImage = originalImage;
    setPixmap(QPixmap::fromImage(processedImage));
  }
}

/**
 * @brief Checks if the initialization successfully loaded an image file.
 */
bool CanvasImageItem::loadSuccess() const { return loaded; }

/**
 * @brief Snapshots the image traits and inserts them onto the history stack.
 * Called right before executing an action (sliders dragging stopped, crop
 * confirmed).
 */
void CanvasImageItem::pushUndoState() {
  State s;
  s.originalImage = originalImage;
  s.brightness = currentBrightness;
  s.contrast = currentContrast;
  s.drawnLines = drawnLines;
  undoStack.push_back(s);
}

/**
 * @brief Reverts the properties to the previous State saved in the
 * history stack.
 * @return true if popped successfully, false if the history is empty.
 */
bool CanvasImageItem::undo() {
  if (undoStack.isEmpty())
    return false;

  State s = undoStack.takeLast();
  originalImage = s.originalImage;
  currentBrightness = s.brightness;
  currentContrast = s.contrast;
  drawnLines = s.drawnLines;

  // Reproject the image visually to reflect the returned state
  applyIntensity();
  return true;
}

/**
 * @brief Checks current intensities and remakes the viewable Pixmap.
 */
void CanvasImageItem::applyIntensity() {
  if (!originalImage.isNull()) {
    processedImage = ImageProcessor::applyBrightnessAndContrast(
        originalImage, currentBrightness, currentContrast);
    setPixmap(QPixmap::fromImage(processedImage));
    update(); // Forces Qt to call paint() to redraw strokes / overlay
  }
}

// Resets/updates value and re-processes matrix
void CanvasImageItem::setBrightness(int value) {
  if (currentBrightness != value) {
    currentBrightness = value;
    applyIntensity();
  }
}

void CanvasImageItem::setContrast(int value) {
  if (currentContrast != value) {
    currentContrast = value;
    applyIntensity();
  }
}

/**
 * @brief Resets properties completely while saving original image.
 */
void CanvasImageItem::resetProcessing() {
  currentBrightness = 0;
  currentContrast = 0;
  applyIntensity();
}

/**
 * @brief Adds a new stroke trajectory line when user annotates the image.
 */
void CanvasImageItem::addLine(const QLine &line) {
  drawnLines.append(line);
  update(); // Calls paint() visually overlapping red stroke map over image
}

/**
 * @brief Crops the current image down to a bounding box.
 * Discards old drawn markers since resolution boundaries change.
 */
void CanvasImageItem::crop(const QRect &rect) {
  if (rect.isValid() && rect.width() > 10 && rect.height() > 10) {
    originalImage = originalImage.copy(rect);
    drawnLines.clear();
    applyIntensity();
  }
}

/**
 * @brief Combines calculated Brightness/Contrast outputs and Red pencil layers
 * so it can be fully exported and saved onto user's hard drive.
 */
QImage CanvasImageItem::getFinalImage() const {
  if (!drawnLines.isEmpty() || currentBrightness != 0 || currentContrast != 0) {
    QImage fullProcess = processedImage.copy();
    QPainter p(&fullProcess);
    p.setPen(QPen(Qt::red, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    for (const auto &line : drawnLines) {
      p.drawLine(line);
    }
    return fullProcess;
  }
  return processedImage;
}

/**
 * @brief Override rendering event for the image logic.
 * Handled by Qt UI event loop when the canvas region needs repainting.
 */
void CanvasImageItem::paint(QPainter *painter,
                            const QStyleOptionGraphicsItem *option,
                            QWidget *widget) {
  // 1. Draw the pixel image first via class implementation
  QGraphicsPixmapItem::paint(painter, option, widget);

  // 2. Overlay any manual user annotations (stored as generic geometric lines)
  if (!drawnLines.isEmpty()) {
    painter->save();
    QPen pen(Qt::red, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    for (const QLine &l : drawnLines) {
      painter->drawLine(l);
    }
    painter->restore();
  }

  // 3. Highlight Canvas Image bounding edges if it is the selected target
  if (isSelected()) {
    painter->save();
    QPen selectPen(QColor(0, 120, 215, 200), 4, Qt::SolidLine);
    painter->setPen(selectPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(boundingRect());
    painter->restore();
  }
}
