#include  <stdio.h>
#include  <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "pico/multicore.h"
#include "hardware/regs/dreq.h"


#define ADC_MASK 0x0f
#define ADC_CLK 48000000
#define SAMPLE_RATE 8000
#define SAMPLE_NUM 4

uint8_t sample_array[SAMPLE_NUM] = {0,1,2,3};
uint8_t* data_ptr[1] = {sample_array};
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
   adc_gpio_init(28);
   adc_gpio_init(29);
   adc_init(); 
   adc_set_clkdiv(0);
   adc_set_round_robin(ADC_MASK); 
   adc_select_input(0);
   adc_fifo_setup(
      true,
      true,
      1,
      false,
      true
   );
   // adc_set_clkdiv(ADC_CLK/SAMPLE_RATE );
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
   channel_config_set_irq_quiet(&c0,true);
   channel_config_set_dreq(&c0,DREQ_ADC);
   channel_config_set_chain_to(&c0,ctrl_chan);
   channel_config_set_enable(&c0,true);
   dma_channel_configure(
      sample_chan0,
      &c0,
      NULL,
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
   channel_config_set_irq_quiet(&ctrl,true);
   channel_config_set_dreq(&ctrl,DREQ_FORCE);
   channel_config_set_enable(&ctrl,true);
   dma_channel_configure(
      ctrl_chan,
      &ctrl,
      &dma_hw->ch[sample_chan0].al2_write_addr_trig,
      data_ptr,
      1,
      false
   );
   dma_channel_start(ctrl_chan);
   adc_run(true);

   while (true) 
   {  
      printf("K1: %03d, K2: %03d, K3: %03d, K4: %03d\r",sample_array[1],sample_array[0],sample_array[2],sample_array[3]);
      sleep_ms(16);

   }
}