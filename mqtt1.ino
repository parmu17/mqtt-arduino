#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "SmilePlease";
const char* password = "sahu123456";
const char* mqtt_server = "lwptech.co.in";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++){
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1'){
    digitalWrite(BUILTIN_LED, 0);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  }else{
    digitalWrite(BUILTIN_LED, 1);  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), "parmu17", "Parm@18001800")){
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("inTopic");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup(){
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, 1); //ACTIVE LOW PIN, SO LED OFF
  Serial.begin(115200);
  setup_wifi(); //execution will be inside while loop untill connection
  client.setServer(mqtt_server, 8883);
  client.setCallback(callback); //define all task after incoming message (published by a client)
}

void loop(){
  if (!client.connected()) {
    reconnect();  //once again susbscribe and publish the topics after connection
  }
  client.loop();  //Infinite loop serving mqtt request, on every request the callback function will be executed once.

  long now = millis();
  if(now - lastMsg > 2000){
    lastMsg = now;
    ++value;
    snprintf(msg, 75, "hello world #%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopic", msg);
  }
}
