// 블루투스컨트롤러

// 리모컨 -> 블루투스컨트롤러만 구현 된 코드

// 블루투스컨트롤러 -> 핸드폰 관련 코드는 추가하여 합쳐야 함
// 블루투스컨트롤러 -> 리모컨 (부저로 완료된 상태 나타내는 코드) 추가하여 합쳐야 함

#include <SoftwareSerial.h>
#define BT_RX 7 // RX를 7번핀으로 설정
#define BT_TX 8 // TX를 8번핀으로 설정

SoftwareSerial BTController(BT_RX,BT_TX);  // RX핀(7번)은 BT모듈의 TX에 연결 
                                           // TX핀(8번)은 BT모듈의 RX에 연결  
void setup() {
  Serial.begin(9600); // baut rate 9600으로 시리얼 통신
  Serial.println("Hello! I am BTController!"); // 시리얼 모니터에 시작 문장 출력
  BTController.begin(9600); // 블루투스 통신 rate 9600으로 시작
  pinMode(LED_BUILTIN, OUTPUT); // 코드 작동 확인 위해 내장 led를 output모드로 설정
}
void loop() {
  if (BTController.available()) { // 블루투스 통신으로 인풋이 있을 때
    if(BTController.read()=='N') // 'N'을 입력 받은 경우
      digitalWrite(LED_BUILTIN, HIGH); // 내장 led 켬
    else // 'R'을 입력 받은 경우 <--- 이긴 한데 인풋이 두종류라 else로 처리함
      digitalWrite(LED_BUILTIN, LOW); // 내장 led 끔
  }
  if (Serial.available()) { // 시리얼 통신으로 인풋이 있을 때
    BTController.write(Serial.read()); // 블루투스로 출력
  }
}
