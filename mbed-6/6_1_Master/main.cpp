#include "mbed.h"


Serial device(D12, D11);  // tx, rx

DigitalIn button0(SW2);

DigitalIn button1(SW3);


int main() {

   char cmd = 0;

   while(1) {

       cmd = button0<<1 | button1;

       device.putc(cmd);

   }

}
