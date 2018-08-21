#include "configurators/dz_aeotec_zw095.hpp"

DZConfigurator::ConfigValues DZAeotecZw095::configMap = {
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {101}}, 0b00000000000001110000011100000000}, // Voltages and Watts. kWh we get through polling
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {111}}, 120}, // Report every second
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {3}}, 0}, // Always report, not only when changes occur
};
DZConfigurator::PollIntensities DZAeotecZw095::pollingMap = {
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {1}, {0}}, 1}, // Phase 1? energy
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {2}, {0}}, 1}, // Phase 2? energy
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {3}, {0}}, 1}, // Phase 3? energy
};
