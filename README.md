# HandyFan

HighPowerHandyFan is a high-power handheld fan project using a CH32V003 microcontroller, a 4-pin PWM fan, and a 3D printed enclosure.

強力な 4 ピン PWM ファンを、CH32V003 で回転数制御するハンディファンの製作データです。ファームウェア、3D プリントケース、関連する技術同人誌の情報をまとめています。

![Dojinshi cover](DojinshiTop.png)


## Contents

- `CH32V003F4P6/` - MounRiver Studio 用の CH32V003F4P6 ファームウェアプロジェクト
- `3DP_Case/SliderFAN_SanAce60/` - San Ace 60 用 3D プリントケースの STL データ
- `DojinshiTop.png` - 技術同人誌の表紙画像
- `LICENSE` - ライセンス

## Firmware

ファームウェアは `CH32V003F4P6/` に入っています。`main.c` だけではなく、スタートアップ、リンカスクリプト、WCH の peripheral library、MounRiver Studio のプロジェクト設定を含めています。そのため、MounRiver Studio でプロジェクトごと開いてビルドしやすい構成です。

主なピン割り当て:

| Function | CH32V003 pin | Note |
| --- | --- | --- |
| Fan PWM | `PC6` / `TIM1_CH1` | 25 kHz PWM。4 ピンファンの PWM 入力用 |
| Speed command | `PD2` / `ADC A3` | ボリューム等のアナログ入力 |
| LED | `PA1` | active-low 想定。PA1 を使うためクロックは HSI 48 MHz |

PWM は `TIM1_CH1` を `PC6` にリマップして出力しています。4 ピンファンの PWM 入力を想定して、出力は open-drain にしています。

## Build

1. MounRiver Studio をインストールします。
2. `CH32V003F4P6/` を既存プロジェクトとしてワークスペースへインポートします。
3. ターゲットが `CH32V003F4P6` になっていることを確認します。
4. ビルドして書き込みます。

## 3D Printed Case

ケースデータは `3DP_Case/SliderFAN_SanAce60/` にあります。

- `SliderFAN-L.stl`
- `SliderFAN-R.stl`

San Ace 60 クラスのファンを想定した左右分割ケースです。使用するファン、バッテリー、配線に合わせて必要に応じて調整してください。

## Doujinshi

このプロジェクトは、製作過程や設計意図をまとめた技術同人誌とも連動しています。同人誌では、部品選定、4 ピン PWM ファンの制御、CH32V003 の使い方、バッテリーや安全面の注意点などを解説しています。


## Safety

このプロジェクトでは高出力ファンとバッテリーを扱います。短絡、過電流、配線の発熱、ファンブレードへの接触には注意してください。

- 初回通電時は電流制限やヒューズを使う
- ファンガードを付ける
- バッテリーの極性を確認する
- 配線やコネクタの電流容量を確認する
- 異音、発熱、焦げ臭さがある場合はすぐ停止する

## License

This repository is released under the Apache License 2.0. See `LICENSE`.

Some source files under `CH32V003F4P6/` are based on WCH-provided startup, core, and peripheral library files. Those files retain their original copyright notices.
