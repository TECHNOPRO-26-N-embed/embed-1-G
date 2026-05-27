# 詳細設計書 — 組込み開発実習

<!-- 作成者: トゥタココ　マウン / 日付: 2026-05-25 / グループ: 1-G -->

> **このドキュメントの目的**
> 基本設計書（basic_design.md）で「**どのような構造で作るか**」を決めました。
> この詳細設計書では「**各処理を具体的にどう実装するか**」を決めます。
> 書き終わったとき、**コードの骨格がほぼ完成している**状態を目指してください。

> [!NOTE]
> **V字モデルにおける位置づけ**
> 詳細設計書 ←→ **単体テスト**（関数・部品ごとのテスト）が対応します。
> 「この関数が正しく動くか」の確認は Section 5 の単体テスト仕様書で計画します。
> ※ 必須機能全体が動くかの「結合テスト」は基本設計書（Section 6）に記載します。

---

## 0. 基本設計書との接続確認

| 項目 | basic_design.md から転記 |
|:--|:--|
| 作品タイトル | ジョイスティックで動かす超音波スキャンとアラート装置 |
| 状態の種類（1-2 状態遷移から） | 待機中、スキャン計測中、警告中 |
| 実装する関数の数（2-2 関数一覧から） | 4個 |
| グローバル変数の合計バイト数（2-1 SRAM確認から） | 約40B |

---

## 1. グローバル変数・定数の設計

> ※ 基本設計書（2-1 データ設計）をもとに、**型と初期値まで**決めます。
> ここで設計した変数は、この後の関数設計でそのまま使います。

```
【ピン定義】（basic_design.md 3-1 から転記）
  PIN_JOYSTICK_X      = A0   // Joystick X-axis
  PIN_JOYSTICK_SW     = 2    // Joystick button (INPUT_PULLUP)
  PIN_SERVO           = 3    // SG90 servo
  PIN_LED             = 6    // Alert LED
  PIN_BUZZER          = 8    // Active buzzer
  PIN_ULTRASONIC_TRIG = 9    // HC-SR04 Trig
  PIN_ULTRASONIC_ECHO = 10   // HC-SR04 Echo

【状態管理】（basic_design.md 1-2 の状態名から転記）
  alertState : bool = false  // false: 待機/スキャン, true: 警告中

【タイマー（millis()用）】（basic_design.md 2-3 から転記）
  lastServoMove  : unsigned long = 0
  servoInterval  : unsigned long = 10
  lastAlertBlink : unsigned long = 0
  alertInterval  : unsigned long = 200

【センサー・入力値】（basic_design.md 2-1 から転記）
  duration   : long = 0
  distance   : int  = 0
  servoAngle : int  = 90

【その他のフラグ・カウンター】
  buzzerState      : bool = false
  buttonState      : int = HIGH
  lastButtonState  : int = HIGH
  lastDebounceTime : unsigned long = 0
  debounceDelay    : unsigned long = 50
```

---

## 2. 各関数の詳細設計

> ※ 基本設計書（2-2 関数一覧）で定義した各関数の「中身」を設計します。
> **疑似コード**（日本語＋処理の流れ）で書いてください。実際のC++コードは書かなくてOKです。

---

### `setup()` — 初期化処理

```
【処理の流れ】
1. ピンモードを設定する
   - PIN_BUTTON  → INPUT_PULLUP
   - PIN_LED_*   → OUTPUT
   - PIN_BUZZER  → OUTPUT

2. ライブラリの初期化（使うものだけ）
   - 例: lcd.begin(16, 2)
   - 例: servo.attach(PIN_SERVO)

3. Serial.begin(9600)（デバッグ用）

4. 起動確認（任意）: 緑LEDを1秒点灯して消灯
```

**↓ 自分の setup() を設計してください**
```
【処理の流れ】
1. サーボを初期化する
   - myServo.attach(PIN_SERVO)

2. ピンモードを設定する
   - PIN_JOYSTICK_SW     → INPUT_PULLUP
   - PIN_LED             → OUTPUT
   - PIN_BUZZER          → OUTPUT
   - PIN_ULTRASONIC_TRIG → OUTPUT
   - PIN_ULTRASONIC_ECHO → INPUT

3. Serial.begin(9600)（デバッグ用）

4. サーボを初期角度90°に設定する
   - myServo.write(servoAngle)

5. 起動ログを出力する
   - Serial.println("System Started...")
```

---

### `loop()` — メインループ

> ※ loop() は「状態ごとに何をするか」だけ書く。細かい処理は各関数に任せる。

```
【処理の流れ】

＜毎ループ実行すること＞
  - 入力を読む（readButton(), readSensor() などを呼ぶ）
  - 現在時刻を取得: now = millis()

＜currentState が 0（待機中）のとき＞
  - センサー値を監視する
  - 検知条件を満たしたら → currentState = 1

＜currentState が 1（動作中）のとき＞
  - メイン処理を行う
  - 終了条件を満たしたら → currentState = 2

＜currentState が 2（完了）のとき＞
  - 完了表示をする
  - リセットボタンが押されたら → currentState = 0

＜currentState が 3（エラー）のとき＞
  - エラー表示をする / リセットを待つ

```

**↓ 自分の loop() を設計してください**
```
【処理の流れ】
＜毎ループ実行すること＞
  - handleJoystick() を呼び、X軸入力に応じて角度を更新
  - handleResetButton() を呼び、押し込みボタンで90°リセットを処理
  - readUltrasonic() を呼んで distance を更新
  - handleAlert(distance) を呼び、LED/ブザー/ログを制御

＜警告中相当（distance > 0 かつ distance <= 20）のとき＞
  - LED ON
  - alertIntervalごとにブザーをトグル
  - 距離が前回から2cm以上変化したときだけSerialに出力

＜待機/スキャン相当（上記以外）のとき＞
  - LED OFF
  - ブザー停止
  - 警告状態から復帰した瞬間だけ「safe」ログを出力
```

```
  【処理の流れ】
＜毎ループ実行すること＞
  handleJoystick();
  handleResetButton();
  distance = readUltrasonic();
  handleAlert(distance);

＜currentState が 警告中 相当のとき＞
  distance <= 20cm（かつ >0）なら警告出力

＜currentState が 待機/スキャン 相当のとき＞
  それ以外は警告解除
```

---

### （関数ごとに以下のブロックをコピーして追加してください）

> ※ 基本設計書 2-2 の関数一覧に記載した関数を1つずつ設計します。

---

### `handleJoystick()` — ジョイスティック入力でサーボ角度を調整する

**basic_design.md 2-2 との対応：** X軸値に応じてサーボを左右へ動かす。

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. xValue = analogRead(PIN_JOYSTICK_X) を取得する
2. now = millis() を取得する
3. now - lastServoMove >= servoInterval のときのみ更新する
4. xValue < 400 なら servoAngle を -1（0未満にはしない）
5. xValue > 600 なら servoAngle を +1（180超にはしない）
6. 角度が変わった場合のみ myServo.write(servoAngle) を実行
7. 角度が減少なら "Left | Angle:"、増加なら "Right | Angle:" を出力
8. lastServoMove = now を更新する

【エラー・異常ケース】
- 角度範囲外にならないよう 0〜180 でガードする。
```

---

### `handleResetButton()` — ボタン押下でサーボを90°に戻す

**basic_design.md 2-2 との対応：** ボタン入力をデバウンスして1回押下を確定する。

**引数：** なし

**戻り値：** void

```
【処理の流れ】
1. reading = digitalRead(PIN_JOYSTICK_SW)
2. 前回読み取りと異なれば lastDebounceTime = millis()
3. millis() - lastDebounceTime > debounceDelay なら確定判定へ
4. reading != buttonState のとき buttonState を更新
5. buttonState == LOW（押下）なら
   - servoAngle = 90
   - myServo.write(servoAngle)
   - リセットメッセージをSerial出力
6. lastButtonState = reading を更新

【エラー・異常ケース】
- チャタリング中の短時間変化は無視する。
```

---

### `readUltrasonic()` — 超音波センサーで距離を測る

**basic_design.md 2-2 との対応：** Trig/Echoで距離を計測しcmに換算する。

**引数：** なし

**戻り値：** int（cm）

```
【処理の流れ】
1. trig を LOW にして2us待つ
2. trig を HIGH にして10us待つ
3. trig を LOW に戻す
4. duration = pulseIn(PIN_ULTRASONIC_ECHO, HIGH) で時間を取得
5. distance = duration * 0.034 / 2 でcm換算
6. distance を返す

【エラー・異常ケース】
- pulseIn が 0 を返した場合は distance=0 となる（無効値扱い）。
```

---

### `handleAlert(int distance)` — 距離に応じてLED/ブザーを制御する

**basic_design.md 2-2 との対応：** 20cm以内で警告、離れたら停止。

**引数：** `distance`（int）: 測定距離（cm）

**戻り値：** void

```
【処理の流れ】
1. now = millis() を取得し、static lastDistance を参照する
2. distance > 0 かつ distance <= 20 の場合（警告中）
   - LEDをON
   - now - lastAlertBlink >= alertInterval なら buzzerState を反転
   - buzzerState が true なら tone(1000Hz)、false なら noTone
   - 距離差が2cm以上のときだけ Distanceログを出力
   - alertState = true
3. それ以外（安全）
   - LEDをOFF、ブザー停止
   - alertState が true だった場合のみ safeログを出力
   - alertState = false
   - lastDistance = -1 にリセット

【エラー・異常ケース】
- distance<=0 は無効値として警告判定から除外する。
```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

> ※ ボタンを使う場合は必ず設計してください。

```
【考え方】
  ボタンが押されたとき、50ms 以内の連続入力は「同じ1回の押下」として無視する。

【処理の流れ】
  1. ボタンのデジタル値を読む（digitalRead）
  2. 読み取り値が変化した瞬間の時刻を lastDebounceTime に保存する
  3. 経過時間 < DEBOUNCE_DELAY（50ms）なら確定しない
  4. 経過時間 >= DEBOUNCE_DELAY になったら状態を確定する
  5. LOW 押下が確定したときだけリセット処理を実行する

【必要な変数（Section 1 に追加済みか確認）】
  lastDebounceTime : unsigned long
  debounceDelay    : unsigned long = 50
```

---

### 3-2. millis() を使ったタイマー管理

```
【考え方】
  「前回実行時刻」を保存し、now - last >= interval のときだけ処理する。

【処理の流れ（このシステム）】
  1. サーボ更新:
     - now - lastServoMove >= servoInterval(10ms) のときのみ角度更新
  2. ブザー点滅:
     - now - lastAlertBlink >= alertInterval(200ms) のときだけON/OFFを反転
  3. 条件未達なら何もしない（次ループで再判定）
```

---

### 3-3. その他の重要ロジック（任意）

```
【処理の流れ】
1. Joystick中立域（400〜600）では角度を変化させない
2. 角度が変わったときのみSerialを出してログ量を抑える
3. 距離は2cm以上変化したときのみSerial出力する

【入力値と出力値の関係】
- xValue < 400  : 左へ1°
- 400〜600      : 角度維持
- xValue > 600  : 右へ1°
- distance <=20 : 警告ON
- distance >20  : 警告OFF
```

---

## 4. デバッグ出力計画（任意）

> **【任意】** 関数設計（Section 2）と並行して記入すると効果的です。
> 「動かない」ときに何を確認すればいいかを事前に計画しておきます。
> 実装後は不要な Serial.println() を削除すること。

| No | 確認したい内容 | 挿入する関数 | Serial.println の内容例 |
|:---|:---|:---|:---|
| 1 | 角度更新が起きているか | `handleJoystick()` | `Serial.println(servoAngle);` |
| 2 | ボタン押下が確定したか | `handleResetButton()` | `Serial.println("reset to 90");` |
| 3 | 距離計測の変化が取れているか | `handleAlert()` | `Serial.println(distance);` |
| 4 | 警告解除タイミング | `handleAlert()` | `Serial.println("safe");` |

---

## 5. 単体テスト仕様書（V字モデル：詳細設計 ↔ 単体テスト）

> ※ 各関数・部品が「単体で正しく動くか」を確認するテスト項目を設計します。
> 「実際の結果」欄は実装後に記入します。

### 5-1. 入力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | handleJoystick() | Joystickを左端に倒す | 角度が減少し最小0°で停止する | | [ ] |
| 2 | handleJoystick() | Joystickを右端に倒す | 角度が増加し最大180°で停止する | | [ ] |
| 3 | handleResetButton() | ボタンを1回押す | サーボ角度が90°に戻る | | [ ] |
| 4 | handleResetButton() | ボタンを素早く2回押す | チャタリング誤検出なく安定動作する | | [ ] |
| 5 | readUltrasonic() | 既知距離で測定 | 許容誤差内の距離が得られる | | [ ] |

### 5-2. 出力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | handleAlert(distance) | distance=15 | LED ON、ブザー点滅、距離ログ出力 | | [ ] |
| 2 | handleAlert(distance) | distance=25 | LED OFF、ブザー停止、safeログ出力 | | [ ] |
| 3 | handleJoystick() | 角度変化あり/なし | 変化あり時のみ角度ログが出る | | [ ] |

### 5-3. タイミング・並行動作テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | サーボ更新周期 | Joystick保持で角度変化速度を確認 | 約10ms間隔で1°ずつ変化 | | [ ] |
| 2 | ブザー点滅周期 | 警告中のON/OFF間隔を確認 | 約200ms間隔でトグル | | [ ] |

---

## 6. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 実装上の問題確認

> 「この詳細設計書に書いた関数と処理フローをもとに Arduino でコードを書きます。バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**
- `pulseIn()` はブロッキングなので、他処理の応答性に影響する可能性がある。
- ボタン判定は INPUT_PULLUP 前提で LOW を押下とする実装が正しい。
- 距離0は無効値として警告条件から除外するのが妥当。

**対応した内容：**
- ドキュメントに `pulseIn()` の注意点を追記。
- 押下判定LOW、`distance > 0` 条件を明記。

---

### Q2: 単体テスト仕様の確認

> 「Section 5 の単体テスト仕様書で、各関数の動作が正しく検証できていますか？テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**
- 0°/180° の角度端、20cm閾値、ボタン連打（デバウンス）を含めると網羅性が上がる。

**対応した内容：**
- 入力系テストに角度端とデバウンステストを追加。
- 出力系テストに閾値近傍（15/25cm）を反映。

---

## 7. グループレビュー記録

### 7-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 | `handleAlert` の distance 引数型 | 山田さん | `int` で統一し設計書へ反映 |

### 7-2. レビューを受けて変更した点

- distance 引数型の記述を `int` に統一。
- 警告条件（distance > 0 かつ <=20）を明確化。

---

*初版: 2026-05-25 / AIレビュー: 2026-05-25 / グループレビュー後更新: 2026-05-25*
