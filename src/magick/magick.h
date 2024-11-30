#ifndef WAMPY_MAGICK_H
#define WAMPY_MAGICK_H

#include <Magick++/lib/Magick++.h>
#include "MagickWand/pixel-iterator.h"

namespace MyMagick {

    void InitMagick();

    void Upscale(Magick::Image *image, Magick::Geometry &g, bool fill);

    void Crop(Magick::Image *image, Magick::RectangleInfo g);

    void FillRectangle(Magick::Image *image, Magick::RectangleInfo g, const Magick::Color &color);
}

#endif