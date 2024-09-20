#include <WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include "Ticker.h"
//#include <Servo.h>
#include "esp32-hal-ledc.h"

//Servo myServo;
#define COUNT_LOW 0
#define COUNT_HIGH 8888
#define TIMER_WIDTH 16

int servo = 27;

const char* ssid = "microelectronics";
const char* pass = "microelectronics2018";
const char* host = "10.0.26.207";
String message;
WiFiClient wifi;
PubSubClient mqtt(wifi);

int input1 = 23;
int input2 = 22;

int input3 = 21;
int input4 = 19;

char state;

int distanceSensorTrigerPin = 26; //sensor input
int distanceSensorEchoPin = 25; //sensor output
unsigned long duration = 0; // the actual distance the potensiometer makes
int distance = 0; // the distance the potenciometer will make

DHTesp dht;

void tempTask(void *pvParameters);
bool getTemperature();
void triggerGetTemp();

/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Comfort profile */
ComfortState cf;
/** Flag if task should run */
bool tasksEnabled = false;
int dht11 = 4; //dht11 signal //todo
int celsius = 0; //temprature in celcius

int ldr = 35; //light source (pin)
int ldrValue = 0;
int ldrValueCertification = 0;

// --------- SETUP ---------
void setup()
{


   ledcSetup(1, 50, TIMER_WIDTH); // channel 1, 50 Hz, 16-bit width
   ledcAttachPin(servo, 1);   // GPIO 22 assigned to channel 1
  
  Serial.begin(9600);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected. Your IP is: ");
  Serial.println(WiFi.localIP());

  //myServo.attach(digitalPin9);

  mqtt.setServer(host, 1883);
  mqtt.setCallback(callback);

  pinMode(input1, OUTPUT);
  pinMode(input2, OUTPUT);

  pinMode(input3, OUTPUT);
  pinMode(input4, OUTPUT);

  pinMode(distanceSensorTrigerPin, OUTPUT); // distance sensor output
  pinMode(distanceSensorEchoPin, INPUT); //distance sensor input

  initTemp();
}

void loop()
{
  connexion();
  thermostat();
//  sonar();
//  light();
  digitalWrite(input1, LOW);
  digitalWrite(input2, LOW);
  digitalWrite(input3, LOW);
  digitalWrite(input4, LOW);
  switch ( state )
  {
    case 'f':
      digitalWrite(input2, HIGH);
      digitalWrite(input3, HIGH);
      break;
    case 'b':
      digitalWrite(input1, HIGH);
      digitalWrite(input4, HIGH);
      break;
    case 'l':
      digitalWrite(input3, HIGH);
      break;
    case 'r':
      digitalWrite(input2, HIGH);
      break;
    default:
      break;
  }
  delay(1000);
}
// END LOOP
bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
  dht.setup(dht11, DHTesp::DHT11);
  Serial.println("DHT initiated");

  // Start task to get temperature
  xTaskCreatePinnedToCore(
      tempTask,                       /* Function to implement the task */
      "tempTask ",                    /* Name of the task */
      4000,                           /* Stack size in words */
      NULL,                           /* Task input parameter */
      5,                              /* Priority of the task */
      &tempTaskHandle,                /* Task handle. */
      1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("Failed to start task for temperature update");
    return false;
  } else {
    // Start update of environment data every 20 seconds
    tempTicker.attach(20, triggerGetTemp);
  }
  return true;
}

void callback(char* topic, byte * payload, unsigned int length) {

  for (int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.println("The message:");
  Serial.println(message);
  if (String(topic) == "forwardButton/esp32") {
    state = 'f';
    Serial.println(message);
    mqtt.publish("movement/esp32", String(message).c_str());
  }
  if (String(topic) == "backwardButton/esp32") {
    state = 'b';
    Serial.println(message);
    mqtt.publish("movement/esp32", String(message).c_str());
  }
  if (String(topic) == "leftButton/esp32") {
    state = 'l';
    Serial.println(message);
    mqtt.publish("movement/esp32", String(message).c_str());
  }
  if (String(topic) == "rightButton/esp32") {
    state = 'r';
    Serial.println(message);
    mqtt.publish("movement/esp32", String(message).c_str());
  }
  if (String(topic) == "stopButton/esp32") {
    state = 's';
    Serial.println(message);
    mqtt.publish("movement/esp32", String(message).c_str());
  }

  message = "";
}

void connexion() {
  if (!mqtt.connected()) {
    reconnect();
  }

  mqtt.loop();
}

void reconnect()
{
  while (!mqtt.connected())
  {
    if (mqtt.connect("ESP32 client"))
    {
      mqtt.subscribe("forwardButton/esp32");
      mqtt.subscribe("backwardButton/esp32");
      mqtt.subscribe("leftButton/esp32");
      mqtt.subscribe("rightButton/esp32");
      mqtt.subscribe("stopButton/esp32");
      
    }
    else
    {
      Serial.print("Reconnected.");
      Serial.print(mqtt.state());
      delay(1000);
    }
  }
}

void thermostat(){
  getTemperature();    
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
     xTaskResumeFromISR(tempTaskHandle);
  }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters) {
  Serial.println("tempTask loop started");
  while (1) // tempTask loop
  {
    if (tasksEnabled) {
      // Get temperature values
      getTemperature();
    }
    // Got sleep again
    vTaskSuspend(NULL);
  }
}

/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 * @return bool
 *    true if temperature could be aquired
 *    false if aquisition failed
*/
bool getTemperature() {
  // Reading temperature for humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
  TempAndHumidity newValues = dht.getTempAndHumidity();
  // Check if any reads failed and exit early (to try again).
  if (dht.getStatus() != 0) {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    return false;
  }

  float heatIndex = dht.computeHeatIndex(newValues.temperature, newValues.humidity);
  float dewPoint = dht.computeDewPoint(newValues.temperature, newValues.humidity);
  float cr = dht.getComfortRatio(cf, newValues.temperature, newValues.humidity);

  String comfortStatus;
  switch(cf) {
    case Comfort_OK:
      comfortStatus = "Comfort_OK";
      break;
    case Comfort_TooHot:
      comfortStatus = "Comfort_TooHot";
      break;
    case Comfort_TooCold:
      comfortStatus = "Comfort_TooCold";
      break;
    case Comfort_TooDry:
      comfortStatus = "Comfort_TooDry";
      break;
    case Comfort_TooHumid:
      comfortStatus = "Comfort_TooHumid";
      break;
    case Comfort_HotAndHumid:
      comfortStatus = "Comfort_HotAndHumid";
      break;
    case Comfort_HotAndDry:
      comfortStatus = "Comfort_HotAndDry";
      break;
    case Comfort_ColdAndHumid:
      comfortStatus = "Comfort_ColdAndHumid";
      break;
    case Comfort_ColdAndDry:
      comfortStatus = "Comfort_ColdAndDry";
      break;
    default:
      comfortStatus = "Unknown:";
      break;
  };

  Serial.println(" T:" + String(newValues.temperature) + " H:" + String(newValues.humidity) + " I:" + String(heatIndex) + " D:" + String(dewPoint) + " " + comfortStatus);
  mqtt.publish("thermostat/esp32", String(newValues.temperature).c_str());
  mqtt.publish("humidity/esp32", String(newValues.humidity).c_str());
  sonar();
  light();
  return true;
}

void sonar(){
  digitalWrite(distanceSensorTrigerPin, LOW);
  delayMicroseconds(5);
  digitalWrite(distanceSensorTrigerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(distanceSensorTrigerPin, LOW);
  pinMode(distanceSensorEchoPin, INPUT);
  duration = pulseIn(distanceSensorEchoPin, HIGH);
  distance = duration * 0.034/2; // distance in cm
  Serial.print("Distance from object: ");
  Serial.println(distance);
  mqtt.publish("distance/esp32", String(distance).c_str());
}

void light(){
  ldrValue = analogRead(ldr); //reads the value of the potensiometer
  ldrValueCertification = map(ldrValue,0,4095,255,0); // translates 0 to 0 and 1023 to 255
  Serial.print("Light is: ");
  Serial.println(ldrValueCertification);
  mqtt.publish("light/esp32", String(ldrValueCertification).c_str());
}
