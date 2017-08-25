// lots of this is copied from https://github.com/adafruit/Adafruit_BluefruitLE_nRF51/blob/master/examples/midi/midi.ino 

#include <Arduino.h>
#include <SPI.h>

#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLEMIDI.h"

#include "BluefruitConfig.h"

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.7.0"

/* ...hardware SPI, using SCK/MOSI/MISO hardware SPI pins and then user selected CS/IRQ/RST */
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

Adafruit_BLEMIDI midi(ble);

bool isConnected = false;
int base_note = 60;
int fsrAnalogPin = 0; 
int muxValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
byte midiVelocities[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

int CONTROL0 = 10;
int CONTROL1 = 11;
int CONTROL2 = 12;
int CONTROL3 = 13;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void sendAfterTouch() {
    for (byte i = 0; i < 16; i++) {
        if(midiVelocities[i]>0) {
            midi.send(0xA0, base_note+i, midiVelocities[i]); // poly pressure
//            midi.send(0xD0+i, midiVelocities[i]); // channel pressure
        }
    }
}

// mod wheel
void sendAveragePressure() {
  int sumValue = 0;
  byte countValues = 0;
   for (int i = 0; i < 16; i++) {
        if(midiVelocities[i]>0) {
           sumValue += muxValues[i];
           countValues++;
        }
    }

  if(countValues > 0) {
     midi.send(0xB0, 0x01, (sumValue / countValues) >> 3); // 10 bit analog input to 7 bit coarse value
     midi.send(0xB0, 0x21, ((sumValue / countValues) << 4) % 128); // 10 bit analog input to 7 bit finevalue
  }
  else {
//    midi.send(0xB0, 0x01, 0);
  }
    
}

void setPin(int inputPin)
// function to select pin on 74HC4067
{
   digitalWrite(CONTROL0, (inputPin&15)>>3); 
   digitalWrite(CONTROL1, (inputPin&7)>>2);  
   digitalWrite(CONTROL2, (inputPin&3)>>1);  
   digitalWrite(CONTROL3, (inputPin&1));
}

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

// callback
void connected(void)
{
  isConnected = true;

  Serial.println(F(" CONNECTED!"));
  delay(1000);

}

void disconnected(void)
{
  Serial.println("disconnected");
  isConnected = false;
}

void BleMidiRX(uint16_t timestamp, uint8_t status, uint8_t byte1, uint8_t byte2)
{
  Serial.print("[MIDI ");
  Serial.print(timestamp);
  Serial.print(" ] ");

  Serial.print(status, HEX); Serial.print(" ");
  Serial.print(byte1 , HEX); Serial.print(" ");
  Serial.print(byte2 , HEX); Serial.print(" ");

  Serial.println();
}

void setup(void)
{

  pinMode(CONTROL0, OUTPUT);
  pinMode(CONTROL1, OUTPUT);
  pinMode(CONTROL2, OUTPUT);
  pinMode(CONTROL3, OUTPUT);  

  pinMode(fsrAnalogPin, INPUT);
  
//  while (!Serial);
  delay(500);

  Serial.begin(115200);
  Serial.println(F("Adafruit Bluefruit MIDI Example"));
  Serial.println(F("---------------------------------------"));

  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  //ble.sendCommandCheckOK(F("AT+uartflow=off"));
  ble.echo(false);

  Serial.println("Requesting Bluefruit info:");
  /* Print Bluefruit information */
  ble.info();

  /* Set BLE callbacks */
  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);

  // Set MIDI RX callback
  midi.setRxCallback(BleMidiRX);

  Serial.println(F("Enable MIDI: "));
  if ( ! midi.begin(true) )
  {
    error(F("Could not enable MIDI"));
  }

  ble.verbose(false);
  Serial.print(F("Waiting for a connection..."));

  lastDebounceTime = millis();
}

bool wasOn;
bool newOn;
void loop(void)
{
  // interval for each scanning ~ 500ms (non blocking)
  ble.update(500);

  // bail if not connected
  if (! isConnected)
    return;

  for (int i = 0; i < 16; i++)
  {
    wasOn = midiVelocities[i] > 0;
    setPin(i);
    muxValues[i] = analogRead(fsrAnalogPin);
    midiVelocities[i] = muxValues[i] >> 3; // TODO: pressure sensitivity curve
    
    newOn = midiVelocities[i] > 0;
    if(!wasOn && newOn) { // note on
        midi.send(0x90, base_note+i, midiVelocities[i]);
    }
    else if (wasOn && !newOn) { // note off
        midi.send(0x80, base_note+i, midiVelocities[i]);
    }
  }

  // send note on and note off immediately, but only send aftertouch every so often so as not to get the buffer backed up
  if ((millis() - lastDebounceTime) > debounceDelay) {
    sendAfterTouch();
    sendAveragePressure();
    lastDebounceTime = millis();
  }

}
