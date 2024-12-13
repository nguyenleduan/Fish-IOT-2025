#include <WiFi.h>

// Thông tin Wi-Fi của bạn
const char* ssid = "No Internet";
const char* password = "Aaa@2233";
// upesy esp32 wroom

const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;
//-----------------------------------SETTING-------------------------
int foodTime = 500; // thòi gian cho ăn 2(s) 
int waterMax = 6; // mực nước tối đa 2(s) 
int waterMin = 16; // mực nước tối thiểu 2(s) 
// ----------------------------------PIN------------------------------- 
#define LED_PIN 5 // đèn cá
#define MOTOR_PIN  4 // motor cho ăn
#define PW_PIN  21// nước Chung
#define WIN_PIN  22 // nước vào
#define WOUT_PIN  23 // Nước ra
#define LED_BOARD_PIN  25 // đèn bảng điều khiển
#define IN_TRACKING_PIN  26 // input nhận diện  
#define POWER_TRACKING_PIN  27 // nguồn nhận diện 
#define TRIG_PIN  33 // cảm biển siêu âm trắng
#define ECHO_PIN 32  // cảm biển siêu âm vàng

// ----------------------------------END - PIN-------------------------------
int foodDay = 0;
int waterDay = 0;
struct tm timeinfo;

///
void setup() {
  Serial.begin(115200);
  
  pinMode(PW_PIN, OUTPUT);  //  Out FoodPW_PIN
  pinMode(LED_PIN, OUTPUT);  //  Out 
  pinMode(WIN_PIN, OUTPUT);  //  Out Food
  pinMode(WOUT_PIN, OUTPUT);  //  Out Food
  pinMode(LED_BOARD_PIN, OUTPUT);  //  Out Food
  pinMode(POWER_TRACKING_PIN, OUTPUT);   
  
  pinMode(IN_TRACKING_PIN, INPUT);  //  Out Food
  pinMode(MOTOR_PIN, OUTPUT);  //  Out Food
  pinMode(TRIG_PIN, OUTPUT);  // Thiết lập chân Trig là đầu ra
  pinMode(ECHO_PIN, INPUT);   // Thiết lập chân Echo là đầu vào


//

    digitalWrite(PW_PIN, LOW); // tắt nguôn nhận diện 
    digitalWrite(MOTOR_PIN, LOW); // tắt nguôn nhận diện 
    digitalWrite(LED_PIN, LOW); // tắt nguôn nhận diện 
    digitalWrite(WIN_PIN, LOW); // tắt nguôn nhận diện 
    digitalWrite(WOUT_PIN, LOW); // tắt nguôn nhận diện 
    digitalWrite(LED_BOARD_PIN, LOW); // tắt nguôn nhận diện 
    digitalWrite(POWER_TRACKING_PIN, LOW); // tắt nguôn nhận diện 




  // Kết nối Wi-Fi
  connectToWiFi();
  configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
}

void loop() {
  checkWifi(); 

  // Lấy giờ hiện tại
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Không lấy được thời gian");
    return;
  }
  // Hiển thị thời gian
 // Serial.printf("Giờ hiện tại: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec); 
 // Serial.printf("Ngày hiện tại: %02d/%02d/%04d\n", timeinfo.tm_mday, timeinfo.tm_mon + 1, 1900 + timeinfo.tm_year);
    
  food(); 
    checkWater();
  tracking();
  checkAction(); 
} 
void checkAction(){
  if(timeinfo.tm_hour >= 18|| timeinfo.tm_hour < 6){ 
     digitalWrite(POWER_TRACKING_PIN, HIGH);   // bật nguôn nhận diện
 }else{
    digitalWrite(POWER_TRACKING_PIN, LOW); // tắt nguôn nhận diện 
 }  
  if((timeinfo.tm_hour >= 18|| timeinfo.tm_hour < 6)  && digitalRead(IN_TRACKING_PIN) == HIGH){ 
      digitalWrite(LED_PIN, HIGH); 
      digitalWrite(LED_BOARD_PIN, HIGH);  
         Serial.println("có người di chuhển 19h - 6"); 
  }else  { 
      digitalWrite(LED_PIN, LOW); 
      digitalWrite(LED_BOARD_PIN, LOW);   
  }
}
void checkWater(){
  if((timeinfo.tm_hour >= 17|| timeinfo.tm_hour < 8)){

  }else if(tracking() >  (waterMax + 2)){   
      digitalWrite(WIN_PIN, HIGH);   
      digitalWrite(PW_PIN, HIGH);    
      digitalWrite(LED_PIN, HIGH); 
      while (tracking() > waterMax ) { 
      digitalWrite(LED_PIN, HIGH); 
      digitalWrite(LED_BOARD_PIN, HIGH);    
         Serial.println("Đang thêm nước");
          delay(1000);
      }  
      digitalWrite(WIN_PIN, LOW);  // Đóng nước 
      digitalWrite(LED_PIN, LOW); 
      digitalWrite(PW_PIN, LOW);    
      digitalWrite(LED_BOARD_PIN, LOW);   
  }else 
  if(tracking() <  (waterMax - 1 )){
      digitalWrite(WOUT_PIN, HIGH);
      digitalWrite(PW_PIN, HIGH);    
      while (tracking() <  waterMax) {
      digitalWrite(LED_PIN, HIGH); 
      digitalWrite(LED_BOARD_PIN, HIGH);   
         Serial.println("Đang xả nước");
          delay(1000);
      }   
      digitalWrite(PW_PIN, LOW); 
      digitalWrite(LED_PIN, LOW); 
      digitalWrite(LED_BOARD_PIN, LOW);   
      digitalWrite(WOUT_PIN, LOW);   
  }
} 
void changWater(){  
  if(   waterDay != timeinfo.tm_mday && timeinfo.tm_mday % 4 == 0 && timeinfo.tm_hour == 12 && timeinfo.tm_min < 10  ){ 
      digitalWrite(PW_PIN, HIGH);
      digitalWrite(WOUT_PIN, HIGH); 
      while  (tracking() <= waterMin) { 
      digitalWrite(LED_PIN, HIGH); 
      digitalWrite(LED_BOARD_PIN, HIGH);   
         Serial.println("Đang xả nước");
          delay(1000);
      }  
      digitalWrite(WIN_PIN, HIGH);  // mở nước
      delay(1000);
      digitalWrite(WOUT_PIN, LOW); // đóng xả 

      while (tracking() >= waterMax) { 
      digitalWrite(LED_PIN, HIGH); 
      digitalWrite(LED_BOARD_PIN, HIGH);   
         Serial.println("Đang thêm nước");
          delay(1000);
      }  
      digitalWrite(LED_PIN, LOW); 
      digitalWrite(PW_PIN, HIGH);
      digitalWrite(LED_BOARD_PIN, LOW);   
      digitalWrite(WIN_PIN, LOW);  // Đóng nước
      waterDay = timeinfo.tm_mday;
  }
}
void food(){
  if(  foodDay != timeinfo.tm_mday && timeinfo.tm_mday  % 2 == 0 && timeinfo.tm_hour == 6  && timeinfo.tm_min < 5 ){  
    Serial.println("Đang cho ăn !!! ");
      digitalWrite(MOTOR_PIN, HIGH);
      delay(foodTime);
      digitalWrite(MOTOR_PIN, LOW); 
      foodDay = timeinfo.tm_mday;
  }
}
float tracking(){ // do khoảng cách
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW); 
  long duration = pulseIn(ECHO_PIN, HIGH); 
  float distance = (duration * 0.034) / 2;  // Tốc độ âm thanh là 340 m/s = 0.034 cm/μs 
  Serial.print("Khoảng cách: ");
  Serial.print(distance);
  Serial.println(" cm");
  return distance;
}
// Hàm kết nối Wi-Fi
void connectToWiFi() {
  WiFi.begin(ssid, password);
  
  // Đợi cho đến khi kết nối
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Đã kết nối Wi-Fi!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());
}
void checkWifi(){
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(ssid, password);

    // Chờ đến khi kết nối lại
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }

    Serial.println("\nReconnected to Wi-Fi!");
  }
}
