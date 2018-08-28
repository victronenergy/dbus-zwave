#include "configurators/dz_aeotec_zw095.hpp"

DZConfigurator::ConfigValues DZAeotecZw095::configMap = {
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {101}}, (int32) 0b00000000000000000000011100000000}, // Watts
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {102}}, (int32) 0b00000000000001110000000000000000}, // Voltages
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {111}}, (int32) 10}, // Report watts every second
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {112}}, (int32) 10}, // Report voltages every 10 seconds
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {3}}, (int32) 1}, // Don't report if value unchanged to save some bandwidth
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {5}}, (int16) 0}, // 0W change triggers automatic report
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {6}}, (int16) 0}, // 0W change triggers automatic report
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {7}}, (int16) 0}, // 0W change triggers automatic report
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {9}}, (uint8) 0}, // 0% change triggers automatic report
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {10}}, (uint8) 0}, // 0% change triggers automatic report
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {112}, {1}, {11}}, (uint8) 0}, // 0% change triggers automatic report
};
DZConfigurator::PollIntensities DZAeotecZw095::pollingMap = {
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {1}, {0}}, 10}, // Phase 1 / Total energy
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {2}, {0}}, 1}, // Phase 1 energy
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {3}, {0}}, 1}, // Phase 2 energy
    {{{"0x0086"}, {"0x0002", "0x0102", "0x0202"}, {"0x005f"}, {50}, {4}, {0}}, 1}, // Phase 3 energy
};
