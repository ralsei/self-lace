#include <AutoPID.h>

/// PIN DEFINITIONS
const int FSR_F_PIN = A0; // the actual force sensor
const int FSR_S_PIN = A1; // is there a shoe?
const int POT_PIN = A2;   // change force

const int MOTOR_1A_PIN = 5; // hbridge leg 1
const int MOTOR_2A_PIN = 6; // hbridge leg 2
const int RELEASE_PIN = 4;
const int LIMIT_PIN = 7;

const int LED_ON_PIN = 2;
const int LED_STAT_PIN = 3;
const int LED_FWD_PIN = 8;
const int LED_BKW_PIN = 9;

/// GLOBAL STATES
bool output = false;
bool wasPressed = false;
bool relayState = false;

/// PID 
const int Kp = 1, Ki = 0, Kd = 0;
double m_setpoint, m_input, m_output;
AutoPID m_control{&m_input, &m_setpoint, &m_output, -255, 255, Kp, Ki, Kd};

void motorCtrl_write(int val) { // set motor controller to given power
  if (val > 0) {
    analogWrite(MOTOR_1A_PIN, val);
    analogWrite(MOTOR_2A_PIN, 0);

    digitalWrite(LED_FWD_PIN, HIGH);
    digitalWrite(LED_BKW_PIN, LOW);
  } else if (val < 0) {
    analogWrite(MOTOR_2A_PIN, -val);
    analogWrite(MOTOR_1A_PIN, 0);

    digitalWrite(LED_FWD_PIN, LOW);
    digitalWrite(LED_BKW_PIN, HIGH);
  } else {
    analogWrite(MOTOR_1A_PIN, 0);
    analogWrite(MOTOR_2A_PIN, 0);

    digitalWrite(LED_FWD_PIN, LOW);
    digitalWrite(LED_BKW_PIN, LOW);
  }
}

bool fsr_isDetected() { // is the bottom FSR detecting a human?
  return (analogRead(FSR_S_PIN) > 100); // threshold for mg
}

int fsr_getAnalog() { // returns the raw value of the force FSR
  return analogRead(FSR_F_PIN);
}

int fsr_getNewtons() { // "should" (RFC 2119) return the FSR value in Newtons
  int fsrVoltage = map(fsr_getAnalog(), 0, 1023, 0, 5000); // 0-1023 to 0-5V (5000 mV)
  long fsrForce;

  if (fsrVoltage == 0) {
    return 0; // no net force on the sensor
  } else {
    // the voltage = vcc * R / (R + FSR) where R = 10K and Vcc = 5V
    // so FSR = ((Vcc - V) * R) / V
    unsigned long fsrResistance = 5000 - fsrVoltage;
    fsrResistance *= 10000;
    fsrResistance /= fsrVoltage;

    unsigned long fsrConductance = 1000000; // microMhos
    fsrConductance /= fsrResistance;

    // fsr graphs from datasheet: https://cdn-shop.adafruit.com/datasheets/FSR400Series_PD.pdf
    if (fsrConductance <= 1000) {
      fsrForce = fsrConductance / 80;
    } else {
      fsrForce = fsrConductance - 1000;
      fsrForce /= 30;
    }
  }

  return fsrForce;
}

int pot_getAnalog() {
  return analogRead(POT_PIN);
}

int pot_getSetpoint() {
  return map(pot_getAnalog(), 0, 1023, 0, 50); // last value is threshold
}

void setup() {
  pinMode(MOTOR_1A_PIN, OUTPUT);
  pinMode(MOTOR_2A_PIN, OUTPUT);

  pinMode(LED_ON_PIN, OUTPUT);
  pinMode(LED_STAT_PIN, OUTPUT);
  pinMode(LED_FWD_PIN, OUTPUT);
  pinMode(LED_BKW_PIN, OUTPUT);

  m_input = fsr_getNewtons();
  m_setpoint = 15; // default
  
  Serial.begin(9600);
  m_control.setTimeStep(50);
}

void loop() {
  digitalWrite(LED_ON_PIN, HIGH);

  m_input = fsr_getAnalog();
  if (wasPressed) {
    m_setpoint = 0;
  } else {
    m_setpoint = pot_getAnalog();
  }

  if (fsr_isDetected()) {
    output = true;
  } 

  if (digitalRead(RELEASE_PIN)) {
    wasPressed = true;
  } else {
    wasPressed = false;
  }

  if (digitalRead(LIMIT_PIN) == 1) {
    output = false;
    m_output = 0;
  }

  if (output) {
    m_control.run();
  }
  motorCtrl_write(m_output);

  Serial.print("FSR value (N): ");
  Serial.print(m_input);
  Serial.print("Attempted output: ");
  Serial.print(m_output);
  Serial.print("Pot value: ");
  Serial.println(pot_getAnalog());
  delay(100); // i have no clue why this works. but it does. so leave me alone...
              // also it helps with serial debug... i guess...
}
