/*
 * Description: test ifttt for sending sms, gmail and updating gsheets 
 *
 * The following code tests the transmission of smart trap data from the trap
 * to the ESP where (using WIFI transmission in this test) it formats the data 
 * into a google sheets csv format ("|||" - as the seperator) and then uses IFTTT
 * to the Google sheet which is scanned every 15 minutes by google data studio
 * to unpdate the NZ map that shows where traps have been triggered
 * this code is a test harness to verify that this integration and data transmission works.
 * The code also sends and SMS and gmail to notify the trap has been triggered
 * For all notifications sufficient trap detail is provided to find the trap location
 * and reset it.
 * NOTE: the test does note use the wrap around encryption logic for the data in transit. 
 *       This logic is present in the final simulator code.
 *
 * Voltage: N/A
 * Sampling rate: N/A
 * Pinout: N/A
 * 
 * Test Data:
 * 1. SMS: <mobile number> 100 DOC150
 * 2. Gmail: 100 DOC150
 * 3. Gsheets: 100 ||| DOC150 ||| Trap-100 Predator-Stoat ||| 10 Aug 2021 ||| 10:30 ||| Stoat ||| - ||| 10 ||| 50 ||| 110 ||| -41.110011 ||| 174.910000
 * NOTE: use ||| to seperate data rows added to Google sheets. 
 * NOTE: the data sent to the google sheet includes date, time, geo location, temperature, humidity, altitude above sealevel 
 * 
 */

#include <WiFi.h>  

static const int BAUD_SERIAL = 115200; 
const char* ssid     = "<WIFI-SSID>";
const char* password = "<WIFIPASSWORD>";
const char* resourceSms = "/trigger/SmartMonitorNotificationSMS/with/key/d3ocpfgsuH0h-7gfaf9u5e"; // ifttt applet for sms
const char* resourceGmail = "/trigger/SmartMonitorNotificationGmail/with/key/o5MHus1uEz07bJJLVJPZKq_IaE11p_ZnZ1sTT0RxZvg"; // ifttt applet for gmail
const char* resourceGSheet = "/trigger/TrapSmartMonitorData/with/key/d3ocpfgsuH0h-7gfaf9u5e"; // ifttt for gsheet 
String jsonObjectSms, jsonObjectGmail, jsonObjectGSheet;
const char* server = "maker.ifttt.com"; // ifttt webhooks url to run Json and applet
String smsMobileNumber = "021XXXXXXXXX";  // sms mobile number

/* debugging switches */
boolean Debug0 = true;  /* error - failures*/
boolean Debug1 = true;  /* Information - progress*/

void setup() {
  // configure ifttt request data
  jsonObjectSms = String("{\"value1\":\"") + smsMobileNumber + "\",\"value2\":\"" + "100" + "\",\"value3\":\"" + "DOC150" + "\"}";
  jsonObjectGmail = String("{\"value1\":\"") + "100" + "\",\"value2\":\"" + "DOC150" + "\"}";
  // NOTE: use ||| to seperate data fields sent to Google sheets 
  jsonObjectGSheet = String("{\"value1\":\"") + "100 ||| DOC150 ||| Trap-100 Predator-Stoat ||| 10 Aug 2021 ||| 10:30 ||| Stoat ||| - ||| 10 ||| 50 ||| 110 ||| -41.110011 ||| 174.910000" + "\"}";
  
  // start wifi for serial 
  Serial.begin(BAUD_SERIAL);          
  if (Debug1)Serial.print("Connecting to " + String(ssid));              
  WiFi.begin(ssid, password);          
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (Debug1)Serial.print(".");
  }
  if (Debug1)Serial.println("\nWiFi connected.");

  /* now send the google sheet, sms and gmail notification that the trap has been activated */
  makeIFTTTRequest(resourceSms, jsonObjectSms); 
  delay(1000); // notify via sms 
  makeIFTTTRequest(resourceGmail, jsonObjectGmail); 
  delay(1000); // notify via gmail
  makeIFTTTRequest(resourceGSheet, jsonObjectGSheet); // add row to Google Sheet
}

void loop() {
// do nothing - this is only a test harness all work done in setup()
}

/* */
void makeIFTTTRequest(const char* resource, String jsonObject) {
  // init wifi client for ifttt server
  if (Debug1)
    Serial.print("Connecting to..." + String(server));
  WiFiClient client;  
  
  // send ifttt request to server
  int retries = 5;
  while(!!!client.connect(server, 80) && (retries-- > 0)) {
    if (Debug1)Serial.print(".");
  }
  if (Debug1)
    Serial.println();
  if(!!!client.connected()) {
    if (Debug0)
      Serial.println("Failed to connect...");
  }
  
  if (Debug1)
    Serial.println("json: " + String(jsonObject));
  
  if (Debug1)
    Serial.println("resource: " + String(resource));
  
  // write the jason header to send to IFTTT
  client.println(String("POST ") + resource + " HTTP/1.1");
  client.println(String("Host: ") + server); 
  client.println("Connection: close\r\nContent-Type: application/json");
  client.print("Content-Length: ");
  client.println(jsonObject.length()); 
  client.println();
  client.println(jsonObject);
    
  // check the response for success
  int timeout = 5 * 10; // 5 seconds             
  while(!!!client.available() && (timeout-- > 0)){
    delay(100);
  }
  // client is not availble so got no reponse
  if(!!!client.available()){
    if (Debug0)
      Serial.println("No response...");
  }
  
  // if there is a client write the package to it
  while(client.available()){
    Serial.write(client.read());
  }
      
  if (Debug1)
    Serial.println("\nclosing connection");
  
  // closing the wifi connection before leaving
  client.stop(); 
}
