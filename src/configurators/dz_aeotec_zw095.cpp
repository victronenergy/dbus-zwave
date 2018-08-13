#include "configurators/dz_aeotec_zw095.hpp"

DZConfigurator::ConfigValues DZAeotecZw095::configMap = {
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {101}}, 0b00000000000001110011111100000000},
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {111}}, 60},
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {3}}, 0},
};
DZConfigurator::PollIntensities DZAeotecZw095::pollingMap = {
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {2}, {16}}, 1}, // Phase 2 voltage
};
