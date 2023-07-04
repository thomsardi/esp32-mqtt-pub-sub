#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Vector.h>

// Replace the next variables with your SSID/Password combination
const char* ssid = "BLACKZONE";
const char* password = "tetanggamalingwifi02";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "192.168.1.10";
const int ledPin = 2;

IPAddress ipAddr(192, 168, 1, 10);

long lastReconnectAttempt = 0;
unsigned long lastTime = 0;
uint8_t data[1] = {0};

uint8_t byteStorage[10];
Vector<uint8_t> buffer(byteStorage);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
    Serial.print(message[i]);
    buffer.push_back(message[i] - 48);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(String(topic) == "data/output")
  {
    if (buffer.at(0) == 1)
    {
      digitalWrite(ledPin, HIGH);
    }
    else
    {
      digitalWrite(ledPin, LOW);
    }
  }

  buffer.clear();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off". 
  // Changes the output state according to the message
  // if (String(topic) == "esp32/output") {
  //   Serial.print("Changing output to ");
  //   if(messageTemp == "on"){
  //     Serial.println("on");
  //     digitalWrite(ledPin, HIGH);
  //   }
  //   else if(messageTemp == "off"){
  //     Serial.println("off");
  //     digitalWrite(ledPin, LOW);
  //   }
  // }
}

boolean reconnect() {
  if (client.connect("esp32Client")) {
    // Once connected, publish an announcement...
    // uint8_t buff[1] = {255};
    // client.publish("data/input",buff,1);
    // ... and resubscribe
    client.subscribe("data/output");
  }
  return client.connected();
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  // client.setServer(ipAddr, 1883);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connect("esp32Client"))
  {
    Serial.println("Try connect to MQTT broker");
  }
  client.subscribe("data/output");
  lastReconnectAttempt = 0;
  pinMode(ledPin, OUTPUT);
  lastTime = millis();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    Serial.println("Disconnected from MQTT Broker");
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      if (reconnect()) {
        lastReconnectAttempt = 0;
        Serial.println("Connected to MQTT Broker");
      }
    }
    lastTime = millis();
  } else {
    // Client connected
    if(millis() - lastTime > 200)
    {
      char temp[10];
      utoa(data[0], temp, 10);
      // Serial.println("Publish data");
      // Serial.println("Data : " + String(data[0]));
      // client.publish("data/input", data, 1);
      client.publish("data/input", temp);
      data[0]++;
      if(data[0] >= 255)
      {
        data[0] = 0;
      }
      lastTime = millis();
    }
    client.loop();
  }
}



