#include "axp202.h"
#include "esphome/core/log.h"
#include <Esp.h>

namespace esphome {
namespace axp202 {

static const char *TAG = "axp202.sensor";

AXP202Component *AXP202Component::instance = nullptr;

AXP202Component::AXP202Component() { instance = this; }

void AXP202ComponentStore::gpio_intr(AXP202ComponentStore *store) { store->click = true; }

void AXP202Component::setup() {
  axp20x->begin(read_cb, write_cb);

  axp20x->setShutdownTime(AXP_POWER_OFF_TIME_4S);
  // Turn off the charging instructions, there should be no
  axp20x->setChgLEDMode(AXP20X_LED_OFF);
  // Turn off external enable
  axp20x->setPowerOutPut(AXP202_EXTEN, false);
  // axp202 allows maximum charging current of 1800mA, minimum 300mA
  axp20x->setChargeControlCur(300);

  // New features of Twatch V3
  axp20x->limitingOff();

  // Audio power domain is AXP202 LDO4
  axp20x->setLDO4Voltage(AXP202_LDO4_3300MV);

  // Backlight
  axp20x->setLDO2Voltage(3300);
  axp20x->setPowerOutPut(AXP202_LDO2, true);

  this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  this->interrupt_pin_->setup();

  this->store_.pin = this->interrupt_pin_->to_isr();
  this->interrupt_pin_->attach_interrupt(AXP202ComponentStore::gpio_intr, &this->store_, gpio::INTERRUPT_FALLING_EDGE);

  this->store_.click = false;

  axp20x->enableIRQ(AXP202_PEK_SHORTPRESS_IRQ, true);
  axp20x->clearIRQ();
}

void AXP202Component::loop() {
  if (this->store_.click) {
    this->store_.click = false;
    axp20x->readIRQ();
    /*
    ttgo->power->isVbusPlugInIRQ()
    ttgo->power->isVbusRemoveIRQ()
    */
    if (axp20x->isPEKShortPressIRQ()) {
      this->click_callback_.call();
    }
    axp20x->clearIRQ();
  }
}

void AXP202Component::add_on_click_callback(std::function<void()> &&callback) {
  this->click_callback_.add(std::move(callback));
}

void AXP202Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AXP202:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  LOG_SENSOR("  ", "Battery Level", this->batterylevel_sensor_);
}

float AXP202Component::get_setup_priority() const { return setup_priority::DATA; }

void AXP202Component::update() {
  if (this->batterylevel_sensor_ != nullptr) {
    this->batterylevel_sensor_->publish_state(axp20x->getBattPercentage());
  }
}

int AXP202Component::getBattPercentage() { return axp20x->getBattPercentage(); }

bool AXP202Component::isChargeing() { return axp20x->isChargeing(); }

bool AXP202Component::isEnable(uint8_t ch) {
  if (AXP202_LDO2)
    return axp20x->isLDO2Enable();
  return false;
}

void AXP202Component::setPowerOutPut(uint8_t ch, bool en) { axp20x->setPowerOutPut(ch, en); }

uint8_t AXP202Component::read_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  return instance->read_register(reg_addr, data, len) == i2c::ErrorCode::ERROR_OK ? 0 : -1;
}

uint8_t AXP202Component::write_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  return instance->write_register(reg_addr, data, len) == i2c::ErrorCode::ERROR_OK ? 0 : -1;
}

}  // namespace axp202
}  // namespace esphome
