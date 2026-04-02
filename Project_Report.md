# Technical Report - Pixeon Image Viewer

**Developer:** Felipe Scherer Lacerda
**Language & Framework:** C++14 / Qt

---

## 1. Methodology Used

The solution's architecture is based on a simplified model of the **Model-View-Controller (MVC)** pattern combined with Qt Signal & Slot principles:

- **Model:** The `ImageProcessor` component handles mathematical logic and the pixel processing. It encapsulates the state of pixels into a memory copy (`processedImage`), ensuring the integrity of the original data (`originalImage`) is not deteriorated.
- **View:** `MainWindow` and the `QGraphicsView` components form the GUI instances, managing Menus, Sliders, and Docks.
- **Controller:** The interconnection between the View (`MainWindow`) and the customized click dynamics are embedded in the `ImageViewer` directive, which dispatches user commands (drag, drop, scroll) that alter the Model's values and trigger visual updates for the Screen.

### Implemented Extra Features

1. **Pan and Zoom:** A zoom feature using `QGraphicsView` scale transformations triggered by the mouse wheel.
2. **Crop Selection:** An image cropping feature using `QRubberBand` around the selected area.
3. **Pencil Drawing:** Red marking implementations intercepting mouse events and mapped to scene coordinates.
4. **Side-by-Side Canvas Mode:** Multiple images can be loaded into the scene at the same time, enabling freeform draggable repositioning of the images in any place the user desires.
5. **Undo History (Ctrl+Z):** A historical snapshot stack tracks individual item modifications, allowing `Ctrl+Z` editing rollbacks.

Throughout the project, it was used **CMake** for compiling and orchestrating it instead of QMake, the reason being in order to alignment with modern implementations and C++ market standards. Additionally, the `build_and_run.bat` utility script was provided, which automatically configures the PATH on Windows systems where Qt is maintained in the default directory `C:\Qt`, avoiding the problems caused when tools (`g++.exe`, `cmake`, `ninja`) are not properly exported in the user's terminal session.

## 2. Applied Techniques

1. **Local Bit Processing (`ImageProcessor.cpp`):**
   Instead of delegating the process solely to GPU shader scheduling or external libraries, the color lines (`r`, `g`, `b`) are retrieved directly by the `scanLine` function, iterating over arrays in `QRgb` order. This technique avoids the overhead of loops using long Qt primitives (`pixelColor`) that tend to cause degradation. Values are transformed using the linear clamp and scalar contrast multiplier threshold conversions.

2. **Graphical Interactivity Management (`QGraphicsScene`):**
   Utilizing the `QGraphicsScene` framework alongside `QGraphicsPixmapItem` allows for a standardized manipulation without needing to recreate formulas for translating the canvas offset or transformations (scale/zoom) on the screen. The `QGraphicsScene` backend manages dozens of floating `QGraphicsPixmapItem` elements concurrently. This enables the user to drag items independently, with the possibility of freely placing them side-by-side, and manipulate the layers or rotations with ease, which is an architectural advantage over other UI wrappers like `QTabWidget`, which usually restrict users to viewing one image per screen.

3. **Event-Driven Architecture (Signals and Slots):**
   Modifications to tools in the panel (`QSlider`) transfer their changes to the viewer, and the interactions within the viewer feed back into the menus, providing a two-way communication between the signals and slots. An example can be the `ImageViewer::brightnessChanged` signal being emitted and connected to the `MainWindow::brightnessSliderChanged` slot.

## 3. Difficulties Encountered

- The environment where this project took place operated with limitations due to the lack of installed compilers and direct modules from the Qt extension package (clang++ / g++). Therefore, to perform real validations during development, it was necessary to manually seek and implement the correct tools and dependencies in order to complete the task.
- Native processing via nested iterators ($O(n \times m)$) for giant matrices (4k) requires heavy sequential computation on the main CPU (Single-thread), which can lead to possible graphical instability in the UI while rendering occurs.
- A lack of familiarity with using CMake on Windows and Qt Creator hindered the development and testing process, as it was not possible to initially compile and run the project, making external tool installations necessary.

## 4. Evaluation of Results and Possible Future Improvements

The code is readable, uses modern C++ features, and is functional. The communication between the touch area (mouse drag) and the sliders was well conceived, avoiding recursive feedback (using `blockSignals(true)`). The extra tools added an extra bit of utility and value to the application.

**Future Suggestions:**

1. **Multithreading:** Isolate the work of `ImageProcessor::applyBrightnessAndContrast` into a routine under `QtConcurrent::run` to support background processing, emitting a signal to draw when concluded, thereby avoiding frame delays on the UI Thread.
2. **Shaders:** In order to achieve better performance gains based on OpenGL/Vulkan GPUs in calculations for saturation/intensity using GLSL (QOpenGLWidget).
3. **More Advanced Modes:** Addition of convolution filters (Gaussian Blur, Edge Detection, Sharpen).
4. **Undo/Redo:** Recording the commands executed by Crop/Draw, implementing the "Command Pattern" architecture combined with `QUndoStack` functionalities.

## **Task Requirements Fulfilled:**

- **Requirement 1:** C++ 14+ / Multiple Files / Formats (PNG, JPEG, BMP are opened using FileDialog with filters).
- **Requirement 2:** Native modal opening via QFileDialog.
- **Requirement 3 and 4:** Centered image and Connected Sliders.
- **Bonus 4.3:** Horizontal/vertical drag interaction linked to labels and sliders.
- **Bonus 5:** Selection via `QListWidget`.
- **Bonus 6:** Zoom (Scale Control Modifier), Pan, Rotation and Feature Selection.
- **Bonus 7:** Crop Tool, Painting Tool on screen, Switch to Dark Mode, and post-iteration saving.
