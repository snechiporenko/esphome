#include "ft6336.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <vector>

namespace esphome {
namespace ft6336 {

static const char *const TAG = "ft6336";

FT6336Touchscreen *FT6336Touchscreen::instance = nullptr;

FT6336Touchscreen::FT6336Touchscreen() { instance = this; }

void FT6336TouchscreenStore::gpio_intr(FT6336TouchscreenStore *store) { store->touch = true; }

void FT6336Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up EKT2232 Touchscreen...");

  this->rts_pin_->pin_mode(gpio::FLAG_OUTPUT);
  this->rts_pin_->setup();
  hard_reset_();

  focaltech->begin(read_cb, write_cb);

  /*
  The time period of switching from active mode to monitor mode when there is no touching,
  unit position, the manual indicates that the default value is 0xA,
  and the default value is written here
  */
  focaltech->setMonitorTime(0x0A);
  /*
  Report rate in monitor mode,
  unit location, default value is 0x28, 40ms?
  */
  focaltech->setMonitorPeriod(0x28);

  this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT);
  this->interrupt_pin_->setup();

  this->store_.pin = this->interrupt_pin_->to_isr();
  this->interrupt_pin_->attach_interrupt(FT6336TouchscreenStore::gpio_intr, &this->store_,
                                         gpio::INTERRUPT_FALLING_EDGE);

  this->store_.touch = false;

  focaltech->enableINT();
}

void FT6336Touchscreen::hard_reset_() {
  this->rts_pin_->digital_write(false);
  delay(8);
  this->rts_pin_->digital_write(true);
}

void FT6336Touchscreen::active(bool is_active) {
  if (is_active) {
    focaltech->setPowerMode(FOCALTECH_PMODE_ACTIVE);
    focaltech->enableINT();
  } else {
    focaltech->disableINT();
    focaltech->setPowerMode(FOCALTECH_PMODE_DEEPSLEEP);
  }
}

void FT6336Touchscreen::loop() {
  if (!this->store_.touch) {
    return;
  }
  this->store_.touch = false;

  uint16_t x, y;
  if (!focaltech->getPoint(x, y)) {
    for (auto *listener : this->touch_listeners_)
      listener->release();
    return;
  }

  TouchPoint tp;
  switch (this->rotation_) {
    case ROTATE_0_DEGREES:
      tp.x = this->display_width_ - x;
      tp.y = this->display_height_ - y;
      break;
    case ROTATE_90_DEGREES:
      tp.x = this->display_width_ - y;
      tp.y = x;
      break;
    case ROTATE_180_DEGREES:
      tp.x = x;
      tp.y = y;
      break;
    case ROTATE_270_DEGREES:
      tp.x = y;
      tp.y = this->display_height_ - x;
      break;
  }

  this->defer([this, tp]() { this->send_touch_(tp); });
}

void FT6336Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "FT6336 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  LOG_PIN("  RTS Pin: ", this->rts_pin_);
}

uint8_t FT6336Touchscreen::read_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  return instance->read_register(reg_addr, data, len) == i2c::ErrorCode::ERROR_OK;
}

uint8_t FT6336Touchscreen::write_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len) {
  return instance->write_register(reg_addr, data, len) == i2c::ErrorCode::ERROR_OK;
}

}  // namespace ft6336
}  // namespace esphome
