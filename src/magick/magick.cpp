#include "magick.h"
#include "../util/util.h"
#include "MagickCore/draw.h"

void MyMagick::InitMagick() { Magick::InitializeMagick(""); }

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

    if (g.height == 0 || g.width == 0) {
        DLOG("nothing to crop\n");
        return;
    }

    if (image->depth() != 8) {
        image->depth(8);
    }

    if ((g.width + g.x) > image->columns() || (g.height + g.y) > image->rows()) {
        DLOG("unexpected crop %zux%zu, at %zd:%zd, image size %zux%zu\n", g.width, g.height, g.x, g.y, image->columns(), image->rows());
        auto transparent = Magick::Color{0.0f, 0.0f, 0.0f, 0.0f}; // alpha 0->255
        image->backgroundColor(transparent);
        image->erase();
        return;
    }

    try {
        image->crop(g);
    } catch (Magick::WarningOption &warningOption) {
        DLOG("WARNING: %s\n", warningOption.what());
        auto transparent = Magick::Color{0.0f, 0.0f, 0.0f, 0.0f}; // alpha 0->255
        image->backgroundColor(transparent);
        image->erase();
    }
}

void MyMagick::Mask(Magick::Image *image, const std::vector<int> &pointlist) {
    if (pointlist.size() % 2 != 0) {
        DLOG("uneven point list\n");
        return;
    }

    if (pointlist.empty()) {
        return;
    }

    Magick::CoordinateList coords{};
    Magick::Coordinate c;
    for (int i = 0; i < pointlist.size(); i = i + 2) {
        c.x((double)pointlist.at(i));
        c.y((double)pointlist.at(i + 1));
        coords.push_back(c);
    }

    Magick::Image mask;
    mask.size("275x116");
    mask.fillColor("#ffffff");
    mask.backgroundColor("#000000");
    mask.erase();
    mask.strokeWidth(0);
    //    mask.strokeColor("#ff0000");

    auto d = Magick::DrawablePolygon(coords);

    mask.draw(d);
    mask.write("bmp:/tmp/out.bmp");

    image->composite(mask, "+0+0", MagickCore::MultiplyCompositeOp);
}

void MyMagick::FillRectangle(Magick::Image *image, Magick::RectangleInfo g, const Magick::Color &color) {
    image->fillColor(color);
    image->strokeWidth(0);
    auto rectangle = Magick::DrawableRectangle(g.x, g.y, g.width, g.height);
    //    DLOG("color is %s, alpha %f\n", std::string(color).c_str(), color.quantumAlpha());
    image->draw(rectangle);
}