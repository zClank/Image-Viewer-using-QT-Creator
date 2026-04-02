# Image Viewer using QT Creator

An image viewer app as part of a challenge from the company Pixeon. It was developed in C++14 using Qt (QGraphicsView, QImage, QListWidget). The project supports real-time manipulation, including contrast and brightness adjustments, zoom and pan, as well as additional features such as cropping, drawing on the image, and a built in option to toggle between dark and light mode.Its appearance is meant to be reminiscent of the classic Windows XP image viewer.

## Requirements

- **C++14 or higher**
- **Qt5 or Qt6** (Modules: Core, Gui, Widgets)
- **CMake 3.5+**

## Implemented Features

1. **Opening Multiple Files:** Viewing and selecting multiple images in a list on the left panel.
2. **Format Support:** Compatible with formats based on QImageReader (BMP, PNG, JPEG).
3. **Image Viewing:** Responsive display using `QGraphicsView`.
4. **Mouse and UI Interactions:**
   - **Pan & Zoom:** Mouse wheel for zoom; click and drag the canvas to move/pan.
   - **Instinctive Adjustment:** Adjust brightness (X Axis) and contrast (Y Axis) by clicking and dragging on the image.
   - **Independent Rotation:** Rotate selected images on the Canvas using the **[Q]** and **[E]** keyboard hotkeys.
   - **Property Sliders:** Two *QSliders* synchronized with the intensity metrics.
5. **Crop Mode:** Draws a rectangle using your mouse, releasing it to performs the crop around the area.
6. **Drawing:** Ability to highlight areas of the image using a red brush.
7. **Export:** Save the modified file, along with any modifications made.
8. **Dark Mode / Light Mode:** Simple transition of the application's color palette.
9. **Placeholder Graphics:** Built-in text that displays on the blank Canvas when no images are opened.

## Building the Project (Windows + Qt MinGW)

If you installed Qt via the official **Maintenance Tool** in the default directory (`C:\Qt`), you won't need to configure your PATH manually or run loosely typed commands. Simply:

1. Run **`build_and_run.bat`** (with a double-click or via terminal).
2. The script will locate the native MinGW and CMake compilers from your Qt installation and launch the program automatically.

### Manual Method (CMake)

If you use Linux, macOS, or have Qt installed in a different path, these steps should help you build the project:

```sh
mkdir build
cd build
cmake ..
cmake --build .
```
