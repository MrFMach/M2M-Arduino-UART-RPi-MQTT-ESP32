
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>


// Create object
Adafruit_BMP085 bmp;

// GPIOs acccording your wiring
#define YEL 12
#define RED 13
#define GRE 14

const char* transArdu = "O";

// Wi-Fi ssid and password
const char* ssid = "networkname";
const char* password = "network#password";

// Your Raspberry Pi IP, in this format
IPAddress server(192, 168, 0, 13);

// Callback function, listen to the incoming message
void callback(char* topic, byte* payload, unsigned int length) {

  // message handling
  switch (*payload) {
    case 114:                     // the letter "r" ASCII
      digitalWrite(RED, HIGH);
      digitalWrite(YEL, LOW);
      digitalWrite(GRE, LOW);

      break;
    case 121:                     // the letter "y" ASCII
      digitalWrite(YEL, HIGH);
      digitalWrite(RED, LOW);
      digitalWrite(GRE, LOW);
      break;
    case 103:                     // the letter "g" ASCII
      digitalWrite(GRE, HIGH);
      digitalWrite(RED, LOW);
      digitalWrite(YEL, LOW);
      break;
    default:
      digitalWrite(RED, LOW);
      digitalWrite(YEL, LOW);
      digitalWrite(GRE, LOW);
      break;
  }
  publishBmp();
}

WiFiClient esp32Client;
PubSubClient client(server, 1883, callback, esp32Client);

void startWifi() {
  Serial.print("\nEsp32 connect with ");
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Starting wifi connect...");
    WiFi.begin(ssid, password);
    delay(2000);
  }
  Serial.println("Wifi connected!");
  Serial.print("Esp32 IP address: ");
  Serial.println(WiFi.localIP());
}

void connectBroker() {
  // User and password configured in the broker
  if (client.connect("espClient", "mrfmach", "mosquitto")) {
    client.subscribe("esp32/led");
    Serial.println("Broker connected!");
    delay(2000);
  }
}

void startBmp() {
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085/BMP180 sensor, check wiring!");
    while (1) {}
  }
}

void publishBmp() {

  char tempString[5];
  char pressString[6];

  float temperature = bmp.readTemperature();
  int pressure = bmp.readPressure();

  dtostrf(temperature, 4/*width*/, 1 /*precision*/, tempString );
  client.publish("esp32/temp", tempString, 2);

  dtostrf(pressure, 5/*width*/, 0 /*precision*/, pressString );
  client.publish("esp32/press", pressString, 5);

  Serial.print(tempString);
  Serial.print(" | ");
  Serial.println(pressString);

  if (temperature > 23 && temperature <= 26.5) {
    transArdu = "Y";
    client.publish("arduino/led", transArdu);
  } else if (temperature > 26.5) {
    transArdu = "R";
    client.publish("arduino/led", transArdu);
  } else {
    transArdu = "O";
    client.publish("arduino/led", transArdu);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(RED, OUTPUT);
  pinMode(YEL, OUTPUT);
  pinMode(GRE, OUTPUT);
  startWifi();
  startBmp();
  connectBroker();
}

void loop() {
  // check connection
  while (!client.connected()) {
    Serial.print("Broker disconnected! Returned state =  ");
    Serial.println(client.state());
    connectBroker();
  }

  // maintain the connection to the server
  client.loop();

}
