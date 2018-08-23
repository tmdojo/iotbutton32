/*
IoT Button Workshop example code

Author: Shunya Sato

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

#include "secret.h"

/*
secret.h defines following variables

// Change to your key and event name
const char* IFTTT_KEY = "ifttt_key";
const char* IFTTT_EVENT = "ifttt_event";

// AP at home so that you can use it at home
#define WLAN_SSID_HOME ""
#define WLAN_PASS_HOME ""

// AP at workshop location
#define WLAN_SSID ""
#define WLAN_PASS ""
*/

const int BTN_PIN = 21; // GPIO0 is pull-up to 3.3v. Press BTN to GND.
const int LED_PIN = 13;

int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

WiFiMulti wifiMulti;
bool connected = false;

void setup() {
  Serial.begin(115200);

  for(uint8_t t = 4; t > 0; t--) {
      Serial.printf("[SETUP] WAIT %d...\n", t);
      Serial.flush();
      delay(1000);
  }

  wifiMulti.addAP(WLAN_SSID_HOME, WLAN_PASS_HOME);
  wifiMulti.addAP(WLAN_SSID, WLAN_PASS);

  Serial.println("Connecing Wifi...");
  pinMode(BTN_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  if (check_wifi_connection() == false) return;

  // check Button status with debounce
  int reading = digitalRead(BTN_PIN);
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // stayed in the state for debounce time
    if (reading != buttonState) {
      // button state has changed:
      buttonState = reading;
      if (buttonState == LOW) {
        Serial.println("Button is pressed.");
        trigger_ifttt();
      }
    }
  }
  lastButtonState = reading;
}

void trigger_ifttt(){
  String query = "/trigger/";
  query += IFTTT_EVENT;
  query += "/with/key/";
  query += IFTTT_KEY;
  query += "?value1=21.0"; // for future use

  HTTPClient http;
  Serial.print("Sending request: ");
  Serial.print("maker.ifttt.com");
  Serial.println(query);
  http.begin("maker.ifttt.com",80,query);
  int httpCode = http.GET();
  if (httpCode > 0){
    Serial.println("Success!");
    light_success();
  } else {
    Serial.print("Error code: ");
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    light_fail();
  }
  http.end();
}

void light_success(){
  // flash LED slowly 5 times
  for (int i=0; i<5; i++){
    digitalWrite(LED_PIN, LOW);
    delay(300);
    digitalWrite(LED_PIN, HIGH);
    delay(300);
  }
}

void light_fail(){
  // flash LED quickly 10 times
  for (int i=0; i<10; i++){
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}

bool check_wifi_connection(){
  if (wifiMulti.run() != WL_CONNECTED) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("WiFi not connected!");
    connected = false;
    delay(1000);
    return false;
  } else {
    if (connected == false){
      // changed from not connected to connected
      digitalWrite(LED_PIN, HIGH);
      connected = true;
      Serial.println("");
      Serial.println("WiFi connected");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    }
    return true;
  }
}
