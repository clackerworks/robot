/*
 Basic ESP8266 MQTT example
 This sketch demonstrates the capabilities of the pubsub library in combination
 with the ESP8266 board/library.
 It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off
 It will reconnect to the server if the connection is lost using a blocking
 reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
 achieve the same result without blocking the main loop.
 To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <VL53L0X.h>

// Uncomment this line to use long range mode. This
// increases the sensitivity of the sensor and extends its
// potential range, but increases the likelihood of getting
// an inaccurate reading because of reflections from objects
// other than the intended target. It works best in dark
// conditions.

//#define LONG_RANGE


// Uncomment ONE of these two lines to get
// - higher speed at the cost of lower accuracy OR
// - higher accuracy at the cost of lower speed

//#define HIGH_SPEED
#define HIGH_ACCURACY


VL53L0X sensor;

// Update these with values suitable for your network.

const char *ssid = "BOOTES3348";
const char *password = "625116A131551";
const char *mqtt_server = "x22.ddns.net";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi()
{

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char *topic, byte * payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
	Serial.print((char) payload[i]);
    }
    Serial.println();

    // Switch on the LED if an 1 was received as first character
    if ((char) payload[0] == '1') {
	digitalWrite(BUILTIN_LED, LOW);	// Turn the LED on (Note that LOW is the voltage level
	// but actually the LED is on; this is because
	// it is active low on the ESP-01)
    } else {
	digitalWrite(BUILTIN_LED, HIGH);	// Turn the LED off by making the voltage HIGH
    }

}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected()) {
	Serial.print("Attempting MQTT connection...");
	// Create a random client ID
	String clientId = "ESP8266Client-";
	clientId += String(random(0xffff), HEX);
	// Attempt to connect
	if (client.connect(clientId.c_str(), "testuser", "testuser")) {
	    Serial.println("connected");
	    // Once connected, publish an announcement...
	    client.publish("outTopic", "hello world");
	    // ... and resubscribe
	    client.subscribe("inTopic");
	} else {
	    Serial.print("failed, rc=");
	    Serial.print(client.state());
	    Serial.println(" try again in 5 seconds");
	    // Wait 5 seconds before retrying
	    delay(5000);
	}
    }
}

void listnetworks()
{
    String ssid;
    unsigned char *b;
    char bssid[32];
    float rssi;
    char t[512];
    // scan for nearby networks:
//  Serial.println("** Scan Networks **");
    int numSsid = WiFi.scanNetworks();
    if (numSsid == -1) {
	Serial.println("Couldn't get a WiFi connection");
    }
    // print the list of networks seen:
//  Serial.print("number of available networks:");
//  Serial.println(numSsid);

    // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
//    Serial.print(thisNet);
//    Serial.print(") ");
//    Serial.print(WiFi.SSID(thisNet));
//    Serial.print("\tSignal: ");
//    Serial.print(WiFi.RSSI(thisNet));
//    Serial.print(" dBm");
//    Serial.print("\tEncryption: ");
//    printEncryptionType(WiFi.encryptionType(thisNet));
	ssid = WiFi.SSID(thisNet);
	b = WiFi.BSSID(thisNet);
	snprintf(bssid, 32, "%02x:%02x:%02x:%02x:%02x:%02x", b[0], b[1],
		 b[2], b[3], b[4], b[5]);
	rssi = WiFi.RSSI(thisNet);
	snprintf(msg, MSG_BUFFER_SIZE, "%s %f", bssid, rssi);
	delay(5);
	snprintf(t, 512, "sensor/rssi/%s", bssid);
	snprintf(msg, MSG_BUFFER_SIZE, "%f", rssi);
	client.publish(t, msg);
    }
}


void setup()
{
//  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 9338);
    client.setCallback(callback);

    Wire.begin();

    sensor.setTimeout(500);
    if (!sensor.init()) {
	Serial.println("Failed to detect and initialize sensor!");
    }
#if defined LONG_RANGE
    // lower the return signal rate limit (default is 0.25 MCPS)
    sensor.setSignalRateLimit(0.1);
    // increase laser pulse periods (defaults are 14 and 10 PCLKs)
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, 18);
    sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, 14);
#endif

#if defined HIGH_SPEED
    // reduce timing budget to 20 ms (default is about 33 ms)
    sensor.setMeasurementTimingBudget(20000);
#elif defined HIGH_ACCURACY
    // increase timing budget to 200 ms
    sensor.setMeasurementTimingBudget(200000);
#endif
}

void loop()
{
    int range;
    float rssi_bootes;

    if (!client.connected()) {
	reconnect();
    }
    client.loop();

    unsigned long now = millis();
    if (now - lastMsg > 5000) {
	lastMsg = now;
	++value;
	range = sensor.readRangeSingleMillimeters();
	if (sensor.timeoutOccurred())
	    range = -1;
	snprintf(msg, MSG_BUFFER_SIZE, "%ld", range);
	client.publish("sensor/proximity/distance/front", msg);
	snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
//    Serial.print("Publish message: ");
//    Serial.println(msg);
	client.publish("outTopic", msg);
//      rssi_bootes = WiFi.RSSI();
//      snprintf (msg, MSG_BUFFER_SIZE, "rssi bootes: %f", rssi_bootes);
//      client.publish("outTopic", msg);
	listnetworks();
    }
}
