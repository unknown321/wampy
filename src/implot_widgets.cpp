//
// Created by unknown on 2/14/25.
//

#include "implot_widgets.h"

ImPlotPoint ImPlot::ImVec2Getter(int i, void *data) {
    ImVec2 f = *(((ImVec2 *)data) + i);
    return {(double)i, f.y};
}
