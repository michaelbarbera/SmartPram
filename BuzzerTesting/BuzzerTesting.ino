#define BUZZFREQUENCY 800
const int seatBuzzer = 7; //buzzer to arduino pin 9

void setup() {
  // put your setup code here, to run once:
  pinMode(seatBuzzer, OUTPUT); // Set buzzer - pin 9 as an output
}

void loop() {
  // put your main code here, to run repeatedly:
  ControlBuzzer(true);
}

void ControlBuzzer(bool On) {
  if (On) {
    tone(seatBuzzer, BUZZFREQUENCY);
  } else {
    noTone(seatBuzzer);
  }
}
