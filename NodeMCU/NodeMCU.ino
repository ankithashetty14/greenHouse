
#include <ESP8266WiFi.h> // Include the WiFi library
#include <ESP8266WiFiMulti.h> // Include the WiFi-Multi library
#include <ESP8266HTTPClient.h> 

#define ssid_1 "ssid"    // your Wifi network 1 SSID (name) 
#define pass_1 "****"  // your Wifi network 1 password

#define ssid_2 "" // your Wifi network 2 SSID (name) 
#define pass_2 ""  // your Wifi network 2 password

#define ssid_3 "" // open wiFi network

#define TSWriteAPIKey "asdfghvcsdfb"  //SECRET_WRITE_APIKEY;
#define TSField_1 1 // Enter Field Number
#define TSField_2 2 
#define TSField_3 3  
#define TSField_4 4

float sensor_1 = 0;
float sensor_2 = 0;
float sensor_3 = 0;
String message = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

WiFiClient  client;
ESP8266WiFiMulti wifiMulti; // Create an instance of the ESP8266WiFiMulti class, called WifiMulti

void serialevent()
{
  while (Serial.available())
  {
    char inChar = Serial.read();
    message += inChar;
    if (inChar == '\n') 
    {
      stringComplete = true;
    }
  }
}

void ConnectToWiFi(void){
  wifiMulti.addAP(ssid_1,pass_1); //  add WiFi networks you want to connect to
  wifiMulti.addAP(ssid_2,pass_2);
  wifiMulti.addAP(ssid_3);
  
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("\nAttempting to connect to WiFi\n");
    while(wifiMulti.run() != WL_CONNECTED){ // Wait for thr WiFi to connect: scan for WiFi networss, and connect to the strongest of the networks above
      Serial.println("Connecting...");
      delay(1000);     
    } 
    Serial.print("\nConnected to " + String(WiFi.SSID()) + "\n"); // Tell us what network we are connected to 
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP()); // Send the IP address of the ESP8266 to the computer
  }
}

void Write_ThingSpeak_Data(String Value_1, String Value_2, String Value_3, String Value_4){
  if (WiFi.status() != WL_CONNECTED) { //Check WiFi connection status
    Serial.println("\nWiFi Disconected ?!!");
    ConnectToWiFi();
  }
  HTTPClient http;  //Declare an object of class HTTPClient
  String upLink = "http://api.thingspeak.com/update?api_key="+String(TSWriteAPIKey)+"&field"+String(TSField_1)+"=";
  upLink += String(Value_1);  //update field 1 value
  upLink += "&field"+String(TSField_2)+"="+String(Value_2);  // update field 2 value
  upLink += "&field"+String(TSField_3)+"="+String(Value_3); // update field 3 value
  //Serial.println(upLink);
  http.begin(upLink);  //Specify request destination
  int httpCode = http.GET();  //Send the GET request
  http.end();   //Close connection
}

void setup() 
{
  Serial.begin(115200);
  message.reserve(30);
  WiFi.mode(WIFI_STA); // set-up a WPA2-PSK network
  ConnectToWiFi();
}

void loop() 
{
  serialevent();
  
  if(stringComplete)
  {
    if(message.substring(0,4)=="Temp")
      sensor_1 = ((message.substring(5)).toFloat());
    if(message.substring(0,4)=="Humi")
      sensor_2 = ((message.substring(5)).toFloat());
    if(message.substring(0,4)=="Mois")
      sensor_3 = ((message.substring(5)).toFloat());
    if(message.substring(0,4)=="Lite")
      sensor_4 = ((message.substring(5)).toFloat());
  }
  
   Write_ThingSpeak_Data(String(sensor_1), String(sensor_2), String(sensor_3), String(sensor_4));
   //Write_ThingSpeak_Data(String(random(0, 9) * 10), String(random(0, 9) * 10), String(random(0, 9) * 10), String(random(0, 9) * 10));
   delay(1000);
}
