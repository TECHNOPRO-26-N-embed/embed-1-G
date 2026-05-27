# 基本設計書 — 組込み開発実習

<!-- 作成者: トゥタココ　マウン / 日付: 2026-05-22 / グループ: 1-G -->

> **このドキュメントの目的**
> 要件定義書（requirements.md）で「**何を作るか**」を決めました。
> この基本設計書では「**どのような構造で作るか**」を決めます。
> コードは書きません。図・表・テキストで「設計の骨格」を作るドキュメントです。

> [!NOTE]
> **V字モデルにおける位置づけ**
> 基本設計書 ←→ **結合テスト**（部品を組み合わせて必須機能が動くか）が対応します。
> Section 6 の結合テスト計画は、設計を終えた後に記入します。

---

## 0. 要件定義書との接続確認

> ※ まず要件定義書（requirements.md）の内容を確認・転記します。

### 日本語 :
| 項目 | requirements.md から転記 |
|:--|:--|
| 作品タイトル | ジョイスティックで動かす超音波スキャンとアラート装置 |
| 一言説明（何ができるガジェットか） | 物の距離をはかり、近くに物があればアラートを出す装置 |
| 使用する共通部品（グループ共通） | ジョイスティック |
| 追加部品（個人・主なもの） | SG90 サーボモーター / HC‑SR04 超音波センサー / アクティブブザー/LED |
| 必須機能の数（3-1の件数） | 　4件 |
| 追加機能の数（3-2の件数） | 　0件 |

### English :
| Item | Copied from requirements.md |
| --- | --- |
| Project Title | Ultrasonic Scanning & Alert Device Controlled by Joystick |
| One‑sentence description (What the gadget does) | Measures the distance to objects and triggers an alert when something is close |
| Common parts used (shared by group) | Joystick |
| Additional parts (individual / main components) | SG90 Servo Motor / HC‑SR04 Ultrasonic Sensor / Active Buzzer / LED |
| Number of required functions (from Section 3‑1) | 4 |
| Number of additional functions (from Section 3‑2) | 0 |

> [!IMPORTANT]
> 要件定義書の「やらないこと（3-3）」に書いたことは、この設計書に登場させないこと。
> 設計中にアイデアが増えた場合は、まず要件定義書の「任意機能（3-2）」に追加して確認する。
> **追加機能も設計書に含めること。** 優先度が低いだけで、構造は先に考えておく。

---

## 0-2. グループ共通部品との接続設計

> ※ グループで決めた「共通部品」と、あなたの個人設計の接点を明確にします。

### 日本語 :
| 項目 | 内容 |
|:--|:--|
| グループ共通部品（名称） | ジョイスティック |
| 共通部品が担う役割 | サーボの角度を操作するため |
| 接続するArduinoピン（予定） | A0 |
| 個人の設計と共通部品が協調する箇所 | ジョイスティックを左に倒すと、サーボが左に回転し、超音波センサーがその方向をスキャンする。 |
| グループ内で統一すべき仕様 | A0,A1,D2,左閾値<400,右閾値>600 |

### English :
| Item | Content |
| --- | --- |
| Common group component (name) | Joystick |
| Role of the common component | To control the servo angle |
| Arduino pins to be connected (planned) | A0 |
| Where the individual design cooperates with the common component | When the joystick is pushed to the left, the servo rotates left and the ultrasonic sensor scans in that direction. |
| Specifications that must be unified within the group | A0,A1,D2, left threshold <400, right threshold >600 |

> [!TIP]
> グループメンバー全員が同じピン番号・同じ閾値の単位を使っているか、
> グループレビューで確認し合ってください。


---
> **Purpose of this document**

## 1. システム全体像
> **Position in the V-Model**


### 1-1. ブロック構成（入力 → 処理 → 出力）

| Item | Transcription from requirements.md |

**記入例：**

```
【入力】                 【Arduino UNO R3】            【出力】
タクトスイッチ ─────→┐                         ┌──→ LED（赤・緑）
超音波センサー ─────→┤   状態管理・判定処理      ├──→ パッシブブザー
                    └─────────────────────────┘
```


**↓ 自分のシステムを書いてください**

### 日本語 :

```

【入力】                 【Arduino UNO R3】            【出力】
ジョイスティック(X軸)   ─→┐                         　　　　　　　　 ┌──→ SG90サーボモーター
超音波センサー(HC-SR04)─→┤ ジョイスティック値→角度制御／距離計測・判定 ├──→ LED
                        └───────────────────────────────────────┘ └──→ アクティブブザー
                                                                  └──→ シリアルモニター（角度・距離）
                                                     
```

### English :

```

[Input]                     [Arduino UNO R3]                       [Output]
Joystick (X‑axis)     ─→┐                                      ┌──→ SG90 Servo Motor
Ultrasonic Sensor     ─→┤  Joystick value → angle control      │
(HC‑SR04)               │  Distance measurement → judgment     ├──→ LED
                        └──────────────────────────────────────┘
                                                               └──→ Active Buzzer
                                                               └──→ Serial Monitor (angle / distance)

```

補足説明（図で表現しきれない場合）：  ジョイスティックを動かすとサーボがその方向に回り、超音波センサーが距離を測定します。近くに物体があると LED とブザーが作動し、角度と距離はシリアルモニターに表示されます。


---


### 1-2. 状態遷移（ガジェットの「ふるまい」設計）

> ※ ガジェットが「どんな状態を持つか」を定義します。
> この設計が loop() の骨格になります。


**記入例：**
```
[電源ON / 初期化]
  ↓（初期化完了）
[待機中] ──（センサー検知）──→ [計測中] ──（閾値超え）──→ [警告中]
   ↑                                                            │
   └────────────────────（リセットボタン）────────────────────────┘

```


**↓ 自分の状態遷移を書いてください**


### 日本語 :

```
[電源ON / 初期化]

  ↓（初期化完了）
[待機中] ──（loop開始）──→ [スキャン計測中]

[スキャン計測中] ──（distance > 0 かつ <= 20cm）──→ [警告中]
[警告中] ──（distance == 0 または > 20cm）──→ [スキャン計測中]

※ ボタン押下（LOW確定）時は、状態遷移せずにサーボ角度のみ90°へリセット


```

### English :

```
[Power ON / Initialization]
  ↓ (Initialization complete)
[Standby] ── (loop starts) ──→ [Scanning / Measuring]

[Scanning / Measuring] ── (distance > 0 and <= 20 cm) ──→ [Alert]
[Alert] ── (distance == 0 or > 20 cm) ──→ [Scanning / Measuring]

*When joystick button press is confirmed (LOW), servo resets to 90° without changing state.*

```

**各状態の意味**

### 日本語 :
| 状態名 | この状態のとき、何をしているか | LEDやブザーなど出力の様子 |
|:--|:--|:--|
| 待機中 | setup直後の初期状態。loop開始でスキャン計測へ移行する。 | LED OFF、ブザーOFF。|
| スキャン計測中 | 毎ループでジョイスティック処理・ボタン判定・距離計測を実行する。 | LED OFF、ブザーOFF（安全時）。|
| 警告中 | 距離が20cm以下の対象を検知し、警告状態を維持する。| LED ON、ブザーON（距離が20cmを超えたら停止）。|

### English :
| State Name | What the system is doing in this state | LED, buzzer, and other outputs in this state |
|:--|:--|:--|
| Standby | Initial state right after setup; transitions to scanning when loop starts. | LED OFF, buzzer OFF. |
| Scanning-Measuring | Runs joystick handling, reset-button check, and ultrasonic measurement every loop. | LED OFF, buzzer OFF (safe condition). |
| Alert | Keeps alert state when an object is detected within 20 cm. | LED ON, buzzer ON (turns off when distance is over 20 cm). |

補足（実装整合）：現行実装は `currentState` 変数を持たず、条件分岐（distance と入力値）で同等の状態遷移を実現している。

> [!TIP]
> すべての状態から「抜け出せるルート」があるか確認してください。
> 「このボタンを押せば必ずリセットに戻る」が保証されていますか？

---

## 2. ソフトウェア設計

### 2-1. データ設計（使う変数・定数の洗い出し）

> ※ プログラムで扱う「情報の種類」を先に整理します。
> ソフトウェアワークシート（AI駆動開発）でやった「データの洗い出し」と同じです。

### 日本語 :
| 情報の名前 | 何を表すか | 型（イメージ） | バイト数 | 初期値 | 備考 |
| --- | --- | --- | --- | --- | --- |
| servoAngle | サーボモーターの角度 | int | 2B | 90 | 0〜180度 |
| distance | 超音波センサーの計測距離 | int | 2B | 0 | loopごとに更新（cm） |
| alertState | アラート状態フラグ | bool | 1B | false | 20cm以下で true |
| buttonState | ボタンの確定状態 | int | 2B | HIGH | INPUT_PULLUPでLOW押下 |
| lastButtonState | ボタンの前回読み取り状態 | int | 2B | HIGH | 変化検出用 |
| lastDebounceTime | デバウンス開始時刻 | unsigned long | 4B | 0 | millis() 用 |
| debounceDelay | デバウンス時間 | unsigned long | 4B | 50 | ms |
| lastServoMove | サーボ更新用タイマー | unsigned long | 4B | 0 | millis() 用（10ms） |
| servoInterval | サーボ更新周期 | unsigned long | 4B | 10 | ms |
| lastAlertBlink | ブザー点滅用タイマー | unsigned long | 4B | 0 | millis() 用（200ms） |
| alertInterval | ブザー点滅周期 | unsigned long | 4B | 200 | ms |
| buzzerState | ブザーON/OFF状態 | bool | 1B | false | トグル制御 |
| duration | 超音波エコー時間 | long | 4B | 0 | pulseIn() 結果 |

### English :
| Data Name | What it represents | Type (image) | Bytes | Initial Value | Notes |
| --- | --- | --- | --- | --- | --- |
| servoAngle | Servo motor angle | int | 2B | 90 | Range 0–180° |
| distance | Ultrasonic sensor measured distance | int | 2B | 0 | Updated every loop (cm) |
| alertState | Alert state flag | bool | 1B | false | True when distance ≤ 20 cm |
| buttonState | Debounced button state | int | 2B | HIGH | LOW means pressed |
| lastButtonState | Previous button reading | int | 2B | HIGH | For change detection |
| lastDebounceTime | Debounce start timestamp | unsigned long | 4B | 0 | Used with millis() |
| debounceDelay | Debounce window | unsigned long | 4B | 50 | ms |
| lastServoMove | Timer for servo update | unsigned long | 4B | 0 | Used with millis() (10 ms) |
| servoInterval | Servo update interval | unsigned long | 4B | 10 | ms |
| lastAlertBlink | Timer for buzzer blink | unsigned long | 4B | 0 | Used with millis() (200 ms) |
| alertInterval | Buzzer blink interval | unsigned long | 4B | 200 | ms |
| buzzerState | Current buzzer ON/OFF state | bool | 1B | false | Toggle control |
| duration | Ultrasonic echo duration | long | 4B | 0 | pulseIn() result |

> [!CAUTION]
> **SRAM使用量チェック（Arduino UNO R3 の上限は 2048B）**
>
> グローバル変数の合計: ＿＿＿ B
>
> | 合計バイト数 | 判定 |
> |:--|:--|
> | ～1000B | ✅ 余裕あり |
> | 1000～1500B | ⚠️ char配列の長さを見直す |
> | 1500B超 | 🔴 AIに相談し変数を整理すること |

### 2-2. 機能一覧・関数一覧（役割の定義）

> ※ プログラムを「処理の部品（関数）」に分割します。
> **ここではコードを書かず、「何をする関数か」だけ日本語で書いてください。**
> 具体的な実装は詳細設計書（detailed_design.md）で行います。

> [!TIP]
> **単一責任の原則（1関数 = 1つの仕事）**
> 「機能名」欄に requirements.md の機能名（必須・追加）を書き、それを実現する関数名を対応させてください。
> 入出力の読み取り（readButton 等）や制御（updateOutput 等）は複数機能から使われる共通処理として「共通」と書いてOKです。

### 日本語 :
| 機能ID | 機能名 | 関数名 | 担う「1つの仕事」 | 主な引数 | 戻り値 | 呼び出す場所 |
| --- | --- | --- | --- | --- | --- | --- |
| F01 | サーボをジョイスティックで制御 | handleJoystick() | ジョイスティックX軸の閾値（<400, >600）で角度を±1更新しサーボを動かす | なし | なし | loop() 内 |
| F02 | 超音波センサーで距離計測 | readUltrasonic() | 超音波で距離を測定して戻り値で返す | なし | int（cm） | loop() 内 |
| F03 | アラート出力 | handleAlert() | 距離が20cm以下ならLED/ブザー制御、離れたら停止 | distance | なし | loop() 内 |
| F04 | ボタンで90°リセット | handleResetButton() | ボタン入力をデバウンスし、押下時にサーボを90°へ戻す | なし | なし | loop() 内 |

### English :
| Function ID | Function Name | Function | Single Responsibility (What it does) | Main Arguments | Return Value | Called From |
| --- | --- | --- | --- | --- | --- | --- |
| F01 | Control servo with joystick | handleJoystick() | Updates servo angle by threshold (<400, >600) and moves servo | None | None | Inside loop() |
| F02 | Measure distance with ultrasonic sensor | readUltrasonic() | Measures distance using ultrasonic sensor and returns cm value | None | int (cm) | Inside loop() |
| F03 | Alert output | handleAlert() | Turns LED/buzzer ON if distance ≤ 20 cm, OFF otherwise | distance | None | Inside loop() |
| F04 | Reset servo by button | handleResetButton() | Debounces button input and resets servo to 90° when pressed | None | None | Inside loop() |

> [!CAUTION]
> loop() の中で全部書こうとしていませんか？
> 機能ごとに関数を分けることで、グループレビューや単体テストがしやすくなります。

### 2-3. タイミング設計（delay vs millis）

> ※ `delay(N)` はN ms間、**すべての処理**が止まります。
> 複数の処理を並行させたい場合は `millis()` を使います。

### 日本語 :
| 処理 | 必要な周期 | delay / millis | 判断理由 |
| --- | --- | --- | --- |
| サーボ制御 | 10ms | millis | 実装の servoInterval=10ms に合わせるため |
| 距離計測 | 毎ループ | 関数呼び出し | loop() ごとに readUltrasonic() を実行するため |
| アラート出力 | 200ms（ブザー点滅） | millis | 実装の alertInterval=200ms に合わせるため |
| 角度・距離表示 | イベント発生時 | Serial出力 | 角度変化時/距離2cm以上変化時のみ出力するため |

### English :
| Process | Required Interval | delay / millis | Reason |
| --- | --- | --- | --- |
| Servo control | 10 ms | millis | Matches implementation value `servoInterval=10` |
| Distance measurement | Every loop | Function call | `readUltrasonic()` is called in each loop |
| Alert output | 200 ms (buzzer blink) | millis | Matches implementation value `alertInterval=200` |
| Angle & distance display | On event | Serial output | Printed only on angle change / distance delta >= 2 cm |

> [!CAUTION]
> `delay()` を使う場合、その間はボタン入力もセンサー読み取りも**すべて止まります**。

---

## 3. ハードウェア設計

### 3-1. 部品一覧とピン接続

> ※ デジタルピン: D2〜D13 / アナログピン: A0〜A5
> 特殊用途: D0/D1（シリアル通信・使用禁止）、A4/A5（I2C）、D10〜D13（SPI）

### 日本語 :
| 部品名 | 型番・種類 | Arduinoピン | 入力 / 出力 | 備考 |
| --- | --- | --- | --- | --- |
| SG90サーボモーター | サーボ | D3 | 出力 | PWM制御 |
| HC‑SR04超音波センサー | 距離センサー | D9, D10 | 入力/出力 | Trig: D9, Echo: D10 |
| ジョイスティック | アナログ入力 | A0 | 入力 | X軸のみ使用 |
| LED | 5mm LED | D6 | 出力 | 220Ω抵抗と直列 |
| アクティブブザー | ブザー | D8 | 出力 | — |

### English :
| Part Name | Model / Type | Arduino Pin | Input / Output | Notes |
| --- | --- | --- | --- | --- |
| SG90 Servo Motor | Servo | D3 | Output | PWM control |
| HC‑SR04 Ultrasonic Sensor | Distance sensor | D9, D10 | Input / Output | Trig: D9, Echo: D10 |
| Joystick | Analog input | A0 | Input | Using X‑axis only |
| LED | 5mm LED | D6 | Output | Use 220Ω resistor in series |
| Active Buzzer | Buzzer | D8 | Output | — |

> [!CAUTION]
> **ピン競合チェック（必須）**
> - D0/D1 は Serial 通信専用のため使用禁止
> - I2C 機器（LCD等）は A4/A5 を使用
> - 同じピンに複数の部品を繋いでいないか確認

**ピン競合チェック結果：** 競合なし / 競合あり → （内容）
**ピン競合チェック結果：** 競合なし
**Pin conflict check result:** No conflict

### 3-2. 電気的考慮事項

> **【任意】** 余裕があれば記入してください。抵抗値はAIに計算させてOKです。

### 日本語 :
| 用途 | 判断・計算メモ |
|:--|:--|
| LED電流制限抵抗 | （例）(5V − 2.0V) ÷ 0.02A = 150Ω → 220Ω使用 |
| ボタンのプルアップ | INPUT_PULLUP使用 / 外付け10kΩ / 不要（理由：） |
| センサーの動作電圧 | （データシートで確認した結果） |

### English :
| Purpose | Calculation/Notes |
|:--|:--|
| LED current limiting resistor | (5V − 2.0V) ÷ 0.02A = 150Ω → use 220Ω |
| Button pull-up | Use INPUT_PULLUP / external 10kΩ / Not needed (reason:) |
| Sensor operating voltage | Confirmed by datasheet |

---

## 4. 異常系・エラー処理の方針

### 日本語 :
| 異常ケース | 発生条件 | 検知方法 | 対応方針 |
| --- | --- | --- | --- |
| チャタリング | ボタンを押したとき | 前回入力から50ms未満の入力を検出 | デバウンス処理でスキップ |
| センサー異常値 | センサーが誤読したとき | 値が0（pulseInタイムアウト相当） | distance>0 条件でアラート判定から除外 |
| アナログノイズ | アナログ入力を読んだとき | 閾値付近で値が揺れる | 中立域（400〜600）で不必要な移動を抑制 |
| サーボ異常 | サーボが動かない／異常音 | サーボ角度が変化しない | エラー表示・再初期化 |

### English :
| Abnormal Case | Condition | Detection Method | Response Policy |
| --- | --- | --- | --- |
| Chattering | When a button is pressed | Detect input occurring within 50 ms of previous input | Skip using debounce processing |
| Sensor abnormal value | When the sensor misreads | Distance becomes 0 (pulse timeout equivalent) | Exclude from alert condition by requiring `distance > 0` |
| Analog noise | When reading analog input | Value fluctuates near thresholds | Use neutral band (400–600) to suppress unnecessary movement |
| Servo malfunction | Servo not moving / strange noise | Servo angle does not change | Show error / reinitialize |
---

## 5. 要件 ←→ 設計対応表（トレーサビリティ）

### 日本語 :
| No | 必須機能（requirements.md 3-1 から転記） | 対応するSW設計（関数名等） | 対応するHW設計（ピン等） | 結合テスト No |
|:---|:---|:---|:---|:---|
| 1 | ジョイスティックの動きに合わせてサーボモーターを回すことができる | handleJoystick() | D3, A0 | Test#1 |
| 2 | 超音波センサーで距離をはかることができる | readUltrasonic() | D9, D10 | Test#2 |
| 3 | 現在の角度と距離をシリアルモニターに表示できる | handleJoystick(), handleAlert() | D3, D9, D10 | Test#3 |
| 4 | 近くに物があるとLEDとブザーでアラートを出すことができる | handleAlert() | D6, D8 | Test#4 |

### English :
| No | Required Feature (from requirements.md 3-1) | Corresponding SW Design (function name) | Corresponding HW Design (pins) | Integration Test No |
|:---|:---|:---|:---|:---|
| 1 | Rotate servo with joystick | handleJoystick() | D3, A0 | Test#1 |
| 2 | Measure distance with ultrasonic sensor | readUltrasonic() | D9, D10 | Test#2 |
| 3 | Display angle & distance on Serial Monitor | handleJoystick(), handleAlert() | D3, D9, D10 | Test#3 |
| 4 | Alert with LED & buzzer when object is close | handleAlert() | D6, D8 | Test#4 |

---

## 6. 結合テスト計画（V字モデル：基本設計 ↔ 結合テスト）

### 日本語 :
| No | テスト対象（requirements.md の必須機能から転記） | テスト手順（操作方法） | 期待する結果 | 実際の結果 | 合否 |
|:---|:---|:---|:---|:---|:---|
| 1 | ジョイスティックでサーボを動かす | ジョイスティックを左右に動かす | サーボが0-180度で追従する | | [ ] |
| 2 | 超音波センサーで距離計測 | 前方に物体を置く | 距離が正しく表示される | | [ ] |
| 3 | 角度・距離をシリアル表示 | サーボを動かしながら計測 | シリアルに角度・距離が表示される | | [ ] |
| 4 | LED・ブザーでアラート | 20cm以内に物体を近づける | LED・ブザーがONになる | | [ ] |
| 5 | 異常系：チャタリング | ボタンを素早く2回押す | 1回だけ反応する | | [ ] |
| 6 | 異常系：センサー異常値 | センサーを塞ぐ/最大距離に向ける | 誤動作しない | | [ ] |

### English :
| No | Test Target (from requirements.md) | Test Procedure (operation) | Expected Result | Actual Result | Pass/Fail |
|:---|:---|:---|:---|:---|:---|
| 1 | Move servo with joystick | Move the joystick left and right | Servo follows 0-180 degrees | | [ ] |
| 2 | Measure distance with ultrasonic sensor | Place an object in front | Distance is displayed correctly | | [ ] |
| 3 | Display angle & distance on Serial Monitor | Move servo while measuring | Angle and distance are shown on serial | | [ ] |
| 4 | Alert with LED & buzzer | Bring object within 20cm | LED and buzzer turn ON | | [ ] |
| 5 | Chattering (abnormal) | Press button quickly twice | Only reacts once | | [ ] |
| 6 | Sensor abnormal value | Block sensor / point to max distance | No malfunction occurs | | [ ] |

---

## 7. AIレビュー記録

> グループレビューの前に必ず実施してください。

### Q1: 設計の妥当性確認

> 「この基本設計書をもとに Arduino UNO R3 でコードを書きます。ピン競合・ライブラリ競合・状態遷移の抜け・delay() の問題点はありますか？」

**AIの回答（要約）：**
- ピン競合はありません。各部品のピン割り当ても適切です。
- ライブラリ競合も特に発生しません（Servo, NewPing等の標準的なライブラリで十分）。
- 状態遷移も抜けがなく、すべての状態から抜け出せるルートが設計されています。
- `delay()` は未使用だが、`delayMicroseconds()` と `pulseIn()`（ブロッキング呼び出し）を使うため、厳密には完全非ブロッキングではありません。

**AI’s Answer (Summary) :**
- There are no pin conflicts. Pin assignments for each component are appropriate.
- No library conflicts are expected (standard libraries like Servo and NewPing are sufficient).
- The state transitions are complete, and every state has a clear exit route.
- `delay()` is not used, but `delayMicroseconds()` and blocking `pulseIn()` are used, so the loop is not fully non-blocking.

**対応した内容：**
- ブロッキング呼び出し（`pulseIn()`）の注意点を設計上の補足として明記。

**Actions Taken**
- Added a design note that `pulseIn()` is blocking and may affect responsiveness.

---

### Q2: チャタリング・delay・ノイズの確認

> 「この設計にチャタリング対策は含まれていますか？delay() を使っている箇所で問題はありますか？アナログセンサーを使う場合、ノイズ対策は必要ですか？」


**AIの回答（要約）：**
- チャタリング対策として、ボタン入力にデバウンス処理が設計されています。
- `delay()` は未使用だが、`delayMicroseconds()` と `pulseIn()` を使うため、完全にノンブロッキングではありません。
- アナログセンサーのノイズ対策として、閾値にバッファ（±10〜20）を設けています。

**AI’s Answer (Summary) :**
- Debounce processing is included for button input to prevent chattering.
- `delay()` is not used, but `delayMicroseconds()` and `pulseIn()` are used, so the loop is not fully non-blocking.
- For analog sensor noise, a buffer (±10–20) is set around the threshold.

**対応した内容：**
- `pulseIn()` のブロッキング特性を注意点として追記済みです。

**Actions Taken**
- Added note that `pulseIn()` can block the loop and affect responsiveness.
---

### Q3: 要件への対応漏れ確認（任意）

> **【任意】** トレーサビリティ表（Section 5）を書いた場合に実施してください。
>
> 「要件定義書の必須機能と、この基本設計書のSection 5 を比較して、設計漏れや結合テスト計画の抜けを教えてください。」

**AIの回答（要約）：**
- Section 5のトレーサビリティ表に、要件定義書の必須機能がすべて対応付けられています。
- 結合テスト計画（Section 6）にも全必須機能のテストが記載されており、抜けはありません。

**AI’s Answer (Summary) :**
- All required features from the requirements are mapped in the traceability table (Section 5).
- All required features are also covered in the integration test plan (Section 6); there are no omissions.

**対応した内容：**
- 設計・テスト計画ともに対応漏れなし。現状のままで問題ありません。

**Actions Taken**
- No omissions in design or test plan. No further action needed.
---

## 8. グループレビュー記録

### 8-1. 指摘一覧

| No | 指摘内容 | 指摘者 | 対応 |
|:---|:---|:---|:---|
| 1 | この二日間ではできないかなと思って辞めた機能ありますか。 | マウン | 今回は見送り、後日追加機能として検討する  |

### 8-2. レビューを受けて変更した点

- ありません。

---

*初版: 2026-05-22 / AIレビュー: 2026-05-22 / グループレビュー後更新: 2026-05-22*
