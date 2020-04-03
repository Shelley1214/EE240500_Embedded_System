#include "mbed.h"

DigitalOut led(LED_BLUE);
Ticker time_count;

void blink(){
	led = !led;
}

int main(){
	time_count.attach( &blink, 0.5);
	while(1);
}
