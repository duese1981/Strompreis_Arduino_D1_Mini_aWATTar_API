#include <ArduinoJson.h>

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

HTTPClient sender;
WiFiClientSecure wifiClient;


#define USE_SERIAL Serial



char ssid[] = "ssid";
char pass[] = "password";


void setup() {
  USE_SERIAL.begin(9600);




  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    USE_SERIAL.println(".");
    delay(500);
  }

  USE_SERIAL.println("");
  USE_SERIAL.println("Wi-Fi connected");
  USE_SERIAL.print("IP Address : ");
  USE_SERIAL.println(WiFi.localIP());



}

void loop() {

 int marketprice [23]; //Array für den eingelesenen Marktpreis
 int mpmin[2][2]; // Kleinste Werte im Array + Zeilennummer
 int znr; // Loop Zeilennummer



  wifiClient.setInsecure();
  if (sender.begin(wifiClient, "https://api.awattar.at/v1/marketdata")) {
    // HTTP-Code der Response speichern
    int httpCode = sender.GET();
    if (httpCode > 0) {
      // Anfrage wurde gesendet und Server hat geantwortet
      // Info: Der HTTP-Code für 'OK' ist 200
      if (httpCode == 200) {
        // Hier wurden die Daten vom Server empfangen
        // String vom Webseiteninhalt speichern
        String payload = sender.getString();


        // Stream& input;

        DynamicJsonDocument doc(3072);

        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
          return;
        }

        const char* object = doc["object"]; // "list"

        int i = 0;

        for (JsonObject data_item : doc["data"].as<JsonArray>()) {

          long long data_item_start_timestamp = data_item["start_timestamp"]; // 1672700400000, 1672704000000, ...
          long long data_item_end_timestamp = data_item["end_timestamp"]; // 1672704000000, 1672707600000, ...
          float data_item_marketprice = data_item["marketprice"]; // 130.01, 120, 118.76, 115, 113.63, 116.27, ...
          const char* data_item_unit = data_item["unit"]; // "Eur/MWh", "Eur/MWh", "Eur/MWh", "Eur/MWh", ...

         
          marketprice[i] = data_item_marketprice;

          Serial.print (i);
          Serial.print (":");
          Serial.println (marketprice[i]);
          i++;

        }

        const char* url = doc["url"]; // "/at/v1/marketdata"



      } else {
        // Falls HTTP-Error
        Serial.print("HTTP-Error: " +  String(httpCode));
      }
    }
    // Wenn alles abgeschlossen ist, wird die Verbindung wieder beendet
    sender.end();
  } else {
    Serial.printf("HTTP-Verbindung konnte nicht hergestellt werden!");
  }


// Findet die guenstigsten Stundentarife im Array marketprice
if(marketprice[0] < marketprice[1]) {
      mpmin[0][0] = marketprice[0];
      mpmin[0][1] = 0;
      mpmin[1][0]  = marketprice[1];
      mpmin[1][1] = 1;
   } else {
      mpmin[0][0] = marketprice[1];
      mpmin[0][1] = 1;
      mpmin[1][0]  = marketprice[0];
      mpmin[1][1] = 0;
   }

   for(znr = 2; znr < 23; znr++) { //znr 2 ist eigentlich die dritte Zeile des Array, da bei 0 begonnen wird zu zählen
      if( mpmin[0][0] > marketprice[znr] ) {
         mpmin[1][0] = mpmin[0][0];
         mpmin[0][0] = marketprice[znr];
         mpmin[0][1] = znr;
      } else if( mpmin[1][0] > marketprice[znr] ) {
         mpmin[1][0] =  marketprice[znr];
         mpmin[1][1] = znr;
      }
   }

Serial.print ("1 guenstigster Preis in Zeile: ");
Serial.print (mpmin[0][1]);
Serial.print (" mit ");
Serial.println (mpmin[0][0]);



Serial.print ("2 guenstigster Preis in Zeile: ");
Serial.print (mpmin[1][1]);
Serial.print (" mit ");
Serial.println (mpmin[1][0]);



delay(100000);
}
