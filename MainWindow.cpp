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
    : QMainWindow(parent), isDarkMode(true) {
  // 1. Establish central working area (the Viewer)
  imageViewer = new ImageViewer(this);
  setCentralWidget(imageViewer);

  // 2. Setup Menus & Docks
  createActions();
  createMenus();
  createDockWindows();

  setWindowTitle(tr("Pixeon Image Viewer"));
  resize(1024, 768);

  toggleDarkMode(); // Ensures default state follows Dark Mode scheme

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

  darkModeAct = new QAction(tr("Toggle &Dark Mode"), this);
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
  // Navigate dynamically out of the 'build' folder to point directly to user's
  // test images
  QString defaultDir =
      QCoreApplication::applicationDirPath() + "/../Stored_Test_Images";
  QString fileName =
      QFileDialog::getOpenFileName(this, tr("Open Image"), defaultDir,
                                   tr("Images (*.png *.jpeg *.jpg *.bmp)"));
  if (!fileName.isEmpty()) {
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
  for (const QString &fileName : fileNames) {
    QListWidgetItem *item = new QListWidgetItem(QFileInfo(fileName).fileName());
    item->setData(Qt::UserRole, fileName);
    imageListWidget->addItem(item);
  }
  // Select the latest implicitly assuming immediate viewing priority
  if (!fileNames.isEmpty() && imageListWidget->count() > 0) {
    imageListWidget->setCurrentRow(imageListWidget->count() - 1);
  }
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
 * @brief Manipulates OS color palette, mimicking dark mode mechanics on
 * QWidgets.
 */
void MainWindow::toggleDarkMode() {
  isDarkMode = !isDarkMode;
  if (isDarkMode) {
    qApp->setStyle("Fusion");
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    qApp->setPalette(darkPalette);
  } else {
    qApp->setPalette(style()->standardPalette());
  }
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
