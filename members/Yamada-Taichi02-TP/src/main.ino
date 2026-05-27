#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#define PIN_JOYSTICK_Y A0   // ジョイスティックX軸
#define PIN_BUTTON     2    // 速度切替ボタン（INPUT_PULLUP）
#define PIN_MOTOR_1    5    // モーター制御ピン1
#define PIN_MOTOR_2    6    // モーター制御ピン2
#define PIN_LED_GREEN  13   // 緑LED

#define JOYSTICK_DEADZONE_HIGH 892
#define JOYSTICK_DEADZONE_LOW  132

#define JOYSTICK_INTERVAL_MS 50
#define BUTTON_INTERVAL_MS   50
#define MOTOR_INTERVAL_MS    50
#define CHATTERING_DELAY_MS  50

#define SPEED_COUNT 3

// 状態管理
typedef enum{
  STATE_IDLE,
  STATE_MOVE,
  STATE_ERROR
} State;
State currentState = STATE_IDLE;

// タイミング管理
unsigned long lastMillis_Joystick = 0;
unsigned long lastMillis_Button = 0;
unsigned long lastMillis_Motor = 0;
unsigned long lastChatteringTime = 0;

// 入力値・状態
int joystickInput = 0;  
bool buttonState = false;
bool prevRawPressed = false;

// 速度モード管理
int speedMode = 0; // 0: HIGH, 1: MID, 2: LOW
const int SPEED_PWM_TABLE[SPEED_COUNT] = {255, 170, 100};
const char* SPEED_LABEL_TABLE[SPEED_COUNT] = {"HIGH", "MID", "LOW"};

// 関数定義
int readJoystick();
bool readButton();
void setSpeedMode();
void showSpeedLCD();
void updateOutput();
void moveForward();
void moveBackward();
void stopMotor();
int getSpeedPwm();
void blinkErrorLED(unsigned long now);
void recoverFromError(unsigned long now);

void setup() {
  // ピンモードの設定
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_MOTOR_1, OUTPUT);
  pinMode(PIN_MOTOR_2, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);

  // 起動確認
  digitalWrite(PIN_LED_GREEN, HIGH);
  delay(1000);
  digitalWrite(PIN_LED_GREEN, LOW);

  // ディスプレイ表示
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("SPEED = ");
  showSpeedLCD();

  Serial.begin(9600);
}

void loop() {

  // 現在時間を取得
  unsigned long now = millis();

  // ジョイスティック監視
  if(now - lastMillis_Joystick >= JOYSTICK_INTERVAL_MS){

    // 値の受け取り・表示
    readJoystick();
    lastMillis_Joystick = now;
  }

  // ボタン監視
  if(now - lastMillis_Button >= BUTTON_INTERVAL_MS){
    // ボタンの値読み取り
    if(readButton()){
      setSpeedMode();
    }
    lastMillis_Button = now;
  }

  // 状態遷移・出力
  if (now - lastMillis_Motor >= MOTOR_INTERVAL_MS) {
    switch (currentState) {
      case STATE_IDLE:
        stopMotor();
        if (joystickInput < JOYSTICK_DEADZONE_LOW || joystickInput > JOYSTICK_DEADZONE_HIGH) {
          currentState = STATE_MOVE;
        }
        break;

      case STATE_MOVE:
        updateOutput();
        break;

      case STATE_ERROR:
        stopMotor();
        blinkErrorLED(now);
        recoverFromError(now);
        break;
    }

    lastMillis_Motor = now;
  }
}


// ジョイスティックの値を受け取る
int readJoystick(){
  int raw = analogRead(PIN_JOYSTICK_Y);

  if(raw >= 0 && raw <= 1023){
    joystickInput = raw;
  }
  else{
    // 異常値ならエラー遷移
    currentState = STATE_ERROR;
  }

  return joystickInput;
}

// ボタン入力を読み取る
bool readButton(){
  bool rawPressed = (digitalRead(PIN_BUTTON) == LOW);
  unsigned long now = millis();

  if(rawPressed != prevRawPressed){
    lastChatteringTime = now;
    prevRawPressed = rawPressed;
  }

  if ((now - lastChatteringTime) >= CHATTERING_DELAY_MS) {
    if (rawPressed != buttonState) {
      buttonState = rawPressed;
      if (buttonState) {
        return true;
      }
    }
  }

  return false;
}

// 速度モードを設定・LCD表示
void setSpeedMode() {
  speedMode = (speedMode + 1) % SPEED_COUNT;
  showSpeedLCD();
}

// 現在の速度モードをLCDに表示
void showSpeedLCD() {
  lcd.setCursor(8, 0);
  lcd.print("    ");
  lcd.setCursor(8, 0);

  lcd.print(SPEED_LABEL_TABLE[speedMode]);
}

// モードごとの速度を取得する
int getSpeedPwm(){
  return SPEED_PWM_TABLE[speedMode];
}

// スティック入力値をもとに分岐させる
void updateOutput(){

  // ジョイスティック値が異常ならエラー遷移
  if (joystickInput < 0 || joystickInput > 1023) {
    currentState = STATE_ERROR;
    return;
  }

  if(joystickInput < JOYSTICK_DEADZONE_LOW){
    moveForward();
  }
  else if(joystickInput > JOYSTICK_DEADZONE_HIGH){
    moveBackward();
  }
  else{
    currentState = STATE_IDLE;
  }
}

// エラー表示用のLED点滅処理
void blinkErrorLED(unsigned long now) {
  static bool ledState = false;
  static unsigned long lastBlink = 0;
  if (now - lastBlink > 300) {
    ledState = !ledState;
    digitalWrite(PIN_LED_GREEN, ledState ? HIGH : LOW);
    lastBlink = now;
  }
}

// エラー回復処理
void recoverFromError(unsigned long now) {
  static unsigned long errorButtonStart = 0;
  if (digitalRead(PIN_BUTTON) == LOW) {
    if (errorButtonStart == 0) errorButtonStart = now;
    if (now - errorButtonStart > 1000) {
      // 回復処理
      currentState = STATE_IDLE;
      digitalWrite(PIN_LED_GREEN, LOW);
      errorButtonStart = 0;
    }
  } else {
    errorButtonStart = 0;
  }
}

// モーターを前回転
void moveForward() {
  int pwm = getSpeedPwm();
  analogWrite(PIN_MOTOR_1, pwm);
  analogWrite(PIN_MOTOR_2, 0);
}

// モーターを後回転
void moveBackward() {
  int pwm = getSpeedPwm();
  analogWrite(PIN_MOTOR_1, 0);
  analogWrite(PIN_MOTOR_2, pwm);
}

// モーターを止める
void stopMotor() {
  analogWrite(PIN_MOTOR_1, 0);
  analogWrite(PIN_MOTOR_2, 0);
}