#include "mbed.h"
#include "arm_math.h"
#include "bbcar.h"
#include <math.h>
#include <cstdlib>
#include <string>  

Serial pc(USBTX, USBRX);
Ticker servo_ticker;
Timer debounce;
Thread t;
Serial uart(D1,D0); //tx,rx
DigitalIn pin3(D3);
PwmOut pin8(D8), pin9(D9);
DigitalInOut pin10(D10);
RawSerial xbee(D12, D11);
DigitalOut red(LED1);
DigitalOut green(LED2);
DigitalOut blue(LED3);
BBCar car(pin8, pin9, servo_ticker);
parallax_ping  ping1(pin10);

string msg = "";
void time_counter(int sec); 
void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);
void xbee_func();
void guess_obj();

int main(){
    // XBee setting
    char xbee_reply[4];
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY <0x240>\r\n");
    reply_messange(xbee_reply, "setting MY : <0x240>");
    xbee.printf("ATDL <0x140>\r\n");
    reply_messange(xbee_reply, "setting DL : <0x140>");
    xbee.printf("ATID <0x87>\r\n");
    reply_messange(xbee_reply, "setting PAN ID : <0x1>");
    xbee.printf("ATWR\r\n");
    reply_messange(xbee_reply, "write config");
    xbee.printf("ATMY\r\n");
    check_addr(xbee_reply, "MY");
    xbee.printf("ATDL\r\n");
    check_addr(xbee_reply, "DL");
    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");
    xbee.getc();
    t.start(&xbee_func);

    uart.baud(9600);
    red = 1, green =1, blue = 1;
   // left
    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table0[] = {-16.986, -16.507, -15.470, -12.202, -5.902, 0.000, 6.459, 12.839, 15.630,  16.587, 16.987};
    // right
    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table1[] = {-16.986, -16.507, -15.470, -12.202, -5.902, 0.000, 6.459, 12.839, 15.630,  16.587, 16.987};   
    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);
    debounce.start();

    /*  
        -20 3sec 51cm
        -10 1.5sec 15.5cm
    */

    msg = "start! go straight";
    green = 0; 
    car.goStraightCalib(-20), time_counter(3000);
    car.turn(-60, -0.5),time_counter(500); //calibration

    car.goStraightCalib(-20), time_counter(3000);
    car.turn(-60, -0.5),time_counter(500); //calibration
    car.goStraightCalib(-10), time_counter(1000);
    green = 1;
    
    // msg = "calibration";
    // green = 0, red = 0, blue = 0;
    char s[21];
    char recv[256],tmp;
    // sprintf(s,"calibration");
    // uart.puts(s);
    // int i =0;
    // while(1) {
    //     if(uart.readable()){
    //         tmp = uart.getc();
    //         if(tmp != '.')
    //             recv[i] = tmp;
    //         else break;
    //         i ++;
    //     }
    // }
    // green = 1, red = 1, blue =1 ;

    msg = "turn left and enter checkpoint 1.";
    blue = 0;
    car.turn(-75, -0.4),time_counter(3500); //turn left
    blue = 1;
    
    // // 100-7.5-36 = 56.5cm
    msg = "go straight";
    green = 0;
    car.goStraightCalib(-20), time_counter(3000); 
    car.goStraightCalib(-10), time_counter(1200); 
    green = 1;

    msg = "reverse parking";
    red = 0;
    car.turn(75, -0.4), time_counter(3000);
    car.goStraightCalib(10), time_counter(1800); 
    red = 1;

    green = 0;
    car.goStraightCalib(-10),time_counter(1000); 
    green = 1;

    car.turn(-60, -0.3);
    time_counter(500); 
    car.goStraightCalib(-10), time_counter(1500);

    msg = "image classification";
    green = 0, red = 0, blue = 0;
    char st[21];
    sprintf(st,"mnist");
    uart.puts(st);
    while(1) {
        if(uart.readable()){
            tmp = uart.getc();
            break;
        }
    }
    green = 1, red = 1, blue =1 ;
    msg = msg + "_GetAns:" + tmp + ", ready to leave checkpoint 1.";
    car.goStraightCalib(10), time_counter(1500);

    car.turn(60, -0.3);
    time_counter(500); 

    car.goStraightCalib(-10),time_counter(1500); 

    blue = 0;
    car.turn(-75, 0.4), time_counter(3000); //turn right
    blue = 1;

    msg = "go straight";
    green = 0;
    car.goStraightCalib(-10), time_counter(500);
    green = 1;
    msg = "turn to row2";
    blue = 0;
    car.turn(-75, 0.4), time_counter(3000); //turn right
    car.turn(-60, -0.5),time_counter(500); //calibration
    car.turn(-60, -0.5),time_counter(500); //calibration
    blue = 1;

    msg = "go straight";
    green = 0; 
    car.goStraightCalib(-20), time_counter(3000);
    car.goStraightCalib(-10), time_counter(2500);
    green = 1;

    msg = "turn right to enter second checkpoint";
    blue = 0;
    car.turn(-75, 0.4), time_counter(2500); //turn right
    blue = 1;

    msg = "go straight";
    green = 0;
    car.goStraightCalib(-10), time_counter(1300); 
    green = 1;

    msg = "turn right to face the obj";
    blue = 0;
    car.turn(-75, 0.4), time_counter(2700); //turn right
    blue = 1;

    green = 0;
    car.goStraightCalib(-10), time_counter(1500); 
    car.goStraightCalib(-10), time_counter(1300); 
    green = 1;

    green = 0, red = 0, blue = 0;
    msg = "detecting object";
    guess_obj();
    green = 1, red = 1, blue = 1;

    msg = "back up and leave checkpoint 2";
    red = 0;
    car.goStraightCalib(10), time_counter(1500); 
    car.goStraightCalib(10), time_counter(1000);
    red = 1;
    blue = 0;
    car.turn(75, 0.4), time_counter(3000);    
    blue = 1;

    msg = "go straight";
    green = 0;
    car.goStraightCalib(-20), time_counter(3200); 
    green = 1;

    msg = "turn to row3";
    blue = 0;
    car.turn(-75, 0.4), time_counter(3000); //turn right
    blue = 1;

    msg = "go straight";
    green = 0; 
    car.goStraightCalib(-10), time_counter(3000);
    car.turn(-60, -0.5), time_counter(500); //calibration
    car.goStraightCalib(-20), time_counter(3000);
    car.turn(-60, -0.5), time_counter(500); //calibration
    car.goStraightCalib(-20), time_counter(3000); 
    green = 1;

    msg = "end";
    wait(1);
}

void time_counter(int sec){
    debounce.reset();
    while(true){
        if(debounce.read_ms() >= sec){
            car.stop();
            break;
        }
    }
}

void guess_obj(){
    red = 0;
    float position = float(ping1), right, left;
    car.turn(-60, -0.3);
    time_counter(500); 
    wait(1);
    right = float(ping1);
    car.turn(60, -0.3);
    time_counter(500); 
    car.turn(-60, 0.3);
    time_counter(500); 
    wait(1);  
    left = float(ping1);
    car.turn(60, 0.3);
    time_counter(500); 

    if(right > position+6 && left > position+6){
        msg += "guess_ans:0(regular triangle)";
    }else if(right > position+6 || left > position+6){
        msg += "guess_ans:2(rectangular triangle)";
    }else if(right < position || left < position){
        msg += "guess_ans:3(scissors)";
    }else{
        msg += "guess_ans:1(square)";
    }
    msg = msg + "_" +  to_string(left) + "_" + to_string(position) + "_" + to_string(right);
    wait(2);
    red = 1;
}

void xbee_func(){
    while (true){
        xbee.printf("%s\r\n",msg.c_str());
        wait(1);
    }
}

void reply_messange(char *xbee_reply, char *messange){
  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';
  }
}

void check_addr(char *xbee_reply, char *messenger){
  for(int i=0; i<4; i++)
    xbee_reply[i] = xbee.getc();
  for(int i=0; i<4; i++)
    xbee_reply[i] = '\0';
}