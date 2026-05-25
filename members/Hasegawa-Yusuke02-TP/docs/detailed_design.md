# 詳細設計書 — 組込み開発実習

<!-- 作成者: あなたの名前 / 日付: YYYY-MM-DD / グループ: 〇-〇 -->

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
| 作品タイトル |ジョイスティックで4 Digit 7-Segment Displayを操作してLEDの点灯を操作しよう|
| 状態の種類（1-2 状態遷移から） |5|
| 実装する関数の数（2-2 関数一覧から） | 14個 |
| グローバル変数の合計バイト数（2-1 SRAM確認から） | 29B |

---

## 1. グローバル変数・定数の設計

> ※ 基本設計書（2-1 データ設計）をもとに、**型と初期値まで**決めます。
> ここで設計した変数は、この後の関数設計でそのまま使います。

```
【ピン定義】（basic_design.md 3-1 から転記）
  PIN_JOY_X       = A0   // ジョイスティックX
  PIN_JOY_Y       = A1   // ジョイスティックY
  PIN_JOY_SW      = 2    // ジョイスティックSW（INPUT_PULLUP）
  PIN_TM1637_CLK  = 3    // 4桁7セグ CLK
  PIN_TM1637_DIO  = 4    // 4桁7セグ DIO
  PIN_LED_R       = 5    // RGB LED 赤（PWM）
  PIN_LED_G       = 6    // RGB LED 緑（PWM）
  PIN_LED_B       = 9    // RGB LED 青（PWM）

【状態管理】（basic_design.md 1-2 の状態名から転記）
  STATE_INIT         = 0  // 電源ON/初期化
  STATE_IDLE         = 1  // 待機状態
  STATE_SELECT_DIGIT = 2  // 桁選択状態
  STATE_EDIT_VALUE   = 3  // 値変更状態
  STATE_LED_EFFECT   = 4  // LED出力・エフェクト状態
  currentState : uint8_t = STATE_INIT

【表示・編集データ】（basic_design.md 2-1 から転記）

  selectedDigit  : uint8_t = 0           // 現在選択中の桁（0〜3）
  digitValues[4] : uint8_t = {0, 0, 0, 0}  // 4桁7セグメント表示用の各桁の値（0〜9）

【入力値】（basic_design.md 2-1 から転記）

  joystickX  : int  = 512      // ジョイスティックX軸のアナログ値（中心=512）
  joystickY  : int  = 512      // ジョイスティックY軸のアナログ値（中心=512）
  joystickSW : bool = false    // ジョイスティック押し込みスイッチの状態

【LED制御データ】（basic_design.md 2-1 から転記）

  ledR       : uint8_t = 0     // RGB LEDの赤成分（0〜255）
  ledG       : uint8_t = 0     // RGB LEDの緑成分（0〜255）
  ledB       : uint8_t = 0     // RGB LEDの青成分（0〜255）
  effectMode : uint8_t = 0     // LEDのエフェクトモード（0:固定, 1:点滅, ...）

【タイマー（millis()用）】（basic_design.md 2-3 から転記）

  lastMillisJoystick : unsigned long = 0  // ジョイスティック入力の前回処理時刻
  lastMillisDisplay  : unsigned long = 0  // 7セグ表示の前回更新時刻
  lastMillisEffect   : unsigned long = 0  // LEDエフェクトの前回更新時刻
  lastDebounceTime   : unsigned long = 0  // ボタンデバウンス判定の前回確定時刻

【定数（しきい値・デバウンス）】

  JOY_CENTER      : const int = 512           // ジョイスティック中心値
  JOY_DEADZONE    : const int = 80            // 入力無効とみなすデッドゾーン幅
  JOY_HYSTERESIS  : const int = 20            // 入力判定のヒステリシス幅
  DEBOUNCE_MS     : const unsigned long = 50  // ボタンデバウンス判定時間（ms）
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
1. 使用ピンを初期化する
   - PIN_JOY_SW を INPUT_PULLUP に設定する
   - PIN_LED_R / PIN_LED_G / PIN_LED_B を OUTPUT に設定する
   - TM1637（PIN_TM1637_CLK, PIN_TM1637_DIO）を使用できる状態にする

2. 初期状態の値を設定する
   - currentState を STATE_INIT にする
   - selectedDigit を 0、digitValues を {0,0,0,0} にする
   - ledR/ledG/ledB/effectMode を 0 にする
   - 各タイマー変数（lastMillis...）を現在時刻で初期化する

3. 起動時の表示と状態遷移を行う
   - 4桁7セグに初期値を表示する（0000）
   - RGB LED を消灯する
   - 初期化完了後、currentState を STATE_IDLE に遷移させる
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
- now = millis() を取得する
- readJoystick() を呼び、joystickX / joystickY / joystickSW を更新する
- updateDisplay() を呼び、現在の digitValues と selectedDigit を表示に反映する
- doOptionalEffect() を呼び、effectMode に応じた LED 効果を更新する（millis 制御）

＜currentState が 0（初期化状態）のとき＞
- 初期表示（0000）と LED 消灯状態を確認する
- 初期化が完了したら currentState = STATE_IDLE に遷移する

＜currentState が 1（待機状態）のとき＞
- ジョイスティック横操作を監視する
- 横入力がしきい値（JOY_CENTER ± JOY_DEADZONE）を超えたら currentState = STATE_SELECT_DIGIT に遷移する
- 押し込みが有効入力なら currentState = STATE_LED_EFFECT に遷移する

＜currentState が 2（桁選択状態）のとき＞
- selectDigit() を呼び、選択桁を左右に移動する
- 縦入力があれば currentState = STATE_EDIT_VALUE に遷移する
- 一定時間入力がなければ currentState = STATE_IDLE に戻る

＜currentState が 3（値変更状態）のとき＞
- changeDigitValue() を呼び、selectedDigit の値を増減する
- 横入力があれば currentState = STATE_SELECT_DIGIT に遷移する
- 押し込みが有効入力なら currentState = STATE_LED_EFFECT に遷移する

＜currentState が 4（LED出力・エフェクト状態）のとき＞
- confirmAndSetLED() を呼び、4桁の値を ledR / ledG / ledB / effectMode に反映する
- 反映完了後、currentState = STATE_IDLE に遷移する

```

---

### （関数ごとに以下のブロックをコピーして追加してください）

> ※ 基本設計書 2-2 の関数一覧に記載した関数を1つずつ設計します。

---

### `関数名()` — （役割を1行で書く）

**basic_design.md 2-2 との対応：** （基本設計書の関数一覧の説明を転記）

**引数：** `引数名`（型）: 何の値か

**戻り値：** 型（なしの場合は void）

```
【処理の流れ】
1.
2.
3.

【エラー・異常ケース】
- 異常な値が来た場合:
```

---

### `readButton()` — SW押し込み入力をデバウンス判定する

**basic_design.md 2-2 との対応：** （共通）ボタン読出

**引数：** なし

**戻り値：** bool

```
【処理の流れ】
1. 現在のSW入力を読み取り、押下状態かどうかを判定する。
2. 前回確定時刻との差分を取り、DEBOUNCE_MS未満なら無効にする。
3. 有効な押下ならtrueを返し、lastDebounceTimeを更新する。

【エラー・異常ケース】
- 異常な値が来た場合: falseを返し、状態遷移を行わない。
```

---

### `readSensor()` — センサー値を取得する（未使用時は互換維持）

**basic_design.md 2-2 との対応：** （共通）センサー読出

**引数：** なし

**戻り値：** int (cm)

```
【処理の流れ】
1. 本システムで利用する入力源を確認する。
2. センサー未使用時は既定値または前回値を返す。
3. 拡張時はここで実センサー値を取得して返す。

【エラー・異常ケース】
- 異常な値が来た場合: 前回の有効値を返す。
```

---

### `updateOutput()` — 現在状態に応じて出力を更新する

**basic_design.md 2-2 との対応：** （共通）出力更新

**引数：** int state

**戻り値：** なし（void）

```
【処理の流れ】
1. stateを参照して必要な出力処理を判定する。
2. 7セグ表示とLED効果の更新関数を呼ぶ。
3. 出力を反映し、次ループへ移る。

【エラー・異常ケース】
- 異常な値が来た場合: 出力を前回値に保持する。
```

---

### `editDigits()` — 必須機能①の処理を実行する

**basic_design.md 2-2 との対応：** F01 必須機能①（requirements転記）

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. ジョイスティック入力を取得する。
2. 横入力で桁選択、縦入力で値変更を行う。
3. 更新後の値を表示へ反映する。

【エラー・異常ケース】
- 異常な値が来た場合: 入力を無効化して前回表示を維持する。
```

---

### `applyDigitsToLed()` — 必須機能②の処理を実行する

**basic_design.md 2-2 との対応：** F02 必須機能②（requirements転記）

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. 押し込み入力の有効判定を行う。
2. 4桁の数値をRGB値とeffectModeへ変換する。
3. 変換結果をLED出力に反映する。

【エラー・異常ケース】
- 異常な値が来た場合: LED値を更新せず前回出力を維持する。
```

---

### `runLedPattern()` — 追加機能①を実行する

**basic_design.md 2-2 との対応：** A01 追加機能①（余裕があれば）

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. 現在のeffectModeを確認する。
2. モードに応じた装飾パターンを選択する。
3. 周期条件を満たすときのみ効果を更新する。

【エラー・異常ケース】
- 異常な値が来た場合: mode 0の安全動作にフォールバックする。
```

---

### `readJoystick()` — ジョイスティックのX/Y/SW値を取得し変数に格納する

**basic_design.md 2-2 との対応：** ジョイスティック読出

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. analogRead()でA0/A1を読み取り、joystickXとjoystickYを更新する。
2. digitalRead()でD2を読み取り、joystickSWを更新する。
3. 値が極端（0または1023付近）で不安定な場合は入力を無視し、前回の有効値を保持する。

【エラー・異常ケース】
- 異常な値が来た場合: 仕様範囲外として入力を無効化し、状態遷移を行わない。
```

---

### `selectDigit()` — ジョイスティック横操作で選択中の桁を変更する

**basic_design.md 2-2 との対応：** 桁選択処理

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. joystickXがしきい値を超えたか判定する。
2. 入力方向に応じてselectedDigitを増減する。
3. selectedDigitを0〜3の範囲に循環補正する。

【エラー・異常ケース】
- 異常な値が来た場合: 桁移動を行わず現状を維持する。
```

---

### `changeDigitValue()` — ジョイスティック縦操作で選択中桁の値を変更する

**basic_design.md 2-2 との対応：** 値変更処理

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. joystickYがしきい値を超えたか判定する。
2. 入力方向に応じてdigitValues[selectedDigit]を増減する。
3. 値を0〜9の範囲に循環補正する。

【エラー・異常ケース】
- 異常な値が来た場合: 値変更を無効化し前回値を保持する。
```

---

### `confirmAndSetLED()` — ジョイスティック押し込みで4桁値をRGB/効果に反映する

**basic_design.md 2-2 との対応：** LED確定・効果反映

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. 押し込み入力が有効かどうかを判定する。
2. digitValues[0..2]をledR/ledG/ledBへ変換する。
3. digitValues[3]からeffectModeを決定する。

【エラー・異常ケース】
- 異常な値が来た場合: 反映を中止し前回のLED出力を維持する。
```

---

### `updateDisplay()` — 4桁7セグメントディスプレイの表示を更新する

**basic_design.md 2-2 との対応：** 7セグ表示更新

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. 表示更新周期に達したかを判定する。
2. digitValuesの4桁をディスプレイへ出力する。
3. 選択中の桁を点滅表示して操作対象を示す。

【エラー・異常ケース】
- 異常な値が来た場合: 更新をスキップして前回表示を保持する。
```

---

### `doOptionalEffect()` — effectModeに応じたLED点滅や効果を実行する

**basic_design.md 2-2 との対応：** エフェクト処理

**引数：** なし

**戻り値：** なし（void）

```
【処理の流れ】
1. effectModeを参照し実行する効果を選択する。
2. millis()で更新周期を判定する。
3. 条件成立時にLEDの点滅/効果を更新する。

【エラー・異常ケース】
- 異常な値が来た場合: effectModeを0に戻し固定出力へ切り替える。
```

---

## 3. 重要ロジックの詳細設計

### 3-1. チャタリング防止（デバウンス処理）

> ※ ボタンを使う場合は必ず設計してください。

```
【考え方】
  ボタン入力は押下/離上の瞬間に短時間でON/OFFが揺れるため、入力変化後すぐには確定しない。
  入力変化を検知してから DEBOUNCE_MS（50ms）以上経過した場合のみ有効な入力として扱う。
  また、押しっぱなし中は連続で確定せず、「未押下→押下」の変化を1回の押下として扱う。

【処理の流れ】
  1. ボタンのデジタル値を読む（digitalRead）。
  2. 前回の生入力値と異なる場合、lastDebounceTime を現在時刻に更新する。
  3. 現在時刻 - lastDebounceTime を計算する。
  4. 経過時間 < DEBOUNCE_MS の場合は揺れ期間として無視し、状態を確定しない。
  5. 経過時間 >= DEBOUNCE_MS の場合は入力を確定し、未押下→押下の変化時のみ押下イベントを成立させる。

【必要な変数（Section 1 に追加済みか確認）】
  lastDebounceTime : unsigned long          // 入力変化を最後に検知した時刻
  DEBOUNCE_MS      : const unsigned long = 50  // チャタリング判定時間（ms）
```

---

### 3-2. millis() を使ったタイマー管理

```
【考え方】
  delay() を使わず、各処理の「前回実行時刻」を記録して周期実行する。
  now - lastMillisX >= interval の条件で判定することで、入力監視・表示更新・LED効果を並行して動かす。
  差分計算で判定するため、millis() のオーバーフロー後も同じロジックで継続できる。

【処理の流れ（例: LED点滅）】
  1. now = millis() を取得する。
  2. ジョイスティック入力処理:
     now - lastMillisJoystick >= JOYSTICK_INTERVAL の場合に readJoystick() を実行し、lastMillisJoystick = now に更新する。
  3. 表示更新処理:
     now - lastMillisDisplay >= DISPLAY_INTERVAL の場合に updateDisplay() を実行し、lastMillisDisplay = now に更新する。
  4. LEDエフェクト処理:
     now - lastMillisEffect >= EFFECT_INTERVAL の場合に doOptionalEffect() を実行し、lastMillisEffect = now に更新する。
  5. いずれの条件も満たさない場合は何もしない（次ループで再判定）。

【自分のシステムで millis() を使う処理】
  - ジョイスティック入力の周期監視（lastMillisJoystick）
  - 4桁7セグの表示更新周期管理（lastMillisDisplay）
  - LED点滅・エフェクト更新周期管理（lastMillisEffect）

【使用する主な変数】
  now                : unsigned long  // 現在時刻（millis）
  lastMillisJoystick : unsigned long  // ジョイスティック入力の前回処理時刻
  lastMillisDisplay  : unsigned long  // 表示の前回更新時刻
  lastMillisEffect   : unsigned long  // エフェクトの前回更新時刻

【周期パラメータ（実装時に定数化）】
  JOYSTICK_INTERVAL  : const unsigned long  // 入力読み取り周期（例: 20ms）
  DISPLAY_INTERVAL   : const unsigned long  // 表示更新周期（例: 10ms）
  EFFECT_INTERVAL    : const unsigned long  // エフェクト更新周期（例: 100ms）
```

---

### 3-3. その他の重要ロジック（任意）

> **【任意】** 複雑なロジックがある場合のみ記入してください。
> 例：「距離に応じたLED点灯パターン」「ゲームの衝突判定」「温度の閾値判定」

```
【処理の流れ】
1. LED確定時に digitValues[0]〜digitValues[3] を読み取る。
2. digitValues[0]〜digitValues[2] を 0〜9 の範囲で正規化し、RGB輝度へ変換する。
  - ledR = digitValues[0] * 28
  - ledG = digitValues[1] * 28
  - ledB = digitValues[2] * 28
  （0〜252の範囲でPWM出力し、0〜255に近い明るさ段階を作る）
3. digitValues[3] を effectMode に割り当て、対応する効果を選択する。
  - effectMode = digitValues[3] % 4
  - 0: 固定点灯 / 1: 点滅 / 2: フェード(未定) / 3: レインボー等（未定）

【入力値と出力値の関係】
- 入力: digitValues = [d0, d1, d2, d3]（各0〜9）
- 出力: ledR = d0*28, ledG = d1*28, ledB = d2*28, effectMode = d3%4
- 例1: [9,0,0,0] → ledR=252, ledG=0, ledB=0, effectMode=0（赤固定）
- 例2: [0,9,9,1] → ledR=0, ledG=252, ledB=252, effectMode=1（シアン点滅）
- 例3: [3,5,7,6] → ledR=84, ledG=140, ledB=196, effectMode=2（フェード）

```

---

## 4. デバッグ出力計画（任意）

> **【任意】** 関数設計（Section 2）と並行して記入すると効果的です。
> 「動かない」ときに何を確認すればいいかを事前に計画しておきます。
> 実装後は不要な Serial.println() を削除すること。

| No | 確認したい内容 | 挿入する関数 | Serial.println の内容例 |
|:---|:---|:---|:---|
| 1 | センサー値が正しく取れているか | `readSensor()` | `Serial.println(sensorValue);` |
| 2 | 状態遷移が正しく起きているか | `loop()` | `Serial.println(currentState);` |
| 3 | チャタリング処理が効いているか | `readButton()` | `Serial.println("btn confirmed");` |
| 4 | ジョイスティックの生値が安定して読めているか | `readJoystick()` | `Serial.println(String("X=") + joystickX + ",Y=" + joystickY + ",SW=" + joystickSW);` |
| 5 | 桁選択が意図どおり循環しているか（0〜3） | `selectDigit()` | `Serial.println(String("selectedDigit=") + selectedDigit);` |
| 6 | 桁値変更が0〜9で循環しているか | `changeDigitValue()` | `Serial.println(String("digit[") + selectedDigit + "]=" + digitValues[selectedDigit]);` |
| 7 | 4桁値→RGB/effectMode変換が正しいか | `confirmAndSetLED()` | `Serial.println(String("R=") + ledR + ",G=" + ledG + ",B=" + ledB + ",M=" + effectMode);` |
| 8 | millis周期判定が想定間隔で動いているか | `doOptionalEffect()` | `Serial.println(String("dt=") + (millis() - lastMillisEffect));` |

---

## 5. 単体テスト仕様書（V字モデル：詳細設計 ↔ 単体テスト）

> ※ 各関数・部品が「単体で正しく動くか」を確認するテスト項目を設計します。
> 「実際の結果」欄は実装後に記入します。

### 5-1. 入力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | readButton() | タクトスイッチを1回押す | true が返る | | [ ] |
| 2 | readButton() | スイッチを素早く2回押す | 1回分だけ true になる | | [ ] |
| 3 | readButton() | スイッチを長押しする | 連続してtrueが返らず、1回だけtrueになる | | [ ] |
| 4 | readButton() | チャタリングが多い状態で押す | 誤検知せず、1回だけtrueになる | | [ ] |
| 5 | readJoystick() | スティックを中央で静止させる（X/Yともに中立） | joystickX/joystickY が中心値付近（512付近）で安定して取得できる | | [ ] |
| 6 | readJoystick() | スティックを左右・上下に最大まで倒す | joystickX/joystickY が変化し、倒した方向に応じた値（低値/高値）になる | | [ ] |
| 7 | readJoystick() | スティックを中心付近で微小に揺らす（デッドゾーン内） | 判定が過敏に変化せず、桁選択や値変更が不要に発生しない | | [ ] |
| 8 | selectDigit() | selectedDigit=3で右操作 | selectedDigitが0に戻る（循環） | | [ ] |
| 9 | selectDigit() | selectedDigit=0で左操作 | selectedDigitが3に戻る（循環） | | [ ] |
| 10 | changeDigitValue() | digitValues=9で+1操作 | digitValuesが0に戻る（循環） | | [ ] |
| 11 | changeDigitValue() | digitValues=0で-1操作 | digitValuesが9に戻る（循環） | | [ ] |
| 12 | readJoystick() | joystickX/joystickYが仕様外値（-10や1030など） | 前回の有効値を保持し、異常動作しない | | [ ] |

### 5-2. 出力系テスト

| No | テスト対象の関数 | 入力・操作 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | confirmAndSetLED() | digitValues を [9,0,0,0] に設定して確定操作する | ledR=252, ledG=0, ledB=0 となり、LEDが赤系で点灯する | | [ ] |
| 2 | confirmAndSetLED() | digitValues を [0,9,9,1] に設定して確定操作する | ledR=0, ledG=252, ledB=252, effectMode=1 となる | | [ ] |
| 3 | confirmAndSetLED() | digitValues を [0,0,0,9] に設定して確定操作する | effectMode=1〜3の範囲で循環し、異常値にならない | | [ ] |
| 4 | confirmAndSetLED() | digitValues を [0,0,0,0] に設定して確定操作する | ledR=0, ledG=0, ledB=0, effectMode=0 となる | | [ ] |
| 5 | updateDisplay() | selectedDigit=2, digitValues=[1,2,3,4] で表示更新する | 7セグに 1234 が表示され、選択中の3桁目が点滅表示される | | [ ] |
| 6 | updateDisplay() | digitValues=[9,9,9,9] で表示更新する | 7セグに 9999 が表示される | | [ ] |
| 7 | updateDisplay() | digitValues=[0,0,0,0] で表示更新する | 7セグに 0000 が表示される | | [ ] |
| 8 | runLedPattern() | effectMode を 1（点滅）に設定して一定時間動作させる | 設計周期でLEDが点滅し、他色成分や表示が不正に乱れない | | [ ] |

### 5-3. タイミング・並行動作テスト

| No | テスト内容 | テスト手順 | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | 非ブロッキング動作確認（入力取りこぼしなし） | effectMode=1でLED点滅中に、ジョイスティックを左右に連続操作し桁選択を行う | LED点滅を継続したまま selectedDigit が遅延なく更新される（操作が止まらない） | | [ ] |
| 2 | エフェクト周期精度（EFFECT_INTERVAL） | effectMode=1で1分間動作させ、点滅間隔をストップウォッチで複数回測定する | 測定した点滅間隔が設計値（例:100ms）に対して大きくずれない | | [ ] |
| 3 | エフェクト周期の境界値 | effectMode=1でEFFECT_INTERVAL直前/直後にLED点滅が正しく切り替わる | 境界で誤動作せず、1周期ごとに正しく点滅する | | [ ] |
| 4 | 表示更新と入力処理の並行性 | 7セグ表示更新中にジョイスティックを上下操作して値変更を連続実行する | 表示のちらつきや停止なく digitValues が追従して更新される | | [ ] |

---

## 6. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 実装上の問題確認

> 「この詳細設計書に書いた関数と処理フローをもとに Arduino でコードを書きます。バグになりやすい箇所・処理の抜け・型の問題はありますか？」

**AIの回答（要約）：**
変数・定数定義、状態遷移（5状態）、デバウンス・タイマー管理など、実装に必要な要素が具体的に記載されている
各関数について、引数・戻り値・処理内容・エラー処理まで明記されている
関数名や変数名は簡潔で意味が分かりやすいものに統一されている
デバッグ出力やユニットテスト計画も網羅されている
最終レビューで、関数名の統一・デバウンス変数の扱い・タイマー定数の定義漏れ・不要なテンプレート関数の削除など、実装前に直すべきポイントが指摘されている

**対応した内容：**
全体的に再確認

---

### Q2: 単体テスト仕様の確認

> 「Section 5 の単体テスト仕様書で、各関数の動作が正しく検証できていますか？テストが不足している項目や、境界値テストが必要な箇所を教えてください。」

**AIの回答（要約）：**
境界値テスト（桁や値の循環、しきい値付近の入力判定）
異常系テスト（仕様外値や連続入力時の動作）
タイミングの境界（周期判定の前後での動作）

**対応した内容：**

---

## 7. グループレビュー記録

### 7-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 | doOptional1()などのOptionalは再度見た際にすぐにどのことを言っているか分かるのか | 山田さん | 関数名を runLedPattern() など短く分かる名前へ修正 |
| 2 |  |  |  |
| 3 |  |  |  |

### 7-2. レビューを受けて変更した点

- doFeature1() を editDigits() に変更
- doFeature2() を applyDigitsToLed()、doOptional1() を runLedPattern() に変更

---

*初版: YYYY-MM-DD / AIレビュー: YYYY-MM-DD / グループレビュー後更新: YYYY-MM-DD*
