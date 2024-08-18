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

const uint32_t gpio_test = 1ul << 2;
const uint32_t* gpio_test_ptr = &gpio_test;
const uint multiplexer_select_addresses[4] = {2, 3, 4, 5};
const uint32_t gpio_mask[16] = {0000ul << 2, 0001ul << 2, 0010ul << 2, 0011ul << 2, 0100ul << 2, 0101ul << 2, 0110ul << 2, 0111ul << 2, 1000ul << 2, 1001ul << 2, 1010ul << 2, 1011ul << 2, 1100ul << 2, 1101ul << 2, 1110ul << 2, 1111ul << 2};
const uint32_t* gpio_ptr[1] = {gpio_mask};
uint8_t sample_array[SAMPLE_NUM] = {0,1,2,3};
uint8_t* data_ptr[1] = {sample_array};
uint8_t buffer_array[SAMPLE_NUM];


int main() {
   stdio_init_all();

   //
   // ADC SETUP
   //
   adc_gpio_init(26);
   adc_gpio_init(27);
   adc_gpio_init(28);
   adc_gpio_init(29);
   for (int i = 0; i < 4; i++) {
      gpio_init(multiplexer_select_addresses[i]);
      gpio_set_dir(multiplexer_select_addresses[i], GPIO_OUT);
   }
   adc_init(); 
   // adc_set_clkdiv(0);
   adc_set_clkdiv(ADC_CLK/SAMPLE_RATE);
   adc_set_round_robin(ADC_MASK); 
   adc_select_input(0);
   adc_fifo_setup(
      true,
      true,
      4,
      false,
      true
   );
   adc_fifo_drain();

   //
   // DMA SETUP
   //
   int ctrl_chan = 0;
   int sample_chan0 = 1;
   int gpio_chan = 2;
   int gpio_ctrl_chan = 3;

   //
   // Setup SAMEPLE channel
   //
   dma_channel_config samp_conf = dma_channel_get_default_config(sample_chan0);
   channel_config_set_transfer_data_size(&samp_conf,DMA_SIZE_8);
   channel_config_set_write_increment(&samp_conf,true);
   channel_config_set_read_increment(&samp_conf,false);
   channel_config_set_irq_quiet(&samp_conf,true);
   channel_config_set_dreq(&samp_conf,DREQ_ADC);
   channel_config_set_chain_to(&samp_conf,ctrl_chan);
   channel_config_set_enable(&samp_conf,true);
   dma_channel_configure(
      sample_chan0,
      &samp_conf,
      NULL,
      &adc_hw->fifo,
      SAMPLE_NUM+4,
      false
   );

   //
   // Setup CONTROL channel
   //
   dma_channel_config ctrl_conf = dma_channel_get_default_config(ctrl_chan);
   channel_config_set_transfer_data_size(&ctrl_conf,DMA_SIZE_32);
   channel_config_set_write_increment(&ctrl_conf,false);
   channel_config_set_read_increment(&ctrl_conf,true);
   channel_config_set_irq_quiet(&ctrl_conf,true);
   channel_config_set_dreq(&ctrl_conf,DREQ_FORCE);
   channel_config_set_enable(&ctrl_conf,true);
   // channel_config_set_chain_to(&ctrl_conf,gpio_chan);
   dma_channel_configure(
      ctrl_chan,
      &ctrl_conf,
      &dma_hw->ch[sample_chan0].al2_write_addr_trig,
      data_ptr,
      1,
      false
   );

   //
   // GPIO channel
   //
   dma_channel_config gpio_conf = dma_channel_get_default_config(gpio_chan);
   channel_config_set_transfer_data_size(&gpio_conf,DMA_SIZE_32);
   channel_config_set_write_increment(&gpio_conf,false);
   channel_config_set_read_increment(&gpio_conf,true);
   channel_config_set_irq_quiet(&gpio_conf,true);
   dma_timer_set_fraction(0,0x0001,0xffff);
   channel_config_set_dreq(&gpio_conf,DREQ_DMA_TIMER0);
   channel_config_set_chain_to(&gpio_conf,gpio_ctrl_chan);
   channel_config_set_enable(&gpio_conf,true);
   dma_channel_configure(
      gpio_chan,
      &gpio_conf,
      &sio_hw->gpio_togl,
      NULL,
      16,
      false
   );

   //
   // GPIO CTRL channel
   //
   dma_channel_config gpio_ctrl_conf = dma_channel_get_default_config(gpio_ctrl_chan) ;
   channel_config_set_transfer_data_size(&gpio_ctrl_conf,DMA_SIZE_32);
   channel_config_set_write_increment(&gpio_ctrl_conf,false);
   channel_config_set_read_increment(&gpio_ctrl_conf,false);
   channel_config_set_irq_quiet(&gpio_ctrl_conf,true);
   // dma_timer_set_fraction(1,0x0001,0xffff);
   channel_config_set_dreq(&gpio_ctrl_conf,DREQ_FORCE);   
   // channel_config_set_chain_to(&gpio_ctrl_conf,gpio_ctrl_chan);
   channel_config_set_enable(&gpio_ctrl_conf,true);
   dma_channel_configure(
      gpio_ctrl_chan,
      &gpio_ctrl_conf,
      &dma_hw->ch[gpio_chan].al3_read_addr_trig,
      gpio_ptr,
      1, 
      false
   );

   dma_channel_start(ctrl_chan);
   dma_channel_start(gpio_ctrl_chan);
   adc_run(true);
   // gpio_put(2,1);
   int k = 0;
   while (true)
   {  
      k = k%16+1;
      sio_hw->gpio_togl = 1u<<2;
      // printf("%d\r",gpio_mask[0]);
      printf("K1: %03d, K2: %03d, K3: %03d, K4: %03d, gpio: %d\r",sample_array[1],sample_array[0],sample_array[2],sample_array[3],&sio_hw->gpio_set);
      sleep_ms(16);

   }
}