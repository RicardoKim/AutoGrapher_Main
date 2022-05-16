// 리모컨 아두이노 나노 코드

// 리모컨 -> 블루투스컨트롤러
// Next, Reset 두 개의 버튼으로부터 입력을 감지하고
// Next 버튼에 입력이 들어올 때 블루투스 통신을 통해 'N'을 출력
// Reset 버튼에 입력이 들어올 때 블루투스 통신을 통해 'R'을 출력

// 블루투스컨트롤러 -> 리모컨 코드는 추가하여 합쳐야 함

#include <SoftwareSerial.h>
#define BT_RX 7 // RX를 7번핀으로 설정
#define BT_TX 8 // TX를 8번핀으로 설정

SoftwareSerial RCSerial(BT_RX,BT_TX);  // RX핀(7번)은 블루투스 모듈의 TX에 연결 
                                       // TX핀(8번)은 블루투스 모듈의 RX에 연결
const int NextButton = 2; //Define NextButton to pin 2
const int ResetButton = 3; //Define NextButton to pin 3
boolean lastNextButton= LOW; // Declare the pressed state of the previous NextButton as a Boolean variable
boolean currentNextButton= LOW; // Declares the pressed state of the current NextButton as a Boolean variable
boolean lastResetButton= LOW; // Declare the pressed state of the previous ResetButton as a Boolean variable
boolean currentResetButton= LOW; // Declares the pressed state of the current ResetButton as a Boolean variable
boolean NextState= false; // 
boolean ResetState = false; // 

void setup()
{
  pinMode(NextButton, INPUT_PULLUP); // Assign NextButton (pin 2) as input
  pinMode(ResetButton, INPUT_PULLUP); // Assign ResetButton (pin 3) as input
  Serial.begin(9600); // baut rate 9600으로 시리얼 통신
  Serial.println("Hello! I am RemoteController!"); // 시리얼 모니터에 시작 문장 출력
  RCSerial.begin(9600); // 블루투스 통신 rate 9600으로 시작
}

void loop()
{
  if (RCSerial.available()) { // 블루투스 통신 인풋이 있을 때
    Serial.write(RCSerial.read()); // 시리얼 통신으로 인풋 내용 출력
  }
  if (Serial.available()) { // 시리얼 통신으로 인풋이 있을 때
    RCSerial.write(Serial.read()); // 블루투스 통신으로 인풋 내용 출력
  }
  
  currentNextButton= debounce(lastNextButton, NextButton); //read debouncing NextButton state
  if(lastNextButton== HIGH && currentNextButton== LOW) //press the NextButton
  {
    NextState= !NextState; // NextState의 상태 변경
  }
  lastNextButton= currentNextButton; //change the previous NextButton value to the current NextButton value

  currentResetButton= debounce(lastResetButton, ResetButton); //read debouncing ResetButton state
  if(lastResetButton== HIGH && currentResetButton== LOW) //press the ResetButton
  {
    ResetState= !ResetState; // ResetState의 상태 변경
  }
  lastResetButton= currentResetButton; //change the previous ResetButton value to the current ResetButton value

  if(NextState==true){ // Next 버튼이 눌린 경우
    RCSerial.write('N'); // 블루투스 통신으로 'N'을 출력
    Serial.write('N'); // 시리얼 통신으로 'N'을 출력
    NextState=false; // NextState를 false로 초기화
  }
  if(ResetState==true){ // Reset 버튼이 눌린 경우
    RCSerial.write('R'); // 블루투스 통신으로 'R'을 출력
    Serial.write('R'); // 시리얼 통신으로 'R'을 출력
    ResetState=false; // ResetState를 false로 초기화
  }
}

boolean debounce(boolean last, int BUTTON) // 버튼의 기계적 노이즈를 제거하는 디바운스 함수
{
  boolean current = digitalRead(BUTTON); //Check the current button status
  if(last != current) //current state is different from the previous status
  {
    delay(5); //wait for 5ms
    current = digitalRead(BUTTON); //save current button state on ‘current’
  }
  return current; //return the current status
}
