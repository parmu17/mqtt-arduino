#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define RELAY1 (5) //D1
#define RELAY2 (4)  //D
//#define SERIAL_DEBUG
//#define TLS_DEBUG

/* CA Cert in PEM format */
const char caCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIEJTCCAw2gAwIBAgIULtu+fJoHrngQ1UEX0QqxlcNSG9UwDQYJKoZIhvcNAQEL
BQAwgaExCzAJBgNVBAYTAklOMRQwEgYDVQQIDAtNQUhBUkFTSFRSQTENMAsGA1UE
BwwEUFVORTEbMBkGA1UECgwSTFdQIFNJTVBMSUZJRUQgT1BDMRQwEgYDVQQLDAtF
TEVDVFJPTklDUzEWMBQGA1UEAwwNbHdwdGVjaC5jby5pbjEiMCAGCSqGSIb3DQEJ
ARYTcHNhaHVAbHdwdGVjaC5jby5pbjAeFw0yMDA0MDYwODE4MjBaFw0yMzA0MDYw
ODE4MjBaMIGhMQswCQYDVQQGEwJJTjEUMBIGA1UECAwLTUFIQVJBU0hUUkExDTAL
BgNVBAcMBFBVTkUxGzAZBgNVBAoMEkxXUCBTSU1QTElGSUVEIE9QQzEUMBIGA1UE
CwwLRUxFQ1RST05JQ1MxFjAUBgNVBAMMDWx3cHRlY2guY28uaW4xIjAgBgkqhkiG
9w0BCQEWE3BzYWh1QGx3cHRlY2guY28uaW4wggEiMA0GCSqGSIb3DQEBAQUAA4IB
DwAwggEKAoIBAQDWxorr0M7UvglJ9SoJsM2MUU5vHys4l0D6RXlI64xv2qHdmqVm
00Z//guU3PEU8Q8juVFGYtWPiOgkfwn6vk/O5ZtGlgitIIOLcUzVbseVvoCMIgWK
nn6kSIoO3hjiJISMgVns//20vEn9I8bDVvFARbe15Co/J9jAERVRUuxN4PoYo2IK
a+sh+ImPS2ibcWWe7oOD4iyai6R505EVWaqDSUKsH+LZ2FhTKNE3lNtZk/glNHZg
R5tSMUaxPngaYup6pT80+32NKZENl8iJoibfrC5NLSyNJR0Z1gxiPFVzLg3gJJXt
ukpDKJJqnseob4JQr6Vg1g/DpbwEXVrTkT/bAgMBAAGjUzBRMB0GA1UdDgQWBBTj
ZEsxGyHeJ50tSS1h38HQGwbEcDAfBgNVHSMEGDAWgBTjZEsxGyHeJ50tSS1h38HQ
GwbEcDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQBFve1JgcHV
+cIySt8zmrfi8wT5/GFwvZ+vwsGyWJsfr2u1WCQbGcjky7y/KSyHTFUy6xo0cTqK
RiD/WOSf5XRZWnqtr14FN3UUu1NFkBJq0DIQPZnZwoD2Q3veXgoPNUQdMzFLv+Fv
5Oq/nN6r6MmotKPp4WfA2uQMx2+P04nnxcfntNBC4/OVfZFTXJeGoC+UfaQVn85U
bWKR7k9jCjd7D3bdDV+Otbj5Po1hfZhEkNHFUkOqlCgAZC8yLH8lRIZqlvs+UlMM
w+ZPEZ3WpSWBoJHEz8wF+RDCn+EyV2Bc6SyiE0I7xtoLQaUccC/WXh8bwYf3hL0j
4hYvL0Lh9Mkc
-----END CERTIFICATE-----
)EOF";

const uint8_t mqttCertFingerprint[] = "SHA1 Fingerprint=A5:40:71:7B:8E:B5:48:4F:CC:9A:DB:F2:3E:2D:15:2F:07:F2:23:C7";
X509List caCertX509(caCert);        /* X.509 parsed CA Cert */
WiFiClientSecure espClient;         /* Secure client connection class, as opposed to WiFiClient */
PubSubClient mqttClient(espClient);
String clientId = "ESP8266Client-";

/* Test WiFiClientSecure connection using supplied cert and fingerprint */
bool verifytls(){
  bool success = false;
  Serial.print("Verifying TLS connection to ");
  Serial.println("167.71.219.86");
  success = espClient.connect("167.71.219.86", 8883);
  if(success){
    Serial.println("Connection complete, valid cert, valid fingerprint.");
  }
  else{
    Serial.println("Connection failed TLS connection to 167.71.219.86!");
  }
  return (success);
}

void reconnect(){
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT broker connection...");
    if (mqttClient.connect(clientId.c_str(), "parmu17", "Parm@18001800")) {
      Serial.println("connected");
      mqttClient.subscribe("lwp/brd5/esp1");      
    } 
    else{
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Trying again in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() 
{
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, 1); //Initially led off, (active low pin)
//  Serial.setDebugOutput(true);
  Serial.begin(115200);
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.begin("SmilePlease", "sahu123456");
  Serial.print("Connecting");
  delay(100);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
  /* Configure secure client connection */
//  espClient.setTrustAnchors(&caCertX509);         
//  espClient.allowSelfSignedCerts();               
//  espClient.setFingerprint(mqttCertFingerprint);
//  
//  /* Optionally do none of the above and allow insecure connections.                                             
//   * This will accept any certificates from the server, without validation and is not recommended.
//   */
  espClient.setInsecure();
                                             
  /* Call verifytls to verify connection can be done securely and validated - this is optional but was useful during debug */
  verifytls();

  /* Configure MQTT Broker settings */
  mqttClient.setServer("lwptech.co.in", 8883);
  mqttClient.setCallback(subCallback);

  /* Add random hex client ID suffix once during each reboot */
  clientId += String(random(0xffff), HEX); 
}

void loop() 
{
  if(!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();  //execute callback function
}

void subCallback(char *topic, byte *payload, unsigned int length){
  static int led1sts = 1; // OFF STATE for builtin led, ACTIVE LOW PIN
  static int relay1sts = 0;
  static int relay2sts = 0;
  DynamicJsonDocument doc(256); //instance of json document
  deserializeJson(doc, (char*)payload);  //string message in stored in the raw json format
  JsonObject root = doc.as<JsonObject>(); //instance of json object
  
  serializeJson(root, Serial);  //store in json format
  Serial.println();

  if(!root["led1"].isNull()) {
    if(root["led1"] == "toggle") {
      led1sts = !led1sts;
    } else if (root["led1"] == "on") {
      led1sts = 0;  //active low pin for builtin led
    } else if (root["led1"] == "off") {
      led1sts = 1;
    } else {
      return;
    }
    digitalWrite(BUILTIN_LED, led1sts);
    mqttClient.publish("lwp/brd5/esp1/status/led1", led1sts == 0 ? "The LED-1 is on" : "The LED-1 is off"); //active low pins
  }else if(!root["getstatus"].isNull()) {
    if(root["getstatus"] == "led1") {
      mqttClient.publish("lwp/brd5/esp1/status/led1", led1sts == 0 ? "The LED-1 on" : "The LED-1 is off");
    }
  }

  if(!root["relay1"].isNull()) {
    if(root["relay1"] == "toggle") {
      relay1sts = !relay1sts;
    } else if (root["relay1"] == "on") {
      relay1sts = 1;
    } else if (root["relay1"] == "off") {
      relay1sts = 0;
    } else {
      return;
    }
    digitalWrite(RELAY1, relay1sts);
    mqttClient.publish("lwp/brd5/esp1/status/relay1", relay1sts == 0 ? "The Relay-1 is de-activated" : "The Relay-1 is activated");
  }else if(!root["getstatus"].isNull()) {
    if(root["getstatus"] == "relay1") {
      mqttClient.publish("lwp/brd5/esp1/status/relay1", relay1sts == 0 ? "The Relay-1 is de-activated" : "The Relay-1 is activated");
    }
  }

  if(!root["relay2"].isNull()) {
    if(root["relay2"] == "toggle") {
      relay2sts = !relay2sts;
    } else if (root["relay2"] == "on") {
      relay2sts = 1;
    } else if (root["relay2"] == "off") {
      relay2sts = 0;
    } else {
      return;
    }
    digitalWrite(RELAY2, relay2sts);
    mqttClient.publish("lwp/brd5/esp1/status/relay2", relay2sts == 0 ? "The Relay-2 is de-activated" : "The Relay-2 is activated");
  }else if(!root["getstatus"].isNull()) {
    if(root["getstatus"] == "relay2") {
      mqttClient.publish("lwp/brd5/esp1/status/relay2", relay2sts == 0 ? "The Relay-2 is de-activated" : "The Relay-2 is activated");
    }
  }
}
