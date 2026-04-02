#include "ImageProcessor.h"
#include <algorithm>
#include <cmath>

/**
 * @brief Applies brightness and contrast adjustments to an image without
 * modifying the original.
 *
 * This function processes the image matrix directly on the CPU by iterating
 * through raw pixels using QImage::scanLine. This approach should be better
 * than the pixel-by-pixel QColor lookups.
 *
 * @param image The source image to process.
 * @param brightness Linear shift to be applied to RGB values [-100, 100].
 * @param contrast Exponent/Multiplier factor to reduce or expand RGB variance
 * [-100, 100].
 * @return QImage Which is a new image with the recalculated pixels.
 */
QImage ImageProcessor::applyBrightnessAndContrast(const QImage &image,
                                                  int brightness,
                                                  int contrast) {
  if (image.isNull())
    return image;

  // Convert to a 32-bit RGB format to ensure a better scanLine casting
  QImage result = image.convertToFormat(QImage::Format_RGB32);

  // --- Step 1: Normalize Intensity User Properties ---

  // Translate Brightness (-100 to 100) to actual 8-bit shifts (-255 to +255)
  double b = (brightness / 100.0) * 255.0;

  // Translate Contrast (-100 to 100) into a contrast focal multiplier (cFactor)
  double cFactor = 1.0;
  if (contrast > 0) {
    cFactor = 255.0 / (255.0 - (contrast / 100.0) * 254.0);
  } else if (contrast < 0) {
    cFactor = (100.0 + contrast) / 100.0;
  }

  int width = result.width();
  int height = result.height();

  // --- Step 2: Iterate and Modify Pixels ---
  // Instead of x/y getters, we grab raw pointers to row memory sizes for speed.
  for (int y = 0; y < height; ++y) {
    QRgb *scanLine = reinterpret_cast<QRgb *>(result.scanLine(y));
    for (int x = 0; x < width; ++x) {
      QRgb pixel = scanLine[x];

      int r = qRed(pixel);
      int g = qGreen(pixel);
      int bVal = qBlue(pixel);

      // Formula for Contrast: (Color - Midpoint) * Factor + Midpoint
      r = static_cast<int>((r - 128) * cFactor + 128);
      g = static_cast<int>((g - 128) * cFactor + 128);
      bVal = static_cast<int>((bVal - 128) * cFactor + 128);

      // Formula for Brightness: Scalar addition
      r += b;
      g += b;
      bVal += b;

      // Re-clamp bounds that should prevent numerical Overflow/Underflow
      r = std::clamp(r, 0, 255);
      g = std::clamp(g, 0, 255);
      bVal = std::clamp(bVal, 0, 255);

      // Writes modified channels back into the scanline buffer
      scanLine[x] = qRgba(r, g, bVal, qAlpha(pixel));
    }
  }

  return result;
}
