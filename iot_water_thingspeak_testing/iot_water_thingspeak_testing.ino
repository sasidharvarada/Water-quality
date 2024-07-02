#include <SPI.h>
#include <WiFiNINA.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char ssid[] = "SCRC_LAB_IOT";           // Replace with your Wi-Fi network name (SSID)
char password[] = "Scrciiith@123"; // Replace with your Wi-Fi network password

// char ssid[] = "myssid";           // Replace with your Wi-Fi network name (SSID)
// char password[] = "password"; // Replace with your Wi-Fi network password



int status = WL_IDLE_STATUS;

#define ONE_WIRE_BUS 14 // temp pin 
#define tdssensorPin 15 // tds pin 

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

int tdssensorValue = 0;
float tdsValue = 0;
float Voltage = 0;
float Temp = 0;
float tdsValue_without_temp = 0;

#define CHANNEL_ID "1699304"
#define CHANNEL_API_KEY "09IBZN66Q1UHSCPY"

WiFiClient wifiClient;

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }

  // Connect to Wi-Fi
  connectToWiFi();

  // Initialize ThingSpeak
  ThingSpeak.begin(wifiClient);

  // Print IP and MAC addresses
  printConnectionDetails();
}

void loop() {
  // Request temperature reading
  tempSensor.requestTemperaturesByIndex(0);
  // Get temperature value
  Temp = tempSensor.getTempCByIndex(0);
  Serial.print("Temperature: ");
  Serial.print(Temp);
  Serial.println(" °C");

  // Read TDS sensor value
  tdssensorValue = analogRead(tdssensorPin);
  Voltage = tdssensorValue * 3.3 / 1024.0;

  // Calculate TDS value with temperature compensation
  float compensationCoefficient = 1.0 + 0.02 * (Temp - 25.0);
  float compensationVoltage = Voltage / compensationCoefficient;
  tdsValue = (133.42 * compensationVoltage * compensationVoltage * compensationVoltage
    - 255.86 * compensationVoltage * compensationVoltage
    + 857.39 * compensationVoltage) * 0.5;
  Serial.print("TDS Value (with Temp Compensation) = ");
  Serial.print(tdsValue);
  Serial.println(" ppm");

  // Calculate TDS value without temperature compensation
  float compensationCoefficient_without_temp = 1.0 + 0.02 * (25.0 - 25.0);
  float compensationVoltage_without_temp = Voltage / compensationCoefficient_without_temp;
  tdsValue_without_temp = (133.42 * compensationVoltage_without_temp * compensationVoltage_without_temp
    - 255.86 * compensationVoltage_without_temp * compensationVoltage_without_temp
    + 857.39 * compensationVoltage_without_temp) * 0.5;
  Serial.print("TDS Value (without Temp Compensation) = ");
  Serial.print(tdsValue_without_temp);
  Serial.println(" ppm");

  // Send data to ThingSpeak
  ThingSpeak.setField(1, Temp);
  ThingSpeak.setField(2, tdsValue_without_temp);
  ThingSpeak.setField(3, tdsValue);
  ThingSpeak.setField(4, Voltage);

  unsigned long channelID = atol(CHANNEL_ID);
  int statusThingspeak = ThingSpeak.writeFields(channelID, CHANNEL_API_KEY);

  // Serial.print("Status Code thingspeak: ");
  // Serial.println(statusThingspeak);

  if (statusThingspeak == 200) {
    Serial.println("Data sent to ThingSpeak successfully!");
  } else {
    Serial.print("Failed to send data to ThingSpeak. HTTP error code: ");
    Serial.println(statusThingspeak);
  }

  delay(6000); // Wait for 15 seconds before the next iteration
}

void connectToWiFi() {
  // Check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }
  // Attempt to connect to Wi-Fi network
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    // delay(15000); // Delay for connection attempt
  }

  // Print connection details
  Serial.println("Connected to Wi-Fi");
  printConnectionDetails();
}

void printConnectionDetails() {
  // Print the board's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the MAC address
  byte mac[6];
  WiFi.macAddress(mac);
  Serial.print("MAC Address: ");
  for (int i = 0; i < 6; i++) {
    Serial.print(mac[i], HEX);
    if (i < 5) {
      Serial.print(":");
    }
  }
  Serial.println();
}