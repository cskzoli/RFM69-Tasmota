# RFM69-Tasmota
Remote sensor for Tasmota with RFM69

Added RFM69 support for Tasmota (https://tasmota.github.io/docs/).
Using LowPowerLab's library (https://github.com/LowPowerLab/RFM69).

Tested with Tasmota 8.4 on ESP-12F module and RFM69CW 868MHz radio

Currently it is a dirty solution as using hard coded pin configuration.

Pin configuration ESP-12F     ->    RFM69

                  IO12        ->    MISO                  
                  IO13        ->    MOSI
                  IO14        ->    SCLK
                  IO15        ->    NSS (CS)
                  IO4         ->    DIO0 (IRQ)
