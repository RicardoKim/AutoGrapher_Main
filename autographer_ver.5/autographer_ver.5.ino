//xy축의 각도를 이용한 서보모터 제어 코드
#include <Servo.h>
#include "MPU9250.h"
#include <SoftwareSerial.h>
//https://github.com/hideakitai/MPU9250 에서 zip 파일로 추가
MPU9250 mpu; // mpu라는 이름으로 MPU9250 설정

#define SE_RX 2 // 시리얼 RX를 2번핀으로 설정
#define SE_TX 3 // 시리얼 TX를 3번핀으로 설정
#define X_SERVO_PIN 10 // x축 서보모터를 9번핀에 연결
#define Y_SERVO_PIN 9 // y축 서보모터를 10번핀에 연결
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

SoftwareSerial SerialtoBTC(SE_RX,SE_TX);

int servo_X = X_initial;    //x축 각도(pitch)
int servo_Y = Y_initial;    //y축 각도(roll)
int prev_X = servo_X;       //기존 각도 유지를 위한 변수
int prev_Y = servo_Y;
int servo_rightLeft = rightLeft_initial; // z축 회전 서보모터의 초기각도
int servo_upDown = upDown_initial; // y축 회전 서보모터의 초기각도

//???????????????????????????????????????
//int movement_data; // 시리얼로부터 받아올 움직일 방향 데이터
//int movement;      // null 값 무시를 위한 명령변수  
//
//int* info1 = (int*) malloc(8 * sizeof(int)); // 좌표 저장용 배열 1
//int* info2 = (int*) malloc(8 * sizeof(int)); // 좌표 저장용 배열 2

int act = 0;                //처리 순서
// 전역변수 넣기

void setup() {

  Serial.begin(115200); // 시리얼 통신의 baut rate를 115200으로 설정
  Wire.begin(); // mpu9250을 사용하기 위하여 I2C 통신을 초기화하고 활성화
  delay(2000); // 2초간 코드 진행 지연시킴
  SerialtoBTC.begin(9600);

  if (!mpu.setup(0x68)) { // mpu의 셋업을 실행하고 연결이 되지 않았다면 아래 if문의 실행문을 실행
    while (1) { // mpu의 연결 오류시 오류문을 반복 출력
      Serial.println("MPU connection failed. Please check your connection with `connection_check` example."); // 오류문 출력
      delay(5000); // 5초 딜레이 후 while문 반복
    }
  }
  mpu.calibrateAccelGyro();
  X_Servo.attach(X_SERVO_PIN); // X축 서보모터의 핀을 지정
  Y_Servo.attach(Y_SERVO_PIN); // Y축 서보모터의 핀을 지정
  rightLeft_Servo.attach(rightLeft_servo_pin); // z축 회전 서보모터 핀 연결
  upDown_Servo.attach(upDown_servo_pin); // 상하 조정용 y축 회전 서보모터 핀 연결
  X_Servo.write(X_initial); // X축 서보모터의 초기 각을 지정
  Y_Servo.write(Y_initial); // Y축 서보모터의 초기 각을 지정
  rightLeft_Servo.write(rightLeft_initial); // Z축 서보모터의 초기 각을 지정
  upDown_Servo.write(upDown_initial); // 상하 조정용 Y축 서보모터의 초기 각을 지정
  delay(500);
  X_Servo.detach(); // X축 서보모터의 핀을 지정
  Y_Servo.detach(); // Y축 서보모터의 핀을 지정
  rightLeft_Servo.detach(); // z축 회전 서보모터 핀 연결
  upDown_Servo.detach(); // 상하 조정용 y축 회전 서보모터 핀 연결


}

void loop() {
  if(SerialtoBTC.available()){
    char c = (char)SerialtoBTC.read();
    if(c=='N'){
      if(act==0){
        act=1;
      }
      else if(act==2){
        act=3;
      }
    }
    else if(c=='R'){
      act=0;
    }
    else { // 리모컨 인풋이 아닌 경우
      // 아마 쓸일 없을거 같긴 한데 일단 냅둠
    }
  }
  
  int loop_count = 0;          //루프 횟수 생각
  switch(act){
    case 1:           //레벨링
      
      while(1){

        Leveling(loop_count);
        if(Leveling(loop_count)==true){
          break;
        }
        if(prev_X != servo_X){
          move_servo(X_Servo,X_SERVO_PIN,servo_X);//서보 움직이기
        }
        if(prev_Y != servo_Y){
          move_servo(Y_Servo,Y_SERVO_PIN,servo_Y);
        }
        prev_X = servo_X;       //기존 각도 유지를 위한 변수
        prev_Y = servo_Y;
        loop_count++;
        
      }
      loop_count=0;
      act++;        //case 2 도 실행하도록 하기 위한 변동
      delay(1000);
      
    case 2:
      //get_frame(); // 구도에 사람 넣는 함수
      Serial.println("FrameWork");
      /*while(1){
        find_person();
        if(find_person() == true){
          Serial.println("target found");
          break;
        }
      }*/
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
  
  tiltX = int(0.96*mpu.getPitch()+0.04*mpu.getAccX()); //계산된 현재 각도
  tiltY = int(0.96*mpu.getRoll()+0.04*mpu.getAccY()); //계산된 현재 각도
  
  
  if(abs(tiltX)>90||abs(tiltY)>90){
    return false;
  }
  if(loop<200){
    mpu.calibrateAccelGyro();
    Serial.println("Calibration...");
    Serial.print(tiltX);
    Serial.print("  ");
    Serial.println(tiltY);
  }
  if(loopCount>200){                       //초기에 불안정한 값 무시하고 400회부터 변형
    Serial.println("Leveling Started..."); // 함수 실행시 안내문 출력
    if(tiltX>0){                            //x축이 -로 기울은 경우
      if(servo_X<135){        
        servo_X++;            //X 축 서보모터 각도 증가
      }
      //X_Servo.write(servo_X); // x축 서보12091모터를 새로운 각도로 회전
      Serial.print("Pitch is :");
      Serial.println(tiltX); // 지금 x축 돌린 각도가 몇인지 출력
      //Serial.println(servo_X);
    }
    if(tiltX<0){            //x축이 +로 기울은 경우
      if(servo_X>45){      
        servo_X--;          //x축 서보모터 각도 감소
      }      
      //X_Servo.write(servo_X); // x축 서보모터를 새로운 각도로 회전
      
      Serial.print("Pitch is :");
      Serial.println(tiltX); // 지금 x축 돌린 각도가 몇인지 출력
      //Serial.println(servo_X);
    }
    if(tiltY>0){            //y축이 -로 기울은 경우
      if(servo_Y<135){
        servo_Y++;          //y축 서보모터 각도 증가
      }
      //Y_Servo.write(servo_Y); // y축 서보모터를 새로운 각도로 회전
      Serial.print("Roll is :");
      Serial.println(tiltY); // 지금 y축 돌린 각도가 몇인지 출력
      //Serial.println(servo_Y);
    }
    if(tiltY<0){          //y축이 +로 기울은 경우
      if(servo_Y>45){
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

int move_servo(Servo servo_motor,int servo_pin, int ang){       //서보 돌리는 코드
  int delay_rate = 30;              //속도조절을 위한 딜레이 시간 높을 수록 느리게
  servo_motor.attach(servo_pin); // X축 서보모터의 핀을 지정  
  servo_motor.write(ang);           //서보 움직임
  delay(delay_rate);            
  servo_motor.detach();
  Serial.println("Done rotate");
}




//만약 0이 나오면 어차피 안 나온 것이다 
// 앱과의 통신을 통해 레퍼런스 좌표 두 개 받아오는 함수
//좌표 받는 코드
int* get_position(){
  int* pos = (int*) malloc(2*sizeof(int));
  int check_pos = 0;
  for(int i=0;i<8;i++){    
    check_pos = SerialtoBTC.parseInt();     //좌표 받기
    if(check_pos == 0){
      pos[0] = 0;
      pos[1] = 0;
      return pos;
    }
    if(i%2 == 0){
      pos[0]+=check_pos;
    }
    if(i%2 == 1){
      pos[1]+=check_pos;
    }
    pos[0] = pos[0]/4;
    pos[1] = pos[1]/4;
  }
  return pos;
}

bool find_person(int* difference){
  






  
  return false;

}
//int a[8] 쓰지 않는 이유는?
int* current_position = (int*) malloc(2*sizeof(int));   //초기 좌표 평균
int* check_angle = (int*) malloc(2*sizeof(int));        //상 10도 좌10도 회전시 좌표 평균
int* angle_difference = (int*) malloc(2*sizeof(int));  //10도 회전 시 구도 변화 측정
//쓰고 나면 malloc은 해제 해줘야하지 않나?

//구도 맞추는
int get_frame(){
  int* angle_difference = (int*) malloc(2*sizeof(int));  //10도 회전 시 구도 변화 측정
  
  SerialtoBTC.write(2); // 블루투스 통신을 통해 앱에게 사진 찍으라고 시킴
  current_position = get_position();    //현재 중앙 좌표
  if(current_position[0]==0){
    return 0;
  }
  
  move_servo(rightLeft_Servo,rightLeft_servo_pin,servo_rightLeft+10); //도.................리
  move_servo(upDown_Servo,upDown_servo_pin,servo_upDown+10);          //끄.................덕
  
  SerialtoBTC.write(2); // 블루투스 통신을 통해 앱에게 사진 찍으라고 시킴
  check_angle = get_position();         //기준치 변경시 구도 확인
  if(check_angle[0]==0){
    return 0;
  }
  angle_difference[0] = check_angle[0]-current_position[0];       //10도마다 생기는 차이
  angle_difference[1] = check_angle[1]-current_position[1];
  
  find_person(angle_difference);
  
  return 1;
  
}
