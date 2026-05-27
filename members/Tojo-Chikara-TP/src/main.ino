#include <LedControl.h>

// ===== ピン定義 
const int PIN_JOY_X = A0;
const int PIN_JOY_Y = A1;
const int PIN_BUTTON = 2;
const int PIN_BUZZER = 3;
const int PIN_DIN = 11;
const int PIN_CS = 10;
const int PIN_CLK = 13;

// ===== LEDマトリクス 
LedControl lc = LedControl(PIN_DIN, PIN_CLK, PIN_CS, 1);

// ===== 状態 
int currentState = 1; // 1:待機 2:プレイ 3:ゲームオーバー

// ===== タイマー 
unsigned long lastMoveMillis = 0;
unsigned long lastEnemyMillis = 0;
unsigned long lastDrawMillis = 0;
unsigned long lastBlinkMillis = 0;

// ===== 入力 
int joyX = 512;
int joyY = 512;
int dx = 0;
int dy = 0;

// ===== 座標 
int playerX = 3;
int playerY = 3;
int enemyX = 0;
int enemyY = 0;

// ===== ボタン 
bool buttonEdge = false;
bool lastButtonState = HIGH;
unsigned long lastDebounceTime = 0;
const int DEBOUNCE_DELAY = 50;

// ===== 周期 
const int MOVE_INTERVAL = 100;
const int ENEMY_INTERVAL = 300;
const int DRAW_INTERVAL = 50;
const int BLINK_INTERVAL = 150;

// ===== 点滅 
bool enemyVisible = true;

// =========================
void setup() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);

  Serial.begin(9600);

  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);

  resetGame();
}

// =====
void loop() {

  readJoystick();
  readButtonEdge();

  unsigned long now = millis();

  // ===== 待機 
  if (currentState == 1) {
    drawWait();

    if (buttonEdge) {
      resetGame();
      currentState = 2;
    }
  }

  // ===== プレイ 
  else if (currentState == 2) {

    if (now - lastMoveMillis >= MOVE_INTERVAL) {
      movePlayer();
      lastMoveMillis = now;
    }

    if (now - lastEnemyMillis >= ENEMY_INTERVAL) {
      updateEnemy();
      lastEnemyMillis = now;
    }

    if (now - lastBlinkMillis >= BLINK_INTERVAL) {
      enemyVisible = !enemyVisible;
      lastBlinkMillis = now;
    }

    if (checkCollision()) {
      tone(PIN_BUZZER, 500, 300);
      currentState = 3;
    }

    if (now - lastDrawMillis >= DRAW_INTERVAL) {
      drawGame();
      lastDrawMillis = now;
    }
  }

  // ===== ゲームオーバー 
  else if (currentState == 3) {
    drawGameOver();

    if (buttonEdge) {
      currentState = 1;
    }
  }
}

// =====
void readJoystick() {
  joyX = analogRead(PIN_JOY_X);
  joyY = analogRead(PIN_JOY_Y);

  dx = 0;
  dy = 0;

  if (joyX > 700) dx = 1;
  else if (joyX < 300) dx = -1;
  else if (joyY > 700) dy = -1;
  else if (joyY < 300) dy = 1;
}

// =====
void readButtonEdge() {
  bool reading = digitalRead(PIN_BUTTON);
  unsigned long now = millis();

  buttonEdge = false;

  if (reading == LOW && lastButtonState == HIGH && (now - lastDebounceTime) > DEBOUNCE_DELAY) {
    buttonEdge = true;
    lastDebounceTime = now;
  }

  lastButtonState = reading;
}

// =====
void movePlayer() {
  playerX += dx;
  playerY += dy;

  if (playerX < 0) playerX = 0;
  if (playerX > 7) playerX = 7;
  if (playerY < 0) playerY = 0;
  if (playerY > 7) playerY = 7;
}

// =====
void updateEnemy() {
  enemyX++;

  if (enemyX > 7) {
    enemyX = 0;
    enemyY = random(0, 8);
  }
}

// 
bool checkCollision() {
  return (playerX == enemyX && playerY == enemyY);
}

// 
void drawGame() {
  lc.clearDisplay(0);

  // プレイヤー
  lc.setLed(0, playerY, playerX, true);

  // 敵
  if (enemyVisible) {
    lc.setLed(0, enemyY, enemyX, true);
  }
}

// 
void drawWait() {
  lc.clearDisplay(0);

  // 中央点滅
  if (millis() / 500 % 2 == 0) {
    lc.setLed(0, 3, 3, true);
  }
}

// 
void drawGameOver() {
  lc.clearDisplay(0);

  // 全点滅
  if (millis() / 300 % 2 == 0) {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        lc.setLed(0, y, x, true);
      }
    }
  }
}

// 
void resetGame() {
  playerX = 3;
  playerY = 3;

  enemyX = 0;
  enemyY = random(0, 8);

  lastMoveMillis = 0;
  lastEnemyMillis = 0;
  lastDrawMillis = 0;
}
