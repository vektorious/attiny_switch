#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>

#ifndef F_CPU
// ATtiny1614 ships with internal 20 MHz oscillator and /6 prescaler by default.
#define F_CPU 3333333UL
#endif

#define SWITCH_PIN_bm PIN1_bm  // PA1 -> ISP772T IN

static void gpio_init(void);
static void rtc_minute_wakeup_init(void);
static void switch_on(void);
static void switch_off(void);

volatile uint8_t wake_flag = 0;
static uint8_t minute_counter = 0;

int main(void)
{
    gpio_init();
    switch_off();       // Boot safely with switch OFF.
    minute_counter = 0; // Start a fresh 60-minute cycle after reset.
    rtc_minute_wakeup_init();
    sei();

    for (;;) {
        // Deep sleep mode that keeps RTC running.
        set_sleep_mode(SLEEP_MODE_STANDBY);
        sleep_mode();

        if (wake_flag) {
            wake_flag = 0;
            minute_counter++;

            if (minute_counter == 50U) {
                switch_on();
            } else if (minute_counter == 60U) {
                switch_off();
                minute_counter = 0;
            }
        }
    }
}

static void gpio_init(void)
{
    // Keep PA1 low before enabling output to avoid unintended pulse.
    VPORTA.OUT &= (uint8_t)~SWITCH_PIN_bm;
    VPORTA.DIR |= SWITCH_PIN_bm;

    // Leave PA0 untouched: dedicated UPDI/RESET pin.

    // Put all other GPIOs into a low-power safe state: digital input buffer off.
    PORTA.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTA.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTA.PIN4CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTA.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTA.PIN6CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTA.PIN7CTRL = PORT_ISC_INPUT_DISABLE_gc;

    PORTB.PIN0CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTB.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTB.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
    PORTB.PIN3CTRL = PORT_ISC_INPUT_DISABLE_gc;
}

static void switch_on(void)
{
    VPORTA.OUT |= SWITCH_PIN_bm;
}

static void switch_off(void)
{
    VPORTA.OUT &= (uint8_t)~SWITCH_PIN_bm;
}

static void rtc_minute_wakeup_init(void)
{
    while (RTC.STATUS > 0) {
        // Wait for RTC sync.
    }

    // 32.768 kHz / 1024 = 32 Hz, overflow every 1920 counts = 60 s.
    RTC.CLKSEL = RTC_CLKSEL_INT32K_gc;
    RTC.PER = 1919U;
    RTC.CNT = 0U;
    RTC.INTCTRL = RTC_OVF_bm;

    RTC.CTRLA = RTC_PRESCALER_DIV1024_gc | RTC_RUNSTDBY_bm | RTC_RTCEN_bm;
}

ISR(RTC_CNT_vect)
{
    RTC.INTFLAGS = RTC_OVF_bm; // Clear overflow interrupt flag.
    wake_flag = 1;
}
