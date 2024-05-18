#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "stdio.h"

const bool bit_mask[16][4] = {{0,0,0,0},{1,0,0,0},{0,1,0,0},{1,1,0,0},{0,0,1,0},{1,0,1,0},{0,1,1,0},{1,1,1,0},
                        {0,0,0,1},{1,0,0,1},{0,1,0,1},{1,1,0,1},{0,0,1,1},{1,0,1,1},{0,1,1,1},{1,1,1,1}};
const uint multiplexer_select_addresses[4] = {2, 3, 4, 5};
const uint multiplexer_input_addresses[4] = {26, 27, 28, 29};
uint current_multipler_address = 0;

//Max Travel of each switch
float max_travel[4][16];



float readChannel(uint8_t multiplexer_address, uint8_t channel) {
   //Validdate Multiplexer Address
   if (multiplexer_address > 5) {
      return 0;
   }
   for (int i = 0; i < 4; i++) {
      gpio_put(multiplexer_select_addresses[i], bit_mask[channel][i]);
   }
   if (multiplexer_address!= current_multipler_address) {
      adc_select_input(multiplexer_address);
      current_multipler_address = multiplexer_address;
   }
   uint16_t reading = adc_read();
   float voltage = (float)reading * 3.3 / (1 << 12);
   return voltage;
}

void setMaxTravel(){
   max_travel[1][0] = readChannel(1,0);
   max_travel[1][1] = readChannel(1,1);
   max_travel[1][2] = readChannel(1,2);
   max_travel[1][3] = readChannel(1,3);
}

// Return true if key is pressed.
bool testMultiplexer() {
   float voltage0 = readChannel(1, 0);
   float voltage1 = readChannel(1, 1);
   float voltage2 = readChannel(1, 2);
   float voltage3 = readChannel(1, 3);
   printf("Voltage0: %f Max Travel: %f\n", voltage0, max_travel[1][0]);
   printf("Voltage1: %f Max Travel: %f\n", voltage1, max_travel[1][1]);
   printf("Voltage2: %f Max Travel: %f\n", voltage2, max_travel[1][2]);
   printf("Voltage3: %f Max Travel: %f\n", voltage3, max_travel[1][3]);
   return  voltage0 < max_travel[1][0]*0.8 || voltage1 < max_travel[1][1]*0.8 || voltage2 < max_travel[1][2]*0.8 || voltage3 < max_travel[1][3]*0.8;
}

int main() {
   const uint led_pin = 15;
   stdio_init_all();
   gpio_init(led_pin);
   gpio_set_dir(led_pin, GPIO_OUT);
   adc_init();
   for (int i = 0; i < 4; i++) {
      adc_gpio_init(multiplexer_input_addresses[i]);
      gpio_init(multiplexer_select_addresses[i]);
      gpio_set_dir(multiplexer_select_addresses[i], GPIO_OUT);
   }
   setMaxTravel();

   while (1) {
      bool key_pressed = testMultiplexer();
      if (key_pressed) {
         gpio_put (led_pin, 1);
      } else {
         gpio_put (led_pin, 0); 
      }
      sleep_ms(100);
   }
}
