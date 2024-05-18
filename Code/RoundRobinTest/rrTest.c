#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "stdio.h"

#define ADC_MASK 0x3
int main() {
    stdio_init_all();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_init(); 
    adc_set_round_robin(ADC_MASK); 

    while (true) {

    }
}