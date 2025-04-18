#include <Arduino.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>

#define LIGHT_ADC_PIN            33
#define SOUND_ADC_PIN            32
#define DHT_DATA_PIN              18
//LUX=A/v^B
int lux_a=1;//LUX转换参数
int lux_b=2;//LUX转换参数
int adc_light_value;
float voltage_light;
float lux;
int adc_mic_value;
float voltage_mic;
float sound;
int humidity;
float temperature;
sensors_event_t a, g, temp;
// const char* ssid = "Ax3000T";
// const char* password = "123zr123";
// const char* serverUrl = "http://192.168.31.95:3232/data"; // 服务器地址（局域网）
const char* ssid = "Xiaomi 14";
const char* password = "123zr123";
const char* serverUrl = "http://192.168.138.188:3232/data"; // Macbook地址（连接Xiaomi 14）
DHT dht(DHT_DATA_PIN,DHT11);
Adafruit_MPU6050 mpu;
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);


void readSensor(){
  adc_light_value = analogRead(LIGHT_ADC_PIN);
  voltage_light = adc_light_value * (3.3/4095.0);
  lux = lux_a/pow(voltage_light,lux_b);
  adc_mic_value = analogRead(SOUND_ADC_PIN);
  voltage_mic = adc_mic_value * (3.3/4095.0);
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  mpu.getEvent(&a, &g, &temp);
}
void displayData(){
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Accelerater - m/s^2");
  display.print(a.acceleration.x, 1);
  display.print(", ");
  display.print(a.acceleration.y, 1);
  display.print(", ");
  display.print(a.acceleration.z, 1);
  display.println("");
  display.println("Gyroscope - rps");
  display.print(g.gyro.x, 1);
  display.print(", ");
  display.print(g.gyro.y, 1);
  display.print(", ");
  display.print(g.gyro.z, 1);
  display.println("");
  display.print("Lux:");display.print(lux);display.println("");
  display.print("Sound:");display.print(adc_mic_value);display.println("");
  display.print("Humidy:");display.print(humidity);display.println("");
  display.print("Temp:");display.print(temperature);display.println("");
  display.display();
}
String packData(){
  if (isnan(humidity) || isnan(temperature)) {
    humidity = 0;
    temperature = 0;
  }
  String jsonData = "{";
  jsonData += "\"temp\":" + String(temperature) + ",";
  jsonData += "\"hum\":" + String(humidity) + ",";
  jsonData += "\"sound\":" + String(voltage_mic) + ",";
  jsonData += "\"light\":" + String(lux) + ",";
  jsonData += "\"air_quality\":" + String(20) + ",";
  jsonData += "\"pir\":" + String(1)+ ",";
  jsonData += "\"acc\":[" + String(a.acceleration.x) + "," + String(a.acceleration.y) + "," + String(a.acceleration.z) + "]"+ ",";
  jsonData += "\"gyro\":[" + String(g.gyro.x) + "," + String(g.gyro.y) + "," + String(g.gyro.z) + "]";
  jsonData += "}";
  return jsonData;
}
void sendData() {
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("WiFi Disconnected");
    return;
  }
  
    String DataToSend = packData();
    HTTPClient http;
    http.begin(serverUrl);
    http.addHeader("Content-Type", "application/json");
    Serial.println(DataToSend);
    int httpResponseCode = http.POST(DataToSend);  
    http.end();
    Serial.println(httpResponseCode);
  
}

Ticker ticker1(sendData, 5000, 0, MILLIS);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //     delay(1000);
  //     Serial.println(".");
  //   }
   //WiFi.enableIpV6();
   //delay(5000);
   Serial.println(WiFi.localIP());
  if (!mpu.begin()) {
    Serial.println("Sensor init failed");
    while (1)
      yield();
  }
  Serial.println("Found a MPU-6050 sensor");

  // SSD1306_SWITCHCAPVCC = gen                                         erate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);
  pinMode(LIGHT_ADC_PIN,INPUT);//ADC光敏电阻
  pinMode(SOUND_ADC_PIN,INPUT);//ADC声敏电阻
  pinMode(5,OUTPUT);
  digitalWrite(5,HIGH);
  pinMode(17,OUTPUT);
  digitalWrite(17 ,LOW);
  dht.begin();
  ticker1.start();
}


void loop() {
  readSensor();
  displayData();
  ticker1.update();
}
