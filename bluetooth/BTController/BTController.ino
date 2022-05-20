// 블루투스컨트롤러

#include <SoftwareSerial.h>
#define SE_RX 2 // 시리얼 RX를 2번핀으로 설정
#define SE_TX 3 // 시리얼 TX를 3번핀으로 설정
#define BT_PH_RX 4 // 핸드폰 블루투스 RX를 4번 핀으로 설정 
#define BT_PH_TX 5 // 핸드폰 블루투스 TX를 5번 핀으로 설정

char *ptr = NULL; //문자열 파싱을 위한 포인터
int* info = (int*) malloc(8 * sizeof(int)); //pose Estimation을 위해 필요한 버
String last_String = "";
int estimation_state = 0;

SoftwareSerial SerialtoServo(SE_RX,SE_TX);
SoftwareSerial BTPHController(BT_PH_RX, BT_PH_TX);                                         
                                           
void setup() {
  Serial.begin(9600); // baud rate 9600으로 시리얼 통신
  Serial.println("Hello! I am BTController!"); // 시리얼 모니터에 시작 문장 출력
  SerialtoServo.begin(9600);
  BTPHController.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT); // 코드 작동 확인 위해 내장 led를 output모드로 설정
}
void loop() {
  if (Serial.available()) { // 리모컨 블루투스 통신으로 인풋이 있을 때
    char c = (char)Serial.read(); 
    if(c=='N') { // 'N'을 입력 받은 경우
      SerialtoServo.write('N'); // 시리얼 통신으로 'N'을 출력함
      digitalWrite(LED_BUILTIN, HIGH); // 내장 led 켬
    }
    else if (c=='R') { // 'R'을 입력 받은 경우
      SerialtoServo.write('R'); // 시리얼 통신으로 'R'을 출력함
      digitalWrite(LED_BUILTIN, LOW); // 내장 led 끔
    }  
  }
  
  SerialtoServo.listen();
  if (SerialtoServo.available()) { // 시리얼 통신으로 인풋이 있을 때
    char data = SerialtoServo.read();
    if (data=='A'){
      Serial.write(data);
    }
    else {
      BTPHController.write(data);
      delay(1500);
      int temp = 4;
      BTPHController.write(temp);
    }
  }
  
  BTPHController.listen();
  String data = BTPHController.readString();
  if(data != 0){
    int str_len = data.length() + 1; 
    char char_array[str_len];
    data.toCharArray(char_array, str_len);
    split_string(char_array);
    int info_length = 0;
    for(int i = 0 ; i < 8 ; i ++){
      if(info[i]){
        Serial.println(info[i]);
        info_length += 1;
      }
    }
  }
}

void split_string(char data[]){
  byte index = 0;
  ptr = strtok(data, ",");  // delimiter
  while (ptr != NULL)
  {
     info[index] = String(ptr).toInt();
     index++;
     ptr = strtok(NULL, ",");
  }
  
  return ;
}
