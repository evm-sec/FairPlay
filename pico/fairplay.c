#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "hardware/sync.h"

//#define DEBUG

//#ifdef DEBUG
#define dbg_print printf
//#else
//#define dbg_print
//#endif

#define MSG_BUF_LEN 9
#define MSG_START_CHAR 0x55
int g_chars_rxed = 0;
unsigned char g_msg_buf[MSG_BUF_LEN];
unsigned char g_output_msg[MSG_BUF_LEN];

void gpio_setup_out(int gp) {
  gpio_init(gp);
  gpio_set_dir(gp, GPIO_OUT);
  //could gpio_set_outover here to invert if necessary
}

#define UART_ID uart0
#define BAUD_RATE 9600
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY    UART_PARITY_NONE

// We are using pins 0 and 1, but see the GPIO function select table in the
// datasheet for information on which other pins can be used.
#define UART_TX_PIN 0
#define UART_RX_PIN 1

void copy_msg_buf_from_ptr(unsigned char * local_buf, int ptr) {
  int i=0;
  for (i=ptr; i<ptr+MSG_BUF_LEN; i++) {
    local_buf[i-ptr] = g_msg_buf[i%MSG_BUF_LEN];
  }
}

unsigned char msg_checksum(unsigned char * msg) {
  int i;
  unsigned char ck=0;
  for(i=0; i<MSG_BUF_LEN-1; i++) {
    ck += msg[i];
  }
  return ck;
}

int valid_msg_rxed(unsigned char * rx_msg) {
  int i=0;
  unsigned char local_buf[MSG_BUF_LEN];
  unsigned char ck;

  for (i=0; i<MSG_BUF_LEN; i++) {
    if (g_msg_buf[i] == MSG_START_CHAR) {
	copy_msg_buf_from_ptr(local_buf, i);
	ck=msg_checksum(local_buf);
	//dbg_print("msg rxed, ck: %02hhx ck: %02hhx", ck, local_buf[MSG_BUF_LEN-1]);
	if (ck == local_buf[MSG_BUF_LEN-1]) {
	  memcpy(rx_msg, local_buf, MSG_BUF_LEN);
	  return 1;
        }
    }    
  }
  return 0;
}

// RX interrupt handler
void handle_uart_rx() {
  unsigned char c;
  while (uart_is_readable(UART_ID)) {
    c = uart_getc(UART_ID);
    g_msg_buf[g_chars_rxed] = c;      
    g_chars_rxed++;
    g_chars_rxed %= MSG_BUF_LEN;
    //dbg_print("Received UART msg %02hhX, ptr at %d \n", c, g_chars_rxed);
  }
  if (valid_msg_rxed(g_output_msg)) {
    dbg_print("New valid message received!");
  }
}

//from Pico SDK UART advanced example
void uart_setup() {
    // Set up our UART with a basic baud rate.
    uart_init(UART_ID, 2400);

    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Actually, we want a different speed
    // The call will return the actual baud rate selected, which will be as close as
    // possible to that requested
    int actual = uart_set_baudrate(UART_ID, BAUD_RATE);

    // Set UART flow control CTS/RTS, we don't want these, so turn them off
    uart_set_hw_flow(UART_ID, false, false);

    // Set our data format
    uart_set_format(UART_ID, DATA_BITS, STOP_BITS, PARITY);

    // Turn off FIFO's - we want to do this character by character
    uart_set_fifo_enabled(UART_ID, false);

    // Set up a RX interrupt
    // We need to set up the handler first
    // Select correct interrupt for the UART we are using
    int UART_IRQ = UART_ID == uart0 ? UART0_IRQ : UART1_IRQ;

    // And set up and enable the interrupt handlers
    irq_set_exclusive_handler(UART_IRQ, handle_uart_rx);
    irq_set_enabled(UART_IRQ, true);

    // Now enable the UART to send interrupts - RX only
    uart_set_irq_enables(UART_ID, true, false);

}

void txBit(char b, int gp)
{
  int d=5;
  b&=1;
  if (b==1) {
    d=20;
  }
  //LOW/HIGH inverted from
  //the output signal because the
  //MAX3232 will invert it
  gpio_put(gp, 0); //low 
  sleep_us(d);
  gpio_put(gp, 1); //high
  sleep_us(30-d);
} 

//size in bits
void txMsg(unsigned char * m, int size, int gp)
{
  int i=0;
  int byte_off, bit_off;
  char b;
  uint32_t int_status;

  //int_status = save_and_disable_interrupts();

  for (i=0; i<size; i++) {
    byte_off=i/8;
    bit_off=i%8;
    b=m[byte_off]>>(7-bit_off);
    txBit(b&1, gp);
  }

  //restore_interrupts(int_status);

}

void msg_print(unsigned char * pmsg)
{
  int i=0;
  int byte_off, bit_off;
  char b;
  int size=56;
  char msg[57];

  for (i=0; i<size; i++) {
    byte_off=i/8;
    bit_off=i%8;
    b=pmsg[byte_off]>>(7-bit_off);
    if ((b&1) == 0) {
	msg[i]='0';
    }
    else {
	msg[i]='1';
    }
  }
  msg[56]=0;
  dbg_print("TX: %s\n", msg);
}

#define GPIO_TO_USE 2

int main() {
  char msg[7];
  stdio_init_all();
  //setup_default_uart();
  dbg_print("Fairplay Scoreboard Driver start\n");
  dbg_print("Setting up gpio...\n");
  gpio_setup_out(GPIO_TO_USE);
  dbg_print("Enabling UART rx...\n");
  uart_setup();

  while (1) {
    dbg_print("TXing...\n");
    msg_print(g_output_msg+1);
    txMsg((char *)g_output_msg+1,56, GPIO_TO_USE);
    sleep_us(48320);  //50ms - (56 * 30 us)
  }

  return 0;
}

