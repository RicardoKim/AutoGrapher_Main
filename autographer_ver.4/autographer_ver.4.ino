//xy축의 각도를 이용한 서보모터 제어 코드
#include <Servo.h>
#include "MPU9250.h"
//https://github.com/hideakitai/MPU9250 에서 zip 파일로 추가
MPU9250 mpu; // mpu라는 이름으로 MPU9250 설정

#define X_SERVO_PIN 9 // x축 서보모터를 9번핀에 연결
#define Y_SERVO_PIN 10 // y축 서보모터를 10번핀에 연결
#define rightLeft_servo_pin 11 // z축 회전 서보모터의 핀
#define upDown_servo_pin 12 // y축 회전 서보모터의 핀

#define X_initial 90 // x축 서보모터의 초기 각도를 90으로 지정
#define Y_initial 90 // y축 서보모터의 초기 각도를 90으로 지정
#define rightLeft_initial 90  // z축 회전 서보모터의 초기각도
#define upDown_initial 90  // y축 회전 서보모터의 초기각도

Servo X_Servo;
Servo Y_Servo;
Servo rightLeft_Servo;
Servo upDown_Servo;

int servo_X = X_initial;    //x축 각도(pitch)
int servo_Y = Y_initial;    //y축 각도(roll)
int prev_X = servo_X;       //기존 각도 유지를 위한 변수
int prev_Y = servo_Y;
int servo_rightLeft = rightLeft_initial; // z축 회전 서보모터의 초기각도
int servo_upDown = upDown_initial; // y축 회전 서보모터의 초기각도
int movement_data; // 시리얼로부터 받아올 움직일 방향 데이터
int movement;      // null 값 무시를 위한 명령변수  

int act = 0;                //처리 순서
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
  rightLeft_Servo.attach(rightLeft_servo_pin); // z축 회전 서보모터 핀 연결
  upDown_Servo.attach(upDown_servo_pin); // 상하 조정용 y축 회전 서보모터 핀 연결
  X_Servo.write(X_initial); // X축 서보모터의 초기 각을 지정
  Y_Servo.write(Y_initial); // Y축 서보모터의 초기 각을 지정
  rightLeft_Servo.write(rightLeft_initial); // Z축 서보모터의 초기 각을 지정
  upDown_Servo.write(upDown_initial); // 상하 조정용 Y축 서보모터의 초기 각을 지정


}

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
  int loop_count = 0;          //루프 횟수 생각
  switch(act){
    case 1:           //레벨링
      while(1){
        Leveling(loop_count);
        if(Leveling(loop_count)==true){
          break;
        }
        if(prev_X != servo_X){
          move_servo(X_Servo,servo_X);//서보 움직이기
        }
        if(prev_Y != servo_Y){
          move_servo(Y_Servo,servo_Y);
        }
        prev_X = servo_X;       //기존 각도 유지를 위한 변수
        prev_Y = servo_Y;
        loop_count++;
        
      }
      loop_count=0;
      delay(1000);
      break;
    case 2:
      //get_frame(); // 구도에 사람 넣는 함수
      Serial.println("FrameWork");
      while(1){
        find_person();
        if(find_person() == true){
          Serial.println("target found");
          break;
        }
      }
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
//수평 맞추는 움직임
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
  if(loopCount>300){                       //초기에 불안정한 값 무시하고 400회부터 변형
    Serial.println("Leveling Started..."); // 함수 실행시 안내문 출력
    if(tiltX<0){                            //x축이 -로 기울은 경우
      if(servo_X<150){        
        servo_X++;            //X 축 서보모터 각도 증가
      }
      //X_Servo.write(servo_X); // x축 서보모터를 새로운 각도로 회전
      Serial.print("Pitch is :");
      Serial.println(tiltX); // 지금 x축 돌린 각도가 몇인지 출력
      //Serial.println(servo_X);
    }
    if(tiltX>0){            //x축이 +로 기울은 경우
      if(servo_X>30){      
        servo_X--;          //x축 서보모터 각도 감소
      }      
      //X_Servo.write(servo_X); // x축 서보모터를 새로운 각도로 회전
      
      Serial.print("Pitch is :");
      Serial.println(tiltX); // 지금 x축 돌린 각도가 몇인지 출력
      //Serial.println(servo_X);
    }
    if(tiltY<0){            //y축이 -로 기울은 경우
      if(servo_Y<150){
        servo_Y++;          //y축 서보모터 각도 증가
      }
      //Y_Servo.write(servo_Y); // y축 서보모터를 새로운 각도로 회전
      Serial.print("Roll is :");
      Serial.println(tiltY); // 지금 y축 돌린 각도가 몇인지 출력
      //Serial.println(servo_Y);
    }
    if(tiltY>0){          //y축이 +로 기울은 경우
      if(servo_Y>30){
        servo_Y--;        //y축 서보모터 각도 감소
      }
      //Y_Servo.write(servo_Y); // y축 서보모터를 새로운 각도로 회전
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

int move_servo(Servo servo_motor, int ang){       //서보 돌리는 코드
  int delay_rate = 20;              //속도조절을 위한 딜레이 시간 높을 수록 느리게
  servo_motor.write(ang);           //서보 움직임
  delay(delay_rate);                
  Serial.println("Done rotate");
  
}
//구도 맞추는 움직임
bool Move_Right(){ // z축 회전 서보모터가 오른쪽으로 회전하는 함수
  if(servo_rightLeft<180){ // 오른쪽으로 끝까지 회전할때까지
    rightLeft_Servo.write(servo_rightLeft+1); // 1도씩 움직임
    delay(10); // 지연시간 10ms
    servo_rightLeft++; // 현재 모터 각도 1도씩 증가
  }
  if(servo_rightLeft == 180){ // 오른쪽으로 끝까지 가면
    return true; // true반환
  }
  return false; // 오른쪽으로 끝까지 안간경우 false 반환 
}

bool Move_Left(){ // 이하 동일
  if(servo_rightLeft>0){
    rightLeft_Servo.write(servo_rightLeft-1);
    delay(10);
    servo_rightLeft--;
  }
  if(servo_rightLeft == 0){
    return true;
  }
  return false;
}

bool Move_Up(){
  if(servo_upDown<180){
    upDown_Servo.write(servo_upDown+1);
    delay(10);
    servo_upDown++;
  }
  if(servo_upDown == 180){
    return true;
  }
  return false;
}

bool Move_Down(){
  if(servo_upDown>0){
    upDown_Servo.write(servo_upDown-1);
    delay(10);
    servo_upDown--;
  }
  if(servo_upDown == 0){
    return true;
  }
  return false;
}
//구도 맞추는
bool find_person(){
  if(Serial.available()>0){ // 시리얼에 입력되었을 때
    movement_data = Serial.parseInt(); // 그 숫자를 읽어와서 data로 지정함
    if(movement_data != 0){
      movement = movement_data;
    }
  } 
  switch(movement){ // movement_data는 1,2,3,4
    case 1: // 1번의 경우 z축 회전 서보모터가 오른쪽으로 회전
    Move_Right(); // z축 회전 서보모터가 오른쪽으로 회전하는 함수
    break;
    case 2: // 2번의 경우 z축 회전 서보모터가 왼쪽으로 회전
    Move_Left(); // z축 회전 서보모터가 왼쪽으로 회전하는 함수
    break;
    case 3: // 3번의 경우 y축 회전 서보모터가 위쪽으로 회전
    Move_Up(); // y축 회전 서보모터가 위쪽으로 회전하는 함수 
    break;
    case 4: // 4번의 경우 y축 회전 서보모터가 아래쪽으로 회전
    Move_Down(); // y축 회전 서보모터가 아래쪽으로 회전하는 함수
    break;
    case 5:
    return true;
  }
  Serial.println(movement_data);
  return false;

}
