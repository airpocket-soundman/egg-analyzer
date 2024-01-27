//subcore1にメッセージが届くと、

#if (SUBCORE != 1)
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include <math.h>

#define VMAX                    100             //　モーターの最高速　100で1回転/sec相当（ちがうかも
#define CYCLETIME               900000          //  Sinカーブに沿って加減速するさいのSin波周期 600000us = 600ms = 0.6sec
#define NUMBER_OF_TIME_SEGMENTS 500             //  sinカーブをこの数値で分周して疑似サインカーブを生成する
#define NUMBER_OF_REPEAT        4               //  sin加速を繰り返す回数

const int SW2Pin = 6;
const int DirPin  = 0;
const int StepPin = 1;

void setup()
{
  MP.begin();
  pinMode(DirPin,  OUTPUT);
  pinMode(StepPin, OUTPUT);
  digitalWrite(DirPin,  LOW);
  digitalWrite(StepPin, LOW);
}

void loop()
{
  int      ret;
  int8_t   msgid;
  uint32_t msgdata;

  ret = MP.Recv(&msgid, &msgdata);
  delayMicroseconds(1000);

  stepper_move();
}

void stepper_move(){
  digitalWrite(DirPin,  LOW);
  MPLog("stepper start\n");
  int Vmax                 = VMAX;
  int timeCycle            = CYCLETIME;                       //usecの時間で１周期
  int numberOfTimeSegments = NUMBER_OF_TIME_SEGMENTS;         //１周期を何分周するか
  int numberOfRepeat       = NUMBER_OF_REPEAT;                //何周期繰り返すか

  for (int i = 0; i < numberOfRepeat; i++){
    sinAccel(Vmax, numberOfTimeSegments, timeCycle);
  }
}

//回転速度Vの時のdelayTimeを返す　1回転/secをV = 100とした場合のtimedelayを計算（まちがってるかも。
int getTimeDelay(int V){
  if (V != 0){
    return 2500 / abs(V);
  } else {
    return -1;
  }
}

//stepper motorを1step/timeDelay*2マイクロ秒の速度でステップ
void stepMotor(int timeDelay){ 
  digitalWrite(StepPin, HIGH);
  delayMicroseconds(timeDelay);
  digitalWrite(StepPin, LOW);
  delayMicroseconds(timeDelay);
}

//stepper motorをtimeSegmetマイクロ秒の間、1step/timeDelay*2マイクロ秒の速度でステップ
void stepMotorInTimeSegment(int V, int timeSegment){
    
  if (V > 0){
    digitalWrite(DirPin, LOW);
  } else if (V < 0) {
    digitalWrite(DirPin, HIGH);
  }

  if (V != 0){
    int timeDelay = getTimeDelay(V);
    int numberOfSteps = timeSegment / (timeDelay * 2);
    for (int i = 0; i < numberOfSteps; i++){
      stepMotor(timeDelay);
    }
  } else{
    delayMicroseconds(timeSegment);
  }
}

//最高速度Vmaxでsinカーブに沿って加減速する。timeがsinカーブの一周期、一周期をnumberOfTimeSegments回に分周して速度制御する。
void sinAccel(int Vmax, int numberOfTimeSegments, int time){
  int timeSegment = time / numberOfTimeSegments;        //timeSegmet = 単位時間(us)
  for (int i = 1 ; i <= numberOfTimeSegments; i++){
    float phase = i / (float)numberOfTimeSegments;      
    float rad   = phase * 2 * PI;                       //rad = 現在の位相
    int V = sin(rad) * Vmax;                            //V = 現在の位相での速度
    stepMotorInTimeSegment(V, timeSegment);             //timeSegment(us)の間、速度Vでステップさせる。
  }
}
