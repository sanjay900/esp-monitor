#include "d7s.h"
#include "esp8266_wrapper.h"

void init(d7s_sensor_t *sensor) {
  sensor->_events = 0;
  i2c_init(sensor->sensor.i2c_bus, sensor->sensor.i2c_scl,
           sensor->sensor.i2c_sda, sensor->sensor.i2c_freq);
}

uint8_t read8bit(d7s_sensor_t *sensor, uint16_t reg) {
  uint8_t data[1];
  i2c_slave_read_16(sensor->sensor.i2c_bus, D7S_ADDRESS, &reg, data, 1);
  return data[0];
}

uint16_t read16bit(d7s_sensor_t *sensor, uint16_t reg) {
  uint8_t data[2];
  i2c_slave_read_16(sensor->sensor.i2c_bus, D7S_ADDRESS, &reg, data, 2);
  return (data[0] << 8) | data[1];
}


void write8bit(d7s_sensor_t *sensor, uint16_t reg, uint8_t data) {
  i2c_slave_write_16(sensor->sensor.i2c_bus, D7S_ADDRESS, &reg, &data, 1);
}

void write16bit(d7s_sensor_t *sensor, uint16_t reg, uint16_t data) {
  i2c_slave_write_16(sensor->sensor.i2c_bus, D7S_ADDRESS, &reg, (uint8_t*)&data, 2);
}

//--- STATUS ---
// return the currect state
d7s_status getState(d7s_sensor_t *sensor) {
  // read the STATE register at 0x1000
  return (d7s_status)(read8bit(sensor, 0x1000) & 0x07);
}

// return the currect state
d7s_axis_state getAxisInUse(d7s_sensor_t *sensor) {
  // read the AXIS_STATE register at 0x1001
  return (d7s_axis_state)(read8bit(sensor, 0x1001) & 0x03);
}

//--- SETTINGS ---
// change the threshold in use
void setThreshold(d7s_sensor_t *sensor, d7s_threshold threshold) {
  // check if threshold is valid
  if (threshold < 0 || threshold > 1) {
    return;
  }
  // read the CTRL register at 0x1004
  uint8_t reg = read8bit(sensor, 0x1004);
  // new register value with the threshold
  reg = (((reg >> 4) << 1) | (threshold & 0x01)) << 3;
  // update register
  write8bit(sensor, 0x1004, reg);
}

// change the axis selection mode
void setAxis(d7s_sensor_t *sensor, d7s_axis_settings axisMode) {
  // check if axisMode is valid
  if (axisMode < 0 || axisMode > 4) {
    return;
  }
  // read the CTRL register at 0x1004
  uint8_t reg = read8bit(sensor, 0x1004);
  // new register value with the threshold
  reg = (axisMode << 4) | (reg & 0x0F);
  // update register
  write8bit(sensor, 0x1004, reg);
}

//--- LASTEST DATA ---
// get the lastest SI at specified index (up to 5) [m/s]
float getLastestSI(d7s_sensor_t *sensor, uint8_t index) {
  // check if the index is in bound
  if (index > 4) {
    return 0;
  }
  // return the value
  return ((float)read16bit(sensor, 0x3008 | (index << 8))) / 1000;
}

// get the lastest PGA at specified index (up to 5) [m/s^2]
float getLastestPGA(d7s_sensor_t *sensor, uint8_t index) {
  // check if the index is in bound
  if (index > 4) {
    return 0;
  }
  // return the value
  return ((float)read16bit(sensor, 0x300A | (index << 8))) / 1000;
}

// get the lastest Temperature at specified index (up to 5) [Celsius]
float getLastestTemperature(d7s_sensor_t *sensor, uint8_t index) {
  // check if the index is in bound
  if (index > 4) {
    return 0;
  }
  // return the value
  return (float)((int16_t)read16bit(sensor, 0x3006 | (index << 8))) / 10;
}

//--- RANKED DATA ---
// get the ranked SI at specified position (up to 5) [m/s]
float getRankedSI(d7s_sensor_t *sensor, uint8_t position) {
  // check if the position is in bound
  if (position > 4) {
    return 0;
  }
  // return the value
  return ((float)read16bit(sensor, 0x3508 | (position << 8))) / 1000;
}

// get the ranked PGA at specified position (up to 5) [m/s^2]
float getRankedPGA(d7s_sensor_t *sensor, uint8_t position) {
  // check if the position is in bound
  if (position > 4) {
    return 0;
  }
  // return the value
  return ((float)read16bit(sensor, 0x350A | (position << 8))) / 1000;
}

// get the ranked Temperature at specified position (up to 5) [Celsius]
float getRankedTemperature(d7s_sensor_t *sensor, uint8_t position) {
  // check if the position is in bound
  if (position > 4) {
    return 0;
  }
  // return the value
  return (float)((int16_t)read16bit(sensor, 0x3506 | (position << 8))) / 10;
}

//--- INSTANTANEUS DATA ---
// get instantaneus SI (during an earthquake) [m/s]
float getInstantaneusSI(d7s_sensor_t *sensor) {
  // return the value
  return ((float)read16bit(sensor, 0x2000)) / 1000;
}

// get instantaneus PGA (during an earthquake) [m/s^2]
float getInstantaneusPGA(d7s_sensor_t *sensor) {
  // return the value
  return ((float)read16bit(sensor, 0x2002)) / 1000;
}

//--- CLEAR MEMORY ---
// delete both the lastest data and the ranked data
void clearEarthquakeData(d7s_sensor_t *sensor) {
  // write clear command
  write8bit(sensor, 0x1005, 0x01);
}

// delete initializzazion data
void clearInstallationData(d7s_sensor_t *sensor) {
  // write clear command
  write8bit(sensor, 0x1005, 0x08);
}

// delete offset data
void clearLastestOffsetData(d7s_sensor_t *sensor) {
  // write clear command
  write8bit(sensor, 0x1005, 0x04);
}

// delete selftest data
void clearSelftestData(d7s_sensor_t *sensor) {
  // write clear command
  write8bit(sensor, 0x1005, 0x02);
}

// delete all data
void clearAllData(d7s_sensor_t *sensor) {
  // write clear command
  write8bit(sensor, 0x1005, 0x0F);
}

//--- INITIALIZATION ---
// initialize the d7s (start the initial installation mode)
void initialize(d7s_sensor_t *sensor) {
  // write INITIAL INSTALLATION MODE command
  write8bit(sensor, 0x1003, 0x02);
}

//--- SELFTEST ---
// start autodiagnostic and resturn the result (OK/ERROR)
void selftest(d7s_sensor_t *sensor) {
  // write SELFTEST command
  write8bit(sensor, 0x1003, 0x04);
}

// return the result of self-diagnostic test (OK/ERROR)
d7s_mode_status getSelftestResult(d7s_sensor_t *sensor) {
  // return result of the selftest
  return (d7s_mode_status)((read8bit(sensor, 0x1002) & 0x07) >> 2);
}

//--- OFFSET ACQUISITION ---
// start offset acquisition and return the rersult (OK/ERROR)
void acquireOffset(d7s_sensor_t *sensor) {
  // write OFFSET ACQUISITION MODE command
  write8bit(sensor, 0x1003, 0x03);
}

// return the result of offset acquisition test (OK/ERROR)
d7s_mode_status getAcquireOffsetResult(d7s_sensor_t *sensor) {
  // return result of the offset acquisition
  return (d7s_mode_status)((read8bit(sensor, 0x1002) & 0x0F) >> 3);
}

//--- SHUTOFF/COLLAPSE EVENT ---
// after each earthquakes it's important to reset the events calling
// resetEvents() to prevent polluting the new data with the old one return true
// if the collapse condition is met (it's the sencond bit of _events)
uint8_t isInCollapse(d7s_sensor_t *sensor) {
  // updating the _events variable
  readEvents(sensor);
  // return the second bit of _events
  return (sensor->_events & 0x02) >> 1;
}

// return true if the shutoff condition is met (it's the first bit of _events)
uint8_t isInShutoff(d7s_sensor_t *sensor) {
  // updating the _events variable
  readEvents(sensor);
  // return the second bit of _events
  return sensor->_events & 0x01;
}

// reset shutoff/collapse events
void resetEvents(d7s_sensor_t *sensor) {
  // reset the EVENT register (read to zero-ing it)
  read8bit(sensor, 0x1002);
  // reset the events variable
  sensor->_events = 0;
}

//--- EARTHQUAKE EVENT ---
// return true if an earthquake is occuring
uint8_t isEarthquakeOccuring(d7s_sensor_t *sensor) {
  // if D7S is in NORMAL MODE NOT IN STANBY (after the first 4 sec to initial
  // delay) there is an earthquake
  return getState(sensor) == NORMAL_MODE_NOT_IN_STANBY;
}

//--- READY STATE ---
uint8_t isReady(d7s_sensor_t *sensor) {
  return getState(sensor) == NORMAL_MODE;
}

//--- READ EVENTS ---
// read the event (SHUTOFF/COLLAPSE) from the EVENT register
void readEvents(d7s_sensor_t *sensor) {
  // read the EVENT register at 0x1002 and obtaining only the first two bits
  uint8_t events = read8bit(sensor, 0x1002) & 0x03;
  // updating the _events variable
  sensor->_events |= events;
}



// //--- INTERRUPT HANDLER ---
// // handle the INT1 events
// void int1() {
//   // enabling interrupts
//   interrupts();
//   // if the interrupt handling is enabled
//   if (_interruptEnabled) {
//     // check what event triggered the interrupt
//     if (isInShutoff()) {
//       // if the handler is defined
//       if (_handlers[2]) {
//         _handlers[2](); // SHUTOFF_EVENT EVENT
//       }
//     } else {
//       // if the handler is defined
//       if (_handlers[3]) {
//         _handlers[3](); // COLLAPSE_EVENT EVENT
//       }
//     }
//   }
// }

// // handle the INT2 events
// void int2() {
//   // enabling interrupts
//   interrupts();
//   // if the interrupt handling is enabled
//   if (_interruptEnabled) {
//     // check what in what state the D7S is
//     if (isEarthquakeOccuring()) { // earthquake started
//       // if the handler is defined
//       if (_handlers[0]) {
//         _handlers[0](); // START_EARTHQUAKE EVENT
//       }
//     } else { // earthquake ended
//       // if the handler is defined
//       if (_handlers[1]) {
//         ((void (*)(float, float, float))_handlers[1])(
//             getLastestSI(0), getLastestPGA(0),
//             getLastestTemperature(0)); // END_EARTHQUAKE EVENT
//       }
//     }
//   }
// }
