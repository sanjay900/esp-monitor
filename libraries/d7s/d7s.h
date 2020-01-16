#include "sensor.h"
#include <stdint.h>
#pragma once
//--- ADDRESS ---
#define D7S_ADDRESS 0x55 // D7S address on the I2C bus

//--- DEBUG ----
// comment this line to disable all debug information
//#define DEBUG

// d7s state
typedef enum d7s_status {
  NORMAL_MODE = 0x00,
  NORMAL_MODE_NOT_IN_STANBY = 0x01, // earthquake in progress
  INITIAL_INSTALLATION_MODE = 0x02,
  OFFSET_ACQUISITION_MODE = 0x03,
  SELFTEST_MODE = 0x04
} d7s_status;

// d7s axis settings
typedef enum d7s_axis_settings {
  FORCE_YZ = 0x00,
  FORCE_XZ = 0x01,
  FORXE_XY = 0x02,
  AUTO_SWITCH = 0x03,
  SWITCH_AT_INSTALLATION = 0x04
} d7s_axis_settings;

// axis state
typedef enum d7s_axis_state {
  AXIS_YZ = 0x00,
  AXIS_XZ = 0x01,
  AXIS_XY = 0x02
} d7s_axis_state;

// d7s threshold settings
typedef enum d7s_threshold {
  THRESHOLD_HIGH = 0x00,
  THRESHOLD_LOW = 0x01
} d7s_threshold;

// message status (selftes, offset acquisition)
typedef enum d7s_mode_status { D7S_OK = 0, D7S_ERROR = 1 } d7s_mode_status;

// events handled externaly by the using using an handler (the d7s int1, int2
// must be connected to interrupt pin)
typedef enum d7s_interrupt_event {
  START_EARTHQUAKE = 0, // INT 2
  END_EARTHQUAKE = 1,   // INT 2
  SHUTOFF_EVENT = 2,    // INT 1
  COLLAPSE_EVENT = 3    // INT 1
} d7s_interrupt_event;

typedef struct d7s_sensor_t {
  sensor_t sensor;
  uint8_t _events;
  uint8_t _interruptEnabled;
  uint8_t int1;
  uint8_t int2;
} d7s_sensor_t;

d7s_sensor_t* d7s_init_sensor(uint8_t bus, uint8_t addr);
//--- STATUS ---
d7s_status d7s_getState(d7s_sensor_t *d7s);         // return the currect state
d7s_axis_state d7s_getAxisInUse(d7s_sensor_t *d7s); // return the current axis in
                                                // use

//--- SETTINGS ---
void d7s_setThreshold(d7s_sensor_t *d7s,
                  d7s_threshold threshold); // change the threshold in use
void d7s_setAxis(d7s_sensor_t *d7s,
             d7s_axis_settings axisMode); // change the axis selection mode

//--- LASTEST DATA ---
float d7s_getLastestSI(
    d7s_sensor_t *d7s,
    uint8_t index); // get the lastest SI at specified index (up to 5) [m/s]
float d7s_getLastestPGA(
    d7s_sensor_t *d7s,
    uint8_t index); // get the lastest PGA at specified index (up to 5) [m/s^2]
float d7s_getLastestTemperature(
    d7s_sensor_t *d7s, uint8_t index); // get the lastest Temperature at
                                       // specified index (up to 5) [Celsius]

//--- RANKED DATA ---
float d7s_getRankedSI(d7s_sensor_t *d7s,
                  uint8_t position); // get the ranked SI at specified position
                                     // (up to 5) [m/s]
float d7s_getRankedPGA(d7s_sensor_t *d7s,
                   uint8_t position); // get the ranked PGA at specified
                                      // position (up to 5) [m/s^2]
float d7s_getRankedTemperature(
    d7s_sensor_t *d7s,
    uint8_t position); // get the ranked Temperature at specified position (up
                       // to 5) [Celsius]

//--- INSTANTANEUS DATA ---
float d7s_getInstantaneusSI(
    d7s_sensor_t *d7s); // get instantaneus SI (during an earthquake) [m/s]
float d7s_getInstantaneusPGA(
    d7s_sensor_t *d7s); // get instantaneus PGA (during an earthquake) [m/s^2]

//--- CLEAR MEMORY ---
void d7s_clearEarthquakeData(
    d7s_sensor_t *d7s); // delete both the lastest data and the ranked data
void d7s_clearInstallationData(d7s_sensor_t *d7s);  // delete initializzazion data
void d7s_clearLastestOffsetData(d7s_sensor_t *d7s); // delete offset data
void d7s_clearSelftestData(d7s_sensor_t *d7s);      // delete selftest data
void d7s_clearAllData(d7s_sensor_t *d7s);           // delete all data

//--- INITIALIZATION ---
void d7s_initialize(d7s_sensor_t *d7s); // initialize the d7s (start the initial
                                    // installation mode)

//--- SELFTEST ---
void d7s_selftest(d7s_sensor_t *d7s); // trigger self-diagnostic test
d7s_mode_status d7s_getSelftestResult(
    d7s_sensor_t *d7s); // return the result of self-diagnostic test (OK/ERROR)

//--- OFFSET ACQUISITION ---
void d7s_acquireOffset(d7s_sensor_t *d7s); // trigger offset acquisition
d7s_mode_status d7s_getAcquireOffsetResult(
    d7s_sensor_t
        *d7s); // return the result of offset acquisition test (OK/ERROR)

//--- SHUTOFF/COLLAPSE EVENT ---
// after each earthquakes it's important to reset the events calling
// resetEvents() to prevent polluting the new data with the old one
uint8_t
d7s_isInCollapse(d7s_sensor_t *d7s); // return true if the collapse condition is met
                                 // (it's the sencond bit of _events)
uint8_t d7s_isInShutoff(d7s_sensor_t *d7s); // return true if the shutoff condition
                                        // is met (it's the first bit of _events)
void d7s_resetEvents(d7s_sensor_t *d7s); // reset shutoff/collapse events

//--- EARTHQUAKE EVENT ---
uint8_t d7s_isEarthquakeOccuring(
    d7s_sensor_t *d7s); // return true if an earthquake is occuring

//--- READY STATE ---
uint8_t d7s_isReady(d7s_sensor_t *d7s);
void d7s_readEvents(d7s_sensor_t *sensor);