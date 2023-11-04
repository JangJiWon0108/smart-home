
int sensorPin = A0;   // select the input pin for the potentiometer
int sensorValue = 0;  // variable to store the value coming from the sensor

int light=35;
int light_val;


#include <ESP32Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

// ap모드 와이파이 이름이랑 비번 정의
const char*ssid="jjw";
const char*pw="11111111";

// ip gw sb
IPAddress ip(192,168,1,1);
IPAddress gw(192,168,1,1);
IPAddress sb(255,255,255,0);

// 웹서버 객체 생성
WebServer server(80);

int led[4]={15,2,4,5};
int temp;
int temp_val=0;
int duty=0;
int ONE_WIRE_BUS=19;

int led_flag=0;
int blind_flag=0;
int door_flag=0;
int door_control=0;
int irPin = 26;
int buzzerPin = 13;
int buzzer_on=0;

Servo servo_light;
Servo servo_door;

// 온도센서 객체 생성
OneWire oneWire(ONE_WIRE_BUS); 
DallasTemperature sensors(&oneWire);

// rgb led 객체 생성
Adafruit_NeoPixel RGB_LED=Adafruit_NeoPixel(3,18,NEO_GRB);  // 3은 LED 갯수, 6은 LED 핀 번호

// 함수 선언
void led_on(int duty);
void servo_light_on();
void RGB_Color(float c, int wait);


void setup(){
  Serial.begin(9600);
  sensors.begin(); // 온도센서 시작

  // wifi
  WiFi.mode(WIFI_AP);           // wifi 모드 설정 (ap모드)
  WiFi.softAP(ssid,pw);          // ssid 랑 pw입력해 연결
  WiFi.softAPConfig(ip,gw,sb);  // ip gw sb

  // neopixel led
  RGB_LED.begin();
  RGB_LED.setBrightness(0); //RGB_LED 밝기조절
  RGB_LED.clear();  // 모두 끈다.
  RGB_LED.show();

  // 웹서버
  
  server.on("/", handle_root);
  server.on("/white_on", HTTP_GET, handle_white_on);
  server.on("/red_on", HTTP_GET, handle_red_on); 
  server.on("/green_on", HTTP_GET, handle_green_on);      
  server.on("/blue_on", HTTP_GET, handle_blue_on);  
  server.on("/rgb_off", HTTP_GET, handle_rgb_off);  

 // led
  server.on("/led_on_0", HTTP_GET, handle_led_0); 
  server.on("/led_on_85", HTTP_GET, handle_led_85);      
  server.on("/led_on_170", HTTP_GET, handle_led_170);  
  server.on("/led_on_255", HTTP_GET, handle_led_255); 

  // blind
  server.on("/blind_on", HTTP_GET, handle_blind_on); 
  server.on("/blind_off", HTTP_GET, handle_blind_off); 
  server.on("/blind_on_light", HTTP_GET, handle_blind_on_light); 
  server.on("/blind_off_light", HTTP_GET, handle_blind_off_light); 

  // door
  server.on("/door_open", HTTP_GET, handle_door_open); 
  server.on("/door_close", HTTP_GET, handle_door_close); 
  server.on("/door_ir_on", HTTP_GET, handle_door_ir_on); 
  server.on("/door_ir_off", HTTP_GET, handle_door_ir_off); 
  server.begin(); // 웹서버 시작

  // LED 핀 모드 설정
  for(int x=0; x<sizeof(led)/sizeof(led[0]); x=x+1) {
    pinMode(led[x], OUTPUT);
  }

  // 블라인드(빛) 서보모터 핀 설정 및 초기 0도
  servo_light.attach(21);
  servo_light.write(0);

  // 현관문 서보모터 핀 
  servo_door.attach(14);
  servo_door.write(0);

  // 조도센서 핀 모드
  pinMode(light, INPUT);

  // 부저랑 ir센서
  pinMode(buzzerPin, OUTPUT);
  pinMode(irPin, INPUT);
}

void loop() {
  Serial.println(analogRead(light));


  server.handleClient(); 

  if (led_flag<=1){
    led_on(0);
    delay(20);
  }
  else if (led_flag==2){
    led_on(85);
    delay(20);
  }
  else if (led_flag==3){
    led_on(170);
    delay(20);
  }
  else if (led_flag==4){
    led_on(255);
    delay(20);
  }

  if (blind_flag==1) servo_light.write(90);
  else if (blind_flag==2) servo_light.write(0);
  else if (blind_flag==3) servo_light_on();
  else if (blind_flag==4); 


  if (door_control==1 && door_flag==1 && digitalRead(irPin)==0){
    door_flag=2;
    buzzer_on=1;
  }

  if (door_flag==1) servo_door.write(90);
  else if (door_flag==2) servo_door.write(0);
  else if (door_flag==3);

  if (buzzer_on==1) {
    for(int x=0; x<7; x=x+1){
    digitalWrite(buzzerPin,1);
    delay(200);
    digitalWrite(buzzerPin,0);
    delay(200);
    }
    buzzer_on=2;
  }
}



void servo_light_on() {
  light_val=analogRead(light);
  if(light_val >= 3000) {
    servo_light.write(0);
  }
  else {
    servo_light.write(90);
  }
}

void led_on(int duty) {
  for(int x=0; x<4; x=x+1) {
    analogWrite(led[x], duty);
  }
}

void RGB_Color(float c, int wait) {

  for (int i = 0; i < 3; i++) {
    RGB_LED.setPixelColor(i, c);
    RGB_LED.show();
  }
  delay(wait);
}


void handle_root() { // 루트 페이지 요청 시 분기할 함수 정의
  sensors.requestTemperatures(); 
  temp=sensors.getTempCByIndex(0);
  Serial.println(temp);
  server.send(200,"text/html",html_page(temp)); // 서버->클라이언트로 요청에 대한 응답을 보냄(html 페이지)
}

String html_page(int temp)  
{
  String str = "<!DOCTYPE html> <html>\n"; 
  str += "<head><style>body {display: flex;flex-direction: column;align-items: center;justify-content: center;height: 100vh;margin: 0;}";
  str += "h1 {font-size: 50px;text-align: center;}";
  str += "p {font-size: 30px;text-align: center;}</style></head>";
  str += "<body>"; 
  str += "<h1>smart home</h1>\n"; 
  str += "<p>Temperature : ";
  str += temp;
  str += "</p>\n";

  // 첫 번째 세트의 버튼 (RGB 컨트롤)
  str += "<p>";
  str += "<button style='font-size: 26px; color: #000000; width: 150px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='rgb_off()'>OFF</button>";  
  str += "<button style='font-size: 26px; color: #808080; width: 150px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='h_on()'>WHITE</button>";
  str += "<button style='font-size: 26px; color: #FF0000; width: 150px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='r_on()'>RED</button>";
  str += "<button style='font-size: 26px; color: #008000; width: 150px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='g_on()'>GREEN</button>";
  str += "<button style='font-size: 26px; color: #0000FF; width: 150px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='blue_on()'>BLUE</button>";
  str += "</p>\n";

  // 두 번째 세트의 버튼 (LED 컨트롤 - 버튼 색: 주황)
  str += "<p>";
  str += "<button style='font-size: 26px; color: #FF8C00; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='on_0()'>LED OFF(0%)</button>";
  str += "<button style='font-size: 26px; color: #FF8C00; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='on_85()'>LED ON(33%)</button>";
  str += "<button style='font-size: 26px; color: #FF8C00; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='on_170()'>LED ON(66%)</button>";
  str += "<button style='font-size: 26px; color: #FF8C00; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='on_255()'>LED ON(100%)</button>";
  str += "</p>\n";

  // 세 번째 세트의 버튼 (블라인드 컨트롤 - 버튼 색: 하늘)
  str += "<p>";
  str += "<button style='font-size: 26px; color: #87CEEB; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='b_on()'>BLIND OPEN</button>";
  str += "<button style='font-size: 26px; color: #87CEEB; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='b_off()'>BLIND CLOSE</button>";
  str += "<button style='font-size: 26px; color: #87CEEB; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='b_on_light()'>AUTO BLIND ON</button>";
  str += "<button style='font-size: 26px; color: #87CEEB; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='b_off_light()'>AUTO BLIND OFF</button>";
  str += "</p>\n";

  // 네 번째 세트의 버튼 (도어 컨트롤 - 버튼 색: 보라색)
  str += "<p>";
  str += "<button style='font-size: 26px; color: #9932CC; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='d_open()'>DOOR OPEN</button>";
  str += "<button style='font-size: 26px; color: #9932CC; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='d_close()'>DOOR CLOSE</button>";
  str += "<button style='font-size: 26px; color: #9932CC; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='d_ir_on()'>AUTO DOOR ON</button>";
  str += "<button style='font-size: 26px; color: #9932CC; width: 200px; height: 100px; border: 2px solid #000; border-radius: 5px; margin: 10px;' onclick='d_ir_off()'>AUTO DOOR OFF</button>";
  str += "</p>\n";



  // SCRIPT

  // NOEPIXEL LED
  str+="<script>\n";
  str+="function h_on() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/white_on',true);";
  str+="client.send();";
  str+="}";

  str+="function r_on() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/red_on',true);";
  str+="client.send();";
  str+="}";

  str+="function g_on() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/green_on',true);";
  str+="client.send();";
  str+="}";

  str+="function blue_on() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/blue_on',true);";
  str+="client.send();";
  str+="}";

  str+="function rgb_off() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/rgb_off',true);";
  str+="client.send();";
  str+="}";


  // LED
  str+="function on_0() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/led_on_0',true);";
  str+="client.send();";
  str+="}";

  str+="function on_85() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/led_on_85',true);";
  str+="client.send();";
  str+="}";

  str+="function on_170() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','led_on_170',true);";
  str+="client.send();";
  str+="}";

  str+="function on_255() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/led_on_255',true);";
  str+="client.send();";
  str+="}";


  // blind
  str+="function b_on() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/blind_on',true);";
  str+="client.send();";
  str+="}";

  str+="function b_off() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/blind_off',true);";
  str+="client.send();";
  str+="}";

  str+="function b_on_light() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/blind_on_light',true);";
  str+="client.send();";
  str+="}";

  str+="function b_off_light() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/blind_off_light',true);";
  str+="client.send();";
  str+="}";

  // door
  str+="function d_open() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/door_open',true);";
  str+="client.send();";
  str+="}";

  str+="function d_close() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/door_close',true);";
  str+="client.send();";
  str+="}";

  str+="function d_ir_on() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/door_ir_on',true);";
  str+="client.send();";
  str+="}";

  str+="function d_ir_off() {";  // 클->서버로 해당 페이지에 대한 'GET'요청을 보내는 함수 정의
  str+="var client=new XMLHttpRequest();";
  str+="client.open('GET','/door_ir_off',true);";
  str+="client.send();";
  str+="}";

  str+="</script>\n";
  str+="</body>\n";  // 바디 끝
  str+="</html>\n";  // html 끝

  return str; // str을 return 한다
}

// neopixel led
void handle_white_on() {
  RGB_LED.setBrightness(20); //RGB_LED 밝기조절
  RGB_Color(RGB_LED.Color(225, 255, 255), 500); // 흰색
  server.send(200,"text/plain","");
}

void handle_red_on() {
  RGB_LED.setBrightness(20);
  RGB_Color(RGB_LED.Color(225, 0, 0), 500); //RED
  server.send(200,"text/plain",""); 
}

void handle_green_on() {
  RGB_LED.setBrightness(20);
  RGB_Color(RGB_LED.Color(0, 255, 0), 500); //GREEN
  server.send(200,"text/plain","");
}

void handle_blue_on() {
  RGB_LED.setBrightness(20);
  RGB_Color(RGB_LED.Color(0, 0, 255), 500); //BLUE
  server.send(200,"text/plain","");
}

void handle_rgb_off() {
  RGB_LED.clear();
  RGB_LED.show();
  RGB_LED.setBrightness(0); //RGB_LED 밝기조절
  server.send(200,"text/plain","");
}


// led
void handle_led_0() {
  led_flag=1;
  server.send(200,"text/plain","");
}

void handle_led_85() {
  led_flag=2;
  server.send(200,"text/plain","");
}

void handle_led_170() {
  led_flag=3;
  server.send(200,"text/plain","");
}

void handle_led_255() {
  led_flag=4;
  server.send(200,"text/plain","");
}

// blind 
void handle_blind_on() {
  blind_flag=1;
  server.send(200,"text/plain","");
}

void handle_blind_off() {
  blind_flag=2;
  server.send(200,"text/plain","");
}

// blind light
void handle_blind_on_light() {  
  blind_flag=3;
  server.send(200,"text/plain","");
}

void handle_blind_off_light() {
  blind_flag=4;
  server.send(200,"text/plain","");
}

// door
void handle_door_open() {
  door_flag=1;
  server.send(200,"text/plain","");
}

void handle_door_close() {
  door_flag=2;
  server.send(200,"text/plain","");
}

// door ir
void handle_door_ir_on() {
  door_control=1;
  server.send(200,"text/plain","");
}

void handle_door_ir_off() {
  door_flag=3;
  door_control=2;
  server.send(200,"text/plain","");
}


