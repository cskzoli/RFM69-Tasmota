/*
  xsns_76_remote.ino - Remote temperature and humidity sensor support for Tasmota

  Copyright (C) 2020  Zoltan Cserkuti

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifdef USE_SPI
#ifdef USE_RFM69
#ifdef USE_REMOTE
/*********************************************************************************************\
 * Remote - Temperature and Humidity
 *
 * 
\*********************************************************************************************/

#define XSNS_76             76

#define MAX_DELAY_BETWEEN_RECEIVE     900  // 15 minute - 900 sec

struct RSENS {
  float   temperature = NAN;
  float   humidity = NAN;
  uint16_t batt = 0;
  int16_t rssi = 0;
  uint8_t valid = 0;
  uint8_t count = 0;
  char    name[6] = "Kinti";
} Rsens;

uint32_t Elapsed_sec = 0;


/********************************************************************************************/

bool Remote_Init() {
  AddLog_P2(LOG_LEVEL_INFO,PSTR("Outside sensor init func"));
  //radio.readAllRegs();
  
  // placeholder
  return true;
}

void Remote_Every_50_msecond(void)
{
char buff[61] = "";
char pars[10][5];
char *context = NULL;
  if (radio.receiveDone())
  {
    //radio.sendACK();
    
    if ( radio.SENDERID == 3) {
      //strncpy(buff, radio.DATA, radio.DATALEN); 
      //Serial.print("rec:");
    for (byte i = 0; i < radio.DATALEN; i++){
     buff[i]=(char)radio.DATA[i] ;
     //Serial.print((char)buff[i]);
    }
    if (radio.ACKRequested()){
       //AddLog_P2(LOG_LEVEL_INFO,PSTR("RFM69: ACK sent"));
       radio.sendACK();
    }
    char* token = strtok_r(buff, ":;", &context);
    int num_tokens = 0; // Index to token list. We will append to the list
 
    while (token != NULL) {
        // Keep getting tokens until we receive NULL from strtok()
        strcpy(pars[num_tokens], token); // Copy to token list
        //Serial.printf("num: %d : %s   ", num_tokens, pars[num_tokens]);
        num_tokens++;
        token = strtok_r(NULL, ":;", &context); // We pass the context variable to strtok_r
    }
      Rsens.temperature=CharToFloat(pars[1]);
      Rsens.humidity=CharToFloat(pars[3]);
      Rsens.batt=atoi(pars[5]);
      Rsens.rssi=radio.RSSI;
      Elapsed_sec=0;
      Rsens.valid=1;
      AddLog_P2(LOG_LEVEL_INFO,PSTR("RFM69: packet received RSSI: %d"), Rsens.rssi);
    }

  }

}



void Remote_Every_second(void)
{
// char buffl[61] = "Valami"; //max packet size is 61 with encryption enabled
 //byte buffLen;
 Elapsed_sec++;
 //buffLen = strlen(buffl);
 if (Elapsed_sec >= MAX_DELAY_BETWEEN_RECEIVE){
   Rsens.valid=0;
   Elapsed_sec=0;
   //if (radio.sendWithRetry(3, buffl, buffLen))
   //    AddLog_P2(LOG_LEVEL_INFO,PSTR("Sent OK s: %d"), radio.RSSI);
   //  else AddLog_P2(LOG_LEVEL_INFO,PSTR("NOK"));
   //AddLog_P2(LOG_LEVEL_INFO,PSTR("RFM69 1sec fut s: %d"), radio.RSSI);
 }
}

#ifdef USE_WEBSERVER
const char HTTP_SNS_OUT_DATA[] PROGMEM =
  "{s}%s " D_BATT " " D_VOLTAGE "{m}%s " D_UNIT_VOLT "{e}"
  "{s}%s " "RSSI" "{m}%d " "dBm" "{e}";
#endif  // USE_WEBSERVER

void Remote_Show(bool json)
{
  //TempHumDewShow(json, (0 == tele_period), Rsens.name, Rsens.temperature, Rsens.humidity);
  if (Rsens.valid) {
    float voltage = (float)(Rsens.batt) / 100;
    char voltage_chr[FLOATSZ];
    dtostrfd(voltage, 2, voltage_chr);
    if (json) {
      ResponseAppend_P(PSTR(",\"%s\":{"), Rsens.name);
      ResponseAppendTHD(Rsens.temperature, Rsens.humidity);
      ResponseAppend_P(PSTR(",\"" D_JSON_VOLTAGE "\":%s,\"" D_JSON_RSSI "\":%d,\"" D_JSON_DATA "\":%s"), voltage_chr, Rsens.rssi,"\"Valid\""); 
      ResponseJsonEnd();
#ifdef USE_DOMOTICZ
    if ((0 == tele_period)) {
      DomoticzTempHumPressureSensor(Rsens.temperature, Rsens.humidity);
    }
#endif  // USE_DOMOTICZ
#ifdef USE_KNX
    if ((0 == tele_period)) {
      KnxSensor(KNX_TEMPERATURE, Rsens.temperature);
      KnxSensor(KNX_HUMIDITY, Rsens.humidity);
    }
#endif  // USE_KNX
#ifdef USE_WEBSERVER
  } else {
    //const char HTTP_SNS_BATTVOLTAGE[]       PROGMEM = "{s}%s "  D_VOLTAGE             "{m}%s " D_UNIT_VOLT                "{e}";
    //const char HTTP_SNS_RSSI[]       PROGMEM = "{s}%s "  "RSSI"             "{m}%s " "dBm"                "{e}";
    
    WSContentSend_THD(Rsens.name, Rsens.temperature, Rsens.humidity);
    WSContentSend_PD(HTTP_SNS_OUT_DATA, Rsens.name, voltage_chr, Rsens.name, Rsens.rssi);
    
#endif  // USE_WEBSERVER
  }
    
  }else {
    if (json) {
      ResponseAppend_P(PSTR(",\"%s\":{"), Rsens.name);
      ResponseAppend_P(PSTR("\"" D_JSON_DATA "\":%s"), "\"Invalid\"" ); 
      ResponseJsonEnd();
    }
  
  }
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xsns76(uint8_t function)
{
  bool result = false;

  
    switch (function) {
      case FUNC_INIT:
        Remote_Init();
        break;
      case FUNC_EVERY_50_MSECOND:
        Remote_Every_50_msecond();
        break;
      case FUNC_EVERY_SECOND:
        Remote_Every_second();
        break;
      case FUNC_JSON_APPEND:
        Remote_Show(1);
        break;
#ifdef USE_WEBSERVER
      case FUNC_WEB_SENSOR:
        Remote_Show(0);
        break;
#endif  // USE_WEBSERVER
    }
  
  return result;
}

#endif  // USE_REMOTE
#endif  // USE_RFM69
#endif  // USE_SPI
