#ifndef APP_WS2812_H
#define APP_WS2812_H

#include "main.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    WS2812_MODE_FIXED = 0,
    WS2812_MODE_BREATH,
    WS2812_MODE_BLINK,
} WS2812_Mode;

#define WS2812_DISABLED                 0u
#define WS2812_ENABLED                  1u

#define WS2812_LED_COUNT                5u
#define WS2812_BRIGHTNESS_MIN           0u
#define WS2812_BRIGHTNESS_MAX         255u
#define WS2812_BRIGHTNESS_DEFAULT     180u
#define WS2812_BRIGHTNESS_STEP         25u

typedef struct
{
    GPIO_TypeDef *data_port;
    uint16_t data_pin;
    uint8_t brightness;
    uint8_t enabled;
    WS2812_Mode mode;
    uint32_t last_update_ms;
    int8_t breath_dir;
    uint8_t blink_visible;
} WS2812_Strip;

void WS2812_Strip_Init(WS2812_Strip *strip);
void WS2812_Strip_SetEnabled(WS2812_Strip *strip, uint8_t enabled);
void WS2812_Strip_Toggle(WS2812_Strip *strip);
void WS2812_Strip_SetBrightness(WS2812_Strip *strip, uint8_t value);
void WS2812_Strip_BrightnessUp(WS2812_Strip *strip);
void WS2812_Strip_BrightnessDown(WS2812_Strip *strip);
void WS2812_Strip_SetMode(WS2812_Strip *strip, WS2812_Mode mode);
void WS2812_Strip_NextMode(WS2812_Strip *strip);
void WS2812_Strip_Task(WS2812_Strip *strip);

#ifdef __cplusplus
}
#endif

#endif
