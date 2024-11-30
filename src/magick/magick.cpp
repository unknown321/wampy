#include "magick.h"
#include "../util/util.h"

void MyMagick::InitMagick() {
    Magick::InitializeMagick("");
}

void MyMagick::Upscale(Magick::Image *image, Magick::Geometry &g, bool fill) {
    if (image == nullptr) {
        DLOG("no image\n");
        return;
    }

    if (!image->isValid()) {
        DLOG("invalid image\n");
        return;
    }

    try {
        image->compressType(MagickCore::NoCompression);
        image->filterType(MagickCore::PointFilter);
        g.aspect(true);
        g.fillArea(fill);
        image->resize(g);
    } catch (...) {
        DLOG("cannot resize\n");
        return;
    }
}

// set g.width/height to 0 to crop to the image edge
void MyMagick::Crop(Magick::Image *image, Magick::RectangleInfo g) {
    if (image == nullptr) {
        DLOG("no image\n");
        return;
    }

    if (g.width == 0) {
        g.width = image->columns() - g.x;
    }

    if (g.height == 0) {
        g.height = image->rows() - g.y;
    }

    if ((g.width + g.x) > image->columns() ||
        (g.height + g.y) > image->rows()) {
        DLOG("unexpected crop %dx%d, %d %d, %dx%d\n", g.width, g.height, g.x, g.y, image->columns(), image->rows());
        image->backgroundColor({0.0f, 0.0f, 0.0f, 1.0f});
        image->erase();
        return;
    }

    try {
        image->crop(g);
    } catch (Magick::WarningOption &warningOption) {
        DLOG("WARNING: %s\n", warningOption.what());
        image->fillColor({0.0f, 0.0f, 0.0f, 1.0f});
    }
}

void MyMagick::FillRectangle(Magick::Image *image, Magick::RectangleInfo g, const Magick::Color &color) {
    image->fillColor(color);
    image->strokeWidth(0);
    auto rectangle = Magick::DrawableRectangle(g.x, g.y, g.width, g.height);
    image->draw(rectangle);
}