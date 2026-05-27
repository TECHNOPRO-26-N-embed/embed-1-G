# 要件定義書 — 組込み開発実習

<!-- 作成者: トゥタココ　マウン / 日付: 2026-05-21 / グループ: 1-G -->
---

## 1. プロジェクト概要

### 1-1. 作品タイトル
<!-- > （例：「ものを置いたら自動でライトがつく省エネ書斎デスク」） -->

**タイトル：**
#### 日本語 

ジョイスティックで動かす超音波スキャンとアラート装置

#### English 

Joystick‑Controlled Ultrasonic Scanning and Alert Device

### 1-2. 一言で言うと何をするガジェットか？
<!-- > ※ 30字以内で。AIに最初に投げるプロンプトの素になります。 -->

**一言説明：**
#### 日本語 

物の距離をはかり、近くに物があればアラートを出す装置

#### English 

A device that scans distance and alerts when objects are detected

### 1-3. なぜこれを作るのか（動機・背景）
<!-- > ※ 解決したい不便さ・あったら嬉しい理由を書いてください。 -->

**動機：**
#### 日本語 

インターネットで、超音波センサーとサーボモーターを使ったArduinoの初心者向けプロジェクトを見て、自分でも作ってみたいと思いました。いろいろな部品（ジョイスティック、サーボ、超音波センサー、LED、ブザー）を組み合わせて動かすことを学びたいです。

#### English 

I decided to make this ultrasonic scanning device because I saw several beginner Arduino projects online that used ultrasonic sensors and servo motors. They looked interesting and fun to try, so I wanted to challenge myself and build my own version. I also want to learn how to combine multiple components such as a joystick, servo, ultrasonic sensor, LED, and buzzer into one working system.

---

## 2. ユーザーと利用シーン

### 2-1. 誰が使うか
<!-- > （例：自分自身、家族、特定の職業の人、など） -->

**ユーザー：**
#### 日本語 

自分自身、家族

#### English 

Myself,Famaily

### 2-2. いつ・どこで使うか
<!-- > （例：「部屋の電気を消してから寝る前に」「職場のデスクで日中」） -->

**利用シーン：**
#### 日本語 

家に一人でいるとき、前で何が起きているか知りたいときに使う。

#### English 

When I am at home by myself and want to detect if anything is happening in front of the device.

---

## 3. 機能要件

### 3-1. このガジェットができること（必須機能）
<!-- > ※ 「〜できる」の形で箇条書き。最低3つ書いてください。 -->
#### 日本語 

- ジョイスティックの動きに合わせてサーボモーターを回すことができる
- 超音波センサーで距離をはかることができる
- 現在の角度と距離をシリアルモニターに表示できる
- 近くに物があるとLEDとブザーでアラートを出すことができる


#### English 

- It can rotate a servo motor based on joystick input.
- It can measure distance using an ultrasonic sensor.
- It can display the current angle and distance on the  Serial Monitor.
- It can activate an LED and buzzer when an object is detected within a set distance.

### 3-2. あったらいい機能（任意・余裕があれば）
#### 日本語 

- ジョイスティックを使わずに自動で左右にスキャンできる
- LCDやOLEDに距離を表示できる
- 距離によってブザーの音を変えることができる

#### English 

- It can automatically scan left and right without joystick input.
- It can show distance on an LCD or OLED display.
- It can change buzzer sound depending on how close the object is.

### 3-3. やらないこと（スコープ外）
<!-- > ※ 「〜はしない」で書いてください。AIが勝手に膨らませる機能を防ぐために重要です。 -->
#### 日本語 

- ロボット本体を動かすことはしない
- パソコンにレーダー画面を表示することはしない
- 複数の超音波センサーは使わない

#### English 

- It will NOT control a moving robot.
- It will NOT display radar graphics on a PC.
- It will NOT use multiple ultrasonic sensors.

### 3-4. 非機能要件（任意）
<!-- > ※ 余裕があれば書いてください。AIはここを無視しがちです。 -->

- **安全性**<!-- センサーの値が異常なとき（0や最大値など）どうするか： -->
#### 日本語 

センサーが0cmや最大値などの異常な値を出したときは、「エラー」と表示し、アラートを出さない。

#### English 

If the ultrasonic sensor reads abnormal values (0 cm or max range), display “Measurement Error” and do not activate alerts.

- **応答時間**<!-- 操作してから何秒以内に反応すればOKか： -->
#### 日本語 

ジョイスティックを動かしてから0.2秒以内にサーボが動くこと。

#### English 

The servo should react within 0.2 seconds after joystick movement.

---

## 4. 使用部品リスト

### 4-1. グループ共通部品（グループ全員で使うもの・1つ）

**共通部品：**
#### 日本語 

ジョイスティック

#### English 

Joystick

**選定理由：**
#### 日本語 

これは360度動かせて、押すこともできるので、いろいろなことに使えると思いました。

#### English 

because it can move 360 and can press so that think we can do so many thing with this

### 4-2. 自分が追加で使う予定の部品（ELEGOOキットより）
#### 日本語 

| 部品名 | 何のために使うか（機能との対応） |
|:---|:---|
| SG90 サーボモーター | 超音波センサーの向きを動かすため |
| HC‑SR04 超音波センサー | 距離を測るため |
| ジョイスティックモジュール | サーボの角度を操作するため |
| LED + 220Ω 抵抗 | 近くに物があると光で知らせるため |
| アクティブブザー | 近くに物があると音で知らせるため |
| ジャンパーワイヤー | すべての部品をつなぐため |
| ブレッドボード | 配線をまとめるため |

#### English 

| Component Name | What it is used for (related function) |
|:---|:---|
| SG90 Servo Motor | Rotate the ultrasonic sensor |
| HC‑SR04 Ultrasonic Sensor | Measure distance |
| Joystick Module | Control servo angle |
| LED + 220Ω resistor | Light alert when object is close |
| Active Buzzer | Sound alert when object is close |
| Jumper wires | Connect all components |
| Breadboard | Organize wiring |

---

## 5. AIレビュー記録

<!-- > ※ 下記の質問をAIに投げ、回答と自分の対応を記録してください。 -->

### Q1: 不足の確認
<!-- > 「この要件定義書をもとに基本設計を作ろうとしたとき、不足している情報や曖昧な点はどこですか？」 -->

#### 日本語 

AIの回答（要約）：
- サーボの回転範囲が書かれていない
- ジョイスティックのどの軸を使うか書かれていない
- アラートを出す距離が書かれていない
- 出力方法がはっきりしていない

#### English 

AI’s Answer (Summary) :
- Servo rotation range was not specified
- Joystick axis was not specified
- Alert distance threshold was missing
- Output method needed clarification

#### 日本語 

対応した内容：
- サーボの範囲を0〜180°にした
- ジョイスティックはX軸を使うと決めた
- アラート距離を20cmにした
- 出力はシリアルモニターだけにした

#### English 

Actions Taken :
- Set servo range to 0–180°
- Use joystick X‑axis
- Set alert distance to 20 cm
- Output will be Serial Monitor only

### Q2: 実現可能性の確認
<!-- > 「この要件はArduino UNO R3とELEGOOキットで実現できますか？難しい要件があれば教えてください。」 -->

#### 日本語 

AIの回答（要約）：
- Arduino UNO R3とELEGOOキットで全部実現できる
- LEDとブザーのアラートは簡単
- オプション機能は少しコードが必要だが可能

#### English 

AI’s Answer (Summary) :
- All required features are possible with Arduino UNO R3 and ELEGOO kit
- LED + buzzer alert is easy to implement
- Optional features require extra coding but are still feasible

#### 日本語 

対応した内容 :
- 必須機能はそのままにした
- オプション機能を整理して追加した
- サーボとセンサーはArduinoの5Vで動かせることを確認した

#### English 

Actions Taken (Scope Changes, etc.) :
- Kept all required features
- Moved advanced features to optional
- Confirmed servo and sensors can be powered from Arduino 5V

---

## 6. グループレビュー記録

<!-- > ※ グループレビュー（15:00〜）で受けた指摘を記録してください。 -->

### 6-1. 指摘一覧

#### 日本語 

| No | 指摘内容 | 対応（修正した / しなかった理由） |
|:---|:---|:---|
| 1 | ジョイスティックをどう動かしたら、センサーがどう動くか？ | 修正しなかった |
| 2 | モーターにセンサをつけて、動かす？ | 修正しなかった |

#### English 

| No | Review Comment | Response (Fixed / Reason Not Fixed) |
|:---|:---|:---|
| 1 | How does moving the joystick make the sensor move? | Not fixed |
| 2 | Is the sensor attached to the motor and moved together? | Not fixed |

### 6-2. レビューを受けて変更した点

#### 日本語 

- ジョイスティックを左右に動かすと、サーボモーターが0〜180°の間で左右に動くことを説明しました。
- 超音波センサーはサーボモーターの上に取り付けてあり、サーボが動くとセンサーも一緒に動くことを説明しました。

#### English 

- I explained that when you move the joystick left or right, the servo motor moves left or right between 0° and 180°.
- I explained that the ultrasonic sensor is attached on top of the servo motor, so when the servo moves, the sensor moves together with it.
---

*初版: 2026-05-21 / グループレビュー後更新: 2026-05-21*
