// thx2 https://github.com/MarkSherstan/MPU-6050-9250-I2C-CompFilter
// and  https://www.instructables.com/Use-a-1-ATTiny-to-drive-addressable-RGB-LEDs/
// For the board, select ATTinyCore in Arduino - ATtiny25/45/85 (No bootloader)- ATtiny
// ATtiny85 @ 8 MHz (internal oscillator; BOD disabled) & burn bootloader to make it run at 8 MHz 
// more tips in tips.h
 
#include <Adafruit_NeoPixel.h>    // NeoPixel library used to run the NeoPixel LEDs

#define Tiny 1                    // compile for ATTiny85
#ifdef  Tiny
  #include <avr/power.h>          // Required for 16 MHz Adafruit Trinket
  #include <TinyWireM.h>          // I2C Master lib for ATTinys which use USI
  #define LED_PIN 3               // PB3 pin to which the NeoPixels are attached  
#else
  #include <Wire.h>
  #define LED_PIN    4            // pin to which the NeoPixels are attached
#endif
#define   LED_COUNT  16           // number of pixels attached to Attiny85

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

long    loopTimer, loopTimer2;
int     temperature,myx,myy,myz,xof,yof,counter,led,oldled,ledx;
double  accelPitch;
double  accelRoll;
long    acc_x, acc_y, acc_z;
double  accel_x, accel_y, accel_z;
double  gyroRoll, gyroPitch, gyroYaw;
int     gyro_x, gyro_y, gyro_z;
long    gyro_x_cal, gyro_y_cal, gyro_z_cal;
double  rotation_x, rotation_y, rotation_z;
double  freq, dt;
double  tau = 0.98;
double  roll = 0;
double  pitch = 0;
int     rgbr[16]{255,192,128, 64, 32,  0,  0,  0,  0,  0,  0, 32, 64,128,192,255};
int     rgbg[16]{  0, 32, 64,128,192,255,192,128, 64, 32,  0,  0,  0,  0,  0,  0};
int     rgbb[16]{  0,  0,  0,  0,  0,  0, 32, 64,128,192,255,192,128, 64, 32,  0};

// 250 deg/s --> 131.0, 500 deg/s --> 65.5, 1000 deg/s --> 32.8, 2000 deg/s --> 16.4
long scaleFactorGyro = 65.5;
// 2g --> 16384 , 4g --> 8192 , 8g --> 4096, 16g --> 2048
long scaleFactorAccel = 8192;

void setup() {
  #ifdef  Tiny
    pinMode        (PB1,OUTPUT);//Pin6
    TinyWireM.begin();
  #else
    Wire.begin();
    Serial.begin(115200);
  #endif

  pixels.begin();                         // This initializes the NeoPixel library.
  pixels.clear(); 
//pixels.setBrightness(12);               // Set brightness of LEDs
  pixels.setBrightness(5);               // Set brightness of LEDs
  
  // Setup the registers of the MPU-6050 and start up
  setup_mpu_6050_registers();

  // Calibration
  #ifndef  Tiny
  Serial.println("Calibrating gyro, place on level surface and do not move.");
  #endif

  for(long firstPixelHue = 0; firstPixelHue < 4*65536; firstPixelHue += 256) {//4*256
    read_mpu_6050_data();
    gyro_x_cal += gyro_x;
    gyro_y_cal += gyro_y;
    gyro_z_cal += gyro_z;
    pixels.rainbow    (firstPixelHue);
    pixels.show();    // Update strip with new contents
    delay(3);         // Pause for a moment
  }
  pixels.clear(); 
  pixels.show();
  
  // Average the values
  gyro_x_cal /= 4*256;
  gyro_y_cal /= 4*256;
  gyro_z_cal /= 4*256;
  
  // Reset the loop timer
  loopTimer = micros();
  loopTimer2 = micros();
}


void loop() {
  freq = 1/((micros() - loopTimer2) * 1e-6);
  loopTimer2 = micros();
  dt = 1/freq;

  // Read the raw acc data from MPU-6050
  read_mpu_6050_data();

  // Subtract the offset calibration value
  gyro_x -= gyro_x_cal;
  gyro_y -= gyro_y_cal;
  gyro_z -= gyro_z_cal;

  // Convert to instantaneous degrees per second
  rotation_x  = (double)gyro_x / (double)scaleFactorGyro;
  rotation_y  = (double)gyro_y / (double)scaleFactorGyro;
  rotation_z  = (double)gyro_z / (double)scaleFactorGyro;

  // Convert to g force
  accel_x     = (double)acc_x /  (double)scaleFactorAccel;
  accel_y     = (double)acc_y /  (double)scaleFactorAccel;
  accel_z     = (double)acc_z /  (double)scaleFactorAccel;

  // Complementary filter
  accelPitch = atan2(accel_y, accel_z) * RAD_TO_DEG;
  accelRoll  = atan2(accel_x, accel_z) * RAD_TO_DEG;
  pitch      = (tau)*(pitch + rotation_x*dt) + (1-tau)*(accelPitch);
  roll       = (tau)*(roll  - rotation_y*dt) + (1-tau)*(accelRoll);

  gyroPitch += rotation_x*dt;
  gyroRoll  -= rotation_y*dt;
  gyroYaw   += rotation_z*dt;

  myx=roll;       //to int
  myy=pitch;      //to int
  myz=gyroYaw;    //to int 

  #ifndef  Tiny
  Serial.print    ("X:");
  Serial.print    (myx-xof);   
  Serial.print    (" Y:");
  Serial.print    (myy-yof);  
  #endif
  
  int x = (myx-xof)/10;
  int y = (myy-yof)/10;

  if (y>0){     //rechts
    
    if (x>0){   //onder x>0 y>0 led 567

      if   (x-y==0){led=6;} else {if(x>y){led=7;}else{led=5;}}
    
    } else {    //boven x<0 y>0 led 123 
       
      if   (x+y==0){led=2;} else {if(x+y>0){led=3;}else{led=1;}}
    } 
  } else {      //links

    if (x>0)
    {           //onder  x>0 y<0 led 9AB

      if   (x+y==0){led=10;} else {if(x+y>0){led=9;}else{led=11;}}

    } else {    //boven  x<0 y<0 led DE0

      if   (x-y==0){led=14;} else {if(x>y){led=13;}else{led=15;}}
    
    }
  }
      
  if (x==0){if (x+y> 0){led=4;} else {led=12;}}
  if (y==0){if (x+y> 0){led=8;} else {led=0; }}
  if (x==0&&y==0){led=99;}
  
  if (counter<100){
      pixels.setPixelColor(ledx,255,0,0);
      if (ledx>0){pixels.setPixelColor(ledx-1,0,0,0);}
      pixels.show();
      ledx++;
      if (ledx>15){ledx=0;}
  }
  if (counter==100){xof=roll;yof=pitch;
    #ifndef  Tiny
    Serial.print("recalibrating ...");
    #endif
  }
  counter++;
  #ifndef  Tiny
  Serial.println(" led:"+String(led));
  #endif
    
if (counter>=100){
  if (led!=oldled){
    if     (led==99){
      for (int i=0;i<LED_COUNT;i++) {pixels.setPixelColor(i,0,255,0);}
      pixels.setBrightness(1);
      pixels.show();
    } else {
      int r,g,b;
      r=rgbr[led];
      g=rgbg[led];
      b=rgbb[led];
      pixels.clear();
      pixels.setBrightness(12);
      pixels.setPixelColor(led,r,g,b);
      pixels.show();
    }
      oldled=led;
  }
}
  // Wait until the loopTimer reaches 4000us (250Hz) before next loop
  while (micros() - loopTimer <= 4000);
  loopTimer = micros();
}


void read_mpu_6050_data() {
#ifdef  Tiny
  // Subroutine for reading the raw data
  TinyWireM.beginTransmission(0x68);
  TinyWireM.write(0x3B);
  TinyWireM.endTransmission();
  TinyWireM.requestFrom(0x68, 14);
  // Read data --> Temperature falls between acc and gyro registers
  acc_x = TinyWireM.read() << 8 | TinyWireM.read();
  acc_y = TinyWireM.read() << 8 | TinyWireM.read();
  acc_z = TinyWireM.read() << 8 | TinyWireM.read();
  temperature = TinyWireM.read() <<8 | TinyWireM.read();
  gyro_x = TinyWireM.read()<<8 | TinyWireM.read();
  gyro_y = TinyWireM.read()<<8 | TinyWireM.read();
  gyro_z = TinyWireM.read()<<8 | TinyWireM.read();
#else
  // Subroutine for reading the raw data
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 14);
  // Read data --> Temperature falls between acc and gyro registers
  acc_x = Wire.read() << 8 | Wire.read();
  acc_y = Wire.read() << 8 | Wire.read();
  acc_z = Wire.read() << 8 | Wire.read();
  temperature = Wire.read() <<8 | Wire.read();
  gyro_x = Wire.read()<<8 | Wire.read();
  gyro_y = Wire.read()<<8 | Wire.read();
  gyro_z = Wire.read()<<8 | Wire.read();
#endif
}


void setup_mpu_6050_registers() {
#ifdef  Tiny
  //Activate the MPU-6050
  TinyWireM.beginTransmission(0x68);
  TinyWireM.write(0x6B);
  TinyWireM.write(0x00);
  TinyWireM.endTransmission();
  // Configure the accelerometer
  // Wire.write(0x__);
  // Wire.write; 2g --> 0x00, 4g --> 0x08, 8g --> 0x10, 16g --> 0x18
  TinyWireM.beginTransmission(0x68);
  TinyWireM.write(0x1C);
  TinyWireM.write(0x08);
  TinyWireM.endTransmission();
  // Configure the gyro
  // Wire.write(0x__);
  // 250 deg/s --> 0x00, 500 deg/s --> 0x08, 1000 deg/s --> 0x10, 2000 deg/s --> 0x18
  TinyWireM.beginTransmission(0x68);
  TinyWireM.write(0x1B);
  TinyWireM.write(0x08);
  TinyWireM.endTransmission();
#else
  //Activate the MPU-6050
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  // Configure the accelerometer
  // Wire.write(0x__);
  // Wire.write; 2g --> 0x00, 4g --> 0x08, 8g --> 0x10, 16g --> 0x18
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x08);
  Wire.endTransmission();
  // Configure the gyro
  // Wire.write(0x__);
  // 250 deg/s --> 0x00, 500 deg/s --> 0x08, 1000 deg/s --> 0x10, 2000 deg/s --> 0x18
  Wire.beginTransmission(0x68);
  Wire.write(0x1B);
  Wire.write(0x08);
  Wire.endTransmission();
#endif
}
