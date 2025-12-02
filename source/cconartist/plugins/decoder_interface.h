#ifndef DECODER_INTERFACE_H
#define DECODER_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#    define EXPORT __declspec(dllexport)
#else
#    define EXPORT __attribute__((visibility("default")))
#endif

struct decoder_property_t
{
    const char* m_name;
    int         m_unit;
    double      m_value;
};

enum EPropertyUnit
{
    // Temperature (0–2)
    UTemperature = 0,
    UFahrenheit  = 1,
    UKelvin      = 2,

    // Pressure (6–9)
    UPressure   = 6,
    UBar        = 7,
    UPascal     = 8,
    UAtmosphere = 9,

    // Humidity (13)
    UHumidity = 13,

    // Speed & Acceleration (17–21)
    UVelocity          = 17,
    UKilometersPerHour = 18,
    UMilesPerHour      = 19,
    UAcceleration      = 20,
    UGForce            = 21,

    // Concentration (25–26)
    UPpm     = 25,
    UPercent = 26,

    // Mass (30–36)
    UKilograms  = 30,
    UGrams      = 31,
    UMilligrams = 32,
    UMicrograms = 33,
    UTons       = 34,
    UPounds     = 35,
    UOunces     = 36,

    // Length & Distance (40–46)
    UKilometers  = 40,
    UMeters      = 41,
    UCentimeters = 42,
    UMillimeters = 43,
    UMiles       = 44,
    UFeet        = 45,
    UInches      = 46,

    // Time (50–55)
    UHours        = 50,
    UMinutes      = 51,
    USeconds      = 52,
    UMilliseconds = 53,
    UMicroseconds = 54,
    UNanoseconds  = 55,

    // Volume (59–62)
    ULiters      = 59,
    UMilliliters = 60,
    UCubicMeters = 61,
    UGallons     = 62,

    // Energy & Power (66–71)
    UJoules       = 66,
    UKiloJoules   = 67,
    UWatts        = 68,
    UKiloWatts    = 69,
    UMegaWatts    = 70,
    UKiloWattHour = 71,

    // Data Size (75–79)
    UBytes     = 75,
    UKiloBytes = 76,
    UMegaBytes = 77,
    UGigaBytes = 78,
    UTeraBytes = 79,

    // Electrical (83–89)
    UVolt        = 83,
    UMilliVolt   = 84,
    UAmpere      = 85,
    UMilliAmpere = 86,
    UOhm         = 87,
    UFarad       = 88,
    UHenry       = 89,

    // Frequency (93–96)
    UHertz     = 93,
    UKiloHertz = 94,
    UMegaHertz = 95,
    UGigaHertz = 96,

    // Angle (100–101)
    UDegrees = 100,
    URadians = 101,

    // Logical / State Units (105–115)
    UOnOff               = 105,
    UOpenClose           = 106,
    UTrueFalse           = 107,
    UActiveInactive      = 108,
    UEnabledDisabled     = 109,
    UStartStop           = 110,
    UAlarmNormal         = 111,
    UFaultNormal         = 112,
    UPresentAbsent       = 113,
    UDetectedNotDetected = 114,
    UUnknown             = 115
};

typedef void* (*decoder_alloc_fn_t)(unsigned int size, unsigned char alignment);


#ifdef __cplusplus
}
#endif

#endif  // DECODER_INTERFACE_H
