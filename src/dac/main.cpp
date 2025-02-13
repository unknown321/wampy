#include "dac.h"

int main() {
    auto m = master_volume{};
    m.FromFile("/tmp/dfdfdf/ov_1291_cew.tbl");
    for (int i = MASTER_VOLUME_VALUE_MIN; i < MASTER_VOLUME_VALUE_MAX; i++) {
        Dac::printTableValue(&m, (MASTER_VOLUME_VALUE)i, "/tmp/compare_cew/data_" + Dac::MasterVolumeValueTypeToString.at(i));
    }
}
