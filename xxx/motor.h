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
