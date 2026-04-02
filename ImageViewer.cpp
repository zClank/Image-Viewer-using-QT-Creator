#include "ImageViewer.h"
#include <QApplication>
#include <QFont>
#include <QGraphicsTextItem>
#include <QKeyEvent>
#include <QMenu>
#include <QScrollBar>
#include <QWheelEvent>

/**
 * @brief Constructs the generic View container.
 * Sets up a scene, renders parameters (like Antialiasing), and registers
 * interaction handlers.
 */
ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent), currentMode(PanZoom), isCanvasMode(false),
      isDragging(false), stateSnapshotTaken(false) {
  scene = new QGraphicsScene(this);
  setScene(scene);

  // Configurations for the viewport
  setRenderHint(QPainter::Antialiasing, true);
  setRenderHint(QPainter::SmoothPixmapTransform, true);
  setDragMode(QGraphicsView::ScrollHandDrag);

  rubberBand = new QRubberBand(QRubberBand::Rectangle, this);

  // Initializes Placeholder Text
  placeholderText = scene->addText(
      "No images loaded.\nClick \"Open Image\" to begin or drag files here.");
  placeholderText->setDefaultTextColor(Qt::gray);
  QFont font = placeholderText->font();
  font.setPointSize(16);
  placeholderText->setFont(font);

  // Tracks when user clicks on different images in the Canvas
  connect(scene, &QGraphicsScene::selectionChanged, this,
          &ImageViewer::onSceneSelectionChanged);
}

/**
 * @brief Identifies which CanvasImageItem is currently highlighted/targeted.
 * @return A casted pointer to the active CanvasImageItem, or nullptr if none.
 */
CanvasImageItem *ImageViewer::getActiveItem() const {
  if (scene->selectedItems().isEmpty()) {
    // Fallback: If there's only 1 item in the scene, prioritize it
    if (canvasItems.size() == 1)
      return canvasItems.first();
    return nullptr;
  }
  return dynamic_cast<CanvasImageItem *>(scene->selectedItems().first());
}

/**
 * @brief Validates if the scene contains at least one loaded image.
 */
bool ImageViewer::hasImage() const { return !canvasItems.isEmpty(); }

/**
 * @brief Toggles between Single Image replacement and Canvas Mode.
 */
void ImageViewer::setCanvasMode(bool canvasMode) { isCanvasMode = canvasMode; }

/**
 * @brief Core loading function handling local disk I/O and scene management.
 * Depending on `isCanvasMode`, it either replaces the scene or appends new
 * draggable items.
 * @return True if parsing succeeded.
 */
bool ImageViewer::loadImage(const QString &fileName) {
  CanvasImageItem *item = new CanvasImageItem(fileName);
  if (!item->loadSuccess()) {
    delete item;
    return false;
  }

  if (!isCanvasMode) {
    // Standard Mode: Purge old imagery
    for (auto *i : canvasItems) {
      scene->removeItem(i);
      delete i;
    }
    canvasItems.clear();
  } else {
    // Canvas Mode: Cascade new items slightly downward so they don't overlap
    // perfectly
    item->setPos(canvasItems.size() * 20, canvasItems.size() * 20);
  }

  canvasItems.append(item);
  scene->addItem(item);

  // Visually assign focus to only the newly loaded item
  scene->clearSelection();
  item->setSelected(true);

  if (!isCanvasMode) {
    scene->setSceneRect(item->boundingRect());
    fitInView(item, Qt::KeepAspectRatio);
  } else {
    scene->setSceneRect(scene->itemsBoundingRect());
  }

  updatePlaceholder();

  return true;
}

/**
 * @brief Toggles visibility of the empty state placeholder.
 */
void ImageViewer::updatePlaceholder() {
  if (placeholderText) {
    placeholderText->setVisible(canvasItems.isEmpty());
  }
}

/**
 * @brief Extracts the final composed pixmap of the active selection and saves
 * it.
 */
bool ImageViewer::saveCurrent(const QString &fileName) {
  CanvasImageItem *active = getActiveItem();
  if (active) {
    return active->getFinalImage().save(fileName);
  }
  return false;
}

/**
 * @brief Modifies the interaction state machine (Pan, Crop, Adjust, Draw).
 * Overrides underlying QGraphicsView native dragging policies to prevent
 * conflicts.
 */
void ImageViewer::setMode(Mode mode) {
  currentMode = mode;
  if (currentMode == PanZoom) {
    setDragMode(QGraphicsView::ScrollHandDrag);
  } else {
    // Disable viewport drag so we can intercept mouse movement for tools
    // (crop/draw)
    setDragMode(QGraphicsView::NoDrag);
  }
}

/**
 * @brief Updates UI slider changes of Brightness to the active item.
 */
void ImageViewer::setBrightness(int value) {
  if (CanvasImageItem *active = getActiveItem()) {
    active->setBrightness(value);
  }
}

/**
 * @brief Updates UI slider changes of Contrast to the active item.
 */
void ImageViewer::setContrast(int value) {
  if (CanvasImageItem *active = getActiveItem()) {
    active->setContrast(value);
  }
}

/**
 * @brief Triggers Ctrl+Z (Undo) operations on the CanvasItem and syncs
 * side-panel UI.
 */
bool ImageViewer::undoLastAction() {
  CanvasImageItem *active = getActiveItem();
  if (active && active->undo()) {
    // Broadcast the reinstated mathematical constraints to left panel dials.
    emit brightnessChanged(active->getBrightness());
    emit contrastChanged(active->getContrast());
    return true;
  }
  return false;
}

/**
 * @brief Synchronizes property sliders with newly focused CanvasImageItem.
 */
void ImageViewer::onSceneSelectionChanged() {
  emit selectionChanged();
  CanvasImageItem *active = getActiveItem();
  if (active) {
    emit brightnessChanged(active->getBrightness());
    emit contrastChanged(active->getContrast());
  }
}

// --- Interaction Handling ---

/**
 * @brief Implements standard Zoom scaling via Mouse Wheel (or Ctrl+Wheel).
 */
void ImageViewer::wheelEvent(QWheelEvent *event) {
  if (currentMode == PanZoom || event->modifiers() == Qt::ControlModifier) {
    const double scaleFactor = 1.15;
    if (event->angleDelta().y() > 0) {
      scale(scaleFactor, scaleFactor);
    } else {
      scale(1.0 / scaleFactor, 1.0 / scaleFactor);
    }
  } else {
    QGraphicsView::wheelEvent(event);
  }
}

/**
 * @brief Handles keyboard shortcuts for rotating the targeted item
 * independently.
 */
void ImageViewer::keyPressEvent(QKeyEvent *event) {
  if (CanvasImageItem *active = getActiveItem()) {
    if (event->key() == Qt::Key_Q) {
      // Re-center transformation origin so it rotates correctly on its own axis
      active->setTransformOriginPoint(active->boundingRect().center());
      active->setRotation(active->rotation() - 90);
    } else if (event->key() == Qt::Key_E) {
      active->setTransformOriginPoint(active->boundingRect().center());
      active->setRotation(active->rotation() + 90);
    }
  }

  // Additional Ctrl+Z (Undo) functionality mapped to Canvas level
  if (event->key() == Qt::Key_Z && (event->modifiers() & Qt::ControlModifier)) {
    undoLastAction();
  }

  QGraphicsView::keyPressEvent(event);
}

/**
 * @brief Coordinates mouse clicks. Takes undo snapshots to memorize its state
 * before users modify them.
 */
void ImageViewer::mousePressEvent(QMouseEvent *event) {
  if (!hasImage())
    return QGraphicsView::mousePressEvent(event);

  CanvasImageItem *active = getActiveItem();

  if (currentMode == Crop && active) {
    rubberBandOrigin = event->pos();
    rubberBand->setGeometry(QRect(rubberBandOrigin, QSize()));
    rubberBand->show();
  } else if ((currentMode == AdjustIntensity || currentMode == Draw) &&
             active) {
    // Backup the intact state before modifying
    active->pushUndoState();
    stateSnapshotTaken = true;
  }

  lastMousePos = event->pos();
  isDragging = true;

  // Bubble event upwards so QGraphicsScene can select the item being clicked
  QGraphicsView::mousePressEvent(event);
}

/**
 * @brief Processes continuous dragged states. Either pans the Scene or tracks
 * Pen coordinates.
 */
void ImageViewer::mouseMoveEvent(QMouseEvent *event) {
  if (!hasImage() || !isDragging)
    return QGraphicsView::mouseMoveEvent(event);

  CanvasImageItem *active = getActiveItem();
  if (!active) {
    QGraphicsView::mouseMoveEvent(event);
    return;
  }

  QPoint delta = event->pos() - lastMousePos;

  if (currentMode == AdjustIntensity) {
    // Pseudo logic: x-axis movement drives Brightness, y-axis drives Contrast
    int newB = qBound(-100, active->getBrightness() + delta.x() / 2, 100);
    int newC = qBound(-100, active->getContrast() - delta.y() / 2, 100);

    lastMousePos = event->pos();

    if (newB != active->getBrightness() || newC != active->getContrast()) {
      active->setBrightness(newB);
      active->setContrast(newC);
      // Reflect visual slide tweaks into panel Docks automatically
      emit brightnessChanged(newB);
      emit contrastChanged(newC);
    }
  } else if (currentMode == Crop) {
    // Expand the visual Crop region using normalized rectangle metrics
    rubberBand->setGeometry(QRect(rubberBandOrigin, event->pos()).normalized());
  } else if (currentMode == Draw) {
    // De-rotate/De-scale the View coordinates back to raw Image coordinates for
    // persistence
    QPointF p1 = active->mapFromScene(mapToScene(lastMousePos));
    QPointF p2 = active->mapFromScene(mapToScene(event->pos()));
    active->addLine(QLine(p1.toPoint(), p2.toPoint()));
    lastMousePos = event->pos();
  } else {
    QGraphicsView::mouseMoveEvent(event);
  }
}

/**
 * @brief Discards temporal memory contexts when user interaction halts.
 */
void ImageViewer::mouseReleaseEvent(QMouseEvent *event) {
  if (!hasImage())
    return QGraphicsView::mouseReleaseEvent(event);

  isDragging = false;
  stateSnapshotTaken = false;

  if (currentMode == Crop && rubberBand->isVisible()) {
    rubberBand->hide();
    finalizeCrop();
  }

  QGraphicsView::mouseReleaseEvent(event);
}

/**
 * @brief Translates visual Rubberband Box dimensions backwards through the
 * coordinate pipeline to cut Matrix traits.
 */
void ImageViewer::finalizeCrop() {
  CanvasImageItem *active = getActiveItem();
  if (!active)
    return;

  QRect selection = rubberBand->geometry();
  // 1. Map widget viewport rectangle to World Scene parameters
  QRectF sceneRectF = mapToScene(selection).boundingRect();
  // 2. Map World Scene coordinates locally relative into underlying Item Origin
  QRectF itemRectF = active->mapFromScene(sceneRectF).boundingRect();

  active->pushUndoState(); // Store before crop execution
  active->crop(itemRectF.toRect());
}

/**
 * @brief Standard UI geometry adaptation listener.
 */
void ImageViewer::resizeEvent(QResizeEvent *event) {
  QGraphicsView::resizeEvent(event);
}
