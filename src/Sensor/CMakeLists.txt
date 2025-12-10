target_include_directories(PicoInf PUBLIC .)
target_sources(PicoInf PRIVATE
    ADCInternal.cpp
    BH1750.cpp
    BME280.cpp
    BMP280.cpp
    DS18X.cpp
    MMC56x3.cpp
    SI7021.cpp
)