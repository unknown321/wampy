#ifndef WAMPY_MAGICK_H
#define WAMPY_MAGICK_H

// clang-format off
#include <Magick++/lib/Magick++.h>
#include "MagickWand/pixel-iterator.h"
// clang-format on

namespace MyMagick {

    void InitMagick();

    void Upscale(Magick::Image *image, Magick::Geometry &g, bool fill);

    void Crop(Magick::Image *image, Magick::RectangleInfo g);

    void FillRectangle(Magick::Image *image, Magick::RectangleInfo g, const Magick::Color &color);

    void Mask(Magick::Image *image, const std::vector<int> &pointlist);
} // namespace MyMagick

#endif