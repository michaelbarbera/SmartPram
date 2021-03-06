/*
 * Smart Pram
 * 
 * Seat and Buckle Monitoring:
 *  Alerts user of buckle and seat presence states.
 * 
 * Hand Presence and Auto-Braking:
 *  Activates brakes when no hands are detected.
 * 
 * Created: 22 April 2018
 * 
 */ 

//---------- Libraries ----------//
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

//----------- Defines -----------//

#define DEBUG 0
#define HANDPRINT 0
#define SEATPRINT 0

#define NUMPIXELS 6 // two lots of these
#define NUMINDICATORS 2

#define BRIGHTNESS 100
#define PIXELTRANSITION 100
#define FREQ1 262 // C4
#define FREQ2 294 // D4
#define FREQ3 349 // E#4

#define PRESSURETHRESHOLD 200
#define SEATTHRESHOLD 100
#define BUZZFREQUENCY 800
#define HANDSOFFTIME 3000
#define BRAKETIME 3000
#define INITUPTIME 3000        
#define INITDOWNTIME 3000      
#define SEATTIME 2000

#define relayPinDirection 3 //2
#define relayPinPower 5 //3
#define pinLED 4 //9

#define handleIndicator 0
#define seatIndicator 1

#define pressurePin A0
#define seatPin A1
#define bucklePin 8 //5
#define seatBuzzer 12 //7
// For Brake
#define checkHands 0
#define activateBrake 1
#define releaseBrake 2

bool justStarted = true;

bool handOff_detected = false;
bool hasActuated = false;

bool massOnSeat = false;
bool buckleState = false;

bool buzzerOn = false;

unsigned long handsOffTime;
unsigned long brakeStarted;
unsigned long brakeReleaseTime;
unsigned long seatTime;

byte state = 0;

Adafruit_NeoPixel pixels;

//-------------------------------//
//------- Helper Functions ------//
void PinSetup(void) {
  pinMode(relayPinPower, OUTPUT);
  pinMode(relayPinDirection, OUTPUT);
  pinMode(pressurePin, INPUT);
  pinMode(bucklePin, INPUT);
  pinMode(seatPin, INPUT);
  pinMode(seatBuzzer, OUTPUT);
}

void InitLEDs(void) {
  bool buzzed = false;
  pixels = Adafruit_NeoPixel(NUMPIXELS*NUMINDICATORS, pinLED, NEO_GRB + NEO_KHZ800);
  pixels.setBrightness(BRIGHTNESS);
  pixels.begin();
  for(byte i=0; i < NUMPIXELS*NUMINDICATORS; i++) {
    if(i < NUMPIXELS*NUMINDICATORS*1/3) {
      pixels.setPixelColor(i, pixels.Color(255,0,0)); // blue
      if(!buzzed) {
        tone(seatBuzzer, FREQ1);
        buzzed = true;
      }
      if(i == NUMPIXELS*NUMINDICATORS*1/3-1) {
        buzzed = false;
      }
    } else if(i < NUMPIXELS*NUMINDICATORS*2/3) {
      pixels.setPixelColor(i, pixels.Color(255,255,0)); // blue
      if(!buzzed) {
        tone(seatBuzzer, FREQ2);
        buzzed = true;
      }
      if(i == NUMPIXELS*NUMINDICATORS*2/3-1) {
        buzzed = false;
      }
    } else if(i < NUMPIXELS*NUMINDICATORS) {
      pixels.setPixelColor(i, pixels.Color(0,255,0)); // blue
      if(!buzzed) {
        tone(seatBuzzer, FREQ3);
        buzzed = true;
      }
      if(i == NUMPIXELS*NUMINDICATORS-1) {
        buzzed = false;
      }
    }
    pixels.show();
    delay(PIXELTRANSITION);
  }
  noTone(seatBuzzer);
}

void OkIndicator(byte indicator) {
  for(byte i=NUMPIXELS*indicator; i<NUMPIXELS*(indicator+1); i++) {
    pixels.setPixelColor(i, pixels.Color(0,255,0)); // green
  }
  pixels.show();
}

void AlertIndicator(byte indicator) {
  for(byte i=NUMPIXELS*indicator; i<NUMPIXELS*(indicator+1); i++) {
    pixels.setPixelColor(i, pixels.Color(255,0,0)); // red
  }
  pixels.show();
}

void WarningIndicator(byte indicator) {
  for(byte i=NUMPIXELS*indicator; i<NUMPIXELS*(indicator+1); i++) {
    pixels.setPixelColor(i, pixels.Color(255,255,0)); // yellow
  }
  pixels.show();
}

void InitActuatorPosition(void) {
  ControlBrakePower(true);
  ReleaseBrake();
  delay(INITUPTIME);
  ControlBrakePower(false);
}

bool HandsOn(void) {
  int pressure = analogRead(pressurePin);
  if(HANDPRINT && DEBUG) {
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

void BrakeControl(void) {
  bool handsTouched = HandsOn();
  switch(state) {
    case checkHands:
      if(handOff_detected) {
        if(handsTouched) {
          handOff_detected = false;
          OkIndicator(handleIndicator);
        } else {
          if(millis() - handsOffTime >= HANDSOFFTIME) {
            if(DEBUG) {
              Serial.println("Activating Brake.");
            }
            state = activateBrake;
            ControlBrakePower(true);
            ActuateBrake();
            brakeStarted = millis();
            AlertIndicator(handleIndicator);
          }
        }
      } else {
        if(!handsTouched) {
          if(!hasActuated) {
            handOff_detected = true;
            handsOffTime = millis();
            WarningIndicator(handleIndicator);
          }
        } else {
          OkIndicator(handleIndicator);
          hasActuated = false;
        }
      }
      break;
    case activateBrake:
      if(millis() - brakeStarted >= BRAKETIME || handsTouched) {
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
  bool seat_status = analogRead(seatPin) > SEATTHRESHOLD;
  if(DEBUG && SEATPRINT) {
    Serial.print("Seat Status: ");
    Serial.println(seat_status);
  }
  return(seat_status);  
}

bool BuckleOn(void) {
  bool buckle_status = digitalRead(bucklePin);
  if(DEBUG && SEATPRINT) {
    Serial.print("Buckle Status: ");
    Serial.println(buckle_status);
  }
  return(buckle_status);  
}

void ControlBuzzer(bool On) {
  buzzerOn = On;
  if(On) {
    tone(seatBuzzer, BUZZFREQUENCY);
  } else {
    noTone(seatBuzzer);
  }
}

void CheckSeat(void) {
  bool currentState = OnSeat();
  bool currentBuckleState = BuckleOn();
  if(justStarted) {
    massOnSeat = !currentState;
    buckleState = !currentBuckleState;
    justStarted = false;
  }
  if(buckleState!=currentBuckleState) {
    if(!currentBuckleState && currentState) {
      seatTime = millis();
      AlertIndicator(seatIndicator);
    } else {
      OkIndicator(seatIndicator);
      ControlBuzzer(false);
    }
    buckleState = currentBuckleState;
  }
  if(massOnSeat!=currentState) {
    if(!currentState && currentBuckleState) {
      WarningIndicator(seatIndicator);
    }
    if(currentState && currentBuckleState) {
      OkIndicator(seatIndicator);
      ControlBuzzer(false);    
    }
    if(!currentState && !currentBuckleState) {
      OkIndicator(seatIndicator);
      ControlBuzzer(false);    
    }
    if(currentState && !currentBuckleState) {
      AlertIndicator(seatIndicator);
      seatTime = millis();
    }    
    massOnSeat = currentState;
  }
  if(!buzzerOn && !buckleState && massOnSeat && (millis() - seatTime >= SEATTIME)) {
    ControlBuzzer(true);
    AlertIndicator(seatIndicator);
    if (DEBUG) {
      Serial.println("Buzzer On");
    }
  }
}

//-------------------------------//
//------------ Setup ------------//
void setup() {
  Serial.begin(115200);
  while(!Serial);
  PinSetup();
  InitLEDs();
  InitActuatorPosition();
  state = 0;
}
//-------------------------------//
//------------ Loop -------------//
void loop() { 
  BrakeControl();
  CheckSeat();
}
//-------------------------------//

