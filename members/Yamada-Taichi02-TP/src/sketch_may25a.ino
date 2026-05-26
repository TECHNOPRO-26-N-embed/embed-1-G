#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

#define PIN_JOYSTICK_Y A0   // ジョイスティックX軸
#define PIN_BUTTON     2    // 速度切替ボタン（INPUT_PULLUP）
#define PIN_MOTOR_1    5    // モーター制御ピン1
#define PIN_MOTOR_2    6    // モーター制御ピン2
#define PIN_LED_GREEN  10   // 緑LED

#define JOYSTICK_DEADZONE_HIGH 892
#define JOYSTICK_DEADZONE_LOW  132

#define JOYSTICK_INTERVAL_MS 50
#define BUTTON_INTERVAL_MS   50
#define MOTOR_INTERVAL_MS    50
#define CHATTERING_DELAY_MS  50

// 状態管理
typedef enum{
  STATE_IDLE,
  STATE_MOVE,
  STATE_ERROR
} State;
State currentState = STATE_IDLE;

typedef enum{
  SPEED_HIGH,
  SPEED_MID,
  SPEED_LOW,
  LENGTH
} Speed;
Speed speedMode = SPEED_MID;

// グローバル変数
unsigned long lastMillis_Joystick = 0;
unsigned long lastMillis_Button = 0;
unsigned long lastMillis_Motor = 0;
unsigned long lastChatteringTime = 0;

int joystickInput = 0;  // ジョイスティックの入力値
bool buttonState = false;
bool prevRawPressed = false;

// 関数定義
int readJoystick();
bool readButton();
void setSpeedMode();
void updateOutput();
void moveForward();
void moveBackward();
void stopMotor();
int getSpeedPwm();

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
  lcd.setCursor(8, 0);
  lcd.print("MID");

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

// 速度モードを設定・表示する
void setSpeedMode() {
  lcd.setCursor(8, 0);

  // 次のモードに遷移させる
  switch (speedMode) {
    case SPEED_HIGH:
      speedMode = SPEED_MID;
      lcd.print("MID ");
      break;
    case SPEED_MID:
      speedMode = SPEED_LOW;
        lcd.print("LOW ");
      break;
    case SPEED_LOW:
      speedMode = SPEED_HIGH;
      lcd.print("HIGH");
      break;
  }
}

// モードごとの速度を取得する
int getSpeedPwm(){
  switch (speedMode) {
    case SPEED_HIGH:
      return 255;
    case SPEED_MID:
      return 170;
    case SPEED_LOW:
      return 100;
  }

  return 0;
}

// スティック入力値をもとに分岐させる
void updateOutput(){
  if(joystickInput < JOYSTICK_DEADZONE_LOW){
    moveForward();
  }
  else if(joystickInput > JOYSTICK_DEADZONE_HIGH){
    moveBackward();
  }
  else{
    stopMotor();
    currentState = STATE_IDLE;
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
