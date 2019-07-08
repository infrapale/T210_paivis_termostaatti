/*    Max6675 Module  ==>   Arduino
 *    CS              ==>     D10
 *    SO              ==>     D12
 *    SCK             ==>     D13
 *    Vcc             ==>     Vcc (5v)
 *    Gnd             ==>     Gnd      */

/*    i2c LCD Module  ==>   Arduino
 *    SCL             ==>     A5
 *    SDA             ==>     A4
 *    Vcc             ==>     Vcc (5v)
 *    Gnd             ==>     Gnd      */

//LCD config
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>    //If you don't have the LiquidCrystal_I2C library, download it and install it
#include <Arduino_FreeRTOS.h>



LiquidCrystal_I2C lcd(0x27,16,2);  //sometimes the adress is not 0x3f. Change to 0x27 if it dosn't work.


#include <SPI.h>

#define MAX6675_CS   10
#define MAX6675_SO   12
#define MAX6675_SCK  13
#define SET_TEMP_PIN A0
#define HYSTERESIS   1.0
#define RELAY_ON_PIN  2
#define RELAY_OFF_PIN 3
#define RELAY_PULSE_WIDTH_ms 100
float temperature_read;
int   set_value;
boolean relay_on;
void  TaskReadTemp( void *pvParameters );
void  TaskCtrlTemp( void *pvParameters );
void  TaskShow( void *pvParameters );

void setup() {
    delay(2000);
    Serial.begin(9600);
    pinMode(RELAY_ON_PIN,OUTPUT);
    pinMode(RELAY_OFF_PIN,OUTPUT);
    digitalWrite(RELAY_ON_PIN, LOW);
    digitalWrite(RELAY_OFF_PIN, LOW);   
    digitalWrite(RELAY_OFF_PIN, HIGH);
    delay(RELAY_PULSE_WIDTH_ms);
    digitalWrite(RELAY_OFF_PIN, LOW);
    relay_on = false;
    
    xTaskCreate(
    TaskReadTemp
    ,  (const portCHAR *)"Read Temp"   // A name just for humans
    ,  128  // Stack size
    ,  NULL
    ,  2  // priority
    ,  NULL );
    
    xTaskCreate(
    TaskShow
    ,  (const portCHAR *)"Show Info"   // A name just for humans
    ,  128  // Stack size
    ,  NULL
    ,  2  // priority
    ,  NULL );

    xTaskCreate(
    TaskCtrlTemp
    ,  (const portCHAR *)"Control Temp"   // A name just for humans
    ,  128  // Stack size
    ,  NULL
    ,  2  // priority
    ,  NULL );

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

    
  delay(100);
}

void TaskReadTemp(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    set_value = map(analogRead(SET_TEMP_PIN),0,1023,0,100);
    //temperature_read = readThermocouple(); 
    temperature_read = 44.0;
    Serial.println(temperature_read);
    vTaskDelay(500/portTICK_PERIOD_MS);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskShow(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    lcd.setCursor(0,0);
    //lcd.clear();
    lcd.print("Lampotila:");
    lcd.setCursor(13,0);
    if(relay_on) lcd.print("On ");
    else lcd.print("Off");
    lcd.setCursor(0,1);  
    lcd.print(temperature_read,0); 
    lcd.print("C   ");
    lcd.setCursor(7,1);  
    lcd.print("->");  
    lcd.setCursor(10,1);  
    lcd.print(set_value);
    lcd.print("C   ");
    //Serial.println(temperature_read); 
    
    vTaskDelay(500/portTICK_PERIOD_MS);  // one tick delay (15ms) in between reads for stability
  }
}

void TaskCtrlTemp(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  for (;;)
  {
    if (relay_on) {
       if ( temperature_read > ((double)set_value)+HYSTERESIS){
          digitalWrite(RELAY_OFF_PIN, HIGH);
          vTaskDelay(RELAY_PULSE_WIDTH_ms/portTICK_PERIOD_MS);
          digitalWrite(RELAY_OFF_PIN, LOW);
          relay_on = false;
       } 
    }
    else {   // relay is off
       if ( temperature_read < (double)set_value){
          digitalWrite(RELAY_ON_PIN, HIGH);
          vTaskDelay(RELAY_PULSE_WIDTH_ms/portTICK_PERIOD_MS);
          digitalWrite(RELAY_ON_PIN, LOW);
          relay_on = true;
       }
    }

    vTaskDelay(2000/portTICK_PERIOD_MS);  // one tick delay (15ms) in between reads for stability
  }
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
