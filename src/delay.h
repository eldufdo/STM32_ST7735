#define DELAY_TICK_FREQUENCY_US 1000000   /* = 1MHZ -> microseconds delay */
#define DELAY_TICK_FREQUENCY_MS 1000      /* = 1kHZ -> milliseconds delay */


volatile uint32_t TimingDelay; // __IO -- volatile


/*
 *   Declare Functions
 */
extern void Delay_ms(uint32_t nTime);
extern void Delay_us(uint32_t nTime);
