#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>


unsigned long ti = 0;


// กำหนดรายละเอียด WiFi
const char* ssid = "PCAT_4G_WIFI";
const char* password = "pcat@1234";

// กำหนดรายละเอียด MQTT Broker
const char* mqtt_server = "mqtt.aj-tay.com";
const int mqtt_port = 1883;             // พอร์ต MQTT
const char* mqtt_user = "Pcat_device";  // ชื่อผู้ใช้สำหรับ MQTT
const char* mqtt_pass = "pcat@1234";    // รหัสผ่านสำหรับ MQTT

WiFiClient espClient;
PubSubClient client(espClient);

const int TX_EN = 13;
const int RX_EN = 32;

const int INPUT_1 = 39;
const int INPUT_2 = 34;
const int INPUT_3 = 25;

const int RELAY = 2;

int INPUT_1_State = 0;
int INPUT_2_State = 0;
int INPUT_3_State = 0;

int State_1 = 0;
int State_2 = 0;
// int State_3 = 0;
// int State_4 = 0;
int State_send = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (String(topic) == "Smart_PCAT/R_RELAY1") {
    if (messageTemp == "On") {
      State_1 = 1;
      Serial.println("Relay ON");
    } else if (messageTemp == "Off") {
      State_1 = 0;
      Serial.println("Relay OFF");
    }
  }
  if (String(topic) == "Smart_PCAT/R_RELAY2") {
    if (messageTemp == "On") {
      State_2 = 1;
      Serial.println("Relay ON");
    } else if (messageTemp == "Off") {
      State_2 = 0;
      Serial.println("Relay OFF");
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // ใช้ชื่อผู้ใช้และรหัสผ่านในการเชื่อมต่อ
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
      // Subscribe to a topic
      client.subscribe("Smart_PCAT/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
/////////////////////////////////////////////////////


volatile bool receivedFlag = false;
void setFlag(void) {
  // we got a packet, set the flag
  receivedFlag = true;
}


void setup() {
  pinMode(TX_EN, OUTPUT);
  pinMode(RX_EN, OUTPUT);
  pinMode(RELAY, OUTPUT);
  pinMode(INPUT_1, INPUT);
  pinMode(INPUT_2, INPUT);
  pinMode(INPUT_3, INPUT);
  digitalWrite(TX_EN, LOW);
  digitalWrite(RX_EN, HIGH);
  digitalWrite(RELAY, LOW);  //ปกติ


  Serial.begin(115200);
  delay(250);

  // เชื่อมต่อ WiFi
  setup_wifi();

  // กำหนดเซิร์ฟเวอร์ MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //  display.begin(0x3C, true);
  //  display.clearDisplay();
  //  display.setRotation(1);
  //  display.setTextSize(2);
  //  display.setTextColor(SH110X_WHITE);
}


void loop() {

  ti = millis();
  if (ti >= 43200000) {
    ESP.restart();
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();


  // Example: publish a message to a topic
  //client.publish("esp32/test", "Hello from ESP32");
  //delay(2000);

  INPUT_1_State = digitalRead(INPUT_1);
  INPUT_2_State = digitalRead(INPUT_2);
  INPUT_3_State = digitalRead(INPUT_3);

  String payload;

  ///////////////////////////////////////////รับสวิตซ์
  if (INPUT_1_State == 0 && State_1 == 0) {
    delay(10);
    payload = "On";
    client.publish("Smart_PCAT/SW1", payload.c_str());
    Serial.println("SW1 : 1");
    delay(1000);
    State_1 = 1;
  } else if (INPUT_1_State == 0 && State_1 == 1) {
    delay(10);
    payload = "Off";
    client.publish("Smart_PCAT/SW1", payload.c_str());
    Serial.println("SW1 : 0");
    delay(1000);
    State_1 = 0;
  }

  if (INPUT_2_State == 0 && State_2 == 0) {
    delay(10);
    payload = "On";
    client.publish("Smart_PCAT/SW2", payload.c_str());
    Serial.println("SW2 : 1");
    delay(1000);
    State_2 = 1;
  } else if (INPUT_2_State == 0 && State_2 == 1) {
    delay(10);
    payload = "Off";
    client.publish("Smart_PCAT/SW2", payload.c_str());
    Serial.println("SW2 : 0");
    delay(1000);
    State_2 = 0;
  }


  if (State_1 == 1 || State_2 == 1) {
    if (State_send == 0) {
      digitalWrite(RELAY, HIGH);
      payload = "On";
      State_send = 1;
      client.publish("Smart_PCAT/RELAY", payload.c_str());
    }
  }
  if (State_1 == 0 && State_2 == 0) {
    if (State_send == 1) {
      digitalWrite(RELAY, LOW);
      payload = "Off";
      State_send = 0;
      client.publish("Smart_PCAT/RELAY", payload.c_str());
    }
  }


  ///////////////////////////////////////////รับRF

  delay(1);
}