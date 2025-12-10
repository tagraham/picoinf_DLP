#include "DS18X.h"
#include "Pin.h"
#include "Shell.h"
#include "Timeline.h"
#include "Utl.h"

#include <string.h>

#include <string>
using namespace std;

#include "StrictMode.h"


///////////////////////////////////////////////////////////////////////////////
// Public
///////////////////////////////////////////////////////////////////////////////


DS18X::DS18X(uint8_t pin)
: pin_(pin)
, ow_(pin_)
{
    ow_.init();

    ReadDeviceAddr();
}

bool DS18X::IsAlive()
{
    return GetAddress() != "0000000000000000";
}

string DS18X::GetAddress()
{
    ReadDeviceAddr();

    string addr;

    for (int i = 0; i < (int)sizeof(address_.rom); i++)
    {
        addr += ToHex(address_.rom[i], false);
    }

    return addr;
}

bool DS18X::SetResolution(uint8_t bits)
{
    bool retVal = false;

    if (9 <= bits && bits <= 12)
    {
        retVal = true;

        resolution_ = bits;
    }

    return retVal;
}

double DS18X::GetTemperatureCelsius()
{
    double tempC = -1000;

    if (IsAlive())
    {
        // always assert resolution beforehand to ensure the device is in a
        // known default state, which changes on startup to default to 12-bit
        ow_.set_resolution(address_, resolution_);
        
        // trigger conversion and wait associated delay based on resolution
        ow_.convert_temperature(address_, true, false);
        
        // read back temperature
        tempC = ow_.temperature(address_);
    }

    return tempC;
}

double DS18X::GetTemperatureFahrenheit()
{
    double tempC = GetTemperatureCelsius();

    return (tempC * 9.0 / 5.0) + 32.0;
}


///////////////////////////////////////////////////////////////////////////////
// Private
///////////////////////////////////////////////////////////////////////////////

void DS18X::ReadDeviceAddr()
{
    memset((void *)&address_.rom, 0, sizeof(address_.rom));
    ow_.single_device_read_rom(address_);
}


///////////////////////////////////////////////////////////////////////////////
// Static
///////////////////////////////////////////////////////////////////////////////

void DS18X::SetupShell()
{
    Timeline::Global().Event("DS18X::SetupShell");

    static DS18X *sensor = nullptr;
    static uint8_t pin = 0;

    static auto GetSensor = []{
        if (sensor == nullptr)
        {
            Timeline::Measure([](Timeline &t){
                sensor = new DS18X(pin);
            }, "Constructor");
        }

        return sensor;
    };

    static auto UpdateSensor = []{
        if (sensor != nullptr)
        {
            delete sensor;
            sensor = nullptr;
        }

        GetSensor();
    };

    Shell::AddCommand("sensor.ds18x.set.pin", [](vector<string> argList){
        pin = (uint8_t)atoi(argList[0].c_str());
        Log("Pin set to ", pin);
        UpdateSensor();
    }, { .argCount = 1, .help = "set bus pin" });

    Shell::AddCommand("sensor.ds18x.get.alive", [](vector<string> argList){
        GetSensor();
        bool alive = sensor->IsAlive();
        Log("Alive: ", alive);
    }, { .argCount = 0, .help = "get sensor aliveness" });

    Shell::AddCommand("sensor.ds18x.get.addr", [](vector<string> argList){
        GetSensor();
        string addr = sensor->GetAddress();
        Log("Address: ", addr);
    }, { .argCount = 0, .help = "get sensor address" });

    Shell::AddCommand("sensor.ds18x.set.res", [](vector<string> argList){
        GetSensor();
        bool worked = sensor->SetResolution((uint8_t)atoi(argList[0].c_str()));
        Log("Worked: ", worked);
    }, { .argCount = 1, .help = "set addr resolution bits [9-12]" });

    Shell::AddCommand("sensor.ds18x.get.tempc", [](vector<string> argList){
        GetSensor();
        double tempC = sensor->GetTemperatureCelsius();
        Log("TempC: ", tempC);
    }, { .argCount = 0, .help = "get tempc" });

    Shell::AddCommand("sensor.ds18x.get.tempf", [](vector<string> argList){
        GetSensor();
        double tempF = sensor->GetTemperatureFahrenheit();
        Log("TempF: ", tempF);
    }, { .argCount = 0, .help = "get tempf" });

    Shell::AddCommand("sensor.ds18x.get.all", [](vector<string> argList){
        GetSensor();

        Timeline::Measure([](Timeline &t){
            bool alive = sensor->IsAlive();
            t.Event("alive");

            string addr = sensor->GetAddress();
            t.Event("addr");

            bool ok9 = sensor->SetResolution(9);
            t.Event("res9");
            double tempF9 = sensor->GetTemperatureFahrenheit();
            t.Event("temp9");

            bool ok10 = sensor->SetResolution(10);
            t.Event("res10");
            double tempF10 = sensor->GetTemperatureFahrenheit();
            t.Event("temp10");

            bool ok11 = sensor->SetResolution(11);
            t.Event("res11");
            double tempF11 = sensor->GetTemperatureFahrenheit();
            t.Event("temp11");

            bool ok12 = sensor->SetResolution(12);
            t.Event("res12");
            double tempF12 = sensor->GetTemperatureFahrenheit();
            t.Event("temp12");

            Log("Alive           : ", alive);
            Log("Addr            : ", addr);
            Log("TempF @  9 bits : ", tempF9,  " (", (ok9  ? "ok" : "fail"), ")");
            Log("TempF @ 10 bits : ", tempF10, " (", (ok10 ? "ok" : "fail"), ")");
            Log("TempF @ 11 bits : ", tempF11, " (", (ok11 ? "ok" : "fail"), ")");
            Log("TempF @ 12 bits : ", tempF12, " (", (ok12 ? "ok" : "fail"), ")");
        });
    }, { .argCount = 0, .help = "exercise and time api" });
}
