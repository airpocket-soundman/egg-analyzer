# egg-analyzer
卵の茹で加減を非破壊分析します。


# 開発環境
開発はArduino IDE 2.2.1を使用します。  
使用するライブラリ類は以下の通りです。  

LovyanGFX 1.1.9  

## LovyanGFX
spresenseでLovyanGFXを使用する際にはspresense向けのclass設定をインクルードする必要があります。  
LovyanGFXライブラリ内の/src/lgfx_user/LGFX_SPRESENSE_sample.hppに、spresenseでILI9341という液晶ディスプレイを使用する設定が用意されていますので今回はこれを使用します。  
設定ファイルをインクルードするには、LGFX_SPRESENSE_sample.hppをプロジェクトフォルダに事前にコピーするか、インストールしたフォルダのpathを指定します。  
ここでは、LGFX_SPRESENSE_sample.hppをプロジェクトフォルダ内にコピーして使用します。  


# Hardware

BOM
|名前|数量|備考|
|-|-|-|
|SPRESENSE メインボード|1|(switch science)[https://www.switch-science.com/products/3900]|
|SPRESENSE 拡張ボード|1|(switch science)[https://www.switch-science.com/products/3901]|
|ILI9341 2.2" TFT LCD|1|amazonなどから|
|A4988 ドライバボード|1|amazonなどから|
|Nema17 HS4023|1|[秋月電子](https://akizukidenshi.com/catalog/g/g105372/)|
|M5Stack用電流計ユニット|1|(switch science)[https://www.switch-science.com/products/6739]|
|60mmx80mm ユニバーサル基板|1||
|M2x10 ボルト|8|基板固定用|
|M3x5 皿ボルト|4|ステッピングモーター固定用|
|M3x10 ボルト|6|トップパネル固定用|
|タクトスイッチ|1|[秋月電子](https://akizukidenshi.com/catalog/g/g115660/)|
|φ2.1×φ5.5電源ジャック|1|[秋月電子](https://akizukidenshi.com/catalog/g/g106568/)|
|XHコネクタオス/メス 2P|1||
|XHコネクタオス/メス 3P|1||
|XHコネクタオス/メス 4P|4||
|XHコネクタオス/メス 5P|1||
|QIコネクタオス 2P|1||
|QIコネクタオス 8P|1||
|CASING BASE|1|3Dプリンタ出力※|
|CASING TOP MOTOR|1|3Dプリンタ出力※|
|CASING TOP DISPLAY|1|3Dプリンタ出力※|
|EGG HOLDER BASE|1|3Dプリンタ出力※|
|EGG HOLDER TOP|1|3Dプリンタ出力※|


## Spresense main board

## Spresense extention board
※ILI9431の動作電圧は3V3の為、IO voltage jumperを3V3側に設定する事  
![pinout](https://docs.neqto.com/images/spresense_hw_pinout_exboard.png)

## ILI9341
※今回使用するILI9431の動作電圧は3V3の為、IO voltage jumperを3V3側に設定する事  
![接続図](https://developer.sony.com/spresense/development-guides/images/connect_ili9341.png)

## A4988 stepper motor driver

## Nema17 stepper motor

## 3D printer parts

## tact switch


# 参考ページ
## steping motor
https://forum.arduino.cc/t/stepper-motor-basics/275223  
https://iot.keicode.com/arduino/arduino-stepper-motor-a4988.php
