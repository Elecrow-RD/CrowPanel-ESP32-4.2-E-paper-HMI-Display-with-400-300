#pragma once
#include "esphome/components/display/display_buffer.h"
#include "esphome/components/spi/spi.h"

namespace esphome::crowpanel_epaper {
class CrowPanelEPaper : public display::DisplayBuffer,
    public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW,
                          spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_2MHZ> {
 public:
  void set_dc_pin(GPIOPin *p) { dc_pin_ = p; }
  void set_reset_pin(GPIOPin *p) { reset_pin_ = p; }
  void set_busy_pin(GPIOPin *p) { busy_pin_ = p; }
  void set_power_pin(GPIOPin *p) { power_pin_ = p; }
  void setup() override;
  void loop() override;
  void update() override { pending_ = true; }
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE - 10; }
  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_BINARY; }
 protected:
  int get_width_internal() override { return 400; }
  int get_height_internal() override { return 300; }
  void draw_absolute_pixel_internal(int x, int y, Color color) override;
  void command_(uint8_t command);
  void data_(const uint8_t *data, size_t length);
  void reg_(uint8_t command, std::initializer_list<uint8_t> data);
  void init_panel_();
  void write_lut_();
  void start_reset_();
  GPIOPin *dc_pin_{nullptr}, *reset_pin_{nullptr}, *busy_pin_{nullptr}, *power_pin_{nullptr};
  enum Phase { IDLE, RESET_HIGH, RESET_LOW, RESET_SETTLE, REFRESH } phase_{IDLE};
  uint32_t deadline_{0}, refresh_start_{0}, refresh_count_{0};
  bool clearing_{true}, pending_{true};
};
}
