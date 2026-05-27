// Snake Game for Arduino (Yuda-Katsunori-TP)
// ---
// 必要なライブラリ
#include <LedControl.h>    // MAX7219用
#include <Wire.h>          // I2C用
#include <LiquidCrystal_I2C.h>

// ピン定義
#define PIN_JOY_X A0
#define PIN_JOY_Y A1
#define PIN_JOY_BTN 2
#define PIN_LED_CS 11
#define PIN_LED_CLK 10
#define PIN_LED_DIN 12
#define PIN_BUZZER_PASSIVE 5
#define PIN_BUZZER_ACTIVE 6
#define PIN_LCD_SDA A4
#define PIN_LCD_SCL A5

// 定数
const int DEBOUNCE_DELAY = 50;
const int FIELD_SIZE = 8;
const int MAX_SNAKE = 16;
const int MIN_SPEED_LEVEL = 1;
const int MAX_SPEED_LEVEL = 5;
const unsigned long SPEED_ADJUST_DELAY = 180;

// グローバル変数
int currentState = 0; // 0:待機 1:ゲーム中 2:ゲームオーバー
unsigned long lastMoveMillis = 0;
int moveInterval = 300;
int joyX = 0, joyY = 0;
bool joyBtn = false;
bool prevJoyBtn = false;
int snake[MAX_SNAKE][2];
int snakeLength = 3;
int foodPos[2];
int score = 0;
int highScore = 0;
int highScoreByStartLevel[MAX_SPEED_LEVEL + 1] = {0};
int gameStartLevel = 3;
bool buzzerFlag = false;
char lcdText[16];
unsigned long lastDebounceTime = 0;
// 進行方向（dx,dy）
int dirX = 1; // 初期は右
int dirY = 0;
int speedLevel = 3;
int currentLevel = 3;
unsigned long lastSpeedAdjustMillis = 0;
int bgmDuration = 250; // BGMのテンポを変更可能に

// レベルアップ演出管理用
bool isLevelUpEffect = false;
unsigned long levelUpEffectStart = 0;
int levelUpEffectPhase = 0; // 0:未実行 1:LCD表示中 2:効果音中

// ライブラリインスタンス
LedControl lc = LedControl(PIN_LED_DIN, PIN_LED_CLK, PIN_LED_CS, 1);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// 関数プロトタイプ

void setup();
void loop();
void readJoystick();
void moveSnake();
void spawnFood();
int checkCollision();
void addScore();
void gameOver();
void updateLCD();
void playBGM();
void levelUp();
void drawField();

extern int bgmIndex;
extern unsigned long lastBgmMillis;

// ---
void setup() {
  pinMode(PIN_JOY_X, INPUT);
  pinMode(PIN_JOY_Y, INPUT);
  pinMode(PIN_JOY_BTN, INPUT_PULLUP);
  pinMode(PIN_BUZZER_PASSIVE, OUTPUT);
  pinMode(PIN_BUZZER_ACTIVE, OUTPUT);
  lc.shutdown(0, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  lc.setRow(0, 0, B11111111); // タイトル表示例
  tone(PIN_BUZZER_PASSIVE, 1000, 100);
  delay(500);
  lc.clearDisplay(0);
  spawnFood();
}

void loop() {
  readJoystick();
  unsigned long now = millis();
  bool pressed = (!prevJoyBtn && joyBtn);

  // レベルアップ演出中は専用処理
  if (isLevelUpEffect) {
    if (levelUpEffectPhase == 1) {
      // LCD表示中
      if (now - levelUpEffectStart >= 1000) {
        updateLCD();
        tone(PIN_BUZZER_PASSIVE, 1400, 120); // 効果音（高音と長さを抑えて聴感上の大きさを軽減）
        levelUpEffectStart = now;
        levelUpEffectPhase = 2;
      }
    } else if (levelUpEffectPhase == 2) {
      // 効果音中
      if (now - levelUpEffectStart >= 120) {
        isLevelUpEffect = false;
        levelUpEffectPhase = 0;
      }
    }
    prevJoyBtn = joyBtn;
    return; // 通常処理をスキップ
  }

  switch (currentState) {
    case 0: // 待機
      lc.setRow(0, 0, B11111111); // タイトル表示
      if (now - lastSpeedAdjustMillis > SPEED_ADJUST_DELAY) {
        if (joyY < 400 && speedLevel < MAX_SPEED_LEVEL) {
          speedLevel++;
          lastSpeedAdjustMillis = now;
          tone(PIN_BUZZER_PASSIVE, 1200, 40);
        } else if (joyY > 600 && speedLevel > MIN_SPEED_LEVEL) {
          speedLevel--;
          lastSpeedAdjustMillis = now;
          tone(PIN_BUZZER_PASSIVE, 900, 40);
        }
      }
      if (!isLevelUpEffect) {
        lcd.setCursor(0, 0);
        lcd.print("Push to Start   ");
        lcd.setCursor(0, 1);
        lcd.print("Speed:");
        lcd.print(speedLevel);
        lcd.print(" (1-5)   ");
      }
      if (pressed) {
        currentState = 1;
        gameStartLevel = speedLevel;
        currentLevel = gameStartLevel;
        snakeLength = 3;
        score = 0;
        highScore = highScoreByStartLevel[gameStartLevel];
        bgmIndex = 0;
        lastBgmMillis = 0;
        bgmDuration = 250; // BGMテンポを初期化
        moveInterval = 420 - (speedLevel * 60);
        spawnFood();
        for (int i = 0; i < snakeLength; i++) {
          snake[i][0] = FIELD_SIZE / 2 - i;
          snake[i][1] = FIELD_SIZE / 2;
        }
        // 進行方向を初期化（右）
        dirX = 1;
        dirY = 0;
        lc.clearDisplay(0);
        drawField();
      }
      break;
    case 1: // ゲーム中
      playBGM();
      if (now - lastMoveMillis >= moveInterval) {
        lastMoveMillis = now;
        moveSnake();
        int col = checkCollision();
        if (col == 1) { // エサ
          addScore();
          spawnFood();
          tone(PIN_BUZZER_PASSIVE, 1500, 80);
        } else if (col == 2) { // 壁/自身
          gameOver();
        }
        drawField();
        if (!isLevelUpEffect) {
          updateLCD();
        }
      }
      break;
    case 2: // ゲームオーバー
      lc.setRow(0, 0, B10011001); // GAME OVER表示例
      tone(PIN_BUZZER_ACTIVE, 500, 300);
      if (pressed) {
        currentState = 0;
        lc.clearDisplay(0);
      }
      break;
  }
  prevJoyBtn = joyBtn;
}

void readJoystick() {
  joyX = analogRead(PIN_JOY_X);
  joyY = analogRead(PIN_JOY_Y);
  bool btn = !digitalRead(PIN_JOY_BTN);
  if (btn != joyBtn && (millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    joyBtn = btn;
    lastDebounceTime = millis();
  }
  // 進行方向の更新（ジョイスティックがしきい値を超えたときのみ）
  if (currentState == 1) {
    if (joyX < 400 && dirX != 1) { dirX = -1; dirY = 0; } // 左
    else if (joyX > 600 && dirX != -1) { dirX = 1; dirY = 0; } // 右
    else if (joyY < 400 && dirY != 1) { dirX = 0; dirY = -1; } // 上（Y軸反転）
    else if (joyY > 600 && dirY != -1) { dirX = 0; dirY = 1; } // 下（Y軸反転）
  }
}

void moveSnake() {
  // 進行方向で進む
  int newX = snake[0][0] + dirX;
  int newY = snake[0][1] + dirY;
  for (int i = snakeLength - 1; i > 0; i--) {
    snake[i][0] = snake[i - 1][0];
    snake[i][1] = snake[i - 1][1];
  }
  snake[0][0] = newX;
  snake[0][1] = newY;
}

void spawnFood() {
  bool valid = false;
  while (!valid) {
    foodPos[0] = random(0, FIELD_SIZE);
    foodPos[1] = random(0, FIELD_SIZE);
    valid = true;
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i][0] == foodPos[0] && snake[i][1] == foodPos[1]) {
        valid = false;
        break;
      }
    }
  }
}

int checkCollision() {
  // 壁
  if (snake[0][0] < 0 || snake[0][0] >= FIELD_SIZE || snake[0][1] < 0 || snake[0][1] >= FIELD_SIZE) return 2;
  // 自身
  for (int i = 1; i < snakeLength; i++) {
    if (snake[0][0] == snake[i][0] && snake[0][1] == snake[i][1]) return 2;
  }
  // エサ
  if (snake[0][0] == foodPos[0] && snake[0][1] == foodPos[1]) return 1;
  return 0;
}

void addScore() {
  score++;
  if (score > highScoreByStartLevel[gameStartLevel]) {
    highScoreByStartLevel[gameStartLevel] = score;
    highScore = score;
  }
  if (score % 5 == 0) levelUp();
  if (snakeLength < MAX_SNAKE) snakeLength++;
}

void gameOver() {
  currentState = 2;
  noTone(PIN_BUZZER_PASSIVE);
  tone(PIN_BUZZER_ACTIVE, 500, 300);
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Score:");
  lcd.print(score);
  lcd.print(" Lv:");
  lcd.print(currentLevel);
  lcd.setCursor(0, 1);
  lcd.print("Hi:");
  lcd.print(highScore);
}

// BGMデータ（音階確認テストデータを本実装化）
const int NOTE_D4 = 294;   // 低いレ
const int NOTE_G4 = 392;   // ソ
const int NOTE_A4 = 440;   // ラ
const int NOTE_B4 = 494;   // シ
const int NOTE_C5 = 523;   // ド
const int NOTE_D5 = 587;   // レ
const int NOTE_E5 = 659;   // ミ
const int NOTE_F5_S = 740; // ファ#
const int NOTE_G5 = 784;   // 高いソ

const int NUM_TONES = 49;
const int TONES[NUM_TONES] = {
  // フレーズ1：ソラシレドドミレレ
  NOTE_G4, NOTE_A4, NOTE_B4, NOTE_D5, NOTE_C5, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_D5,
  // フレーズ2：ソ(高)ファ#ソレシソラシド
  NOTE_G5, NOTE_F5_S, NOTE_G5, NOTE_D5, NOTE_B4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5,
  // フレーズ3：レミレドシラシソ
  NOTE_D5, NOTE_E5, NOTE_D5, NOTE_C5, NOTE_B4, NOTE_A4, NOTE_B4, NOTE_G4,
  // フレーズ4：ソラシドレシラソレ
  NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5, NOTE_D5, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_D4,
  // フレーズ5：ソラシレドドミレレ
  NOTE_G4, NOTE_A4, NOTE_B4, NOTE_D5, NOTE_C5, NOTE_C5, NOTE_E5, NOTE_D5, NOTE_D5,
  // フレーズ6：ソ(高)ファ#ソレシ
  NOTE_G5, NOTE_F5_S, NOTE_G5, NOTE_D5, NOTE_B4
};

int bgmIndex = 0;
unsigned long lastBgmMillis = 0;

void playBGM() {
  unsigned long now = millis();
  if (now - lastBgmMillis >= bgmDuration) {
    lastBgmMillis = now;
    if (TONES[bgmIndex] > 0) {
      tone(PIN_BUZZER_PASSIVE, TONES[bgmIndex], bgmDuration * 0.8);
    } else {
      noTone(PIN_BUZZER_PASSIVE);
    }
    bgmIndex = (bgmIndex + 1) % NUM_TONES;
  }
}

void levelUp() {
  if (currentLevel < MAX_SPEED_LEVEL) currentLevel++;
  if (moveInterval > 100) moveInterval -= 40;

  // LCDに"Level UP"を表示
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level UP!");
  isLevelUpEffect = true;
  levelUpEffectStart = millis();
  levelUpEffectPhase = 1;

  // BGMのテンポを速くする
  if (bgmDuration > 100) {
    bgmDuration -= 20; // テンポを少し速く
  }
}

// スネークとエサをLEDマトリクスに描画
void drawField() {
  lc.clearDisplay(0);
  // スネーク本体
  for (int i = 0; i < snakeLength; i++) {
    int x = snake[i][0];
    int y = snake[i][1];
    if (x >= 0 && x < FIELD_SIZE && y >= 0 && y < FIELD_SIZE) {
      lc.setLed(0, y, x, true);
    }
  }
  // エサ
  int fx = foodPos[0];
  int fy = foodPos[1];
  if (fx >= 0 && fx < FIELD_SIZE && fy >= 0 && fy < FIELD_SIZE) {
    lc.setLed(0, fy, fx, true);
  }
}
