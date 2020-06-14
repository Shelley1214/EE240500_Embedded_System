#include"mbed.h"

Serial pc(USBTX,USBRX); //tx,rx
Serial uart(D12,D11); //tx,rx
DigitalIn button(SW2);

int main(){

   uart.baud(9600);

   while(1) {

      if(uart.readable()){
            char recv = uart.getc();
            pc.putc(recv);
            // pc.printf("\r\n");
      }

      if( button == 0){
         char s[21];
         sprintf(s,"image_classification");
         uart.puts(s);
         pc.printf("send\r\n");
         wait(0.5);
      }

   }

}

