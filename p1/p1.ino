/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete project details at https://RandomNerdTutorials.com/esp32-dc-motor-l298n-motor-driver-control-speed-direction/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.  
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*********/
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
int right_enc;
int left_enc;

void left_forward (int pw) {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW); 
  analogWrite(enable1Pin, 0);
  delay(2);
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, HIGH); 
  analogWrite(enable1Pin, pw);
}

void left_backward (int pw) {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW); 
  analogWrite(enable1Pin, 0);
  delay(2);
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW); 
  analogWrite(enable1Pin, pw);
}

void left_stop (void) {
  digitalWrite(motor1Pin1, LOW);
  digitalWrite(motor1Pin2, LOW); 
  analogWrite(enable1Pin, 0);
}

void right_forward (int pw) {
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW); 
  analogWrite(enable2Pin, 0);
  delay(2);
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, HIGH); 
  analogWrite(enable2Pin, pw);
}

void right_backward (int pw) {
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW); 
  analogWrite(enable2Pin, 0);
  delay(2);
  digitalWrite(motor2Pin1, HIGH);
  digitalWrite(motor2Pin2, LOW); 
  analogWrite(enable2Pin, pw);
}

void right_stop (void) {
  digitalWrite(motor2Pin1, LOW);
  digitalWrite(motor2Pin2, LOW); 
  analogWrite(enable2Pin, 0);
}


void diag_motor(void) {
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

void ICACHE_RAM_ATTR lisr() {
   left_enc++;
}

void ICACHE_RAM_ATTR risr() {
   right_enc++;
}

void setup() {
  // sets the pins as outputs:
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
  
  // configure LEDC PWM
//  ledcAttachChannel(enable1Pin, freq, resolution, pwmChannel);

  Serial.begin(115200);

  // testing
  Serial.print("Testing DC Motor...");
}

void loop() {
  Serial.print("test test\n");
  diag_motor();
  Serial.print(left_enc);
  Serial.print(" ");
  Serial.print(right_enc);
}

