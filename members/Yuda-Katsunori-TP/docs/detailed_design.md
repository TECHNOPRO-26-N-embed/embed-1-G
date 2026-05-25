# 詳細設計書 - 組込み開発実習

<!-- 作成者: Yuda-Katsunori-TP / 日付: 2026-05-22 / グループ: embed-1-G -->

> このドキュメントは、src/snake.ino の実装内容に合わせて更新した詳細設計です。

---

## 0. 基本設計書との接続確認

| 項目 | 内容 |
|:--|:--|
| 作品タイトル | レトロ風・ドット絵ミニゲーム機（スネークゲーム） |
| 状態の種類 | 待機中、ゲーム中、ゲームオーバー |
| 実装関数数 | 12個（setup, loop, readJoystick, moveSnake, spawnFood, checkCollision, addScore, gameOver, updateLCD, playBGM, levelUp, drawField） |
| グローバル変数の合計バイト数 | 約120B（主要変数のみ、ライブラリ内部領域は除く） |

---

## 1. グローバル変数・定数の設計

```
【ピン定義】
PIN_JOY_X = A0
PIN_JOY_Y = A1
PIN_JOY_BTN = 2
PIN_LED_CS = 10
PIN_LED_CLK = 11
PIN_LED_DIN = 12
PIN_BUZZER_PASSIVE = 5
PIN_BUZZER_ACTIVE = 6
PIN_LCD_SDA = A4
PIN_LCD_SCL = A5

【固定値】
DEBOUNCE_DELAY : const int = 50
FIELD_SIZE : const int = 8
MAX_SNAKE : const int = 16
MIN_SPEED_LEVEL : const int = 1
MAX_SPEED_LEVEL : const int = 5
SPEED_ADJUST_DELAY : const unsigned long = 180

【状態管理】
currentState : int = 0   // 0:待機 1:ゲーム中 2:ゲームオーバー

【タイマー】
lastMoveMillis : unsigned long = 0
lastDebounceTime : unsigned long = 0
lastSpeedAdjustMillis : unsigned long = 0
lastBgmMillis : unsigned long = 0

【入力値】
joyX : int = 0
joyY : int = 0
joyBtn : bool = false
prevJoyBtn : bool = false

【ゲームデータ】
snake[MAX_SNAKE][2] : int
snakeLength : int = 3
foodPos[2] : int
dirX : int = 1
dirY : int = 0
score : int = 0
highScore : int = 0
moveInterval : int = 300
speedLevel : int = 3

【音関連】
buzzerFlag : bool = false
bgmIndex : int = 0
bgmTempo : int = 180
melody[] : const int = {262, 294, 330, 349, 392, 440, 494, 523}
noteDurations[] : const int = {4, 4, 4, 4, 4, 4, 4, 4}

【表示用】
lcdText[16] : char
```

---

## 2. 各関数の詳細設計

### setup() - 初期化処理

```
【処理の流れ】
1. 各ピンモード設定
   - ジョイスティックX/Y: INPUT
   - ジョイスティック押し込み: INPUT_PULLUP
   - ブザー2系統: OUTPUT

2. LEDマトリクス初期化
   - shutdown(false), intensity設定, clearDisplay

3. LCD初期化
   - lcd.init(), lcd.backlight()

4. Serial.begin(9600)

5. 起動演出
   - LED先頭行を点灯
   - パッシブブザー短音
   - 500ms待機後に表示クリア

6. 初期エサ配置
   - spawnFood() を呼ぶ
```

### loop() - メインループ

```
【処理の流れ】
1. 毎ループで readJoystick() を実行
2. now = millis() を取得
3. joyBtnの立ち上がり検出
   - pressed = (!prevJoyBtn && joyBtn)

4. 状態分岐
  (A) currentState == 0（待機）
      - LEDにタイトル行を表示
      - joyY上下で speedLevel(1-5) を調整
      - LCDに "Push to Start" と速度表示
      - pressed でゲーム開始初期化
        ・snakeLength=3, score=0
        ・moveInterval = 420 - speedLevel*60
        ・spawnFood(), スネーク初期座標配置
        ・進行方向を右(1,0)へ初期化
        ・LEDをクリアして drawField()

  (B) currentState == 1（ゲーム中）
      - playBGM() を実行
      - moveInterval周期で moveSnake()
      - checkCollision() の結果で分岐
        ・1（エサ）: addScore(), spawnFood(), 効果音
        ・2（壁/自身）: gameOver()
      - drawField(), updateLCD()

  (C) currentState == 2（ゲームオーバー）
      - LEDにゲームオーバーパターン表示
      - アクティブブザーを鳴動
      - pressedで待機状態へ戻す

5. ループ末尾で prevJoyBtn = joyBtn
```

### readJoystick() - ジョイスティック入力取得と方向更新

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. A0/A1のアナログ値を joyX/joyY に取得
2. 押し込みボタンを読み取り（LOW=押下を反転）
3. デバウンス判定
   - btn状態が変化し、かつ前回確定から50ms超なら joyBtn更新
4. ゲーム中のみ方向更新
   - joyX < 400 かつ逆方向でない -> 左
   - joyX > 600 かつ逆方向でない -> 右
   - joyY > 600 かつ逆方向でない -> 上
   - joyY < 400 かつ逆方向でない -> 下
```

### moveSnake() - スネーク座標更新

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. 頭座標 + (dirX, dirY) で新しい頭座標を計算
2. 末尾から先頭へ向かって座標配列を1つ後ろへシフト
3. 先頭に新しい頭座標を代入

【補足】
- 伸長処理は addScore() 内で snakeLength を増やす
```

### spawnFood() - エサ座標生成

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. valid=falseで開始
2. フィールド内ランダム座標を生成
3. スネーク全身と重複チェック
4. 重複なしなら foodPos に確定
5. 重複ありなら再抽選
```

### checkCollision() - 衝突判定

**引数:** なし
**戻り値:** int（0:なし 1:エサ 2:壁/自身）

```
【処理の流れ】
1. 頭座標がフィールド外なら 2 を返す
2. 頭座標が胴体と重なれば 2 を返す
3. 頭座標が foodPos と一致すれば 1 を返す
4. どれにも該当しなければ 0 を返す
```

### addScore() - 得点・成長・レベルアップ

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. scoreを1加算
2. score > highScore なら highScore更新
3. scoreが5の倍数なら levelUp() を呼ぶ
4. snakeLength < MAX_SNAKE なら1伸ばす
```

### gameOver() - ゲーム終了処理

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. currentState を 2 に設定
2. アクティブブザーでゲームオーバー音を鳴らす
```

### updateLCD() - スコア表示更新

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. lcd.clear()
2. 1行目に "Score:" + score を表示
3. 2行目に "Hi:" + highScore を表示
```

### playBGM() - BGM再生制御

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. 現在ノート長を算出
   - noteDuration = 60000 / bgmTempo / noteDurations[bgmIndex]
2. 前回再生から noteDuration 経過したら次ノート再生
3. bgmIndex を循環更新
```

### levelUp() - 移動間隔短縮

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. moveInterval > 100 のときのみ 40ms短縮
2. 下限100msを維持
```

### drawField() - LEDマトリクス描画

**引数:** なし
**戻り値:** なし

```
【処理の流れ】
1. LED表示をクリア
2. スネーク全身を setLed() で点灯
3. エサ座標を setLed() で点灯
4. 範囲外座標は描画しない
```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス）

```
【考え方】
ボタン状態が変化しても即反映せず、50ms以上安定した変化のみ確定する。

【処理の流れ】
1. btn = !digitalRead(PIN_JOY_BTN)
2. btn != joyBtn を確認
3. millis() - lastDebounceTime > 50 を確認
4. 条件成立時のみ joyBtn と lastDebounceTime を更新
```

### 3-2. millis()ベースのタイマー

```
【利用箇所】
1. スネーク移動周期
   - now - lastMoveMillis >= moveInterval

2. 待機中の速度調整周期
   - now - lastSpeedAdjustMillis > SPEED_ADJUST_DELAY

3. BGMの次ノート再生
   - now - lastBgmMillis > noteDuration

4. 押し込みボタンのデバウンス
   - millis() - lastDebounceTime > DEBOUNCE_DELAY
```

### 3-3. 方向反転禁止ロジック

```
【目的】
直前フレームでの逆走入力（右移動中に左入力など）を無効化し、自滅頻度を抑える。

【判定】
- 左入力時: dirX != 1 のときのみ反映
- 右入力時: dirX != -1 のときのみ反映
- 上入力時: dirY != 1 のときのみ反映
- 下入力時: dirY != -1 のときのみ反映
```

---

## 4. デバッグ出力計画（任意）

| No | 確認したい内容 | 挿入する関数 | Serial出力例 |
|:---|:---|:---|:---|
| 1 | 入力しきい値が適切か | readJoystick() | joyX, joyY, joyBtn |
| 2 | 方向更新が正しいか | readJoystick() | dirX, dirY |
| 3 | 状態遷移が正しいか | loop() | currentState |
| 4 | 移動周期が正しいか | loop() | now-lastMoveMillis, moveInterval |
| 5 | エサ重複回避が機能するか | spawnFood() | foodPos |
| 6 | 衝突判定が正しいか | checkCollision() | collision result |
| 7 | スコアと速度が連動するか | addScore(), levelUp() | score, highScore, moveInterval |
| 8 | 描画座標が範囲内か | drawField() | snake[0], foodPos |

---

## 5. 単体テスト仕様書（V字モデル: 詳細設計 ↔ 単体テスト）

### 5-1. 入力系テスト

| No | テスト対象 | 入力・操作 | 期待結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | readJoystick() | 左右上下に倒す | dirX/dirYが対応方向に更新 | | [ ] |
| 2 | readJoystick() | 右移動中に左へ倒す | 逆方向入力は無視される | | [ ] |
| 3 | readJoystick() | 押し込みを高速連打 | 50ms以内の変化は無視される | | [ ] |
| 4 | loop()待機中 | joyYを上下に操作 | speedLevelが1-5範囲で増減 | | [ ] |

### 5-2. ゲームロジックテスト

| No | テスト対象 | 入力・操作 | 期待結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | moveSnake() | 任意方向で1回実行 | 頭が1マス進み、体が追従 | | [ ] |
| 2 | spawnFood() | 複数回実行 | foodPosがスネークと重複しない | | [ ] |
| 3 | checkCollision() | 頭を壁外へ移動 | 2を返す | | [ ] |
| 4 | checkCollision() | 頭を胴体に重ねる | 2を返す | | [ ] |
| 5 | checkCollision() | 頭をエサに重ねる | 1を返す | | [ ] |
| 6 | addScore() | エサ取得時に呼ぶ | score加算、highScore更新、長さ増加 | | [ ] |
| 7 | levelUp() | scoreを5刻みで増やす | moveIntervalが40msずつ短縮（下限100ms） | | [ ] |
| 8 | gameOver() | 壁衝突時に呼ぶ | currentState=2へ遷移 | | [ ] |

### 5-3. 出力・タイミングテスト

| No | テスト対象 | 入力・操作 | 期待結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | drawField() | 初期配置で呼ぶ | LEDにスネークとエサが表示される | | [ ] |
| 2 | updateLCD() | score/highScore更新後に呼ぶ | LCD2行に正しく表示される | | [ ] |
| 3 | playBGM() | ゲーム中に連続実行 | テンポ通りに音階が循環再生 | | [ ] |
| 4 | loop()ゲーム中 | 一定時間放置 | moveInterval周期でのみ移動する | | [ ] |
| 5 | loop()ゲームオーバー | 押し込みボタン入力 | 待機状態へ戻る | | [ ] |

---

## 6. AIレビュー記録

### Q1: 実装上の問題確認

**AIの回答（要約）:**
1. スネーク最大長付近での配列境界を確認すること。
2. spawnFood() の再抽選ループが停止不能にならないことを確認すること。
3. gameOver状態でtoneを毎ループ呼ぶため、鳴動の意図を確認すること。

**対応した内容:**
1. Section 5 に最大長・境界系テストを追加。
2. Section 2/3 に重複回避ロジックを明記。
3. gameOver時の挙動を設計書へ明記。

---

### Q2: 単体テスト仕様の確認

**AIの回答（要約）:**
1. 境界値（speedLevel=1/5, moveInterval=100）の確認を追加すべき。
2. 逆方向入力禁止とデバウンスは独立観点で確認すべき。

**対応した内容:**
1. Section 5-1/5-2 に境界値観点を追加。
2. 入力系テストを readJoystick 中心へ整理。

---

## 7. グループレビュー記録

### 7-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 | ゲームオーバー時に待機画面に遷移する方法 | マウン | 記載されている為対応なし |
| 2 |  |  |  |
| 3 |  |  |  |

### 7-2. レビューを受けて変更した点

-
-

---

*初版: 2026-05-25 / 実装整合更新: 2026-05-25 / グループレビュー後更新: 2026-05-25*
