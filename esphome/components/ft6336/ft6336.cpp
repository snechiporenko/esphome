#include "ft6336.h"
#include "esphome/core/helpers.h"
#include "esphome/core/log.h"

#include <vector>

namespace esphome {
namespace ft6336 {

static const char *const TAG = "ft6336";

#define FT5206_VENDID (0x11)
#define FT6206_CHIPID (0x06)
#define FT6236_CHIPID (0x36)
#define FT6236U_CHIPID (0x64)
#define FT5206U_CHIPID (0x64)

#define FOCALTECH_REGISTER_MODE (0x00)
#define FOCALTECH_REGISTER_GEST (0x01)
#define FOCALTECH_REGISTER_STATUS (0x02)
#define FOCALTECH_REGISTER_TOUCH1_XH (0x03)
#define FOCALTECH_REGISTER_TOUCH1_XL (0x04)
#define FOCALTECH_REGISTER_TOUCH1_YH (0x05)
#define FOCALTECH_REGISTER_TOUCH1_YL (0x06)
#define FOCALTECH_REGISTER_THRESHHOLD (0x80)
#define FOCALTECH_REGISTER_CONTROL (0x86)
#define FOCALTECH_REGISTER_MONITORTIME (0x87)
#define FOCALTECH_REGISTER_ACTIVEPERIOD (0x88)
#define FOCALTECH_REGISTER_MONITORPERIOD (0x89)

#define FOCALTECH_REGISTER_LIB_VERSIONH (0xA1)
#define FOCALTECH_REGISTER_LIB_VERSIONL (0xA2)
#define FOCALTECH_REGISTER_INT_STATUS (0xA4)
#define FOCALTECH_REGISTER_POWER_MODE (0xA5)
#define FOCALTECH_REGISTER_VENDOR_ID (0xA3)
#define FOCALTECH_REGISTER_VENDOR1_ID (0xA8)
#define FOCALTECH_REGISTER_ERROR_STATUS (0xA9)

void FT6336TouchscreenStore::gpio_intr(FT6336TouchscreenStore *store) { store->touch = true; }

void FT6336Touchscreen::setup() {
  ESP_LOGCONFIG(TAG, "Setting up EKT2232 Touchscreen...");
  this->interrupt_pin_->pin_mode(gpio::FLAG_INPUT | gpio::FLAG_PULLUP);
  this->interrupt_pin_->setup();

  this->store_.pin = this->interrupt_pin_->to_isr();
  this->interrupt_pin_->attach_interrupt(FT6336TouchscreenStore::gpio_intr, &this->store_,
                                         gpio::INTERRUPT_FALLING_EDGE);

  this->rts_pin_->setup();

  this->hard_reset_();
  this->write_byte(FOCALTECH_REGISTER_INT_STATUS, 1);
  /*
  The time period of switching from active mode to monitor mode when there is no touching,
  unit position, the manual indicates that the default value is 0xA,
  and the default value is written here
  */
  this->write_byte(FOCALTECH_REGISTER_MONITORTIME, 0x0A);
  /*
  Report rate in monitor mode,
  unit location, default value is 0x28, 40ms?
  */
  this->write_byte(FOCALTECH_REGISTER_MONITORPERIOD, 0x28);

  this->store_.touch = false;
}

void FT6336Touchscreen::setPowerMode(PowerMode_t m) { this->write_byte(FOCALTECH_REGISTER_POWER_MODE, m); }

void FT6336Touchscreen::setTheshold(uint8_t value) { this->write_byte(FOCALTECH_REGISTER_THRESHHOLD, value); }

bool FT6336Touchscreen::getPoint(uint16_t &x, uint16_t &y) {
  uint8_t buffer[5];
  if (this->read_register(FOCALTECH_REGISTER_STATUS, buffer, 5)) {
    if (buffer[0] == 0 || buffer[0] > 2) {
      return false;
    }
    // event = (EventFlag_t)(buffer[1] & 0xC0);
    x = (buffer[1] & 0x0F) << 8 | buffer[2];
    y = (buffer[3] & 0x0F) << 8 | buffer[4];

    return true;
  }
  return false;
}

void FT6336Touchscreen::loop() {
  if (!this->store_.touch)
    return;
  this->store_.touch = false;

  uint16_t x, y;

  if (this->getPoint(x, y)) {
    TouchPoint tp;
    switch (this->rotation_) {
      case ROTATE_0_DEGREES:
        tp.y = this->display_height_ - y;
        tp.x = x;
        break;
      case ROTATE_90_DEGREES:
        tp.x = this->display_height_ - y;
        tp.y = this->display_width_ - x;
        break;
      case ROTATE_180_DEGREES:
        tp.y = y;
        tp.x = this->display_width_ - x;
        break;
      case ROTATE_270_DEGREES:
        tp.x = y;
        tp.y = x;
        break;
    }

    this->defer([this, tp]() { this->send_touch_(tp); });
  } else {
    for (auto *listener : this->touch_listeners_)
      listener->release();
  }
}

void FT6336Touchscreen::hard_reset_() {
  this->rts_pin_->digital_write(false);
  delay(15);
  this->rts_pin_->digital_write(true);
  delay(15);
}

bool FT6336Touchscreen::soft_reset_() {
  /*auto err = this->write(SOFT_RESET_CMD, 4);
  if (err != i2c::ERROR_OK)
    return false;

  uint8_t received[4];
  uint16_t timeout = 1000;
  while (!this->store_.touch && timeout > 0) {
    delay(1);
    timeout--;
  }
  if (timeout > 0)
    this->store_.touch = true;
  this->read(received, 4);
  this->store_.touch = false;

  return !memcmp(received, HELLO, 4);*/
  return true;
}

void FT6336Touchscreen::dump_config() {
  ESP_LOGCONFIG(TAG, "FT6336 Touchscreen:");
  LOG_I2C_DEVICE(this);
  LOG_PIN("  Interrupt Pin: ", this->interrupt_pin_);
  LOG_PIN("  RTS Pin: ", this->rts_pin_);
}

}  // namespace ft6336
}  // namespace esphome
