#include <Servo.h>
#include <SoftwareSerial.h>
#include <TimeLib.h>
SoftwareSerial RFID_O(2, 3); // RX and TX on Arduino (insert RFID wires: yellow, white)
SoftwareSerial RFID_I(4, 5); // RX and TX on Arduino

bool beginAuth = false;                           // flag for authorizing new tags
String master = "485448484866675169515068";       // master token used to authorize new tags (00-00771043)
String authlist[10] = { "485448484867705150566849", "485348486749566952574851", "","","","","","","","" };    // tags: 00-00848680, 00-12684873
//long int authlist_hex[] = { 0x06000BC3E32D, 0x0500C18E4903 };                        // tags: 00-00771043, 00-12684873

//unsigned long int hexint = 0x06000BC3E32D;

time_t t = now(); // Store the current time
int c = 0;
int i = 0;
int count = 0;
int index = 0;
//int tgID [14];
String test = "";
String tag = "";                    // stores the ID of the read
//String input = "";                  // unused??
String reader = "";                 // indicates which RFID reader was used
String stamp = "";                  // timestamp when the tag was read
int fsrPin = 0;                     // FSR touch sensor on analog pin A0
int fsrValue = 0;                   // value for how much the touch sensor is depressed
int fsrValuePrev = 0;               // previous value for the touch sensor
int ledBluePin = 9;                 // blue LED on pin 9
int btnPin = 13;                    // reset button on pin 13
int servoPin = 11;                  // servo on pin 11
int servoPos = 0;                   // variable to store the servo position
int servoTurn = 0;                  // degrees to incrementally turn the servo

Servo myservo;                      // create servo object

void setup()
{
  RFID_O.begin(9600);    // start serial to outside RFID reader
  //RFID_I.begin(9600);    // start serial to inside RFID reader
  Serial.begin(9600);    // start serial to PC

  myservo.attach(servoPin);           // attaches the servo object to the servo pin
  pinMode(ledBluePin, INPUT_PULLUP);  // initialize the blue LED, attach to the LED pin
  digitalWrite(ledBluePin, LOW);      // LED should be off at the start
  resetServo();                       // starts the servo in the closed position

  pinMode(btnPin, INPUT_PULLUP);

  setTime(23,16,40,11,9,2018);
  t = now();
  Serial.print(gettime() + " ");
  Serial.println("Ready");
}

void loop()
{
  //t = now();  //update the timestamp
  
  fsrValuePrev = fsrValue;
  fsrValue = analogRead(fsrPin);      // get the touch sensor value

  if (fsrValue > 300)             // if the FSR touch sensor is noticeably triggered
  {
      Serial.print(gettime() + " ");
      Serial.println("Touch sensor pressed!  Value: " + String(fsrValue));
      beginAuth = false;
      ledBlink(2);
      turnServo(90, 1500);
      fsrValue = 0;
      fsrValuePrev = 0;
      resetServo();
  }

  if (digitalRead(btnPin) == LOW)     // reset everything when the onboard button is pressed
  {
      Serial.println();
      showAuthTags(true);
      Serial.print(gettime() + " ");
      Serial.println("Resetting...");
      Serial.println("");
      beginAuth = false;
      ledBlink(3);                    // blink 5 times
      delay(300);
      ledBlink(3);
      tag = "";
      reader = "";
      fsrValue = 0;
      fsrValuePrev = 0;
      resetServo();
      i=0;
  }

  while (RFID_O.available() > 0 or RFID_I.available() > 0)
  {
     if (beginAuth == true)               // turn LED on when adding a new tag
     { digitalWrite(ledBluePin, HIGH); }
     else
     { digitalWrite(ledBluePin, LOW); }
     
     if (RFID_O.available() > 0)
     {
       c = RFID_O.read();
       reader = "Outside";
     }
     else
     {
       c = RFID_I.read();
       reader = "Inside";
     }

     if (String(c).length() > 1)      // ignore the prefix "2" and the suffix "3"
     {
       tag += c;
     }
     
     //Serial.println("c: " + String(c) + ", c len: " + String(c).length() + ", tag len: " + String(tag.length()) + ", tag: " + tag );
     
     c = 0;

     if (tag.length() == 24)
     {
        Serial.print(gettime() + " ");
        Serial.print("RFID tag (" + reader + "): " + String(tag) + ".  ");
     }

     if (tag.length() == 24 && beginAuth == true)
     {
        if (tag != master && validate(tag) == false)      // adds a tag that isn't already on the list
        {
          index = showAuthTags(false);
          Serial.println("<-- Authorized as tag #" + String(index+1) + ".");
          Serial.println();
          authlist[index] = tag;
          beginAuth = false;
          tag = "";
          reader = "";
          ledBlink(index+1);
        }

        //if (tag == master)                  // scanning the master again cancels the auth process
        else if (validate(tag) or tag == master)
        {
          beginAuth = false;
          Serial.println("Ending tag authorization.");
          tag = "";
          reader = "";
        }
        showAuthTags(true);
     }
     
     else if (tag.length() == 24 && beginAuth == false)
     {
       if (tag == master)
       {
          beginAuth = true;
          Serial.println("Master tag recognized.  Scan new tag to authorize.");
          tag = "";
          reader = "";
       }
       else if (validate(tag))
       {
          Serial.println("Poops detected!  You may enter, Curie.");
          ledBlink(2);
          turnServo(90, 1500);
          turnServo(0, 0);
          tag = "";
          reader = "";
       }
       else
       {
          Serial.println("Unauthorized tag!");
          ledBlink(8);
          tag = "";
          reader = "";          
       }
       
     }
     i += 1;
  }

  //tag = "";
  //delay(500);
}

void ledFlash(int val)
{
    digitalWrite(ledBluePin, HIGH);
    delay(val);
    digitalWrite(ledBluePin, LOW);
}

void ledBlink(int num)
{
    for (int b=0; b<num; b++)
    {
      digitalWrite(ledBluePin, HIGH);
      delay(75);
      digitalWrite(ledBluePin, LOW);
      delay(75);
    }
}

void turnServo(int deg, int len)
{
    myservo.write(deg);
    delay(len);
}

void resetServo()
{
    turnServo(30, 300);
    turnServo(0, 300);
}

bool validate(String tag)
{
  for (int v=0; v < sizeof(authlist)/6 ; v++)    // each array value is 6 bytes, and sizeof returns number of bytes used
  {
    if (authlist[v] == tag)
    { return true; }
  }
  return false;
}

int showAuthTags(bool show)
{
  int num = 0;
  for (int v=0; v < sizeof(authlist)/6 ; v++)
  {
    if (authlist[v].length() > 1)
    {
      num += 1;
      if (show) { Serial.println(authlist[v]); }
    }
  }
  if (show) { Serial.println("There are " + String(num) + " authorized tags."); Serial.println();}
  return num;
}

String gettime()
{
  time_t tm = now();
  return (String(year(tm)) + "-" + pad(month(tm)) + "-" + pad(day(tm)) + " " + pad(hour(tm)) + ":" + pad(minute(tm)) + ":" + pad(second(tm)));
}

String pad(int val)
{
  String nv = String(val);
  if (nv.length() == 1)
  { return "0" + nv; }
  else
  { return nv; }
}

