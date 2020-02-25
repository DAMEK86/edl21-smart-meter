#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <encode.h>
#include <secrets.h>

// Set web server port number to 80
ESP8266WebServer server(80);

Edl21 encoder;


extern const char* ssid;
extern const char* password;

const int BAUDRATE = 9600;
const byte led = 13;

bool foundStart = false;
bool foundEscape = false;
bool foundConsumption = false;
bool showSmlMessage = false;
int startIndex = 0; //start index for start sequence search
int stopIndex; //start index for stop sequence search
unsigned char smlMessage[460]; //byte to store the parsed message
unsigned char visibleMessage[460];
unsigned char inByte;

unsigned char StartSequence[8] = { 0x1b, 0x1b, 0x1b, 0x1b, 0x01, 0x01, 0x01, 0x01 };
unsigned char EscapeSequence[5] = {0x1b, 0x1b, 0x1b, 0x1b, 0x1a};
float currentconsumptionHTkWh, currentconsumptionNTkWh, deliveryHTkWh, deliveryNTkWh, consumptionOverallkWh, deliveredOverallkWh; //variable to calulate actual "Verbrauch Tarif 2" in kWh
int powerAll, powerL1, powerL2, powerL3;
int smlIndex; //index counter within smlMessage array
int state = 0;

void findStopSequence() {
  while (Serial.available())
  {
    inByte = Serial.read();
    smlMessage[smlIndex] = inByte;
    smlIndex++;

    if (encoder.FindEscapeIndexInSequence(smlMessage, smlIndex) != -1) {
        state = 2;
        foundEscape = true;
    }
  }
}

void findStartSequence() {
  while (Serial.available() && state != 1)
  {
    inByte = Serial.read(); //read serial buffer into array
    if (inByte == StartSequence[startIndex]) //in case byte in array matches the start sequence at position 0,1,2...
    {
      smlMessage[startIndex] = inByte; //set smlMessage element at position 0,1,2 to inByte value
      startIndex++;

      if (startIndex == sizeof(StartSequence)) //all start sequence values have been identified
      {
        state = 1;
        foundStart = true;
        smlIndex = startIndex; //set start index to last position to avoid rerunning the first numbers in end sequence search
        startIndex = 0;
      }
    }
    else {
      startIndex = 0;
    }
  }

  if(state == 1) {
    findStopSequence();
  }
}

unsigned int currentconsumption = 0;
float findConsumptionInSequence(unsigned char* sequence, unsigned char sequenceSize) {
  currentconsumption = 0;
  currentconsumption = encoder.FindConsumptionInSequence(smlMessage, sizeof(smlMessage), sequence, sequenceSize);
  if (currentconsumption == 0) {
    return NAN;
  }
  return float(currentconsumption)/10000; // 10.000 impulses per kWh
}

void array_to_string(byte array[], unsigned int len, char buffer[])
{
    for (unsigned int i = 0; i < len; i++)
    {
        byte nib1 = (array[i] >> 4) & 0x0F;
        byte nib2 = (array[i] >> 0) & 0x0F;
        buffer[i*2+0] = nib1  < 0xA ? '0' + nib1  : 'A' + nib1  - 0xA;
        buffer[i*2+1] = nib2  < 0xA ? '0' + nib2  : 'A' + nib2  - 0xA;
    }
    buffer[len*2] = '\0';
}

String SendHTML(){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<META http-equiv=\"refresh\" content=\"10\">\n";
  ptr +="<title>Smart-Meter</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-top: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #1abc9c;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 14px;margin: 25px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #1abc9c;}\n";
  ptr +=".button-on:active {background-color: #16a085;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP8266 Smart-Meter Web Server</h1>\n";
  
  ptr +="<h3>Verbrauch (A+)</h3>\n";
  ptr +="<p>Gesamt  - 1.8.0: " + String(consumptionOverallkWh) + " kWh</p>";
  ptr +="<p>Tarif 1 - 1.8.1: " + String(currentconsumptionHTkWh) + " kWh</p>";
  ptr +="<p>Tarif 2 - 1.8.2: " + String(currentconsumptionNTkWh) + " kWh</p>";
  
  ptr +="<h3>Einspeisung (A-)</h3>\n";
  ptr +="<p>Gesamt  - 2.8.0: " + String(deliveredOverallkWh) + " kWh</p>";
  ptr +="<p>Tarif 1 - 2.8.1: " + String(deliveryHTkWh) + " kWh</p>";
  ptr +="<p>Tarif 2 - 2.8.2: " + String(deliveryNTkWh) + " kWh</p>";

  ptr +="<h3>Aktuelle Gesamtwirkleistung (P+ - P-)</h3>\n";
  ptr +="<p>Gesamt - 16.7.0: " + String(powerAll) + " W</p>";
  ptr +="<p>Phase L1 - 36.7.0: " + String(powerL1) + " W</p>";
  ptr +="<p>Phase L2 - 56.7.0: " + String(powerL2) + " W</p>";
  ptr +="<p>Phase L3 - 76.7.0: " + String(powerL3) + " W</p>";

  if (showSmlMessage) {
    ptr +="<a class=\"button button-off\" href=\"/hide\">Hide RAW</a>\n";
    ptr +="<p>SML Message: ";
    for (unsigned int i = 0; i < sizeof(visibleMessage); i++)
    {
      char hexCar[2];
      sprintf(hexCar, "%02X", visibleMessage[i]);
      ptr += "0x";
      ptr += hexCar;
      ptr += ", ";
    }
    
    ptr += "</p>";
  }
  else {
    ptr +="<a class=\"button button-off\" href=\"/show\">Show RAW</a>\n";
  }

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML()); 
}

void handle_OnHide() {
  showSmlMessage = false;
  handle_OnConnect();
}

void handle_OnShow() {
  showSmlMessage = true;
  handle_OnConnect();
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup() {
  Serial.begin(BAUDRATE);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  WiFi.mode(WIFI_STA);
  WiFi.hostname("smart-meter");
  WiFi.begin(ssid, password);

  Serial.println("Connecting ..."); 
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }

  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer
  
  ArduinoOTA.setHostname("smart-meter");
  ArduinoOTA.setPassword("esp8266");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");

  if (MDNS.begin("smart-meter")) {
    Serial.println("MDNS responder started");
  }

  pinMode(led, OUTPUT);
  digitalWrite(led, 1);

  server.on("/", handle_OnConnect);
  server.on("/hide", handle_OnHide);
  server.on("/show", handle_OnShow);
  server.onNotFound(handleNotFound);
  
  server.begin();
}
void loop() {
  ArduinoOTA.handle();
  switch (state) {
  case 0:
  case 1: findStartSequence();
    break;
  case 2:
    float result1 = findConsumptionInSequence(encoder.ConsumptionOverallSequence, sizeof(encoder.ConsumptionOverallSequence) / sizeof(encoder.ConsumptionOverallSequence[0]));
    consumptionOverallkWh = result1;
    currentconsumptionHTkWh = findConsumptionInSequence(encoder.ConsumptionHTSequence, sizeof(encoder.ConsumptionHTSequence) / sizeof(encoder.ConsumptionHTSequence[0]));
    currentconsumptionNTkWh = findConsumptionInSequence(encoder.ConsumptionNTSequence, sizeof(encoder.ConsumptionNTSequence) / sizeof(encoder.ConsumptionNTSequence[0]));

    float result2 = findConsumptionInSequence(encoder.DeliveredOverallSequence, sizeof(encoder.DeliveredOverallSequence) / sizeof(encoder.DeliveredOverallSequence[0]));
    deliveredOverallkWh = result2;
    
    deliveryHTkWh = findConsumptionInSequence(encoder.DeliveredHTSequence, sizeof(encoder.DeliveredHTSequence) / sizeof(encoder.DeliveredHTSequence[0]));
    float result3 = findConsumptionInSequence(encoder.DeliveredNTSequence, sizeof(encoder.DeliveredNTSequence) / sizeof(encoder.DeliveredNTSequence[0]));
    deliveryNTkWh = !isnan(result3) ? result3 : deliveryNTkWh;
    powerAll = encoder.FindPowerInSequence(smlMessage, sizeof(smlMessage), encoder.PowerAllSequence, sizeof(encoder.PowerAllSequence));
    powerL1 = encoder.FindPowerInSequence(smlMessage, sizeof(smlMessage), encoder.PowerL1Sequence, sizeof(encoder.PowerL1Sequence));
    powerL2 = encoder.FindPowerInSequence(smlMessage, sizeof(smlMessage), encoder.PowerL2Sequence, sizeof(encoder.PowerL2Sequence));
    powerL3 = encoder.FindPowerInSequence(smlMessage, sizeof(smlMessage), encoder.PowerL3Sequence, sizeof(encoder.PowerL3Sequence));
    if (showSmlMessage) {
      memcpy(visibleMessage, smlMessage, sizeof(smlMessage));
    }
    for (unsigned int i = 0; i < sizeof(smlMessage); i++) {
      smlMessage[i] = 0x00;
    }
    state = 0;
    break;
  }

  server.handleClient();
}