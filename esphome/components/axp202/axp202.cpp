#include "axp202.h"
#include "esphome/core/log.h"
#include <Esp.h>

namespace esphome {
namespace axp202 {

static const char *TAG = "axp202.sensor";

AXP202Component *AXP202Component::instance = nullptr;

AXP202Component::AXP202Component() { instance = this; }

void AXP202Component::setup() {
  pinMode(AXP202_INT, INPUT_PULLUP);
  attachInterrupt(AXP202_INT, handleClick, FALLING);

  axp20x->begin(read_cb, write_cb);

  axp20x->setShutdownTime(AXP_POWER_OFF_TIME_4S);
  // Turn off the charging instructions, there should be no
  axp20x->setChgLEDMode(AXP20X_LED_OFF);
  // Turn off external enable
  axp20x->setPowerOutPut(AXP202_EXTEN, false);
  // axp202 allows maximum charging current of 1800mA, minimum 300mA
  axp20x->setChargeControlCur(300);

  axp20x->setLDO2Voltage(3300);

  // New features of Twatch V3
  axp20x->limitingOff();

  // Audio power domain is AXP202 LDO4
  axp20x->setPowerOutPut(AXP202_LDO4, false);
  axp20x->setLDO4Voltage(AXP202_LDO4_3300MV);
  axp20x->setPowerOutPut(AXP202_LDO4, true);

  // No use
  axp20x->setPowerOutPut(AXP202_LDO3, false);

  axp20x->setPowerOutPut(AXP202_LDO2, AXP202_ON);
}

void AXP202Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AXP202:");
  LOG_I2C_DEVICE(this);
  LOG_SENSOR("  ", "Battery Level", this->batterylevel_sensor_);
}

float AXP202Component::get_setup_priority() const { return setup_priority::DATA; }

void AXP202Component::update() {
  if (this->batterylevel_sensor_ != nullptr) {
    this->batterylevel_sensor_->publish_state(axp20x->getBattPercentage());
  }
}

int AXP202Component::getBattPercentage() { return axp20x->getBattPercentage(); }

uint8_t AXP202Component::read_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  return instance->read_register(reg_addr, data, len) == i2c::ErrorCode::ERROR_OK ? 0 : -1;
}

uint8_t AXP202Component::write_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  return instance->write_register(reg_addr, data, len) == i2c::ErrorCode::ERROR_OK ? 0 : -1;
}

void AXP202Component::handleClick() { ESP_LOGD(TAG, "CLICK"); }

}  // namespace axp202
}  // namespace esphome
