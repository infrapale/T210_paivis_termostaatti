/*    Max6675 Module  ==>   Arduino
 *    CS              ==>     D10
 *    SO              ==>     D12
 *    SCK             ==>     D13
 *    Vcc             ==>     Vcc (5v)
 *    Gnd             ==>     Gnd      */

//LCD config
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>    //If you don't have the LiquidCrystal_I2C library, download it and install it
LiquidCrystal_I2C lcd(0x27,16,2);  //sometimes the adress is not 0x3f. Change to 0x27 if it dosn't work.

/*    i2c LCD Module  ==>   Arduino
 *    SCL             ==>     A5
 *    SDA             ==>     A4
 *    Vcc             ==>     Vcc (5v)
 *    Gnd             ==>     Gnd      */

#include <SPI.h>

#define MAX6675_CS   10
#define MAX6675_SO   12
#define MAX6675_SCK  13
#define SET_TEMP_PIN A0

void setup() {
  delay(2000);
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(5,0);
  lcd.print("paivis");
  lcd.setCursor(0,1);
  lcd.print("Lankakeitin 2019");
  delay(2000);
  lcd.clear();
  
}

void loop() {
  float temperature_read = readThermocouple(); 
  int set_value = map(analogRead(SET_TEMP_PIN),0,1023,0,100);
  lcd.setCursor(0,0);
  //lcd.clear();
  lcd.print("Lampotila:");
  lcd.setCursor(0,1);  
  lcd.print(temperature_read,0); 
  lcd.print("C   ");
  lcd.setCursor(7,1);  
  lcd.print("->");  
  lcd.setCursor(10,1);  
  lcd.print(set_value);
  lcd.print("C   ");
  Serial.println(temperature_read); 
    
  delay(100);
}


double readThermocouple() {

  uint16_t v;
  pinMode(MAX6675_CS, OUTPUT);
  pinMode(MAX6675_SO, INPUT);
  pinMode(MAX6675_SCK, OUTPUT);
  
  digitalWrite(MAX6675_CS, LOW);
  delay(1);

  // Read in 16 bits,
  //  15    = 0 always
  //  14..2 = 0.25 degree counts MSB First
  //  2     = 1 if thermocouple is open circuit  
  //  1..0  = uninteresting status
  
  v = shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);
  v <<= 8;
  v |= shiftIn(MAX6675_SO, MAX6675_SCK, MSBFIRST);
  
  digitalWrite(MAX6675_CS, HIGH);
  if (v & 0x4) 
  {    
    // Bit 2 indicates if the thermocouple is disconnected
    return NAN;     
  }

  // The lower three bits (0,1,2) are discarded status bits
  v >>= 3;

  // The remaining bits are the number of 0.25 degree (C) counts
  return v*0.25;
}
