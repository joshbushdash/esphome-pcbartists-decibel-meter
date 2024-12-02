#pragma once

#include "esphome.h"
#include "Wire.h"

class DecibelMeter : public PollingComponent {
 public:
  // Define the I2C pins and address
  static constexpr uint8_t I2C_SDA = 5;
  static constexpr uint8_t I2C_SCL = 18;
  static constexpr uint8_t DBM_ADDR = 0x48;

  // Define the device registers
  enum {
    DBM_REG_VERSION = 0x00,
    DBM_REG_ID3 = 0x01,
    DBM_REG_ID2 = 0x02,
    DBM_REG_ID1 = 0x03,
    DBM_REG_ID0 = 0x04,
    DBM_REG_DECIBEL = 0x0A,
    DBM_REG_MIN = 0x0B,
    DBM_REG_MAX = 0x0C,
  };

  // Constructor with polling interval
  DecibelMeter() : PollingComponent(5000) {}

  void setup() override {
    // Initialize I2C
    Wire.begin(I2C_SDA, I2C_SCL);
    ESP_LOGD("DecibelMeter", "Decibel meter initialized");

    // Optionally read version and unique ID at setup
    uint8_t version = read_register(DBM_REG_VERSION);
    ESP_LOGD("DecibelMeter", "Version: 0x%02X", version);

    uint8_t id[4];
    id[0] = read_register(DBM_REG_ID3);
    id[1] = read_register(DBM_REG_ID2);
    id[2] = read_register(DBM_REG_ID1);
    id[3] = read_register(DBM_REG_ID0);
    ESP_LOGD("DecibelMeter", "Unique ID: %02X %02X %02X %02X", id[0], id[1], id[2], id[3]);
  }

  void update() override {
    // Read decibel, min, and max registers
    uint8_t decibel = read_register(DBM_REG_DECIBEL);
    uint8_t db_min = read_register(DBM_REG_MIN);
    uint8_t db_max = read_register(DBM_REG_MAX);

    ESP_LOGI("DecibelMeter", "dB: %d, MIN: %d, MAX: %d", decibel, db_min, db_max);

    // Publish the readings to ESPHome sensors
    if (this->decibel_sensor_ != nullptr)
      this->decibel_sensor_->publish_state(decibel);
    if (this->min_sensor_ != nullptr)
      this->min_sensor_->publish_state(db_min);
    if (this->max_sensor_ != nullptr)
      this->max_sensor_->publish_state(db_max);
  }

  // Register sensors to ESPHome
  Sensor *decibel_sensor_{nullptr};
  Sensor *min_sensor_{nullptr};
  Sensor *max_sensor_{nullptr};

 protected:
  uint8_t read_register(uint8_t reg) {
    Wire.beginTransmission(DBM_ADDR);
    Wire.write(reg);
    Wire.endTransmission();
    Wire.requestFrom(DBM_ADDR, (uint8_t)1);
    delay(10);
    return Wire.available() ? Wire.read() : 255;  // Return 255 if no data
  }
};