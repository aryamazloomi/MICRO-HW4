/* Define to prevent recursive inclusion -------------------------------*/
#ifndef __WS2812__
#define __WS2812__
/* Includes ------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "LPC8XX.h"
#include "stdint.h"	
typedef struct
	{
uint8_t R;
uint8_t G;
uint8_t B;
}color_t;
typedef struct
{

}led_conf_t;
void set_color(led_conf_t *rgb_led, color_t *colors, uint16_t led_num);
#endif