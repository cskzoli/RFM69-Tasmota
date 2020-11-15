#include <Arduino.h>

#include <LowPower.h>
#include <AM2320_asukiaaa.h>
#include <RFM69.h>     
#include <RFM69_ATC.h> 
//*********************************************************************************************
#define NODEID           3
#define NETWORKID        55
#define GATEWAYID        1   //as a rule of thumb the gateway ID should always be 1
#define FREQUENCY        RF69_868MHZ  //match the RFM69 version! Others: RF69_433MHZ, RF69_868MHZ
//#define FREQUENCY_EXACT 916000000
#define ENCRYPTKEY       "sampleEncryptKey" //same 16 characters on all nodes, comment this line to disable encryption
//#define IS_RFM69HW_HCW   //uncomment only for RFM69HW/HCW! Leave out if you have RFM69W/CW!
//*********************************************************************************************
#define ENABLE_ATC       //comment out this line to disable AUTO TRANSMISSION CONTROL
#define ATC_RSSI         -80
#define ACK_TIME       60 
//*********************************************************************************************
  #define DEBUG(input)   Serial.println(input)
AM2320_asukiaaa mySensor;
int battVolts;   // made global for wider avaliblity throughout a sketch if needed, example a low voltage alarm, etc
RFM69 radio;
char buff[45]; //max packet size is 61 with encryption enabled
byte buffLen;
float temp;
float humi;
char stemp[5];
char shumi[5];
int rssi =1;

int getBandgap(void);
  
void setup(void)
    {
     //pinMode(A3, OUTPUT);
     //digitalWrite(A3, HIGH); 
     Serial.begin(115200);
     //Serial.print("volts X 100");
     Serial.println( "\r\n\r\n" );
     radio.initialize(FREQUENCY,NODEID,NETWORKID); 
     radio.encrypt(ENCRYPTKEY);
     //radio.enableAutoPower(ATC_RSSI);
     //radio.readAllRegs();
     radio.sleep();
     Serial.println("interrupt pin: " + String(RF69_IRQ_PIN) );
     Wire.begin();
     mySensor.setWire(&Wire);
     delay(100);
    }
   
void loop(void)
    {
    if (mySensor.update() != 0) {
     Serial.println("Error: Cannot update sensor values.");
     } else {
      temp=mySensor.temperatureC;
      dtostrf(temp,4, 1, stemp);
      //Serial.println("temperatureC: " + String(stemp) + " C");
      humi=mySensor.humidity;
      dtostrf(humi,4, 1, shumi);
      //Serial.println("humidity: " + String(shumi) + " %");
    }
    Serial.println();   
    Serial.println("at " + String(millis()) + " ms");
     //Serial.println("");

  
     battVolts=getBandgap();  //Determins what actual Vcc is, (X 100), based on known bandgap voltage
     //Serial.print("Battery Vcc volts =  ");
     //Serial.println(battVolts);
     //wakeup (must reinit)
     //radio.RFM69::initialize(FREQUENCY,NODEID,NETWORKID);  //call base init!
     //radio.encrypt(ENCRYPTKEY);

     sprintf(buff, "C:%s;H:%s;B:%d;R:%d", stemp,shumi,battVolts,rssi);
     buffLen = strlen(buff);
         DEBUG("Sending:"); DEBUG(buff);
     if (radio.sendWithRetry(GATEWAYID, buff, buffLen))
       DEBUG("ok!");
     else DEBUG("nok...");
     rssi = radio.RSSI;
     radio.sleep();
     delay(50); 
     LowPower.longPowerDown(239400);  // sleep 4 min 240000 - ~800  = ~ 239400
     delay(50); 
    }

int getBandgap(void) // Returns actual value of Vcc (x 100)
    {
        // For 168/328 boards
     const long InternalReferenceVoltage = 1094L;  // Adjust this value to your boards specific internal BG voltage x1000
        // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc external reference
        // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)         -Selects channel 14, bandgap voltage, to measure
     ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);
       
     delay(50);  // Let mux settle a little to get a more stable A/D conversion
        // Start a conversion 
     ADCSRA |= _BV( ADSC );
        // Wait for it to complete
     while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
        // Scale the value
     int results = (((InternalReferenceVoltage * 1023L) / ADC) + 5L) / 10L; // calculates for straight line value
     return results;

    }