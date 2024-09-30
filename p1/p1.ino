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

// Update these with values suitable for your network.

const char *ssid = "BOOTES3348";
const char *password = "625116A131551";
const char *mqttserver = "x22.ddns.net";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastmsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];
int value = 0;


// Motor global vaiables section 
// Motor Left
int motor1Pin1 = D0;
int motor1Pin2 = D1;
int enable1Pin = D2;
// Motor A
int motor2Pin1 = D3;
int motor2Pin2 = D4;
int enable2Pin = D5;

int left_encoder = D6;
int right_encoder = D7;

// Setting PWM properties
const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 50;
int pw = 150;
volatile int right_enc;
volatile int left_enc;
int totalleftcount = 0;
int totalrightcount = 0;
int state = 0;
int cmd[3];


//Motor functions
void left_forward(int pw)
{
	digitalWrite(motor1Pin1, LOW);
	digitalWrite(motor1Pin2, LOW);
	analogWrite(enable1Pin, 0);
	delay(2);
	digitalWrite(motor1Pin1, LOW);
	digitalWrite(motor1Pin2, HIGH);
	analogWrite(enable1Pin, pw);
}

void left_backward(int pw)
{
	digitalWrite(motor1Pin1, LOW);
	digitalWrite(motor1Pin2, LOW);
	analogWrite(enable1Pin, 0);
	delay(2);
	digitalWrite(motor1Pin1, HIGH);
	digitalWrite(motor1Pin2, LOW);
	analogWrite(enable1Pin, pw);
}

void left_stop(void)
{
	digitalWrite(motor1Pin1, LOW);
	digitalWrite(motor1Pin2, LOW);
	analogWrite(enable1Pin, 0);
}

void right_forward(int pw)
{
	digitalWrite(motor2Pin1, LOW);
	digitalWrite(motor2Pin2, LOW);
	analogWrite(enable2Pin, 0);
	delay(2);
	digitalWrite(motor2Pin1, LOW);
	digitalWrite(motor2Pin2, HIGH);
	analogWrite(enable2Pin, pw);
}

void right_backward(int pw)
{
	digitalWrite(motor2Pin1, LOW);
	digitalWrite(motor2Pin2, LOW);
	analogWrite(enable2Pin, 0);
	delay(2);
	digitalWrite(motor2Pin1, HIGH);
	digitalWrite(motor2Pin2, LOW);
	analogWrite(enable2Pin, pw);
}

void right_stop(void)
{
	digitalWrite(motor2Pin1, LOW);
	digitalWrite(motor2Pin2, LOW);
	analogWrite(enable2Pin, 0);
}


void diag_motor(void)
{
	left_forward(100);
	delay(500);
	left_backward(100);
	delay(500);
	left_stop();
	delay(1000);
	right_forward(100);
	delay(500);
	right_backward(100);
	delay(500);
	right_stop();
	delay(1000);

	left_forward(100);
	right_forward(100);
	delay(100);
	left_forward(20);
	right_forward(20);
	delay(2000);
	left_backward(100);
	right_backward(100);
	delay(2000);
	left_stop();
	right_stop();
	delay(1000);
}

int wait_left(int l)
{
	int j;
	j = 0;
	while(left_enc < l){
		j++;
		delay(1);
		if(j > 200){
			left_stop();
			right_stop();
			return(-1);
		}
	}
	return 0;
}

int wait_right(int l)
{
	int j;
	j = 0;
	while(right_enc < l){
		j++;
		delay(1);
		if(j > 200){
			left_stop();
			right_stop();
			return(-1);
		}
	}
	return 0;
}

int forward(int p){
	int j;
	for (int i = 0; i < p; i++) {
		left_enc = 0;
		right_enc = 0;
		left_forward(100);
		right_forward(100);
		if(wait_left(1) == -1) return -1;
		left_stop();
		right_stop();
		if (left_enc < right_enc) {
			left_forward(100);
			if(wait_left(right_enc) == -1) return -1;
			left_stop();
		} else if (right_enc < left_enc) {
			right_forward(100);
			if(wait_right(left_enc) == -1) return -1;
			right_stop();
		}
		totalleftcount = totalleftcount + left_enc;
		totalrightcount = totalrightcount + right_enc;
		client.loop();
		if(cmd[0] == 10) break;
	}
	return(1);
}

int backward(int p){
	int j;
	for (int i = 0; i < p; i++) {
		left_enc = 0;
		right_enc = 0;
		left_backward(100);
		right_backward(100);
		if(wait_left(1) == -1) return -1;
		left_stop();
		right_stop();
		if (left_enc < right_enc) {
			left_backward(100);
			if(wait_left(right_enc) == -1) return -1;
			left_stop();
		} else if (right_enc < left_enc) {
			right_backward(100);
			if(wait_right(left_enc) == -1) return -1;
			right_stop();
		}
		totalleftcount = totalleftcount - left_enc;
		totalrightcount = totalrightcount - right_enc;
		client.loop();
		if(cmd[0] == 10) break;
	}
	return(1);
}

int left_turn(int p)
{
	int j;
	for (int i = 0; i < p; i++) {
		left_enc = 0;
		right_enc = 0;
		left_backward(100);
		right_forward(100);
		if(wait_left(1) == -1) return -1;
		left_stop();
		right_stop();
		if (left_enc < right_enc) {
			left_backward(100);
			if(wait_left(right_enc) == -1) return -1;
			left_stop();
		} else if (right_enc < left_enc) {
			right_forward(100);
			if(wait_right(left_enc) == -1) return -1;
			right_stop();
		}
		totalleftcount = totalleftcount - left_enc;
		totalrightcount = totalrightcount + right_enc;
		client.loop();
	}
	return(1);
}

int right_turn(int p)
{
	int j;
	for (int i = 0; i < p; i++) {
		left_enc = 0;
		right_enc = 0;
		left_forward(100);
		right_backward(100);
		if(wait_left(1) == -1) return -1;
		left_stop();
		right_stop();
		if (left_enc < right_enc) {
			left_forward(100);
			if(wait_left(right_enc) == -1) return -1;
			while (left_enc < right_enc);
			left_stop();
		} else if (right_enc < left_enc) {
			right_backward(100);
			if(wait_right(left_enc) == -1) return -1;
			right_stop();
		}
		totalleftcount = totalleftcount + left_enc;
		totalrightcount = totalrightcount - right_enc;
		client.loop();
	}
	return(1);
}


void ICACHE_RAM_ATTR lisr()
{
	left_enc++;
}

void ICACHE_RAM_ATTR risr()
{
	right_enc++;
}

void setupwifi()
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
	char msg[512];
	int p;

	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char) payload[i]);
	}
	Serial.println();
	snprintf(msg, length + 1, "%s", payload);

	if(strcmp(topic, "motor/command/forward") == 0){
		cmd[0] = 1;
		cmd[1] = atoi(msg);
		cmd[2] = 0;
	} else if(strcmp(topic, "motor/command/left_turn") == 0){
		cmd[0] = 2;
		cmd[1] = atoi(msg);
		cmd[2] = 0;
	} else if(strcmp(topic, "motor/command/right_turn") == 0){
		cmd[0] = 3;
		cmd[1] = atoi(msg);
		cmd[2] = 0;
	} else if(strcmp(topic, "motor/command/backward") == 0){
		cmd[0] = 4;
		cmd[1] = atoi(msg);
		cmd[2] = 0;
	} else if(strcmp(topic, "motor/command/break") == 0){
		cmd[0] = 10;
		cmd[1] = 0;
		cmd[2] = 0;
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
			client.subscribe("motor/command/forward");
			client.subscribe("motor/command/backward");
			client.subscribe("motor/command/left_turn");
			client.subscribe("motor/command/right_turn");
			client.subscribe("motor/command/break");
		} else {
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 5 seconds");
			// Wait 5 seconds before retrying
			delay(5000);
		}
	}
}

void setup()
{
	Serial.begin(115200);
	setupwifi();
	client.setServer(mqttserver, 9338);
	client.setCallback(callback);

	//Motor setup
	pinMode(motor1Pin1, OUTPUT);
	pinMode(motor1Pin2, OUTPUT);
	pinMode(enable1Pin, OUTPUT);
	pinMode(motor2Pin1, OUTPUT);
	pinMode(motor2Pin2, OUTPUT);
	pinMode(enable2Pin, OUTPUT);
	pinMode(left_encoder, INPUT);
	pinMode(right_encoder, INPUT);
	analogWriteFreq(200);
	attachInterrupt(left_encoder, lisr, FALLING);
	attachInterrupt(right_encoder, risr, FALLING);
	diag_motor();
	state = 1;
}

void loop()
{

	if (!client.connected()) {
		reconnect();
	}
	client.loop();

	switch(cmd[0]){
	case 1:
		state = forward(cmd[1]);
		cmd[0] = 0;
		break;
	case 2:
		state = left_turn(cmd[1]);
		cmd[0] = 0;
		break;
	case 3:
		state = right_turn(cmd[1]);
		cmd[0] = 0;
		break;
	case 4:
		state = backward(cmd[1]);
		cmd[0] = 0;
		break;
	}

	unsigned long now = millis();
	if (now - lastmsg > 2000) {
		lastmsg = now;
		++value;
		snprintf(msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
		Serial.print("Publish message: ");
		Serial.println(msg);
		client.publish("outTopic", msg);
		snprintf(msg, MSG_BUFFER_SIZE, "%ld", totalleftcount);
		client.publish("motor/status/left_encoder", msg);
		snprintf(msg, MSG_BUFFER_SIZE, "%ld", totalrightcount);
		client.publish("motor/status/right_encoder", msg);
		snprintf(msg, MSG_BUFFER_SIZE, "state: %ld", state);
		client.publish("motor/status", msg);
	}
}
