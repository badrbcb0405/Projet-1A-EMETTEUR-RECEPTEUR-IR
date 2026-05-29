#include "app_ws2812.h"

#define WS2812_NS_PER_US              1000u
#define WS2812_US_PER_SECOND       1000000u
#define WS2812_RESET_DELAY_US          80u

#define WS2812_BIT_TOTAL_NS          1250u
#define WS2812_BIT_0_HIGH_NS          350u
#define WS2812_BIT_1_HIGH_NS          700u

#define WS2812_COLOR_OFF                0u
#define WS2812_BLINK_INTERVAL_MS      500u
#define WS2812_BREATH_INTERVAL_MS      20u
#define WS2812_BREATH_STEP              4
#define WS2812_BREATH_DIRECTION_UP      1
#define WS2812_BREATH_DIRECTION_DOWN   -1
#define WS2812_BITS_PER_COLOR           8u
#define WS2812_MSB_MASK              0x80u
#define WS2812_GPIO_RESET_SHIFT        16u

static uint32_t ws2812_bit_total_cycles = 0u;
static uint32_t ws2812_bit_0_high_cycles = 0u;
static uint32_t ws2812_bit_1_high_cycles = 0u;

static void ws2812_dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0u;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t ws2812_ns_to_cycles(uint32_t nanoseconds)
{
    uint32_t cycles_per_us = HAL_RCC_GetHCLKFreq() / WS2812_US_PER_SECOND;
    return (cycles_per_us * nanoseconds) / WS2812_NS_PER_US;
}

static void ws2812_timing_init(void)
{
    ws2812_bit_total_cycles = ws2812_ns_to_cycles(WS2812_BIT_TOTAL_NS);
    ws2812_bit_0_high_cycles = ws2812_ns_to_cycles(WS2812_BIT_0_HIGH_NS);
    ws2812_bit_1_high_cycles = ws2812_ns_to_cycles(WS2812_BIT_1_HIGH_NS);
}

static void ws2812_delay_cycles(uint32_t cycles)
{
    uint32_t start = DWT->CYCCNT;

    while ((uint32_t)(DWT->CYCCNT - start) < cycles)
    {
    }
}

static void ws2812_delay_us(uint32_t microseconds)
{
    uint32_t cycles_per_us = HAL_RCC_GetHCLKFreq() / WS2812_US_PER_SECOND;
    ws2812_delay_cycles(cycles_per_us * microseconds);
}

static void ws2812_pin_high(const WS2812_Strip *strip)
{
    strip->data_port->BSRR = strip->data_pin;
}

static void ws2812_pin_low(const WS2812_Strip *strip)
{
    strip->data_port->BSRR = (uint32_t)strip->data_pin << WS2812_GPIO_RESET_SHIFT;
}

static void ws2812_write_bit(const WS2812_Strip *strip, uint8_t bit_is_one)
{
    uint32_t high_cycles = bit_is_one ? ws2812_bit_1_high_cycles : ws2812_bit_0_high_cycles;

    ws2812_pin_high(strip);
    ws2812_delay_cycles(high_cycles);
    ws2812_pin_low(strip);
    ws2812_delay_cycles(ws2812_bit_total_cycles - high_cycles);
}

static void ws2812_write_byte(const WS2812_Strip *strip, uint8_t value)
{
    uint8_t mask = WS2812_MSB_MASK;
    uint8_t bit_index = 0u;

    while (bit_index < WS2812_BITS_PER_COLOR)
    {
        ws2812_write_bit(strip, (uint8_t)((value & mask) != 0u));
        mask >>= 1u;
        bit_index++;
    }
}

static void ws2812_write_rgb(const WS2812_Strip *strip, uint8_t red, uint8_t green, uint8_t blue)
{
    ws2812_write_byte(strip, green);
    ws2812_write_byte(strip, red);
    ws2812_write_byte(strip, blue);
}

static void ws2812_show_white(const WS2812_Strip *strip, uint8_t brightness)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();
    for (uint32_t index = 0u; index < WS2812_LED_COUNT; index++)
    {
        ws2812_write_rgb(strip, brightness, brightness, brightness);
    }
    if (primask == 0u)
    {
        __enable_irq();
    }

    ws2812_pin_low(strip);
    ws2812_delay_us(WS2812_RESET_DELAY_US);
}

static void ws2812_apply(const WS2812_Strip *strip, uint8_t output_brightness)
{
    if (!strip->enabled)
    {
        output_brightness = WS2812_COLOR_OFF;
    }

    ws2812_show_white(strip, output_brightness);
}

void WS2812_Strip_Init(WS2812_Strip *strip)
{
    ws2812_dwt_init();
    ws2812_timing_init();
    strip->enabled = WS2812_DISABLED;
    strip->brightness = WS2812_BRIGHTNESS_DEFAULT;
    strip->mode = WS2812_MODE_FIXED;
    strip->last_update_ms = HAL_GetTick();
    strip->breath_dir = WS2812_BREATH_DIRECTION_UP;
    strip->blink_visible = WS2812_ENABLED;
    ws2812_pin_low(strip);
    ws2812_apply(strip, WS2812_COLOR_OFF);
}

void WS2812_Strip_SetEnabled(WS2812_Strip *strip, uint8_t enabled)
{
    strip->enabled = enabled ? WS2812_ENABLED : WS2812_DISABLED;
    strip->blink_visible = WS2812_ENABLED;
    ws2812_apply(strip, strip->brightness);
}

void WS2812_Strip_Toggle(WS2812_Strip *strip)
{
    WS2812_Strip_SetEnabled(strip, strip->enabled ? WS2812_DISABLED : WS2812_ENABLED);
}

void WS2812_Strip_SetBrightness(WS2812_Strip *strip, uint8_t value)
{
    strip->brightness = value;
    ws2812_apply(strip, strip->brightness);
}

void WS2812_Strip_BrightnessUp(WS2812_Strip *strip)
{
    uint16_t value = (uint16_t)strip->brightness + WS2812_BRIGHTNESS_STEP;

    if (value > WS2812_BRIGHTNESS_MAX)
    {
        value = WS2812_BRIGHTNESS_MAX;
    }

    WS2812_Strip_SetBrightness(strip, (uint8_t)value);
}

void WS2812_Strip_BrightnessDown(WS2812_Strip *strip)
{
    if (strip->brightness <= WS2812_BRIGHTNESS_STEP)
    {
        WS2812_Strip_SetBrightness(strip, WS2812_BRIGHTNESS_MIN);
    }
    else
    {
        WS2812_Strip_SetBrightness(strip, (uint8_t)(strip->brightness - WS2812_BRIGHTNESS_STEP));
    }
}

void WS2812_Strip_SetMode(WS2812_Strip *strip, WS2812_Mode mode)
{
    strip->mode = mode;
    strip->last_update_ms = HAL_GetTick();
    strip->breath_dir = WS2812_BREATH_DIRECTION_UP;
    strip->blink_visible = WS2812_ENABLED;
    ws2812_apply(strip, strip->brightness);
}

void WS2812_Strip_NextMode(WS2812_Strip *strip)
{
    if (strip->mode == WS2812_MODE_BLINK)
    {
        WS2812_Strip_SetMode(strip, WS2812_MODE_FIXED);
    }
    else
    {
        WS2812_Strip_SetMode(strip, (WS2812_Mode)(strip->mode + 1));
    }
}

void WS2812_Strip_Task(WS2812_Strip *strip)
{
    uint32_t now = HAL_GetTick();

    if (!strip->enabled || strip->mode == WS2812_MODE_FIXED)
    {
        return;
    }

    if (strip->mode == WS2812_MODE_BLINK)
    {
        if ((uint32_t)(now - strip->last_update_ms) >= WS2812_BLINK_INTERVAL_MS)
        {
            strip->last_update_ms = now;
            strip->blink_visible = strip->blink_visible ? WS2812_DISABLED : WS2812_ENABLED;
            ws2812_apply(strip, strip->blink_visible ? strip->brightness : WS2812_COLOR_OFF);
        }
        return;
    }

    if (strip->mode == WS2812_MODE_BREATH
        && (uint32_t)(now - strip->last_update_ms) >= WS2812_BREATH_INTERVAL_MS)
    {
        int16_t value = (int16_t)strip->brightness + ((int16_t)strip->breath_dir * WS2812_BREATH_STEP);

        strip->last_update_ms = now;
        if (value >= WS2812_BRIGHTNESS_MAX)
        {
            value = WS2812_BRIGHTNESS_MAX;
            strip->breath_dir = WS2812_BREATH_DIRECTION_DOWN;
        }
        else if (value <= WS2812_BRIGHTNESS_STEP)
        {
            value = WS2812_BRIGHTNESS_STEP;
            strip->breath_dir = WS2812_BREATH_DIRECTION_UP;
        }

        strip->brightness = (uint8_t)value;
        ws2812_apply(strip, strip->brightness);
    }
}
