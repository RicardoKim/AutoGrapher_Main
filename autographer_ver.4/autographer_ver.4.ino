//xy축의 각도를 이용한 서보모터 제어 코드
#include <Servo.h>
#include "MPU9250.h"
//https://github.com/hideakitai/MPU9250 에서 zip 파일로 추가
MPU9250 mpu; // mpu라는 이름으로 MPU9250 설정

#define X_SERVO_PIN 9 // x축 서보모터를 9번핀에 연결
#define Y_SERVO_PIN 10 // y축 서보모터를 10번핀에 연결
#define X_initial 90 // x축 서보모터의 초기 각도를 90으로 지정
#define Y_initial 90 // y축 서보모터의 초기 각도를 90으로 지정

Servo X_Servo;
Servo Y_Servo;

bool leveling_sign = false;
int servo_X = X_initial;
int servo_Y = Y_initial;
int act = 0;
// 전역변수 넣기

void setup() {

  Serial.begin(115200); // 시리얼 통신의 baut rate를 115200으로 설정
  Wire.begin(); // mpu9250을 사용하기 위하여 I2C 통신을 초기화하고 활성화
  delay(2000); // 2초간 코드 진행 지연시킴

  if (!mpu.setup(0x68)) { // mpu의 셋업을 실행하고 연결이 되지 않았다면 아래 if문의 실행문을 실행
    while (1) { // mpu의 연결 오류시 오류문을 반복 출력
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example."); // 오류문 출력
      delay(5000); // 5초 딜레이 후 while문 반복
    }
  }
  X_Servo.attach(X_SERVO_PIN); // X축 서보모터의 핀을 지정
  Y_Servo.attach(Y_SERVO_PIN); // Y축 서보모터의 핀을 지정
  X_Servo.write(X_initial); // X축 서보모터의 초기 각을 지정
  Y_Servo.write(Y_initial); // Y축 서보모터의 초기 각을 지정

}
//hello
void loop() {
  /*
  while(1){
    // 진행버튼과 리셋버튼이 있다고 상정하고 코드 구현
    // 진행버튼 신호를 받으면 act를 1증가시킴 (초기값 0)
    // 단 진행버튼 신호를 받아도 act가 3이면 그대로 3을 유지
    // 진행 버튼 신호가 오면 위와 같이 act 상태를 변경하고 while문을 break
    // 리셋버튼 신호를 받으면 act를 0으로 초기화함 (주의 : while문을 break하지 않음) <- 왜냐? 진행 버튼이 눌릴때만 밑의 switch문이 실행돼야함
  }
  */
  act++;              //모드 변경 
  int i = 0;          //루프 횟수 생각
  switch(act){
    case 1:           //레벨링
      while(1){
        Leveling(i);
        if(Leveling(i)==true){
          break;
        }
        i++;
      }
      i=0;
      break;
    case 2:
      //get_frame(); // 구도에 사람 넣는 함수
      Serial.println("FrameWork");
      delay(1000);
      break;
    case 3:
      //kimchi(); // 촬영하는 함수 김치~
      Serial.println("Kimchi");
      delay(1000);
      break;
    default:
      //while(1){
      //  delay(10);
      //}
      act=0;
      break;
  }
}
void print_roll_pitch_yaw() {
    Serial.print("Yaw, Pitch, Roll: ");
    Serial.print(mpu.getYaw(), 2);
    Serial.print(", ");
    Serial.print(mpu.getPitch(), 2);
    Serial.print(", ");
    Serial.println(mpu.getRoll(), 2);
}


bool Leveling(int loopCount){               //초기에 불안정한 값 무시하기 위해 루프 횟수 입력
  
  
  int tiltX, tiltY;
  
  if(!mpu.update()){
      Serial.print("mpu update error");
      return false;
  }
  mpu.update(); 
  
  tiltX = int(mpu.getPitch()); //계산된 현재 각도
  tiltY = int(mpu.getRoll()); //계산된 현재 각도
  Serial.println(tiltX);
  Serial.println(tiltY);
  if(abs(tiltX)>90||abs(tiltY)>90){
    return false;
  }
  if(loopCount>400){                       //초기에 불안정한 값 무시하고 400회부터 변형
    Serial.println("Leveling Started..."); // 함수 실행시 안내문 출력
    if(tiltX<0){                            //x축이 -로 기울은 경우
      if(servo_X<90){        
        servo_X++;            //X 축 서보모터 각도 증가
      }
      X_Servo.write(servo_X); // x축 서보모터를 새로운 각도로 회전
      delay(10);
      Serial.print("Pitch is :");
      Serial.println(tiltX); // 지금 x축 돌린 각도가 몇인지 출력
      //Serial.println(servo_X);
    }
    if(tiltX>0){            //x축이 +로 기울은 경우
      if(servo_X>-90){      
        servo_X--;          //x축 서보모터 각도 감소
      }      
      X_Servo.write(servo_X); // x축 서보모터를 새로운 각도로 회전
      delay(10);
      Serial.print("Pitch is :");
      Serial.println(tiltX); // 지금 x축 돌린 각도가 몇인지 출력
      //Serial.println(servo_X);
    }
    if(tiltY<0){            //y축이 -로 기울은 경우
      if(servo_Y<90){
        servo_Y++;          //y축 서보모터 각도 증가
      }
      Y_Servo.write(servo_Y); // y축 서보모터를 새로운 각도로 회전
      Serial.print("Roll is :");
      Serial.println(tiltY); // 지금 y축 돌린 각도가 몇인지 출력
      //Serial.println(servo_Y);
    }
    if(tiltY>0){          //y축이 +로 기울은 경우
      if(servo_Y>-90){
        servo_Y--;        //y축 서보모터 각도 감소
      }
      Y_Servo.write(servo_Y); // y축 서보모터를 새로운 각도로 회전
      Serial.print("Roll is :");
      Serial.println(tiltY); // 지금 y축 돌린 각도가 몇인지 출력
      //Serial.println(servo_Y);
    }
    if(tiltX == 0 && tiltY == 0){ //수평이 된 경우
      return true;                //true 반환
    }    
  }
  return false;                 //수평이 아닌 경우 false 반환
}
