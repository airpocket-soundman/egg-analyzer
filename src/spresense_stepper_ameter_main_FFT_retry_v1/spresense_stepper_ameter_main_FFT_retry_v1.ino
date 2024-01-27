//use spresense_stepper_ameter_subcore_v6.ino
//use spresense_onsen_egg_checker_v3.ino
#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

// データ格納用変数
#define TOTAL_MESUREMENTS 1024              //測定回数
#define MESUREMENT_CYCLE 2500               //測定頻度 us秒/回
#define MESURE_START_DELAY 250000           //モータースタートから測定開始までのディレイタイム　usec
int freq = 1000000 / MESUREMENT_CYCLE;



#include <Arduino.h>
#include <Wire.h>

#include <MP.h>                                   //マルチコア制御
int subcore1   = 1;                               //使用するサブコア

#include "LGFX_SPRESENSE_sample.hpp"              //ディスプレイ制御
static LGFX lcd;                                  //ディスプレイインスタンス作成

#include <arduinoFFT.h>                           //FFT用
double vReal[TOTAL_MESUREMENTS];                  // vReal[]にサンプリングしたデータを入れる
double vImag[TOTAL_MESUREMENTS];
arduinoFFT FFT = arduinoFFT(vReal, vImag, TOTAL_MESUREMENTS, freq);  // FFTオブジェクトを作る


#include <File.h>                                 //ファイル操作
#include <SDHCI.h>                                //microSD操作
SDClass SD;
File myFile;

#include "ammeter.h"                              //M5Stack 電流センサ
Ammeter ammeter;  /**Ameter object**/
double am_value[TOTAL_MESUREMENTS];
int elapsed_time[TOTAL_MESUREMENTS];

#include <DNNRT.h>                                //DNN推論用
DNNRT dnnrt;

//ファイル保存用変数
int number = 0;       //測定の通し番号

//gpio 定義
const int SW1Pin = 4;
const int SW2Pin = 5;
const int SW3Pin = 6;
const int SW4Pin = 7;
const int SW5Pin = 3;
int SW1State    = LOW;
int SW2State    = LOW;
int SW3State    = LOW;
int SW4State    = LOW;
int SW5State    = LOW;
int SW1StateOld = LOW;
int SW2StateOld = LOW;
int SW3StateOld = LOW;
int SW4StateOld = LOW;
int SW5StateOld = LOW;


void setup() {
  Wire.begin();                 //i2cを有効化
  Wire.setClock(1000000);       //standart mode:100000, fast mode:400000, fast mode plus:1000000, high speed mode:3400000
  Serial.begin(115200);         //USBシリアル通信を有効化

  int ret = 0;
  ret = MP.begin(subcore1);                           //subcore1を起動
  if (ret < 0) {                                      //subcore起動失敗時の表示
    MPLog("MP.begin(%d) error = %d\n", 1, ret);
  }

  //ameter 設定
  ammeter.setMode(SINGLESHOT);
  ammeter.setRate(RATE_860);    //8,16,32,64,128(defo),250,475,860
  ammeter.setGain(PAG_2048);
//  hope = page512_volt / ammeter.resolution;
  // | PAG      | Max Input Voltage(V) |
  // | PAG_6144 |        128           |
  // | PAG_4096 |        64            |
  // | PAG_2048 |        32            |
  // | PAG_512  |        16            |
  // | PAG_256  |        8             |

  //Switch pin 設定
  pinMode(SW1Pin, INPUT_PULLUP);
  pinMode(SW2Pin, INPUT_PULLUP);
  pinMode(SW3Pin, INPUT_PULLUP);
  pinMode(SW4Pin, INPUT_PULLUP);
  pinMode(SW5Pin, INPUT_PULLUP);

  lcd.init();                       //ili9431を初期化
  lcd.setRotation(1);               //右へ90°回転
}

void loop() {
  double am_value_local[TOTAL_MESUREMENTS];
  SW1StateOld = SW1State;
  SW1State = digitalRead(SW1Pin);
  SW2StateOld = SW2State;
  SW2State = digitalRead(SW2Pin);
  SW3StateOld = SW3State;
  SW3State = digitalRead(SW3Pin);
  SW4StateOld = SW4State;
  SW4State = digitalRead(SW4Pin);
  SW5StateOld = SW5State;
  SW5State = digitalRead(SW5Pin);

  lcd.setCursor(0,0);

  //SW1が押されたとき
  if (SW1State == LOW && SW1StateOld == HIGH){

    //測定を指定回数自動で繰り返す。
    int mesure_count = 10;   //測定回数(回)
    int mesure_delay = 1500; //測定と測定の間隔（msec) 1500
    Serial.println("SW1 pushed");

    for (int i = 0; i < mesure_count; i ++){
      
      delay(mesure_delay);
      lcd.printf("LOW ");
      Serial.println("mesure current");

      mesure_current();
      Serial.println(vReal[0]);
      Serial.println("f to d");
      for(int i = 0; i < TOTAL_MESUREMENTS; i++){
        am_value[i] = vReal[i];
//        Serial.println(am_value[i]);
      }

      Serial.println("DCRemove");
//      dummy_data();
      DCRemoval(vReal, TOTAL_MESUREMENTS);
      Serial.println("FFT analyze");
      FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // 窓関数
      FFT.Compute(FFT_FORWARD); // FFT処理(複素数で計算)
      FFT.ComplexToMagnitude(); // 複素数を実数に変換
      Serial.println("normalization");
      normalization(vReal, TOTAL_MESUREMENTS);
      normalization(am_value, TOTAL_MESUREMENTS);
      Serial.println("draw graph");
      drawGraph(am_value);
      //drawGraph(vReal); 
      Serial.println("saveData");


      saveData();
      
    }

  } else if (SW1State == HIGH && SW1StateOld == LOW) {
    lcd.printf("HIGH");
  }

  lcd.setCursor(0,20);
  if (SW2State == LOW && SW2StateOld == HIGH){
    lcd.printf("LOW ");
  } else if (SW2State == HIGH && SW2StateOld == LOW) {
    lcd.printf("HIGH");
  }

  lcd.setCursor(0,40);
  if (SW3State == LOW && SW3StateOld == HIGH){
    lcd.printf("LOW ");
        
    double current = ammeter.getCurrent();
    MPLog("Cal volt: %.2f mA             \r\n", current);
  } else if (SW3State == HIGH && SW3StateOld == LOW) {
    lcd.printf("HIGH");
  }

}

void dummy_data(){
  double current;
  int elapsedTime = 2000;
  for (int i = 0; i<1024 ; i++){
    current = sin( double(i) / 100 * PI) + sin(double(i)/10 * PI) * 0.5 + sin(double(i)/3 * PI) * 0.01;

    am_value[i] = current;
    vReal[i] = current;
    vImag[i] = 0;

  }

}

void mesure_current(){
  double current;
  int now;
  int old;
  int elapsedTime;

  int ret = MP.Send(3,50000,subcore1);
  delayMicroseconds(MESURE_START_DELAY);    //測定開始までのdelay
  Serial.println("start mesure cycle");
  for(int i = 0; i < TOTAL_MESUREMENTS; i++){
    if (i == 0){
      now = micros();
      current = ammeter.getCurrent();
      
    } else {
      old = now;
      elapsedTime = now - old; 
      while(elapsedTime <= MESUREMENT_CYCLE){
        now = micros();
        elapsedTime = now - old;
      }
      current = ammeter.getCurrent();
      old = now;
    }
//    Serial.println(elapsedTime);
    elapsed_time[i]  = elapsedTime;
    vReal[i] = current;
    vImag[i] = 0;
  }
  Serial.println(elapsedTime);

}


void normalization (double *data, int sample_num){
  double minValue = data[0];
  double maxValue = data[0];

  for (int i = 0; i < sample_num; i++) {
    if (data[i] < minValue) {
      minValue = data[i];
    }
    if (data[i] > maxValue) {
      maxValue = data[i];
    }
  }
  for (int i = 0; i < sample_num; i++) {
    data[i] = (data[i] - minValue) / (maxValue - minValue);
  }

}

void saveData(){
  SD.begin();
  char fname[16];
  sprintf(fname, "fft_%03d.csv", number);
  myFile = SD.open(fname,FILE_WRITE);
  if (!myFile){
    Serial.println("SD Open Error: "+String(fname));
    return;
  }

  for (int i = 0; i < TOTAL_MESUREMENTS / 2; i++){
    myFile.println(vReal[i]);
  }
  myFile.close();

  sprintf(fname, "am_%03d.csv", number);
  myFile = SD.open(fname,FILE_WRITE);
  if (!myFile){
    Serial.println("SD Open Error: "+String(fname));
    return;
  }

  for (int i = 1; i < TOTAL_MESUREMENTS; i++){
    myFile.println(am_value[i]);
  }
  myFile.close();

  sprintf(fname, "time_%03d.csv", number);
  myFile = SD.open(fname,FILE_WRITE);
  if (!myFile){
    Serial.println("SD Open Error: "+String(fname));
    return;
  }

  for (int i = 1; i < TOTAL_MESUREMENTS; i++){
    myFile.println(elapsed_time[i]);
  }
  myFile.close();

  lcd.setCursor(0,220);
  lcd.printf("number %d saved",number);

  number++;
}

void drawGraph(double *data){
  lcd.fillScreen(lcd.color888(0, 0, 0));
  double max = data[0];
  double min = data[0];
  for (int i = 0; i < TOTAL_MESUREMENTS; i++ ){
    if (data[i] > max) max = data[i];
    if (data[i] < min) min = data[i];
  }

  int graph_size_x = 320;
  int graph_size_y = 200;
  int y0 = graph_size_y - 0;
  int y1;
  int x1;
  for (int i = 0; i < TOTAL_MESUREMENTS; i++ ){
    if (i + 1 < graph_size_x){
      x1 = i;
    }else if(i >= graph_size_x){
      x1 = i % graph_size_x;
    }
    y1 = int(graph_size_y - data[i] / (max - min) * graph_size_y);
    if(i > 0){
      lcd.drawLine(x1 - 1, y0, x1, y1, lcd.color888(0, 255, 0));
    }
    y0 = y1;
  }
}

void DCRemoval(double *data, int sample_num) {
  double mean = 0;
  for (uint16_t i = 1; i < sample_num; i++) {
      mean += data[i];
  }
  mean /= TOTAL_MESUREMENTS;
  for (uint16_t i = 1; i < sample_num; i++) {
      data[i] -= mean;
  }
}