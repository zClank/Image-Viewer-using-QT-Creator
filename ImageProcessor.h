#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QImage>

class ImageProcessor {
public:
    static QImage applyBrightnessAndContrast(const QImage &image, int brightness, int contrast);
};

#endif // IMAGEPROCESSOR_H
