int fsrAnalogPin = 0; 
int muxValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};

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
  for (int i = 0; i < 16; i++)
  {
    setPin(i);
    delayMicroseconds(50); 
    muxValues[i] = analogRead(fsrAnalogPin);
  }

  for (int i = 0; i < 15; i++)
  {
    Serial.print(muxValues[i]);
    Serial.print(',');
  }

  Serial.println(muxValues[15]);
  
}
