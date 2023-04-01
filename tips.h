/*
 thx2 https:github.com/MarkSherstan/MPU-6050-9250-I2C-CompFilter
 https:www.instructables.com/Use-a-1-ATTiny-to-drive-addressable-RGB-LEDs/

 powerconsumptiun CR2032=210mAh x2 = 420 mAh usage Arduino nano=40mAh = 10 hours

 For the board, select ATTinyCore in Arduino - ATtiny25/45/85 (No bootloader)- ATtiny
 ATtiny85 @ 8 MHz (internal oscillator; BOD disabled) & burn bootloader to make it run at 8 MHz 
 
 from strandtest example:
 NEOPIXEL BEST PRACTICES for most reliable operation:
 - Add 1000 uF CAPACITOR between NeoPixel strip's + and - connections.
 - MINIMIZE WIRING LENGTH between microcontroller board and first pixel.
 - NeoPixel strip's DATA-IN should pass through a 300-500 OHM RESISTOR.
 - AVOID connecting NeoPixels on a LIVE CIRCUIT. If you must, ALWAYS
   connect GROUND (-) first, then +, then data.

 ATTINY64
 pin4=GND
 pin5=SDA =PB0=MOSI    MPU SDA
 pin6=MISO=PB1         NEOPIXEL
 pin7=A1=SCL =PB2=SCK  MPU SCL
 pin8=VCC
 
 Arduino NANO
 A4=SDA
 A5=SCL
 
 */
