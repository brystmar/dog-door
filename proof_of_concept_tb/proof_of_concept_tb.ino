#include <Servo.h>
#include <SoftwareSerial.h>
SoftwareSerial RFID(3, 2); // RX and TX

int c = 0;
int i = 0;
int count = 0;
int tagID [14];
String text = "";
String input = "";
int fsrPin = 0;                     // FSR on analog pin A0
int fsrValue;                       // sensor value
int ledBluePin = 8;                 // blue LED on pin 8
int btnPin = 10;                     // reset button on pin 4
int servoPin = 6;                   // servo on pin 6
int servoPos = 0;                   // variable to store the servo position
int servoTurn = 0;                  // degrees to incrementally turn the servo

Servo myservo;                      // create servo object

void setup()
{
  RFID.begin(9600);    // start serial to RFID reader
  Serial.begin(9600);  // start serial to PC

  myservo.attach(servoPin);           // attaches the servo object to the servo pin
  pinMode(ledBluePin, INPUT_PULLUP);  // initialize the blue LED, attaches to the LED pin
  digitalWrite(ledBluePin, LOW);      // light off to start
  resetServo();

  pinMode(btnPin, INPUT_PULLUP);
}

void loop()
{
  fsrValue = analogRead(fsrPin);

  if (analogRead(fsrPin) > 150)
  {
      turnServo(150);
      delay(100);
  }
  
  if (digitalRead(btnPin) == LOW)
  {
      Serial.println("Resetting...");
      Serial.println("");
      text = "";
      turnServo(90);
      i=0;
  }
  
  while (RFID.available() > 0)
  {
     c = RFID.read();
     //Serial.println(c, DEC);
     text += c;
     c = 0;
     ledFlash(10);
     
     if (text.length() == 26)
     {
       Serial.println("RFID tag ID: " + text);

       if (text == "24854484848666751695150683")
       {
          Serial.println("Poops detected!  You may enter, Curie.");
          Serial.println("");
          turnServo(120);
          text = "";
       }
       else
       {
          Serial.println("Unauthorized tag!");
          Serial.println("");
          text = "";
       }
     }

     i += 1;
  }
  
  //text = "";  
  //delay(500);
}

void ledFlash(int val)
{
    digitalWrite(ledBluePin, HIGH);
    delay(val);
    digitalWrite(ledBluePin, LOW);
}

void turnServo(int num)
{
    myservo.write(num);
    delay(300);
}

void resetServo()
{
    myservo.write(0);
    delay(300);
}
