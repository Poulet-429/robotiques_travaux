#include <HCSR04.h>
#include <MeAuriga.h>
#include <Wire.h>

MeUltrasonicSensor ultraSensor(PORT_10);

enum State {
  STATE_START,
  STATE_MARCHE,
  STATE_LENT,
  STATE_ARRET,
  STATE_RECULE,
  STATE_PIVOTE
};
State currentState = STATE_START;


float dist = 400;

#define ALL_LEDS 0
#define LEDNUM  12 // Auriga on-board light ring has 12 LEDs
#define LED_PIN 44

// on-board LED ring, at PORT0 (onboard)
MeRGBLed led( 0, LEDNUM );
// uint8_t red = 8 * (1 + sin(1 / 2.0 / 4.0) );
// uint8_t green = 8 * (1 + sin(1 / 1.0 / 9.0 + 2.1) );
// uint8_t blue = 8 * (1 + sin(1 / 3.0 / 14.0 + 4.2) );

int maxPwm = 255;
int turnPwm = 50;

//Motor Left
const int m1_pwm = 11;
const int m1_in1 = 48; // M1 ENA
const int m1_in2 = 49; // M1 ENB

//Motor Right
const int m2_pwm = 10;
const int m2_in1 = 47; // M2 ENA
const int m2_in2 = 46; // M2 ENB

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  led.setpin( LED_PIN );
  pinMode(m1_pwm, OUTPUT);  //We have to set PWM pin as output
  pinMode(m1_in2, OUTPUT);  //Logic pins are also set as output
  pinMode(m1_in1, OUTPUT);

  pinMode(m2_pwm, OUTPUT);  //We have to set PWM pin as output
  pinMode(m2_in2, OUTPUT);  //Logic pins are also set as output
  pinMode(m2_in1, OUTPUT);

  // test
  
    //led.setColorAt( ALL_LEDS, 0, 0, blue );
}

void loop() {
  unsigned long currentTime = millis();
  // put your main code here, to run repeatedly:
  measureDistance(currentTime);
  printInfo(currentTime);

  switch (currentState) {

    case STATE_START:
      currentState = STATE_MARCHE;
      break;

    case STATE_MARCHE:
      marche();
      break;

    case STATE_LENT:
      lent();
      break;

    case STATE_ARRET:
      arret(currentTime);
      break;

    case STATE_RECULE:
      recule(currentTime);
      break;

    case STATE_PIVOTE:
      pivote(currentTime);
      break;
  }
}

void measureDistance(unsigned long ct) {
  static unsigned long lt;
  if (ct - lt >= 1000) {
    lt = ct;
    dist = ultraSensor.distanceCm();
    Serial.println(dist);
  }
}

void printInfo(unsigned long ct) {
  static unsigned long lt;
  if (ct - lt >= 250) {
    lt = ct;
    Serial.print("Distance : ");
    Serial.println(dist);
    Serial.println(currentState);
  }
}

void marche() {
  static bool firstTime = true;
  if (firstTime) {
    firstTime = false;
    led.setColor( 0, 0, 0, 0 );
    ledTask();
    FullSpeedMode();
  }

  if (dist < 100) {
    currentState = STATE_LENT;
    firstTime = true;
  }
}

void lent() {
  static bool firstTime = true;
  if (firstTime) {
    firstTime = false;
    led.setColor( 0, 0, 0, 0 );
    ledTask();
    ReduceSpeed();
  }

  if (dist >= 100) {
    currentState = STATE_MARCHE;
    firstTime = true;
  }
  else if (dist < 30) {
    currentState = STATE_ARRET;
    firstTime = true;
  }
}

void arret(unsigned long ct) {
  static bool firstTime = true;
  static unsigned long lt = ct;
  if (firstTime) {
    firstTime = false;
    lt = ct;
    led.setColor( 0, 0, 0, 0 );
    ledTask();
      //arrete les moteurs
      Serial.println("HALT!!!");
      Stop();
  }


  if (ct - lt >= 2000) {
    lt = ct;
    currentState = STATE_RECULE;
    firstTime = true;
  }
}

void recule(unsigned long ct) {
  static bool firstTime = true;
  static unsigned long lt = ct;
  if (firstTime) {
    firstTime = false;
    lt = ct;
  //moteurs à reculon
  Reverse();
  Serial.println("poulet");
  }
  if (ct - lt >= 1000) {
    lt = ct;
    currentState = STATE_PIVOTE;
    firstTime = true;
  }
}

void pivote(unsigned long ct) {
  static bool firstTime = true;
  static unsigned long lt = ct;
  if (firstTime) {
    firstTime = false;
    lt = ct;
  TurnRight();
  }
  //tourne de 180¤ vers la gauche

  if (ct - lt >= 1000) {
    lt = ct;
      //Stop();
      firstTime = true;
      currentState = STATE_MARCHE;
  }

  }

void ledTask() {
  switch (currentState) {
    case STATE_START:
      currentState = STATE_MARCHE;
      break;

    case STATE_MARCHE:
      //La moitié arrière de l’anneau est verte.
      for (int i = 5; i < 12; i++) {
        led.setColorAt( i, 0, 10, 0 );
      }
      led.show();
      break;

    case STATE_LENT:
      //Moitié avant de l’anneau est jaune.
      for (int i = 0; i < 6; i++) {
        led.setColorAt( i, 10, 5, 0 );
      }
      led.setColorAt(11, 10, 5, 0);
      led.show();
      break;

    case STATE_ARRET:
     //L’anneau entière est rouge.
      led.setColor( ALL_LEDS, 10, 0, 0 );
      led.show();
      break;

    case STATE_RECULE:
      //L’anneau entière est rouge.
      led.setColor( ALL_LEDS, 0, 0, 0 );
      led.show();
      break;

    case STATE_PIVOTE:
      //L’anneau entière est rouge.
      break;
  }
}

//Je fais n'importe quoi

void FullSpeedMode() {
  digitalWrite(m1_in2, LOW);
  digitalWrite(m1_in1, HIGH);
  analogWrite(m1_pwm, maxPwm * 0.75);

  digitalWrite(m2_in2, LOW);
  digitalWrite(m2_in1, HIGH);
  analogWrite(m2_pwm, maxPwm * 0.75);
}

void ReduceSpeed() {
  digitalWrite(m1_in2, LOW);
  digitalWrite(m1_in1, HIGH);
  analogWrite(m1_pwm, maxPwm * 0.5);  //Set speed via PWM

  digitalWrite(m2_in2, LOW);
  digitalWrite(m2_in1, HIGH);
  analogWrite(m2_pwm, maxPwm * 0.5);  //Set speed via PWM
}

void Stop() {
  analogWrite(m1_pwm, 0);
  analogWrite(m2_pwm, 0);
}

void TurnRight() {
  digitalWrite(m1_in2, LOW);
  digitalWrite(m1_in1, HIGH);
  analogWrite(m1_pwm, turnPwm);  //Set speed via PWM

  digitalWrite(m2_in2, HIGH);
  digitalWrite(m2_in1, LOW);
  analogWrite(m2_pwm, turnPwm);  //Set speed via PWM
}

void Reverse() {
  digitalWrite(m1_in2, HIGH);
  digitalWrite(m1_in1, LOW);
  analogWrite(m1_pwm, maxPwm * 0.3);

  digitalWrite(m2_in2, HIGH);
  digitalWrite(m2_in1, LOW);
  analogWrite(m2_pwm, maxPwm * 0.3);
}
