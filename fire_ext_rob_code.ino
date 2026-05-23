// ============================================================
// FIRE EXTINGUISHER ROBOT v3
// PUMP ONLY FIRES WHEN ROBOT IS CLOSE TO FLAME
// ============================================================

#include <Servo.h>

// ── Pin Definitions ─────────────────────────────────────────
const int IR_LEFT   = A2;
const int IR_CENTER = A3;
const int IR_RIGHT  = A4;

const int SERVO_PIN = 3;

// Motor A (Left wheel)
const int ENA = 11;
const int IN1 = 10;
const int IN2 = 9;

// Motor B (Right wheel)
const int ENB = 5;
const int IN3 = 6;
const int IN4 = 7;

const int PUMP_PIN = A5;

// ── Constants ───────────────────────────────────────────────
const int MOTOR_SPEED = 110;
const int TURN_SPEED  = 125;

const int SERVO_LEFT   = 45;
const int SERVO_CENTER = 90;
const int SERVO_RIGHT  = 135;

const int FIRE_THRESHOLD  = 800;
const int CLOSE_THRESHOLD = 300;  // ~15 cm from flame

// ── Spray Duration ──────────────────────────────────────────
const int SPRAY_DURATION_MS = 3000; // spray for 3 seconds

Servo fireServo;

// ============================================================
// SETUP
// ============================================================

void setup() {
  Serial.begin(9600);

  pinMode(IR_LEFT,   INPUT);
  pinMode(IR_CENTER, INPUT);
  pinMode(IR_RIGHT,  INPUT);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(PUMP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, LOW);

  fireServo.attach(SERVO_PIN);
  fireServo.write(SERVO_CENTER);
  delay(300);

  stopMotors();
  Serial.println("Warming up sensors...");
  delay(300);  // wait 2 seconds for sensors to stabilize
  Serial.println("Robot ready.");
}

// ============================================================
// LOOP
// ============================================================

void loop() {

  int leftValue   = analogRead(IR_LEFT);
  int centerValue = analogRead(IR_CENTER);
  int rightValue  = analogRead(IR_RIGHT);

  // ── Fire Detection ────────────────────────────────────────
  bool fireLeft   = (leftValue   < FIRE_THRESHOLD);
  bool fireCenter = (centerValue < FIRE_THRESHOLD);
  bool fireRight  = (rightValue  < FIRE_THRESHOLD);

  // ── Proximity Detection ───────────────────────────────────
  bool tooCloseLeft   = (leftValue   < CLOSE_THRESHOLD);
  bool tooCloseCenter = (centerValue < CLOSE_THRESHOLD);
  bool tooCloseRight  = (rightValue  <  CLOSE_THRESHOLD);
  bool tooClose = tooCloseLeft || tooCloseCenter || tooCloseRight;

  int fireSensors = (int)fireLeft + (int)fireCenter + (int)fireRight;

  // ── Serial Debug ──────────────────────────────────────────
  Serial.print("L:"); Serial.print(leftValue);
  Serial.print(" C:"); Serial.print(centerValue);
  Serial.print(" R:"); Serial.print(rightValue);
  if (tooClose)                               Serial.print(" [CLOSE]");
  if (!fireLeft && !fireCenter && !fireRight) Serial.print(" [NO FIRE]");
  Serial.println();

  // ── SERVO TRACKING ────────────────────────────────────────
  if (fireLeft && !fireRight) {
    fireServo.write(SERVO_RIGHT);
  } else if (fireRight && !fireLeft) {
    fireServo.write(SERVO_LEFT);
  } else {
    fireServo.write(SERVO_CENTER);
  }

  // ── MOVEMENT + PUMP CONTROL ───────────────────────────────
  // Pump ONLY activates when robot is stopped at close range.
  //
  //   1. tooClose OR (center + multi-sensor) → STOP + SPRAY
  //   2. Center fire only                    → move forward
  //   3. Left fire only                      → turn left
  //   4. Right fire only                     → turn right
  //   5. No fire                             → stop (no spray)

  bool shouldSpray = false;

  if (tooClose) {
    Serial.println("PROXIMITY STOP — spraying!");
    stopMotors();
    shouldSpray = true;

  } else if (fireCenter && fireSensors >= 2) {
    Serial.println("MULTI-SENSOR STOP — spraying!");
    stopMotors();
    shouldSpray = true;

  } else if (fireCenter) {
    digitalWrite(PUMP_PIN, LOW);   // NOT close yet, don't spray
    delay(100);
    moveForward();

  } else if (fireLeft) {
    digitalWrite(PUMP_PIN, LOW);
    turnLeft();

  } else if (fireRight) {
    digitalWrite(PUMP_PIN, LOW);
    turnRight();

  } else {
    digitalWrite(PUMP_PIN, LOW);
    stopMotors();
  }

  // ── RUN PUMP FOR FIXED DURATION THEN STOP ─────────────────
  if (shouldSpray) {
    digitalWrite(PUMP_PIN, HIGH);
    delay(SPRAY_DURATION_MS);
    digitalWrite(PUMP_PIN, LOW);
    Serial.println("Spray done.");

    // Small pause before resuming search
    delay(50);
  }

  delay(30);
}

// ============================================================
// MOTOR FUNCTIONS
// ============================================================

void moveForward() {
  analogWrite(ENA, MOTOR_SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  analogWrite(ENB, MOTOR_SPEED);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft() {
  analogWrite(ENA, TURN_SPEED);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  analogWrite(ENA, TURN_SPEED);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  analogWrite(ENB, TURN_SPEED);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
