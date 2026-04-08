#include "MainWindow.h"
#include "ImageViewer.h"
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QVBoxLayout>

/**
 * @brief Constructs the MainWindow, attaching layouts and registering signals.
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentTheme(MainWindow::AppTheme::Grey) {
  // 1. Establish central working area (the Viewer)
  imageViewer = new ImageViewer(this);
  setCentralWidget(imageViewer);

  // 2. Setup Menus & Docks
  createActions();
  createMenus();
  createDockWindows();

  setWindowTitle(tr("Pixeon Image Viewer"));
  resize(1024, 768);

  toggleDarkMode(); // Ensures default state initializes to Light mode

  // 3. Coordinate internal feedback between ImageViewer math and Left Panel
  // Sliders
  connect(imageViewer, &ImageViewer::brightnessChanged, this,
          [this](int value) {
            brightnessSlider->blockSignals(
                true); // Don't trigger feedback loop inherently
            brightnessSlider->setValue(value);
            brightnessSlider->blockSignals(false);
          });

  connect(imageViewer, &ImageViewer::contrastChanged, this, [this](int value) {
    contrastSlider->blockSignals(true);
    contrastSlider->setValue(value);
    contrastSlider->blockSignals(false);
  });
}

MainWindow::~MainWindow() {}

/**
 * @brief Initializes UI actions mapped to overarching application operations.
 */
void MainWindow::createActions() {
  // The reason for using "&" within the code is to create a keyboard shortcut
  // for the action. For example, &Open Single creates a shortcut for Alt + O.

  openAct = new QAction(tr("&Open Single..."), this);
  connect(openAct, &QAction::triggered, this, &MainWindow::openImage);

  openMultipleAct = new QAction(tr("Open &Multiple..."), this);
  connect(openMultipleAct, &QAction::triggered, this,
          &MainWindow::openMultipleImages);

  saveAct = new QAction(tr("&Save Modified..."), this);
  connect(saveAct, &QAction::triggered, this, &MainWindow::saveImage);

  exitAct = new QAction(tr("E&xit"), this);
  connect(exitAct, &QAction::triggered, this, &QWidget::close);

  undoAct = new QAction(tr("&Undo Edit"), this);
  undoAct->setShortcut(QKeySequence::Undo);
  connect(undoAct, &QAction::triggered, this, &MainWindow::undoEdit);

  darkModeAct = new QAction(tr("Background &Theme Toggle"), this);
  connect(darkModeAct, &QAction::triggered, this, &MainWindow::toggleDarkMode);

  canvasModeAct = new QAction(tr("Side-by-Side &Canvas Mode"), this);
  canvasModeAct->setCheckable(true);
  connect(canvasModeAct, &QAction::triggered, this,
          &MainWindow::toggleCanvasMode);
}

/**
 * @brief Translates QActions into standard Menu Bar layout architecture.
 */
void MainWindow::createMenus() {
  QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(openAct);
  fileMenu->addAction(openMultipleAct);
  fileMenu->addAction(saveAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(undoAct);

  QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(canvasModeAct);
  viewMenu->addAction(darkModeAct);
}

/**
 * @brief Instantiates and aligns generic Qt DockWidgets storing contextual app
 * controllers.
 */
void MainWindow::createDockWindows() {
  // --- Left Dock: Image List ---
  QDockWidget *listDock = new QDockWidget(tr("Open Images"), this);
  listDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  imageListWidget = new QListWidget(listDock);
  listDock->setWidget(imageListWidget);
  addDockWidget(Qt::LeftDockWidgetArea, listDock);

  connect(imageListWidget, &QListWidget::currentItemChanged, this,
          &MainWindow::imageSelectionChanged);

  // --- Right Dock: Properties/Controls ---
  QDockWidget *propsDock = new QDockWidget(tr("Controls"), this);
  propsDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

  QWidget *propsWidget = new QWidget(propsDock);
  QVBoxLayout *propsLayout = new QVBoxLayout(propsWidget);

  // Tools Group Component Collection
  QGroupBox *toolsGroup = new QGroupBox(tr("Tools Selection"));
  QVBoxLayout *toolsLayout = new QVBoxLayout(toolsGroup);
  modeSelector = new QComboBox(toolsGroup);
  modeSelector->addItem("Pan & Zoom (Normal)");
  modeSelector->addItem("Adjust Intensity (Drag XY)");
  modeSelector->addItem("Crop Mode");
  modeSelector->addItem("Draw Mode");
  connect(modeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::selectMode);
  toolsLayout->addWidget(modeSelector);
  propsLayout->addWidget(toolsGroup);

  // Intensity Group Sliders Setup
  QGroupBox *intensityGroup = new QGroupBox(tr("Intensity Adjustment"));
  QFormLayout *formLayout = new QFormLayout(intensityGroup);

  brightnessSlider = new QSlider(Qt::Horizontal);
  brightnessSlider->setRange(-100, 100);
  brightnessSlider->setValue(0);
  connect(brightnessSlider, &QSlider::valueChanged, this,
          &MainWindow::brightnessSliderChanged);

  contrastSlider = new QSlider(Qt::Horizontal);
  contrastSlider->setRange(-100, 100);
  contrastSlider->setValue(0);
  connect(contrastSlider, &QSlider::valueChanged, this,
          &MainWindow::contrastSliderChanged);

  formLayout->addRow(new QLabel(tr("Brightness:")), brightnessSlider);
  formLayout->addRow(new QLabel(tr("Contrast:")), contrastSlider);
  propsLayout->addWidget(intensityGroup);

  propsLayout->addStretch(1);
  propsDock->setWidget(propsWidget);
  addDockWidget(Qt::RightDockWidgetArea, propsDock);

  // Sliders disabled until an image is actually opened
  brightnessSlider->setEnabled(false);
  contrastSlider->setEnabled(false);
}

/**
 * @brief Triggers OS level file dialog to insert one unique asset.
 */
void MainWindow::openImage() {
  QString defaultDir =
      QCoreApplication::applicationDirPath() + "/../Stored_Test_Images";
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Image"), defaultDir,
                                   tr("Images (*.png *.jpeg *.jpg *.bmp)"));
  if (!fileName.isEmpty()) {
    // Check if the image is already listed
    for (int i = 0; i < imageListWidget->count(); ++i) {
      if (imageListWidget->item(i)->data(Qt::UserRole).toString() == fileName) {
        imageListWidget->setCurrentRow(i);
        return; // Selection change forces load
      }
    }

    // Add to list natively
    QListWidgetItem *item = new QListWidgetItem(QFileInfo(fileName).fileName());
    item->setData(Qt::UserRole, fileName);
    imageListWidget->addItem(item);
    imageListWidget->setCurrentItem(item);
  }
}

/**
 * @brief Triggers OS level file dialog for batch imports mapped to Left List.
 */
void MainWindow::openMultipleImages() {
  QString defaultDir =
      QCoreApplication::applicationDirPath() + "/../Stored_Test_Images";
  QStringList fileNames = QFileDialog::getOpenFileNames(
      this, tr("Open Multiple Images"), defaultDir,
      tr("Images (*.png *.jpeg *.jpg *.bmp)"));

  if (fileNames.isEmpty())
    return;

  for (const QString &fileName : fileNames) {
    bool exists = false;
    for (int i = 0; i < imageListWidget->count(); ++i) {
      if (imageListWidget->item(i)->data(Qt::UserRole).toString() == fileName) {
        exists = true;
        break;
      }
    }

    if (!exists) {
      QListWidgetItem *item =
          new QListWidgetItem(QFileInfo(fileName).fileName());
      item->setData(Qt::UserRole, fileName);
      imageListWidget->addItem(item);
    }

    // Load every single image into the Viewer
    // (If in Standard Mode, it replaces the previous one, rendering only the
    // final image)
    // (If in Canvas Mode, it spawns all of them across the board)
    imageViewer->loadImage(fileName);
  }

  // Highlight the final image on the Left Dock without triggering duplicate
  // signals
  if (imageListWidget->count() > 0) {
    imageListWidget->blockSignals(true);
    imageListWidget->setCurrentRow(imageListWidget->count() - 1);
    imageListWidget->blockSignals(false);
  }
  updateWindowTitle();
}

/**
 * @brief Offloads data flushing commands back to active Scene Items.
 */
void MainWindow::saveImage() {
  if (imageViewer->hasImage()) {
    QString defaultDir =
        QCoreApplication::applicationDirPath() + "/../Stored_Test_Images";
    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save Modified Image"), defaultDir,
        tr("Images (*.png *.jpeg *.jpg *.bmp)"));
    if (!fileName.isEmpty()) {
      imageViewer->saveCurrent(fileName);
    }
  }
}

/**
 * @brief Hook responding to user navigation in the Left 'Loaded Images' Dock.
 */
void MainWindow::imageSelectionChanged(QListWidgetItem *current,
                                       QListWidgetItem *previous) {
  if (!current)
    return;

  QString fileName = current->data(Qt::UserRole).toString();
  if (imageViewer->loadImage(fileName)) {
    updateWindowTitle();
    brightnessSlider->setEnabled(true);
    contrastSlider->setEnabled(true);

    // Reset local sliders seamlessly matching standard unedited thresholds
    brightnessSlider->blockSignals(true);
    brightnessSlider->setValue(0);
    brightnessSlider->blockSignals(false);

    contrastSlider->blockSignals(true);
    contrastSlider->setValue(0);
    contrastSlider->blockSignals(false);
  }
}

/**
 * @brief Pipe transferring GUI changes forwards to target active model.
 */
void MainWindow::brightnessSliderChanged(int value) {
  imageViewer->setBrightness(value);
}

void MainWindow::contrastSliderChanged(int value) {
  imageViewer->setContrast(value);
}

void MainWindow::selectMode(int index) {
  imageViewer->setMode(static_cast<ImageViewer::Mode>(index));
}

void MainWindow::undoEdit() { imageViewer->undoLastAction(); }

void MainWindow::toggleCanvasMode(bool checked) {
  imageViewer->setCanvasMode(checked);
}

/**
 * @brief Manipulates OS color palette, triggering dark mode mechanics on
 * QWidgets, 3 options were implement, cycling each of them with a click:
 * (Light -> Dark -> Grey)
 */
void MainWindow::toggleDarkMode() {
  if (currentTheme == AppTheme::Light) {
    currentTheme = AppTheme::Dark;
  } else if (currentTheme == AppTheme::Dark) {
    currentTheme = AppTheme::Grey;
  } else {
    currentTheme = AppTheme::Light;
  }

  qApp->setStyle("Fusion");
  QPalette palette;

  if (currentTheme == AppTheme::Light) {
    // Pure Light Mode Explicit Palette (Fixes the dull grey standardPalette
    // issue)
    palette.setColor(QPalette::Window, QColor(240, 240, 240));
    palette.setColor(QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::AlternateBase, QColor(225, 225, 225));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::black);
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::Button, QColor(240, 240, 240));
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::white);
  } else if (currentTheme == AppTheme::Dark) {
    // Pure Dark Mode
    palette.setColor(QPalette::Window, QColor(33, 33, 33));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(20, 20, 20));
    palette.setColor(QPalette::AlternateBase, QColor(33, 33, 33));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(45, 45, 45));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
  } else if (currentTheme == AppTheme::Grey) {
    // Relaxed 'Lighter' Dark Mode (The previous greyish darkmode)
    palette.setColor(QPalette::Window, QColor(53, 53, 53));
    palette.setColor(QPalette::WindowText, Qt::white);
    palette.setColor(QPalette::Base, QColor(40, 40, 40));
    palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    palette.setColor(QPalette::ToolTipBase, Qt::white);
    palette.setColor(QPalette::ToolTipText, Qt::white);
    palette.setColor(QPalette::Text, Qt::white);
    palette.setColor(QPalette::Button, QColor(65, 65, 65));
    palette.setColor(QPalette::ButtonText, Qt::white);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, QColor(42, 130, 218));
    palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    palette.setColor(QPalette::HighlightedText, Qt::black);
  }

  qApp->setPalette(palette);
}

/**
 * @brief Updates the root App Name with the selected focus image title.
 */
void MainWindow::updateWindowTitle() {
  if (imageListWidget->currentItem()) {
    setWindowTitle(imageListWidget->currentItem()->text() + " - " +
                   tr("Pixeon Image Viewer"));
  }
}
