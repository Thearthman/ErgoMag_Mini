#include  <stdio.h>
#include  <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "pico/multicore.h"
#include "hardware/regs/dreq.h"


#define ADC_MASK 0x03
#define ADC_CLK 48000000
#define SAMPLE_RATE 8000
#define SAMPLE_NUM 2

uint8_t sample_array[SAMPLE_NUM];
uint8_t * sample_array_pointer = &sample_array[0];
uint8_t buffer_array[SAMPLE_NUM];

char leftKey;
char rightKey;

int main() {
   stdio_init_all();

   //
   // ADC SETUP
   //
   adc_gpio_init(26);
   adc_gpio_init(27);
   adc_init(); 
   adc_set_round_robin(ADC_MASK); 
   adc_select_input(0);
   adc_fifo_setup(
      true,
      true,
      2,
      false,
      true
   );
   adc_set_clkdiv(ADC_CLK/SAMPLE_RATE );
   // adc_set_clkdiv(0);
   adc_fifo_drain();

   //
   // DMA SETUP
   //
   int sample_chan0 = 0;
   int ctrl_chan = 1;

   //
   // Setup SAMEPLE channel
   //
   dma_channel_config c0 = dma_channel_get_default_config(sample_chan0);
   channel_config_set_transfer_data_size(&c0,DMA_SIZE_8);
   channel_config_set_write_increment(&c0,true);
   channel_config_set_read_increment(&c0,false);
   channel_config_set_dreq(&c0,DREQ_ADC);
   dma_channel_configure(sample_chan0,
      &c0,
      sample_array,
      &adc_hw->fifo,
      SAMPLE_NUM,
      false
   );

   //
   // Setup CONTROL channel
   //
   dma_channel_config ctrl = dma_channel_get_default_config(ctrl_chan);
   channel_config_set_transfer_data_size(&ctrl,DMA_SIZE_32);
   channel_config_set_write_increment(&ctrl,false);
   channel_config_set_read_increment(&ctrl,false);
   channel_config_set_chain_to(&ctrl,sample_chan0);
   dma_channel_configure(ctrl_chan,
      &ctrl,
      &dma_hw->ch[sample_chan0].write_addr,
      &sample_array_pointer,
      1,
      false
   );

   adc_run(true);

   while (true) 
   {  
      dma_channel_wait_for_finish_blocking(sample_chan0);
      for(int i = 0; i < 2; i++){
         buffer_array[i]=sample_array[i];
      } 
      
      adc_fifo_drain();
      hw_write_masked(&adc_hw->cs, 0 << ADC_CS_AINSEL_LSB, ADC_CS_AINSEL_BITS);
      dma_channel_start(ctrl_chan);
      printf("\e[1;1H\e[2J");
      for(int i=0;i<10;i++){
         leftKey = ((buffer_array[1]-200)/4)>=(10-i)?'#':' ';
         rightKey = ((buffer_array[0]+6-204)/4)>=(10-i)?'#':' ';
         printf(" %c %c \n",leftKey,rightKey);
      }
      printf("Left Key: %03d, Right Key%03d\r",buffer_array[1],buffer_array[0]);
      sleep_ms(16);
      // printf("Left Key: %03d, Right Key%03d\r",sample_array[1],sample_array[0]);


   }
}