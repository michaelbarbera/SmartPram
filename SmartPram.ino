/*
 * Smart Pram
 * 
 * Activates brakes when no hands are detected.
 * 
 * Created 22 April 2018
 * 
 * TODO: Create initialisation function for position of the actuator.
 * 
 */ 
//----------- Defines -----------//

#define DEBUG 1
#define HANDPRINT 1

#define PRESSURETHRESHOLD 100
#define SEATTHRESHOLD 0
#define BUZZFREQUENCY 2000
#define HANDSOFFTIME 3000
#define BRAKETIME 5000
#define SEATTIME 4000

#define relayPinDirection 2
#define relayPinPower 3
#define handLED 5
#define pressurePin A0
#define seatPin A1
#define bucklePin 4
#define seatLED 6
#define seatBuzzer 7
// For Brake
#define checkHands 0
#define activateBrake 1
#define releaseBrake 2

bool handOff_detected = false;
bool hasActuated = false;

bool massOnSeat = false;

unsigned long handsOffTime;
unsigned long brakeStarted;
unsigned long brakeReleaseTime;
unsigned long brakeReleasedTime;
unsigned long seatTime;

byte state = 0;
//-------------------------------//
//------- Helper Functions ------//
void PinSetup(void) {
  pinMode(relayPinPower, OUTPUT);
  pinMode(relayPinDirection, OUTPUT);
  pinMode(pressurePin, INPUT);
  pinMode(bucklePin, INPUT);
  pinMode(seatPin, INPUT);
  pinMode(seatLED, OUTPUT);
  pinMode(handLED, OUTPUT);
  pinMode(seatBuzzer, OUTPUT);
}

bool HandsOn(void) {
  int pressure = analogRead(pressurePin);
  if(HANDPRINT && DEBUG && (pressure > PRESSURETHRESHOLD)) {
    Serial.println(pressure);
  }
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
  bool handsTouched = HandsOn();
  digitalWrite(handLED, handsTouched); // maybe make this a bit smarter
  switch(state) {
    case checkHands:
      if(handOff_detected) {
        if(handsTouched) {
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
        if(!handsTouched) {
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
      if(millis() - brakeStarted >= BRAKETIME || handsTouched) {
        if(handsTouched) {
          brakeReleasedTime = millis() - brakeStarted;
        } else {
          brakeReleasedTime = BRAKETIME;
        }
        if(DEBUG) {
          Serial.println("Begin Retracting Brake.");
        }
        state = releaseBrake;
        ReleaseBrake();
        brakeReleaseTime = millis();
      } 
      break;
    case releaseBrake:
      if(millis() - brakeReleaseTime >= brakeReleasedTime) {
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

bool BuckleOn(void) {
  return(digitalRead(bucklePin));  
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
  bool buckleState = BuckleOn();
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
  if(!buckleState && massOnSeat && (millis() - seatTime >= SEATTIME)) {
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

