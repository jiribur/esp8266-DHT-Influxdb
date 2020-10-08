// Credits: http://cactusprojects.com/esp8266-logging-to-influxdb/ and https://github.com/G6EJD

// Pozn. WeMos D1 R1, EspBoards 2.7.4.
// Nastudovat nejaky sleep ...

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <DHT.h>
#include <InfluxDb.h>

#define DHTPIN 2                          // what digital pin we're connected to
//#define DHTTYPE DHT11                   // DHT 11
#define DHTTYPE DHT22                     // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21                   // DHT 21 (AM2301)
#define WIFI_SSID "xxxxxx"               //Enter SSID of your WIFI Access Point
#define WIFI_PASS "xxxxxxx"        //Enter Password of your WIFI Access Point
#define INFLUXDB_HOST "xxx.xxx.xxx.xxx"     //Enter IP of device running Influx Database
#define INFLUXDB_DATABASENAME "xnejakexx"   //Influxdb name, musí se nejprve vytovorit
#define INFLUXDB_DEVICENAME "BibinaBunny" //Friendly device name ... pokud jich bude casem vice
float TKorekce = -3.9;                    // Korekce meho teplotniho snimace protoze nejak dodelava

DHT dht(DHTPIN, DHTTYPE);                  // Initialize DHT sensor.
ESP8266WiFiMulti WiFiMulti;                // Initialize Wifi.
Influxdb influx(INFLUXDB_HOST);           // Influx host.

void setup() {
  Serial.begin(9600);
  delay(1000);                            // Jinak mi to nechtelo psat na zacatku
  Serial.println("Zkouska nafunet teplotu do domaciho rapi s influxem");
  
  dht.begin();
  
  WiFiMulti.addAP(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WIFI ...");
  while (WiFiMulti.run() != WL_CONNECTED)
    {
    Serial.print(".");
    delay(100);
    }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  influx.setDb(INFLUXDB_DATABASENAME); // Jmeno Influx databaze kam se budou sypat data.
  
  Serial.println("Setup Complete.");
  
}

int loopCount = 0;
void loop() {
  loopCount++;
  
  float h = dht.readHumidity();  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();  // Read temperature as Celsius (the default)
  float f = dht.readTemperature(true); // Read temperature as Fahrenheit (isFahrenheit = true)

   if (isnan(h) || isnan(t) || isnan(f))  // Check if any reads failed and exit early (to try again).
  
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  t=t+TKorekce;  // Uprava teploty čidla.
 
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
 
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  //Serial.print(f);
  //Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  //Serial.print(hif);
  //Serial.print(" *F ");
  Serial.print("Mereni cislo: ");
  Serial.println(loopCount);
  
 InfluxData row("data");   // Prepare row for InfluxDB
  row.addTag("Device", INFLUXDB_DEVICENAME);
  row.addValue("Humidity", h); 
  row.addValue("Temperature", t); 
  // row.addTag("Unit", "Celsius");
  row.addValue("HeatIndex", hic); 
  // row.addValue("LoopCount", loopCount);  
  // row.addValue("RandomValue", random(10, 40));   // jen pro testovani
 influx.write(row);     // Write to InfluxDB
  
delay(600000); // Wait milliseconds between measurements, minimalne 5000 ... Reading temperature or humidity takes about 250 milliseconds!

}
