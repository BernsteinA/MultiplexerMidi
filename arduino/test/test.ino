#define WHICH_SENSOR 0
#define MAX_SENSOR_VALUE 850
#define MIN_SENSOR_VALUE 0
#define NUMBER_OF_SENSORS 16
#define BASE_NOTE 60
int currentPressure;
int noteOnPressure;
byte currentVelocity;
byte currentAftertouch;
bool isOn;
bool wasOn;
bool newOn;

byte getVelocity(int sensorValue) {
   byte outVal = map(sensorValue, MIN_SENSOR_VALUE, MAX_SENSOR_VALUE, 0, 127);
   return constrain(outVal, 0, 127);
}

byte getAfterTouch(int sensorValue, int noteOnSensorValue) {
   byte outVal = map(sensorValue, noteOnSensorValue, MAX_SENSOR_VALUE, 0, 127);
   return constrain(outVal, 0, 127);
}

int fsrAnalogPin = 0; 

int CONTROL0 = 10;
int CONTROL1 = 11;
int CONTROL2 = 12;
int CONTROL3 = 13;

void setPin(int inputPin)
// function to select pin on 74HC4067
{
   digitalWrite(CONTROL0, (inputPin&15)>>3); 
   digitalWrite(CONTROL1, (inputPin&7)>>2);  
   digitalWrite(CONTROL2, (inputPin&3)>>1);  
   digitalWrite(CONTROL3, (inputPin&1));
}

void setup(void)
{

  pinMode(CONTROL0, OUTPUT);
  pinMode(CONTROL1, OUTPUT);
  pinMode(CONTROL2, OUTPUT);
  pinMode(CONTROL3, OUTPUT);  

  pinMode(fsrAnalogPin, INPUT);
  
  Serial.begin(115200);

}


void loop(void)
{
    wasOn = isOn;
    setPin(WHICH_SENSOR);
    delayMicroseconds(50); 
    currentPressure = analogRead(fsrAnalogPin);
    
    newOn = currentPressure >= MIN_SENSOR_VALUE;

    if(newOn && wasOn) { // aftertouch
        currentAftertouch = getAfterTouch(currentPressure, noteOnPressure);
    }
    else if(!wasOn && newOn) {
        noteOnPressure = currentPressure;
        currentVelocity = getVelocity(noteOnPressure);
    }
    else if (wasOn && !newOn) {
        currentVelocity = getVelocity(currentPressure);
    }
   
    isOn = newOn;
    
    Serial.print(currentPressure);
    Serial.print(',');
    Serial.print(currentVelocity);
    Serial.print(',');
    Serial.print(currentAftertouch);
    Serial.print(',');
    Serial.println(isOn);
    
}
