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

## Spresense main board

## Spresense extention board
※今回使用するILI9431の動作電圧は3V3の為、IO voltage jumperを3V3側に設定する事


## B-stem 4CM01 SPRESENSE用4chマイク基板
https://www.switch-science.com/products/7996

## ILI9341
※今回使用するILI9431の動作電圧は3V3の為、IO voltage jumperを3V3側に設定する事
https://developer.sony.com/spresense/development-guides/images/connect_ili9341.png


## A4988 stepper motor driver



# 参考ページ
## steping motor
https://forum.arduino.cc/t/stepper-motor-basics/275223
https://iot.keicode.com/arduino/arduino-stepper-motor-a4988.php
