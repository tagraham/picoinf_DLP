#pragma once

#include <string>

#include "one_wire.h"


// Wraps https://github.com/adamboardman/pico-onewire
// Dallas DS18S20, DS18B20 and DS1822, Maxim MAX31820 and MAX31826

// Simplified implementation of one-wire sensor.
// API expects there only to be a single sensor on the one-wire bus.
// Defaults to 9-bit resolution.


class DS18X
{
public:

    // 5ms
    DS18X(uint8_t pin);

    // 5ms
    bool IsAlive();

    // 5ms
    std::string GetAddress();

    // instant (cached)
    bool SetResolution(uint8_t bits);

    //  9-bit - 145ms
    // 10-bit - 240ms
    // 11-bit - 425ms
    // 12-bit - 800ms
    double GetTemperatureCelsius();
    double GetTemperatureFahrenheit();


private:

    void ReadDeviceAddr();


private:

    uint8_t pin_ = 0;
    uint8_t resolution_ = 9;
    One_wire ow_;
    rom_address_t address_;


public:

    static void SetupShell();
};
