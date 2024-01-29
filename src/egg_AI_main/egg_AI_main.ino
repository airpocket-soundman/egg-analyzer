//たまごAIメインコア用コード
//subcore1用コードはegg_AI_sub.ino
//microSDカードに学習済みモデルを保存して実行します。

#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <Arduino.h>
#include <Wire.h>
#include "arduinoFFT.h"

#include <MP.h>
int subcore1   = 1;               //使用するサブコア

#include "LGFX_SPRESENSE_sample.hpp"
static LGFX lcd;                  //ディスプレイインスタンス作成

#include <DNNRT.h>
DNNRT dnnrt;

#include <SDHCI.h>
#include <Flash.h>
SDClass SD;                       /**< SDClass object */ 
File nnbFile;
File myFile;

//ファイル保存用変数
int number = 0;       //測定の通し番号

// 測定条件
#define TOTAL_MESUREMENTS   1024    //測定回数
#define MESUREMENT_CYCLE    2500    //測定頻度 us秒/回
#define MESURE_START_DELAY  250000   //モータースタートから測定開始までのディレイタイム　usec
int freq = 1000000 / MESUREMENT_CYCLE;

//FFT用
double vReal[TOTAL_MESUREMENTS];  // vReal[]にサンプリングしたデーターを入れる
double vImag[TOTAL_MESUREMENTS];  // 虚数部
arduinoFFT FFT = arduinoFFT(vReal, vImag, TOTAL_MESUREMENTS, freq);  // FFTオブジェクトを作る

#include "ammeter.h"
Ammeter ammeter;                  /**Ameter object**/
double am_value[TOTAL_MESUREMENTS];
int elapsed_time[TOTAL_MESUREMENTS];

//gpio 定義
const int SW1Pin = 4;

//switch state管理用
int SW1State    = LOW;
int SW1StateOld = LOW;

void setup() {
//nnb file open
  SD.begin();
  nnbFile = SD.open("model.nnb");                     //学習済みデータ読み込み
  if (!nnbFile) {
    Serial.println("model.nnb is not found");
    while(1);
  }
  
  int ret = dnnrt.begin(nnbFile);
  if (ret <0){
    Serial.println("DNNRT begin fail: " + String(ret));
    while(1);
  }

  Wire.begin();                 //i2cを有効化
  Wire.setClock(1000000);       //standart mode:100000, fast mode:400000, fast mode plus:1000000, high speed mode:3400000
  Serial.begin(115200);         //USBシリアル通信を有効化

  ret = 0;
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

  lcd.init();                       //ili9431を初期化
  lcd.setRotation(1);               //右へ90°回転
  lcd.setFont(&fonts::Font0);
}

void loop() {

  SW1StateOld = SW1State;
  SW1State = digitalRead(SW1Pin);

  lcd.setCursor(0,0);

  if (SW1State == LOW && SW1StateOld == HIGH){
    lcd.printf("Currently measuring                              \n");

    mesure_current();
    for(int i = 0; i < TOTAL_MESUREMENTS; i++){
      am_value[i] = vReal[i];
    }

    DCRemoval(vReal, TOTAL_MESUREMENTS);              //直流成分除去
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);  // 窓関数
    FFT.Compute(FFT_FORWARD);                         // FFT処理(複素数で計算)
    FFT.ComplexToMagnitude();                         // 複素数を実数に変換    
    Serial.println("normalization");
    normalization(vReal, TOTAL_MESUREMENTS);
    normalization(am_value, TOTAL_MESUREMENTS);    
    //drawGraph(am_value); 
    drawGraph(vReal);
    saveData();

    //推論実行
    DNNVariable input(TOTAL_MESUREMENTS/2);
    float* data = input.data();
    for (int i = 0; i < TOTAL_MESUREMENTS/2; ++i){
      data[i] = vReal[i];
    }
    dnnrt.inputVariable(input, 0);
    dnnrt.forward();
    DNNVariable output = dnnrt.outputVariable(0);

    lcd.setCursor(0,20);
    lcd.printf("\nMax Index = ");
    lcd.println(output.maxIndex());
    
    Serial.println(output.maxIndex());
    Serial.print("\noutput.maxIndex()=");
    Serial.println(output[output.maxIndex()]);
    for (int i = 0; i < 3; i++){
      Serial.println(output[i]);
      lcd.printf("\ni:");
      lcd.print(i);
      lcd.printf("  value:");
      lcd.print(output[i]);
    }
    lcd.setCursor(0,220);

    lcd.setFont(&fonts::Font4);
    if (output.maxIndex() == 0){
      lcd.print("This is RAW EGG.");
    } else if (output.maxIndex() == 1){
      lcd.print("This is ONSEN EGG!");
    } else if (output.maxIndex() == 2){
      lcd.print("This is BOILED EGG.");
    }
    lcd.setFont(&fonts::Font0);

  } else if (SW1State == HIGH && SW1StateOld == LOW) {
    lcd.printf("set TAMAGO and push start SW");
  }
}

//グラフ描画
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
  for (int i = 0; i < TOTAL_MESUREMENTS/2; i++ ){
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

//直流部除去
void DCRemoval(double *vData, int sample_num) {
    double mean = 0;
    for (uint16_t i = 1; i < sample_num; i++) {
        mean += vData[i];
    }
    mean /= sample_num;
    for (uint16_t i = 1; i < sample_num; i++) {
        vData[i] -= mean;
    }
}

//電流測定
void mesure_current(){
  double current;
  int now;
  int old;
  int elapsedTime;

  int ret = MP.Send(3,50000,subcore1);
  delayMicroseconds(MESURE_START_DELAY);    //測定開始までのdelay
  Serial.println("start mesure cycle");
  for(int i = 0; i < TOTAL_MESUREMENTS; i++){
//    Serial.printf("cycle num = %d / ",i);
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
    elapsed_time[i]  = elapsedTime;
    vReal[i] = current;
    vImag[i] = 0;
  }
  Serial.println(elapsedTime);
}

//データを正規化
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

//電流、FFTCSVファイルで保存
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
  number++;
}
