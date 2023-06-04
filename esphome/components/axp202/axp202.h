#ifndef __AXP202_H__
#define __AXP202_H__

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "axp20x.h"

namespace esphome {
namespace axp202 {

class AXP202Component : public PollingComponent, public i2c::I2CDevice {
 public:
  static AXP202Component *instance;
  AXP202Component();

  void set_batterylevel_sensor(sensor::Sensor *batterylevel_sensor) { batterylevel_sensor_ = batterylevel_sensor; }
  void set_brightness(float brightness) { brightness_ = brightness; }

  // ========== INTERNAL METHODS ==========
  // (In most use cases you won't need these)
  void setup() override;
  void dump_config() override;
  float get_setup_priority() const override;
  void update() override;

  static void handleClick();

  int getBattPercentage();

 protected:
  AXP20X_Class *axp20x = new AXP20X_Class();
  sensor::Sensor *batterylevel_sensor_;
  float brightness_{1.0f};
  float curr_brightness_{-1.0f};

  static uint8_t read_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
  static uint8_t write_cb(uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len);
};

}  // namespace axp202
}  // namespace esphome

#endif
