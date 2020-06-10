#include "mbed.h"
#include "bbcar.h"
#include "bbcar_rpc.h"

RawSerial pc(USBTX, USBRX);
RawSerial xbee(D12, D11);
Ticker servo_ticker;
PwmOut pin9(D9), pin8(D8);
BBCar car(pin8, pin9, servo_ticker);
EventQueue queue(32 * EVENTS_EVENT_SIZE);

void reply_messange(char *xbee_reply, char *messange);
void check_addr(char *xbee_reply, char *messenger);

int main(){
  pc.baud(9600);
  char xbee_reply[4];
  // XBee setting
  xbee.baud(9600);
  xbee.printf("+++");
  xbee_reply[0] = xbee.getc();

  xbee_reply[1] = xbee.getc();
  if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
    pc.printf("enter AT mode.\r\n");
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
  }
  xbee.printf("ATMY <0x240>\r\n");
  reply_messange(xbee_reply, "setting MY : <0x240>");

  xbee.printf("ATDL <0x140>\r\n");
  reply_messange(xbee_reply, "setting DL : <0x140>");

  xbee.printf("ATID <0x1>\r\n");
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

  // start
  pc.printf("start\r\n");

  // please contruct you own calibration table with each servo
  double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
  double speed_table0[] = {-10.445, -9.812, -9.647, -9.408, -5.900, 0.000, 5.900, 10.843, 11.880, 11.401, 12.199};
  double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
  double speed_table1[] = {-10.445, -9.812, -9.647, -9.408, -5.900, 0.000, 5.900, 10.843, 11.880, 11.401, 12.199};

  // first and fourth argument : length of table
  car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);
  
  char buf[256], outbuf[256];
  while (1) {
      for (int i = 0; ; i++) {
          buf[i] = xbee.getc();
          if (buf[i] == '\n') 
              break;
      }
      RPC::call(buf, outbuf);
      xbee.printf("%s\r\n", outbuf);
  }
}


void reply_messange(char *xbee_reply, char *messange){

  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();

  if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
    pc.printf("%s\r\n", messange);
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';
  }
}


void check_addr(char *xbee_reply, char *messenger){

  xbee_reply[0] = xbee.getc();
  xbee_reply[1] = xbee.getc();
  xbee_reply[2] = xbee.getc();
  xbee_reply[3] = xbee.getc();

  pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
  xbee_reply[0] = '\0';
  xbee_reply[1] = '\0';
  xbee_reply[2] = '\0';
  xbee_reply[3] = '\0';

}
