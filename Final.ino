/*
 *  Name: Nathaniel Beuck 
 *  Assignment: Final Project
 *  Class: CPE301 Spring 2024
 *  Date: 5/12/24
 */

#include <LiquidCrystal.h>
#include <Stepper.h>
#include <uRTCLib.h>
#include <dht.h>

#define WATER_SIGNAL A1

uRTCLib rtc(0x68);

dht DHT;

const int RS = 12, EN = 11, D4 = 2, D5 = 3, D6 = 4, D7 = 5;
const int fanPin = 6;
const int open = 10, close = 13, IN1 = 46, IN2 = 48, IN3 = 50, IN4 = 52;
const int stop = 32, start = 34, reset = 30;
const int TH = 22;
const int RL = 38, BL = 40, YL = 42, GL = 44;

const int stepsPerRevolution = 2038;

int ventState = 0;

int i = 0;

int coolerState = 0; // 0 = Disabled, 1 = Idle, 2 = Error, 3 = Running

int buttonStateStart = 0; 
int buttonStateStop = 0;
int buttonStateReset = 0; 

LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
Stepper vent = Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(3000);

  pinMode(RL, OUTPUT);
  pinMode(BL, OUTPUT);
  pinMode(YL, OUTPUT);
  pinMode(GL, OUTPUT);
  pinMode(fanPin, OUTPUT);
  pinMode(open, INPUT);
  pinMode(close, INPUT);
  pinMode(start, INPUT);
  pinMode(stop, INPUT);
  pinMode(reset, INPUT);

  URTCLIB_WIRE.begin();
  rtc.set(0, 0, 0, 0, 0, 0, 0);

  lcd.begin(16, 2);
}


void loop() {
  ventControl();
  rtc.refresh();

  if(coolerState != 3){
    digitalWrite(fanPin, LOW);
  }

  if(coolerState == 0){
    lcd.clear();
    DisabledState();
  }
  if(coolerState == 1){
    IdleState();
  }
  if(coolerState == 2){
    ErrorState();
  }
  if(coolerState == 3){
    digitalWrite(fanPin, HIGH);
    RunningState();
  }

  delay(1000);
}

void ventControl(){
  int buttonStateOpen = digitalRead(open);
  int buttonStateClose = digitalRead(close);
  if(buttonStateOpen==HIGH && ventState == 0){
    vent.setSpeed(10);
    vent.step(stepsPerRevolution/4);
    ventState = 1;
  }
  if(buttonStateClose==HIGH && ventState == 1){
    vent.setSpeed(10);
    vent.step(-stepsPerRevolution/4);
    ventState = 0;
  }
}

void waterSensor(){
  int anValue = 0;
  anValue = analogRead(WATER_SIGNAL);

  int waterValue = anValue;

  //Serial.println("Water Level: ");
  //Serial.print(anValue);
  //Serial.print(" | ");
  //Serial.print(waterValue);
  //Serial.println("");

  if(waterValue <= 200){
    coolerState = 2;
    Serial.println("Error Transition: ");
    clockReport();
    digitalWrite(GL, LOW);
    digitalWrite(BL, LOW);
  }
}

void clockReport(){
  Serial.print(rtc.hour());
  Serial.print(':');
  Serial.print(rtc.minute());
  Serial.print(':');
  Serial.print(rtc.second());
  Serial.println("");
}

void THSensor(){
  int data = DHT.read11(22);

  float t = DHT.temperature;
  float h = DHT.humidity;

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp:  ");
  lcd.print(t);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humid: ");
  lcd.print(h);
  lcd.print("%");

  if(t <= 10 && coolerState == 3){
    coolerState = 1;
    digitalWrite(BL, LOW);
    Serial.println("Idle Transition: ");
    clockReport();
  }
  if(t >= 24 && coolerState == 1){
    coolerState = 3;
    digitalWrite(GL, LOW);
    Serial.println("Running Transition: ");
    clockReport();
  }
}

void DisabledState(){
  digitalWrite(YL, HIGH);

  int buttonStateStart = digitalRead(start);
  if(buttonStateStart==HIGH){
    coolerState = 1;
    Serial.println("Idle Transition: ");
    clockReport();
    digitalWrite(YL, LOW);
  }

}

void IdleState(){
  digitalWrite(GL, HIGH);
  waterSensor();
  THSensor();
  int buttonStateStop = digitalRead(stop);
  if(buttonStateStop==HIGH){
    coolerState = 0;
    digitalWrite(GL, LOW);
    Serial.println("Disabled Transition: ");
    clockReport();
  }
}

void ErrorState(){
  digitalWrite(RL, HIGH);
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("ERROR: WATER");
  lcd.setCursor(0, 1);
  lcd.write("LEVEL LOW");


  int buttonStateReset = digitalRead(reset);
  if(buttonStateReset==HIGH){
    coolerState = 1;
    digitalWrite(RL, LOW);
    Serial.println("Idle Transition: ");
    clockReport();
  }
  int buttonStateStop = digitalRead(stop);
  if(buttonStateStop==HIGH){
    coolerState = 0;
    digitalWrite(RL, LOW);
    Serial.println("Disabled Transition: ");
    clockReport();
  }
}

void RunningState(){
  digitalWrite(BL, HIGH);
  waterSensor();
  THSensor();
  int buttonStateStop = digitalRead(stop);
  if(buttonStateStop==HIGH){
    coolerState = 0;
    digitalWrite(BL, LOW);
    Serial.println("Disabled Transition: ");
    clockReport();
  }
}