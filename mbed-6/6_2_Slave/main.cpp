#include "mbed.h"


Serial pc(USBTX, USBRX); // tx, rx

Serial device(D12, D11);  // tx, rx

DigitalOut red_led(LED1);

DigitalOut green_led(LED2);

char recv;


int main() {

    red_led = 1;

    green_led = 1;

    while(1) {

        if(device.readable()) {

            recv = device.getc();

            red_led = (recv&0x01)?1:0;

            green_led = (recv&0x02)?1:0;

        }

    }

}
