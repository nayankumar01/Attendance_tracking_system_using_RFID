#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecureBearSSL.h>

#define RST_PIN  D3
#define SS_PIN   D4
#define BUZZER   D2

MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;  
MFRC522::StatusCode status;      
/* Be aware of Sector Trailer Blocks */
int blockNum = 2;  
/* Create another array to read data from Block */
/* Legthn of buffer should be 2 Bytes more than the size of Block (16 Bytes) */
byte bufferLen = 18;
byte readBlockData[18];

String card_holder_name;
const String sheet_url = "https://script.google.com/macros/s/AKfycbwbVDUWwRvt8Xgsllvj2DA0JYfxxpaV02_xniRsq8tMwoVAT-fwqyHkrVfXKIPfuvxB/exec?name=";

// Fingerprint for demo URL, expires on ‎Monday, ‎May ‎2, ‎2022 7:20:58 AM, needs to be updated well before this date
const uint8_t fingerprint[20] = {0x5d, 0xba, 0x19, 0xa5, 0x81, 0x25, 0x05, 0x32, 0x72, 0x13, 0xa7, 0x18, 0xcf, 0x67, 0x8c, 0xd6, 0x6f, 0x13, 0xb3, 0xef};
//5D BA 19 A5 81 25 05 32 72 13 A7 18 CF 67 8C D6 6F 13 B3 EF
#define WIFI_SSID "kamalnayan"
#define WIFI_PASSWORD "kamalnayan"

// setup() function

void setup()
{

  /* Initialize serial communications with the PC */
  Serial.begin(9600);
  //Serial.setDebugOutput(true);
  //WiFi Connectivity
  Serial.println();
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Set BUZZER as OUTPUT */
  pinMode(BUZZER, OUTPUT);

  /* Initialize SPI bus */
  SPI.begin();

}



// loop() function

 void loop()
{
 /* Initialize MFRC522 Module */
  mfrc522.PCD_Init();
  /* Look for new cards */
  /* Reset the loop if no new card is present on RC522 Reader */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {return;}
  /* Select one of the cards */
  if ( ! mfrc522.PICC_ReadCardSerial()) {return;}
  /* Read data from the same block */
  Serial.println();
  Serial.println(F("Reading last data from RFID..."));
  ReadDataFromBlock(blockNum, readBlockData);
  /* If you want to print the full memory dump, uncomment the next line */
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  
  /* Print the data read from block */
  Serial.println();
  Serial.print(F("Last data in RFID:"));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  for (int j=0 ; j<16 ; j++)
  {
    Serial.write(readBlockData[j]);
  }
  Serial.println();

  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);
  delay(200);
  digitalWrite(BUZZER, HIGH);
  delay(200);
  digitalWrite(BUZZER, LOW);

 if (WiFi.status() == WL_CONNECTED) {
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setFingerprint(fingerprint);
    // Or, if you want to ignore the SSL certificate
    //then use the following line instead:
    // client->setInsecure();
    card_holder_name = sheet_url + String((char*)readBlockData);
    card_holder_name.trim();
    Serial.println(card_holder_name);
    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));
   
     if (https.begin(*client, (String)card_holder_name)){
      // HTTP
      Serial.print(F("[HTTPS] GET...\n"));
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // file found at server
      }
     else 
      {Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());}
      https.end();
      delay(1000);
    }
    else {
      Serial.printf("[HTTPS} Unable to connect\n");
    }
 }
}

 //ReadDataFromBlock() function
void ReadDataFromBlock(int blockNum, byte readBlockData[]) 
{ 
 /* Prepare the ksy for authentication */
  /* All keys are set to FFFFFFFFFFFFh at chip delivery from the factory */
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  /* Authenticating the desired data block for Read access using Key A */
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK){
     Serial.print("Authentication failed for Read: ");
     Serial.println(mfrc522.GetStatusCodeName(status));
     return;
  }
 else {
    Serial.println("Authentication success");
  }
  /* Reading data from the Block */
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &bufferLen);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }
 else {
    Serial.println("Block was read successfully");  
  }
}