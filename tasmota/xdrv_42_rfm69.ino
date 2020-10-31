/*
  xdrv_33_rfm69.ino - rfm69 support for Tasmota

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


  --------------------------------------------------------------------------------------------
  Version yyyymmdd  Action    Description
  --------------------------------------------------------------------------------------------

  0.2 20200624  changes - 
  ---
  0.1 20191127  started - 
 

*/

#ifdef USE_SPI
#ifdef USE_RFM69

/*********************************************************************************************\
* RFM69
*
* Usage: 5 SPI-data-wires plus VCC/ground, use hardware SPI, select GPIO_SPI_CS/GPIO_SPI_DC
\*********************************************************************************************/

#define XDRV_42             42

#include <RFM69.h>      
#include <RFM69_ATC.h> 

#define NODEID          1  //the gateway has ID=1
#define NETWORKID     55  //the network ID of all nodes this node listens/talks to
#define FREQUENCY     RF69_868MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define ENCRYPTKEY    "sampleEncryptKey" //identical 16 characters/bytes on all nodes, not more not less!
//#define IS_RFM69HW_HCW  //required for RFM69HW/HCW, comment out for RFM69W/CW!
//#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
//#define ATC_RSSI         -80
//#define ACK_TIME       60  // # of ms to wait for an ack packet


/********************************************************************************************/

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif

bool RFM69initRadio()
{
bool rfmdetected;
if(radio.initialize(FREQUENCY,NODEID,NETWORKID)) {
#ifdef IS_RFM69HW_HCW
  //radio.setHighPower(); //must include this only for RFM69HW/HCW!
#endif

radio.encrypt(ENCRYPTKEY);
  
#ifdef ENABLE_ATC
  radio.enableAutoPower(ATC_RSSI);
#endif
  DEBUG_DRIVER_LOG(PSTR("RFM69 chip connected"));
  return true;
}
else{
 DEBUG_DRIVER_LOG(PSTR("RFM69 chip NOT !!!! connected"));
 return false;
}

}

bool RFM69Detect(void)
{
  if (PinUsed(GPIO_SPI_CS) && PinUsed(GPIO_SPI_DC)) {
    if(RFM69initRadio()){
      AddLog_P2(LOG_LEVEL_INFO,PSTR("RFM69: Initialized"));
      return true;
    }
  }
  AddLog_P2(LOG_LEVEL_INFO,PSTR("RFM69: Failed to initialize"));
  return false;
}

/*********************************************************************************************\
 * Interface
\*********************************************************************************************/

bool Xdrv42(uint8_t function)
{
  bool result = false;

  if (FUNC_INIT == function) {
    result = RFM69Detect();
  }
  return result;
}

#endif  // USE_RFM69
#endif  // USE_SPI

