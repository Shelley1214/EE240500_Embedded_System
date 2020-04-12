#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"

#include <math.h>

#define UINT14_MAX        16383

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

I2C i2c( PTD9,PTD8);
Serial pc(USBTX, USBRX);

InterruptIn button(SW2);
DigitalOut redLED(LED1);
Ticker time_up;
Timeout tout;
EventQueue queue(32 * EVENTS_EVENT_SIZE);

bool activate = false;

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);
void start();

Thread thread1;

int main() {
   redLED = 1;
   pc.baud(115200);
   uint8_t who_am_i, data[2];

   // Enable the FXOS8700Q
   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
   data[1] |= 0x01;
   data[0] = FXOS8700Q_CTRL_REG1;
   FXOS8700CQ_writeRegs(data, 2);

   // Get the slave address
   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

   thread1.start(callback(&queue, &EventQueue::dispatch_forever));
   button.rise(start);

}

void blink(){
   if(activate){
      redLED = !redLED;
   }
}
void stop(){
   redLED = 1;
   activate = false;
}
void logger(){
   for(int i=0; i<100; i++){
      u_int8_t res[6];
      int16_t acc16;
      float t[3];
      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);
      acc16 = (res[0] << 6) | (res[1] >> 2);
      if (acc16 > UINT14_MAX/2) acc16 -= UINT14_MAX;
      t[0] = ((float)acc16) / 4096.0f;

      acc16 = (res[2] << 6) | (res[3] >> 2);
      if (acc16 > UINT14_MAX/2) acc16 -= UINT14_MAX;
      t[1] = ((float)acc16) / 4096.0f;

      acc16 = (res[4] << 6) | (res[5] >> 2);
      if (acc16 > UINT14_MAX/2) acc16 -= UINT14_MAX;
      t[2] = ((float)acc16) / 4096.0f;

      float value =  t[2] / (sqrt(t[0]*t[0]+t[1]*t[1]+t[2]*t[2]));
      value = acos(value)*180/3.1415;
      printf("%1.4f\r\n%1.4f\r\n%1.4f\r\n",t[0],t[1],t[2]);

      if(value > 45)
         printf("%d\r\n",1);
      else printf("%d\r\n",0);
      wait(0.1);
   }
}

void start(){
   activate = true;
   queue.call(logger);
   time_up.attach(&blink, 0.5);
   tout.attach(&stop, 10);
}


void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   int m_addr = FXOS8700CQ_SLAVE_ADDR1;
   char t = addr;

   i2c.write(m_addr, &t, 1, true);

   i2c.read(m_addr, (char *)data, len);

}


void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   int m_addr = FXOS8700CQ_SLAVE_ADDR1;
   i2c.write(m_addr, (char *)data, len);

}
