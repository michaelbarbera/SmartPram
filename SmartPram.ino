/*
 * Smart Pram
 * 
 * Activates brakes when no hands are detected.
 * 
 * Created 22 April 2018
 * 
 */ 
//----------- Defines -----------//

#define DEBUG 1

#define PRESSURETHRESHOLD 10
#define SEATTHRESHOLD 0
#define BUZZFREQUENCY 2000
#define HANDSOFFTIME 3000
#define BRAKETIME 2000
#define SEATTIME 4000

#define relayPinDirection 2
#define relayPinPower 3
#define handLED 4
#define pressurePin A0
#define seatPin A1
#define seatLED 5
#define seatBuzzer 6
// For Brake
#define checkHands 0
#define activateBrake 1
#define releaseBrake 2

bool handOff_detected = false;
// Improvement add some extra booleans to determine if its been retracted
bool hasActuated = false;

bool massOnSeat = false;

unsigned long handsOffTime;
unsigned long brakeStarted;
unsigned long brakeReleaseTime;
unsigned long seatTime;

byte state = 0;
//-------------------------------//
//------- Helper Functions ------//
void PinSetup(void) {
  pinMode(relayPinPower, OUTPUT);
  pinMode(relayPinDirection, OUTPUT);
  pinMode(pressurePin, INPUT);
  pinMode(seatPin, INPUT);
  pinMode(seatLED, OUTPUT);
  pinMode(handLED, OUTPUT);
  pinMode(seatBuzzer, OUTPUT);
}

bool HandsOn(void) {
  int pressure = analogRead(pressurePin);
//  Serial.println(pressure);
  return(pressure > PRESSURETHRESHOLD);
}

void ControlBrakePower(bool On) {
  if(On) {
    if(DEBUG) {
      Serial.println("Brake Powered ON");
    }
    digitalWrite(relayPinPower, HIGH);
  } else {
    if(DEBUG) {
      Serial.println("Brake Powered OFF");
    }
    digitalWrite(relayPinPower, LOW);
  }
}

void ActuateBrake(void) {
  if(DEBUG) {
    Serial.println("Brake Acuated");
  }
  digitalWrite(relayPinDirection, HIGH);
}

void ReleaseBrake(void) {
  if(DEBUG) {
    Serial.println("Brake Released");
  }
  digitalWrite(relayPinDirection, LOW);
}

void InitPram(void) {
  //ReleaseBrake();
}

void BrakeControl(void) {
  digitalWrite(handLED, HandsOn()); // maybe make this a bit smarter
  switch(state) {
    case checkHands:
      if(handOff_detected) {
        if(HandsOn()) {
          handOff_detected = false;
        } else {
          if(millis() - handsOffTime >= HANDSOFFTIME) {
            if(DEBUG) {
              Serial.println("Activating Brake.");
            }
            state = activateBrake;
            ControlBrakePower(true);
            ActuateBrake();
            brakeStarted = millis();
          }
        }
      } else {
        if(!HandsOn()) {
          if(!hasActuated) {
            handOff_detected = true;
            handsOffTime = millis();
          }
        } else {
          hasActuated = false;
        }
      }
      break;
    case activateBrake:
      if(millis() - brakeStarted >= BRAKETIME || HandsOn()) {
        if(DEBUG) {
          Serial.println("Begin Retracting Brake.");
        }
        state = releaseBrake;
        ReleaseBrake();
        brakeReleaseTime = millis();
      } 
      break;
    case releaseBrake:
      if(millis() - brakeReleaseTime >= BRAKETIME) {
        if(DEBUG) {
          Serial.println("Brake Released.");
        }
        ControlBrakePower(false);
        state = checkHands;
        hasActuated = true;
        handOff_detected = false;
      }
      break;
    default:
      state = checkHands;
  }  
}

bool OnSeat(void) {
  return(analogRead(pressurePin) > SEATTHRESHOLD);  
}

void ControlBuzzer(bool On) {
  if(On) {
    tone(seatBuzzer, BUZZFREQUENCY);
  } else {
    noTone(seatBuzzer);
  }
}

void CheckSeat(void) {
  bool currentState = OnSeat();
  if(massOnSeat!=currentState) {
    if(currentState) {
      seatTime = millis();
      digitalWrite(seatLED, HIGH);
    } else {
      digitalWrite(seatLED, LOW);
      ControlBuzzer(false);
    }
    massOnSeat = currentState;
  }
  if(massOnSeat && (millis() - seatTime >= SEATTIME)) {
    ControlBuzzer(true);
  }
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
  BrakeControl();
//  CheckSeat();
}
//-------------------------------//

