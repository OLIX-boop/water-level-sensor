#include <WiFi.h>
#include <HTTPClient.h>

#define Button 12 //pin sensore di livello
#define Led 2 // internal led
// led blinking = connecting to WIFI
// led still for 3 seconds and then turn off = connected succesfully to WIFI
#define red 23 // error = not connected to wifi
#define green 21 // success = connected succesfully

//WIFI settings
const char * ssid = "Home&Life SuperWiFi-2A8E";
const char * password = "UUP3P4PN38873JN4";

//Alexa Routine Trigger API
String Webhook="https://www.virtualsmarthome.xyz/url_routine_trigger/activate.php?trigger=8ad464b7-3083-451f-a400-1d201f5a9780&token=60048aa8-9581-4024-980f-c53e85441340";

void setup() {
  Serial.begin(9600);

 //Initialize buttons
  pinMode(Button,INPUT_PULLUP);
  pinMode(Led,OUTPUT);

  //Wifi Connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(Led, HIGH);
    delay(500);
    digitalWrite(Led, LOW);
    Serial.print(".");
    delay(500);
  }
  digitalWrite(Led, HIGH);
  Serial.println("Connected");
  delay(3000);
  digitalWrite(Led, LOW);
}


void SendAlert(void)
{
  Serial.println("Sent");
  String url = Webhook;  

  //Starts Connection
  Serial.println(url);
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  http.begin(url);
  Serial.print("[HTTP] GET...\n");
  int httpCode = http.GET();
  
  //If failed connection
  if(httpCode > 0) {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    if(httpCode == HTTP_CODE_OK) {
      Serial.println("Success");
    }
  } else {
    Serial.printf("[HTTP] GET failed: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();
}

int AlertCount = 0;
int maxAlerts = 5;

void AlertLoop() {
  while (true) {
    if (AlertCount >= maxAlerts) 
      break;
      
    digitalWrite(Led, HIGH);
    delay(1500);
    digitalWrite(Led, LOW);

    for (int i=0; i < 5; i++) {
      digitalWrite(Led, HIGH);
      delay(200);
      digitalWrite(Led, LOW);
      delay(200);
    }

    AlertCount++;
  }
}

int SensorActivationSafeZone = 5; // check if the water level in down for at least 5 seconds. To prevent any fake activation ex: maintance
bool Danger = false; // if the sensor is activated the sensor stops checking until the user activates it manually
void loop() {
  
  int ButtonState=digitalRead(Button); //Reads sensor pin

  if (!Danger) {
    if(ButtonState==LOW)
    { // LOW == water level dropped
      bool falseAlarm = false;
      for (int seconds = 0; seconds < SensorActivationSafeZone; seconds++) {
        Serial.println("Checking for false alarm...");
        if (digitalRead(Button) == HIGH) {
          falseAlarm = true;
          Serial.println("Fake Alarm!");
          break;
        }
        delay(1000);
      }

      if (!falseAlarm) {
        SendAlert(); //Sends the alert to alexa that will eventually shut down the plugged device  
        AlertLoop(); // led alert
        Danger=true;
      }
    }
  } else {
    Serial.println("DANGER MODE");
    delay(1000*60*5);
  }
}