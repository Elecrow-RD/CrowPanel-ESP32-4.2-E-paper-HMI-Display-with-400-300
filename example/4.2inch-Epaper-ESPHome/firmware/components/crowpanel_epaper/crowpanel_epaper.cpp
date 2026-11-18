// Initialization and GC waveform from Elecrow's CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2 example.
// Refresh is asynchronous: the main loop keeps serving GPIO and native API events.
#include "crowpanel_epaper.h"
#include "esphome/core/log.h"
#include <cstring>

namespace esphome::crowpanel_epaper {
static const char *const TAG = "crowpanel_epaper";
void CrowPanelEPaper::command_(uint8_t c) {
  dc_pin_->digital_write(false); enable(); write_byte(c); disable();
}
void CrowPanelEPaper::data_(const uint8_t *p, size_t n) {
  dc_pin_->digital_write(true); enable(); write_array(p, n); disable();
}
void CrowPanelEPaper::reg_(uint8_t c, std::initializer_list<uint8_t> d) {
  command_(c); data_(d.begin(), d.size());
}
void CrowPanelEPaper::init_panel_() {
  reg_(0x00, {0x3F,0x4D});
  reg_(0x01, {0x03,0x10,0x3F,0x3F,0x03});
  reg_(0x06, {0x96,0x96,0x29});
  reg_(0x30, {0x09});
  reg_(0x61, {0x01,0x90,0x01,0x2C});
  reg_(0x82, {0x05}); reg_(0x50, {0x97});
  reg_(0x60, {0x22}); reg_(0xE3, {0x88});
}
void CrowPanelEPaper::write_lut_() {
  const uint8_t head[5][7] = {
    {0x01,0x14,0x0A,0x14,0,1,1},
    {0x01,0x54,0x0A,0x94,0,1,1},
    {0x01,0x54,0x0A,0x94,0,1,1},
    {0x01,0x94,0x0A,0x54,0,1,1},
    {0x01,0x94,0x0A,0x54,0,1,1}
  };
  for (int i=0;i<5;i++) {
    uint8_t lut[42]{}; memcpy(lut,head[i],7);
    command_(0x20+i); data_(lut,sizeof(lut));
  }
}
void CrowPanelEPaper::setup() {
  power_pin_->setup(); 
  power_pin_->digital_write(true);
  dc_pin_->setup(); 
  reset_pin_->setup(); 
  busy_pin_->setup();
  spi_setup(); 
  init_internal_(15000);
  if (!buffer_) 
  { 
    mark_failed(); 
    return; 
  }
  memset(buffer_,0xFF,15000);
  start_reset_();
}

void CrowPanelEPaper::start_reset_() {
  reset_pin_->digital_write(true); 
  phase_=RESET_HIGH; 
  deadline_=millis()+10;
}

void CrowPanelEPaper::loop() {
  const uint32_t now=millis();
  if (static_cast<int32_t>(now-deadline_)<0) return;
  switch (phase_) {
    case IDLE:
      if (pending_) 
      { 
        pending_=false; 
        do_update_(); 
        start_reset_(); 
      }
      break;
    case RESET_HIGH:
      reset_pin_->digital_write(false); 
      phase_=RESET_LOW; 
      deadline_=now+100; 
      break;
    case RESET_LOW:
      reset_pin_->digital_write(true); 
      phase_=RESET_SETTLE; 
      deadline_=now+100; 
      break;
    case RESET_SETTLE:
      init_panel_();
      if (clearing_) 
      { 
        command_(0x10); 
        data_(buffer_,15000); 
      }
      else 
      { 
        reg_(0x50,{0xD7}); 
      }
      command_(0x13); 
      data_(buffer_,15000); 
      write_lut_();
      reg_(0x17,{0xA5});
      phase_=REFRESH; 
      refresh_start_=millis(); 
      deadline_=refresh_start_+100;
      ESP_LOGI(TAG,"Refresh %lu started (%s)",static_cast<unsigned long>(refresh_count_+1),clearing_?"initial clear":"state image");
      break;
    case REFRESH:
      if (busy_pin_->digital_read()) {
        if (now-refresh_start_<15000) return;
        ESP_LOGE(TAG,"BUSY timeout; check V1.2 panel version and power");
        status_set_warning();
      } else {
        status_clear_warning();
        ESP_LOGI(TAG,"Refresh %lu completed in %lu ms",static_cast<unsigned long>(++refresh_count_),static_cast<unsigned long>(now-refresh_start_));
      }
      reg_(0x07,{0xA5});
      if (clearing_) { clearing_=false; pending_=true; }
      phase_=IDLE; deadline_=now+500;
      break;
  }
}
void CrowPanelEPaper::draw_absolute_pixel_internal(int x,int y,Color color) {
  if (x<0 || x>=400 || y<0 || y>=300) return;
  const size_t pos=y*50+x/8;
  const uint8_t bit=0x80>>(x%8);
  // ESPHome binary displays: COLOR_ON denotes ink; the panel uses zero for black.
  if (color.is_on()) buffer_[pos]&=~bit;
  else buffer_[pos]|=bit;
}
void CrowPanelEPaper::dump_config() {
  ESP_LOGCONFIG(TAG,"CrowPanel ESP32 E-Paper HMI 4.2-inch Display V1.2; GC refresh");
  LOG_PIN("  DC: ",dc_pin_); LOG_PIN("  Reset: ",reset_pin_);
  LOG_PIN("  Busy: ",busy_pin_); LOG_PIN("  Power: ",power_pin_);
}
}
