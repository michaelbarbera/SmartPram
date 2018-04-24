/*
 * Smart Pram
 * 
 * Activates brakes when no hands are detected.
 * 
 * Created 22 April 2018
 * 
 */

// Improvement add some extra booleans to determine if its been retracted
 
//----------- Defines -----------//
#define PRESSURETHRESHOLD 0
#define HANDSOFFTIME 3000
#define BRAKETIME 2

#define relayPin 2
#define pressurePin A0

#define checkHands 0
#define activateBrake 1
#define releaseBrake 2

bool handOff_detected = false;

unsigned long handsOffTime;
unsigned long brakeStarted;


byte state = 0;
//-------------------------------//
//------- Helper Functions ------//
void PinSetup(void) {
  pinMode(relayPin, INPUT);
  pinMode(pressurePin, OUTPUT);
}

bool HandsOn(void) {
  int pressure = analogRead(pressurePin);
//  Serial.println(pressure);
  return(pressure > PRESSURETHRESHOLD);
}

void ActuateBrake(void) {
  digitalWrite(relayPin, HIGH);
}

void ReleaseBrake(void) {
  digitalWrite(relayPin, LOW);
}

void InitPram(void) {
  ReleaseBrake();
}

//-------------------------------//
//------------ Setup ------------//
void setup() {
  Serial.begin(115200);
  while(!Serial);
  PinSetup();
  InitPram();
  state = 0;
}
//-------------------------------//
//------------ Loop -------------//
void loop() { 
  switch(state) {
    case checkHands:
      if(handOff_detected) {
        if(HandsOn()) {
          handOff_detected = false;
        } else {
          if(millis() - handsOffTime >= HANDSOFFTIME) {
            state = activateBrake;
            ActuateBrake();
            brakeStarted = millis();
          }
        }
      } else {
        if(!HandsOn()) {
          handOff_detected = true;
          handsOffTime = millis();
        }   
      }
      break;
    case activateBrake:
      if(millis() - brakeStarted >= BRAKETIME || HandsOn()) {
        state = releaseBrake;
      } 
      break;
    case releaseBrake:
      ReleaseBrake();
      state = checkHands;
      break;
    default:
      state = checkHands;
  }
}
//-------------------------------//

