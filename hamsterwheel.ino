#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <user_interface.h>

#define wifi_ssid "memfis"
#define wifi_password "01tatonka"
#define wifi_hostname "hamsterwheel"

#define mqtt_server "chilicam"
#define mqtt_user ""
#define mqtt_password ""

const int hallPin =  5;       // the number of the hall effect sensor pin
const int dist = 540;         // inner distance of hamster wheel in mm
const int sleepTimeS = 10;    // deep sleep time in seconds
int lastState, curState = 0;  // variable for reading the hall sensor status
unsigned long contactBegin, contactEnd, durContact, durNonContact, durTotal = 0;
double speeed;
int secsInactive = 0;        // last signal from hall sensor received seconds ago

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    if (client.connect("ESP8266Client")) {
    //if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  if (!Serial) {
    Serial.begin(74880);
  }
  rst_info *resetInfo;
  resetInfo = ESP.getResetInfoPtr();
  Serial.println((*resetInfo).reason);
  Serial.println("HELLO");
  Serial.println(ESP.getResetReason());
  setup_wifi();
  client.setServer(mqtt_server, 1883);
        
  // initialize the hall effect sensor pin as an input:
  pinMode(hallPin, INPUT);
  
//  Serial.println("Going to sleep");
//  ESP.deepSleep(sleepTimeS * 1000000);
//  delay(100); 
}

void loop(){
//   Serial.println("Going to sleep");
//   ESP.deepSleep(sleepTimeS * 1000000);
//   delay(100); 
  
  // read the state of the hall effect sensor:
  curState = digitalRead(hallPin);

  if (lastState == LOW && curState == HIGH) {     
    // magnet contact begin
    secsInactive = 0;
    contactBegin = millis();
    durNonContact = contactBegin - contactEnd;    
  } 
  if (lastState == HIGH && curState == LOW) {     
    // magnet contact end:    
    contactEnd = millis();
    durContact = contactEnd - contactBegin;    
    durTotal = durContact + durNonContact;
    speeed = (double)dist/durTotal*3.6;
    Serial.println(speeed);
  }
  lastState = curState;

    if (!client.connected()) {
    reconnect();
  }
  client.loop();

  speeed = 33.0f;
  Serial.println(String(speeed).c_str());
  client.publish("domoticz/in", String(speeed).c_str(), true);
  delay(1000);
}
