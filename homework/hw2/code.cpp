#include "mbed.h"
#include <math.h>

int PI = 3.141592653589	;
int N = 512	;

Serial pc (USBTX, USBRX);
AnalogOut Aout(DAC0_OUT);
AnalogIn Ain(A0);

DigitalOut LEDG(LED_GREEN);

DigitalOut LEDR(LED_RED);
DigitalIn button(SW3);
DigitalIn button2(SW2);

BusOut display(D6, D7, D9, D10, D11, D5, D4, D8);
char table[20] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
		  0xBF, 0x86, 0xDB, 0xCF, 0xE6, 0xED, 0xFD, 0x87, 0xFF, 0xEF};

void Output(int);

int main(){
	
	LEDG = 0, LEDR = 1;
	float data[N];

	// Analog in
	for(int i=0; i< N; i++){
			data[i] = Ain;
			Aout = data[i];
			wait(1./N);
	}

  	//For FTT
	for(int i=0; i<N; i++){
		pc.printf("%1.3f\r\n", data[i]);
	}

	// calculate frequency
	bool up;
	int round = 0, answer =0;
	if(data[0] > 0.3){
		up = true;
	}else{
		up = false;
	}
	for(int i=1; i<N; i++){
		if(up && data[i] < 0.3){
			up = false;
			round ++;
		}
		if(!up && data[i] > 0.3){
			up = true;
			round ++;
		}
	}
	answer = round/2;

	// mbed display
	while(1){
		
		if(!button){
			LEDG = 1, LEDR = 0;
			Output(answer);
		}else{
			display = 0x00;
			LEDG = 0, LEDR = 1;
		}
		if(!button2) break;
	}
	
	// Analog out
	while(1){
		for (float i=0; i<2; i+= (0.05/answer)){
				Aout = 0.5 + 0.5* sin(i*PI*answer);
				wait(0.025/answer);
		}
	}
	
}

void Output(int answer){
	int hundred, ten, one;
	hundred = answer / 100;
	one = answer % 10;
	ten = (answer - hundred*100 - one)/10;	
	display = table[hundred], wait(1);
	display = table[ten], wait(1);
	display = table[10+one], wait(1);
}
