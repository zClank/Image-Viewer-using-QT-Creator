#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QSlider>

// Forward declarations
class ImageViewer;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

private slots:
  void openImage();
  void openMultipleImages();
  void saveImage();
  void imageSelectionChanged(QListWidgetItem *current,
                             QListWidgetItem *previous);
  void brightnessSliderChanged(int value);
  void contrastSliderChanged(int value);
  void toggleDarkMode();
  void undoEdit();
  void toggleCanvasMode(bool checked);

  // Tools
  void selectMode(int index);

private:
  void createActions();
  void createMenus();
  void createDockWindows();
  void updateWindowTitle();

  QListWidget *imageListWidget;
  ImageViewer *imageViewer;

  QSlider *brightnessSlider;
  QSlider *contrastSlider;
  QComboBox *modeSelector;

  QAction *openAct;
  QAction *openMultipleAct;
  QAction *saveAct;
  QAction *exitAct;
  QAction *darkModeAct;
  QAction *undoAct;
  QAction *canvasModeAct;

  bool isDarkMode;
};

#endif // MAINWINDOW_H
