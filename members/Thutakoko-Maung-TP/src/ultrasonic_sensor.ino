#include <Servo.h>

Servo myServo;

// -----------------------------
// PIN DEFINITIONS
// -----------------------------
const int xPin = A0;       // Joystick X-axis
const int swPin = 2;       // Joystick button (reset)
const int servoPin = 3;    // Servo motor
const int ledPin = 6;      // Alert LED
const int buzzerPin = 8;   // Buzzer
const int trigPin = 9;     // Ultrasonic trigger
const int echoPin = 10;    // Ultrasonic echo

// -----------------------------
// VARIABLES
// -----------------------------
long duration;
int distance;
int servoAngle = 90;
bool alertState = false;

// -----------------------------
// TIMING
// -----------------------------
unsigned long lastServoMove = 0;
unsigned long servoInterval = 10;

unsigned long lastAlertBlink = 0;
unsigned long alertInterval = 200;
bool buzzerState = false;

// -----------------------------
// BUTTON DEBOUNCE
// -----------------------------
int buttonState = HIGH;
int lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

void setup() {

    myServo.attach(servoPin);

    pinMode(swPin, INPUT_PULLUP);
    pinMode(ledPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    Serial.begin(9600);
    delay(500);   // IMPORTANT: allow Serial to stabilize

    myServo.write(servoAngle);

    Serial.println("System Started...");
}

void loop() {

    handleJoystick();
    handleResetButton();
    distance = readUltrasonic();
    handleAlert(distance);
}

// =====================================================
// JOYSTICK SERVO CONTROL
// =====================================================
void handleJoystick() {

    int xValue = analogRead(xPin);
    unsigned long now = millis();

    if (now - lastServoMove >= servoInterval) {

        int previousAngle = servoAngle;

        if (xValue < 400 && servoAngle > 0) {
            servoAngle--;
        }
        else if (xValue > 600 && servoAngle < 180) {
            servoAngle++;
        }

        if (servoAngle != previousAngle) {

            myServo.write(servoAngle);

            if (servoAngle < previousAngle) {
                Serial.print("Left | Angle: ");
            } else {
                Serial.print("Right | Angle: ");
            }

            Serial.println(servoAngle);
        }

        lastServoMove = now;
    }
}

// =====================================================
// BUTTON RESET (with debounce)
// =====================================================
void handleResetButton() {

    int reading = digitalRead(swPin);

    if (reading != lastButtonState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {

        if (reading != buttonState) {
            buttonState = reading;

            if (buttonState == LOW) {
                servoAngle = 90;
                myServo.write(servoAngle);
                Serial.println("Joystick Button Pressed → Servo Reset to 90°");
            }
        }
    }

    lastButtonState = reading;
}

// =====================================================
// ULTRASONIC SENSOR READING
// =====================================================
int readUltrasonic() {

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH, 25000); // timeout added

    delay(50);  // IMPORTANT for real HC-SR04 stability

    return duration * 0.034 / 2;
}

// =====================================================
// ALERT SYSTEM (LED + BUZZER + DISTANCE PRINT)
// =====================================================
void handleAlert(int distance) {

    unsigned long now = millis();
    static int lastDistance = -1;

    if (distance > 0 && distance <= 20) {

        digitalWrite(ledPin, HIGH);

        if (now - lastAlertBlink >= alertInterval) {

            buzzerState = !buzzerState;

            if (buzzerState) tone(buzzerPin, 1000);
            else noTone(buzzerPin);

            lastAlertBlink = now;
        }

        if (abs(distance - lastDistance) >= 2) {
            Serial.print("Distance: ");
            Serial.print(distance);
            Serial.println(" cm");
            lastDistance = distance;
        }

        alertState = true;
    }

    else {

        digitalWrite(ledPin, LOW);
        noTone(buzzerPin);

        if (alertState) {
            Serial.println("✓ Area is safe again.");
        }

        alertState = false;
        lastDistance = -1;
    }
}
