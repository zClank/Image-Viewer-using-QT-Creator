# Architecture Comparison Report

This report evaluates the current Pixeon Image Viewer project against two legacy projects submitted by past candidates, identifying architectural differences, performance benchmarks, and potential features to adopt.

## 1. Multi-Image Architecture (Tabs vs. Infinite Canvas)

- **Candidate 1 & 2:** Both candidates approached the "multi-image" requirement by using a `QTabWidget`. Whenever a user opened a new image, it spawned a new Tab at the top of the interface. Their `QGraphicsView` was restricted to showing exactly **one image at a time**.
- **Our Project:** We implemented a modern **Infinite Canvas Modality**. Using `QGraphicsScene`, users can load dozens of images into the *same view*, drag them around, and select them individually. This "moodboarding" approach is significantly more advanced and interactive than legacy Tab widgets.

## 2. Image Processing & Performance (The Big Difference)

This is where your project heavily outshines Candidate 2 in Technical Interviews!

- **Candidate 2's Approach:** 
  To alter Brightness/Contrast, Candidate 2 used a macro calling `QImage::pixelColor(x, y)` and `QImage::setPixelColor(x, y)` inside a nested `for` loop (Lines 102-114 of their `pixeoncustomview.cpp`). While this works, it is famously slow in C++ Qt because it instantiates a heavy `QColor` class object for *every single pixel* (over 8 million times for a 4K image), causing massive UI freezing.
  
- **Our Project's Approach:** 
  Inside `ImageProcessor.cpp`, we bypass the `QColor` wrapper entirely. We use `QImage::scanLine(y)` to grab raw contiguous memory arrays of integers, and mathematically manipulate the `QRgb` values directly on the CPU level. This results in processing speeds magnitudes faster than Candidate 2.

## 3. Tool Architecture (Hardcoded vs Object-Oriented)

- **Candidate 1:** Baked all manipulations via `QPainter` onto a static `QPixmap` label overlay.
- **Our Project:** Implemented an independent **Command Pattern Undo Stack** inside `CanvasImageItem`. Every single image tracks its own isolated history (colors, crop matrices, drawn lines), completely shielding the global `MainWindow` from handling pixel calculations.

---

## 🚀 Ideas We Can "Steal" to Enhance Our Project

While our core engine is undeniably stronger, the candidates did include a few simple, brilliant Quality-of-Life features that would be very easy to integrate into our project right now:

### Idea 1: Independent Object Rotation (From Candidate 1 & 2)
Both candidates added hotkeys/buttons to rotate the image 90 degrees clockwise or counter-clockwise. 
* **The Upgrade:** Since we have a Multi-Image Canvas, instead of rotating the whole screen, we can add a feature to rotate *only the currently selected image*. We can hook this up to keyboard shortcuts (e.g., `[Q]` and `[E]`).

### Idea 2: Dedicated "Empty" Placeholder Asset (From Candidate 2)
Candidate 2 utilized `QPixmap(":/Pixeon/Assets/Placeholder.jpeg")` to display a nice visual placeholder graphic rather than a blank white screen when no images are loaded. We could drop a Pixeon logo into a resource `.qrc` file and display it natively on the Canvas until an image is opened!

### Idea 3: Externalizing the Toolbar (From Candidate 2)
Candidate 2 created an exclusive `PixeonToolbar.cpp` to handle all the button logic. If our `MainWindow` begins getting too crowded with future features, we could migrate our sliders into an external dock widget component to keep the code pristine.
