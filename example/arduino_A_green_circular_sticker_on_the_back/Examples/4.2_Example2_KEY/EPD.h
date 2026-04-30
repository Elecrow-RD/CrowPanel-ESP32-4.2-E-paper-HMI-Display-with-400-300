#ifndef _EPD_H_
#define _EPD_H_

#include "EPD_SPI.h"

//液晶分辨率
#define EPD_W 400
#define EPD_H 300

#define Fast_Seconds_1_5s 0
#define Fast_Seconds_1_s  1

void EPD_ReadBusy(void);
void EPD_RESET(void);
void EPD_Sleep(void);

void EPD_Update(void);
void EPD_Update_Fast(void);
void EPD_Update_Part(void);
void EPD_Update_4Gray(void);

void EPD_Address_Set(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
void EPD_SetCursor(uint16_t xs, uint16_t ys);

void EPD_Display(const uint8_t *Image);
void EPD_Display_Fast(const uint8_t *Image);
void EPD_Display_Part(uint16_t x, uint16_t y, uint16_t sizex, uint16_t sizey, const uint8_t *Image);

void EPD_Init(void);
void EPD_Init_Fast(uint8_t mode);
void EPD_Init_Part(void);
void EPD_Clear_R26A6H(void);

void EPD_Clear(void);
#endif
