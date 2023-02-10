#include <Adafruit_SSD1306.h>
#include <WiFi.h> // header we need to include for WiFi functions
#include <WebServer.h>
int FloatSensor1 = 14;
int FloatSensor2 = 27;
int FloatSensor3 = 32;
int floatState1 = 1;
int floatState2 = 1;
int floatState3 = 1;
int currState = 1;
int solenoid =  15;
int pumpResovoir = 13;
int pumpDrain = 12;
String states [] = {"OFF", "ON", "AUTO"};
int prevState = 1;
Adafruit_SSD1306 lcd(128, 64);
const char* ssid = "pump-project";
const char* pass = "cics2560";
WebServer server(80);
const char* mainPage = "<body><script>window.buttonSubmit = async e => {\n       const submitString = `/submit?state=${e.value}`;\n       console.log(submitString)\n       await fetch(submitString, { method: \"GET\" })\n    }</script><style>.title{font-family:Arial,sans-serif;text-align:center}.hstack{display:flex;justify-content:center}.button{border:none;color:#fff;padding:15px 32px;text-align:center;text-decoration:none;font-size:16px;margin:4px 2px;cursor:pointer}.green{background-color:#4caf50}.blue{background-color:#008cba}.red{background-color:red}</style><h1 class=title>System Control</h1><div class=hstack><button class=\"button red\"name=pump_off onclick=window.buttonSubmit(this) value=off>Pump Off</button> <button class=\"button green\"name=pump_on onclick=window.buttonSubmit(this) value=on>Pump On</button> <button class=\"button blue\"name=auto onclick=window.buttonSubmit(this) value=auto>Auto</button></div>";
String pumpState = "unset";
void on_home() {
  server.send(200, "text/html", mainPage);
}

void setup() {
  Serial.begin(9600);
  pinMode(FloatSensor1, INPUT_PULLUP);
  pinMode(FloatSensor2, INPUT_PULLUP);
  pinMode(FloatSensor3, INPUT_PULLUP);
  pinMode(solenoid, OUTPUT);
  pinMode(pumpDrain, OUTPUT);
  pinMode(pumpResovoir, OUTPUT);
  lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  lcd.setTextColor(WHITE);
  lcd.clearDisplay();
  lcd.display();
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, pass);
  server.on("/", on_home);
  server.on("/submit", HTTP_GET, []() {
    Serial.println("Got command");
    if (server.arg("state")) {
      String value = server.arg("state");
      Serial.println(value);
      pumpState = value;
    } else {
      Serial.println("ERROR: No state found");
    }
    server.send(200, "text/plain", "success");
  });

  server.begin();  // starts server
}

void loop() {
  floatState1 = digitalRead(FloatSensor1);
  floatState2 = digitalRead(FloatSensor2);
  floatState3 = digitalRead(FloatSensor3);
  server.handleClient();  // handle client requests, must call frequently
  pumpState = server.arg("state");
  if (pumpState == "off") {
    currState = 1;
  } else if (pumpState == "on") {
    currState = 2;
  } else if (pumpState == "auto") {
    currState = 3;
  }

  
  

  switch (currState) {
    //OFF
    case 1 :
      digitalWrite(solenoid, LOW);
      digitalWrite(pumpDrain, LOW);
      digitalWrite(pumpResovoir, LOW);
      lcd.clearDisplay();
      lcd.setCursor(0, 0);
      lcd.println("MODE: OFF");
      lcd.display();
      break;
    //ON
    case 2 :
      digitalWrite(pumpDrain, HIGH);
      digitalWrite(solenoid, LOW);
      digitalWrite(pumpResovoir, LOW);
      lcd.clearDisplay();
      lcd.setCursor(0, 0);
      lcd.println("MODE: ON");
      lcd.println("PUMP: ON");
      if (floatState3 == HIGH) {
        lcd.println("WARNING: NO WATER DETECTED");
      }
      lcd.display();
      break;
    //AUTO
    case 3 :
      lcd.clearDisplay();
      lcd.setCursor(0, 0);
      lcd.println("MODE: AUTO");
      if (floatState1 == HIGH){
        digitalWrite(solenoid, HIGH);
        delay(500);
        digitalWrite(pumpResovoir, HIGH);
        delay(500);
        digitalWrite(pumpDrain, LOW);
        lcd.println("PUMP: OFF");
        delay(2000);
        prevState = 1;
      } else if (floatState2 == HIGH) {
        digitalWrite(pumpResovoir, LOW);
        delay(500);
        digitalWrite(solenoid, LOW);
        delay(1000);
        digitalWrite(pumpDrain, HIGH);
        delay(500);
        lcd.println("PUMP: ON");
        prevState = 2;
      } else if (floatState3 == HIGH) {
        digitalWrite(pumpDrain, LOW);
        delay(500);
        digitalWrite(solenoid, HIGH);
        delay(500);
        digitalWrite(pumpResovoir,HIGH);
        delay(500);
        lcd.println("PUMP: OFF");
        prevState = 3;
      }else if(floatState1 == LOW && floatState2 == LOW and floatState3 == LOW){
        if(prevState == 1){
          digitalWrite(solenoid, HIGH);
          delay(500);
          digitalWrite(pumpResovoir, HIGH);
          delay(500);
          digitalWrite(pumpDrain, LOW);
          lcd.println("PUMP: OFF");
          delay(2000);
        }
        if(prevState == 2){
          digitalWrite(pumpResovoir, LOW);
          delay(500);
          digitalWrite(solenoid, LOW);
          delay(1000);
          digitalWrite(pumpDrain, HIGH);
          delay(500);
          lcd.println("PUMP: ON");
        }
        if(prevState == 3){
          digitalWrite(pumpDrain, LOW);
          delay(500);
          digitalWrite(solenoid, HIGH);
          delay(500);
          digitalWrite(pumpResovoir,HIGH);
          delay(500);
          lcd.println("PUMP: OFF");
        }
      }

      lcd.display();
      break;
    default:
      break;
  }
}