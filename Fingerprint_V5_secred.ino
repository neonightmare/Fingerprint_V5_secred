//Testcode Fingerprintreader// neonightmare 4.2.2015

// Keypad furr die Speicherung neuer Finger
// Lesevorgang durch Aktivierung Klappe zum Reader
// Keypad-Code für Erfassung&Loeschung neuer Fingerprints 0-99
/*
 *Compatible with the Arduino IDE 1.0
 *Library version:1.1
 *Tested now under gitHub & Atom to Edit and compare Code
 */



#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>

uint8_t getFingerprintEnroll(uint8_t id);

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x3f for a 16 chars and 2 line display
// Pin SDA -> A4 / SCL-> A5

// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

int CoverOpen    = 11;  // beim oeffnen des Deckels -> Aktivierung Fingerprint
int garageButton = A0; //  entspridcht PIN14 / A0

int door         = 12;  // T√ºr√∂ffnerkontakt Haust√ºre
int garage       = 13;  // Garagent√ºr√∂ffner

int State        = 0;  // Statusvariable 0= normal, 1= Finger scannen und pr√ºfen
char key;

int CoverOpen_read       = LOW;
int KeyLock_read         = HIGH;
int garageButton_read    = HIGH;
int setgarageButton      = 0;
int ID2        =  -1;
int ID         =  -1;

int getFingerprintIDez();
int GetNumber();

//Defition Keypad
const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1', '2', '3'  },
  {'4', '5', '6'  },
  {'7', '8', '9'  },
  {'*', '0', '#'  }
};

byte rowPins[ROWS] = {  4, 5, 6, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {  8, 9, 10  }; //connect to the column pinouts of the keypad

const byte secret_code_length = 4;
const char secret_code[] = "1234";
const char secret_code2[] = "4321";

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS, secret_code_length);
unsigned long lastPress = 0; // Warning, overflow after 50 days

/* This handler produces a short beep and stores the last key press */
void key_event_handler(char c) {
  if(keypad.getState()==PRESSED) {
       tone(5, 600, 50);
       lastPress = millis();
  }
}



unsigned long      lastTime     = 0;
unsigned long      elapsedTime  = 0;
unsigned long      waitTime     = 5000; // 5s

unsigned long      lastTime2     = 0;
unsigned long      elapsedTime2  = 0;
unsigned long      waitTime2     = 5000; // 5s

void setup()
{
  //Ein - AUsgaenge definieren
  pinMode(CoverOpen, INPUT_PULLUP);
  pinMode(garageButton, INPUT_PULLUP);
  pinMode(door, OUTPUT);
  pinMode(garage, OUTPUT);

  /* Declare the handler */
    keypad.addEventListener(key_event_handler);


  // initialize the lcd
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);

  // initialize the fingerprintsensor
  Serial.println(F("fingertest"));
  lcd.print("fingertest");
  finger.begin(57600);   // set the data rate for the sensor serial port
  if (finger.verifyPassword()) {
    Serial.println(F("Found fingerprint sensor!"));
    lcd.print(" sensor found!");
  }
  else {
    Serial.println(F("Did not find fingerprint sensor :("));
    lcd.print("no sensor :(");
    while (1);
  }
  delay(3000);
  lcd.clear();
  lcd.noDisplay();
  lcd.noBacklight();
  //Stand-By

}

// ***************************************** //
// Main -Loop - Hauptprogramm //
// ***************************************** //

void loop()                     // run over and over again
{

  delay(10);//kleine Zeitverz√∂gerung f√ºr Loop
  //Schalter einlesen
  CoverOpen_read = digitalRead(CoverOpen);
  keypad.getKey();

  garageButton_read = digitalRead(garageButton);
        if (garageButton_read == LOW)
        {
          setgarageButton = 1;
          Serial.println(F("Garagentaster selektiert!"));

          lastTime = millis();
        }

        if (elapsedTime >= waitTime) setgarageButton = 0;

        /* We test the first password and produce a long sound when it is recognized */
          if(keypad.testPassword(secret_code, secret_code_length)) {
            //tone(5, 500, 500);
            keypad.resetKeysInQueue();
          }
          /* We test the second password and produce a different long sound when it is recognized */
          else if(keypad.testPassword(secret_code2, secret_code_length)) {
            //tone(5, 300, 500);
            State = 3;
            lcd.backlight();
            lcd.display();
            keypad.resetKeysInQueue();
          }

          /* Below we make the password buffer expiring after 10 seconds of inactivity */
          if(lastPress + 10000 < millis()) {
            keypad.resetKeysInQueue();
          }


  switch (State) {
    case 0:
      ID  = -1; //war 255
      ID2 = 0;
    //  Serial.println(F("State 0"));
      if (CoverOpen_read == HIGH) {   // Schalterabdeckung offen Aktivierung Fingerprint f√ºr t√ºre
        State = 1; //Finger scannen und auslesen
        lastTime2 =millis(); // Timer für Einlesen Fingerprint
        lcd.backlight();
        lcd.display();
      }
      break;

    case 1: //Finger scannen und auswerten
    //  Serial.println(F("State 1"));
      lcd.clear();
      lcd.print("Finger Scanner!");
      if (elapsedTime2 >= waitTime2)
      {
         State = 0; // Initalstatus
         Serial.println(F("Fingerprint Timeout"));
         lcd.clear();
         lcd.print("Error!");
         delay(2000);
         lastTime2 = millis();
         lcd.noDisplay();
         lcd.noBacklight();
        break;
      }

       ID = getFingerprintIDez();

        Serial.println(ID);
       if (ID != -1)
       {
        if (setgarageButton == 1)
          {
            State = 4; // Garage ge√∂ffnet
          }
          else
          {
            State = 2; //Tuere oeffnen
          }
        Serial.println(F("ID found Nr: "));
        Serial.println(ID);
        lcd.clear();
        lcd.print("ID-Nummer:");
        lcd.setCursor(0, 1);
        lcd.print(ID);
      }
      break;

    case 2:    //T√ºren √∂ffnen
    //  Serial.println(F("State 2"));
      digitalWrite(door, HIGH); // T√ºrrelais schliessen
      lcd.clear();
      lcd.print("Tuere oeffnen");
      Serial.println(F(" Open the door"));
      delay(300);
      digitalWrite(door, LOW); // T√ºrrelais √∂ffnen
      delay(300);
      digitalWrite(door, HIGH); // T√ºrrelais schliessen
      delay(300);
      digitalWrite(door, LOW); // T√ºrrelais √∂ffnen
      State = 0; // Initalstatus
      delay(2000);
      lcd.clear();
      lcd.noDisplay();
      lcd.noBacklight();
      break;

    case 3:  // neuen Finger einlesen (*) oder l√∂schen (#)
    //  Serial.println(F("State 3"));
      // neuen Einlesen eines Fingers
      Serial.println(F("* fuer speichern oder # fuer loeschen "));
      lcd.clear();
      lcd.print("* fuer speichern");
      lcd.setCursor(0, 1);
      lcd.print("# fuer loeschen");

      key = keypad.waitForKey();
      if (key == NO_KEY) break;

      else if (key == '*') {
        Serial.println(F("Type in the ID # you want to save this finger as..."));
        lcd.clear();
        lcd.print("welche ID ");
        lcd.setCursor(0, 1);
        lcd.print("to store (#)?");
        ID2 = GetNumber();
        Serial.println(F("Enrolling ID #"));
        Serial.println(ID2);
        lcd.clear();
        lcd.print("Einlesen auf ID: ");
        lcd.setCursor(0, 1);
        lcd.print(ID2);
        while (!getFingerprintEnroll(ID2));
        // neuen Finger Einlesen Ende
        State = 0; // Initalstatus
        delay(2000);
        lcd.clear();
        lcd.noDisplay();
        lcd.noBacklight();
      }

      else if (key == '#') {
        Serial.println(F("Type in the ID # you want to delete"));
        lcd.clear();
        lcd.print("Welche ID ");
        lcd.setCursor(0, 1);
        lcd.print("loeschen?");
        ID2 = GetNumber();
        Serial.println(F("Deleting ID #"));
        Serial.println(ID2);
        lcd.clear();
        lcd.print("ID loeschen: ");
        lcd.setCursor(0, 1);
        lcd.print(ID2);
        deleteFingerprint(ID2);
        State = 0; // Initalstatus
        delay(3000);
        lcd.clear();
        lcd.noDisplay();
        lcd.noBacklight();
      }
      break;

    case 4: //Garagentor öffnen
    //  Serial.println(F("State 4"));
      digitalWrite(garage, HIGH); // T√ºrrelais schliessen
      lcd.clear();
      lcd.print("Garage oeffnen");
      Serial.println(F(" Open the garage- door"));
      delay(400);
      digitalWrite(garage, LOW); // T√ºrrelais √∂ffnen
      State = 0; // Initalstatus
      delay(2000);
      lcd.clear();
      lcd.noDisplay();
      lcd.noBacklight();
     break;

    default:
      Serial.println(F("default"));
      break;

  } //switch State

     elapsedTime = (millis() - lastTime);
     elapsedTime2 = (millis() - lastTime2);

} // end loop

///////END LOOP///////////////////////////////////////////////////////////////////////////////////////

// ***************************************** //
// Fingerprint einlesen
// ***************************************** //
uint8_t getFingerprintEnroll(uint8_t id) {
  uint8_t p = -1;
  Serial.println(F("Waiting for valid finger to enroll"));
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println(F("Image taken"));
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(F("."));
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }

  Serial.println(F("Remove finger"));
  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }

  p = -1;
  Serial.println(F("Place same finger again"));
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println(F("Image taken"));
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(F("."));
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println(F("Communication error"));
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println(F("Imaging error"));
        break;
      default:
        Serial.println(F("Unknown error"));
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }


  // OK converted!
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Prints matched!"));
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return p;
  }
  else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println(F("Fingerprints did not match"));
    return p;
  }
  else {
    Serial.println(F("Unknown error"));
    return p;
  }

  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Stored!"));
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println(F("Could not store in that location"));
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR) {
    Serial.println(F("Error writing to flash"));
    return p;
  }
  else {
    Serial.println(F("Unknown error"));
    return p;
  }
}
// ***************************************** //
// ID auslesen Fingerprint
// ***************************************** //


uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image taken"));
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(F("No finger detected"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println(F("Imaging error"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println(F("Image converted"));
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println(F("Image too messy"));
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println(F("Could not find fingerprint features"));
      return p;
    default:
      Serial.println(F("Unknown error"));
      return p;
  }

  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println(F("Found a print match!"));
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return p;
  }
  else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println(F("Did not find a match"));
    return p;
  }
  else {
    Serial.println(F("Unknown error"));
    return p;
  }

  // found a match!
  Serial.println(F("Found ID #"));
  Serial.println(finger.fingerID);
  Serial.println(F(" with confidence of "));
  Serial.println(finger.confidence);
}



uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println(F("Deleted!"));
    lcd.clear();
    lcd.print("ID Deleted! ");
    lcd.setCursor(0, 1);
    lcd.print(id);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return p;
  }
  else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println(F("Could not delete in that location"));
    lcd.clear();
    lcd.print("Error delete ID");
    lcd.setCursor(0, 1);
    lcd.print(id);
    return p;
  }
  else if (p == FINGERPRINT_FLASHERR) {
    Serial.println(F("Error writing to flash"));
    lcd.print("Error flash");
    return p;
  }
  else {
    Serial.println(F("Unknown error: 0x"));
       lcd.print("Error Nr: ");
    lcd.setCursor(0, 1);
    lcd.print(p);
    Serial.println(p, HEX);
    return p;
  }
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.println(F("Found ID #"));
  Serial.println(finger.fingerID);
  Serial.println(F(" with confidence of "));
  Serial.println(finger.confidence);
  return finger.fingerID;
}


/////////// Number aus Keypad lesen /////////////////////
int GetNumber()
{
  int num   =   0;
  int count =   0;
  char key = keypad.waitForKey();
  count = 1;
  while (key != '#')
  {
    switch (key)
    {
      case NO_KEY:
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        lcd.print(key);
        Serial.println(key);
        num = num * 10 + (key - '0');
        count++;
        break;

      case '*':
        num = 0;
        lcd.clear();
        break;
    }
    key = keypad.waitForKey();
    if (count >= 3)
    {
      key = '*';
      Serial.println(F("Digit-Counter Maximum 2 Digits erreicht:"));
      Serial.println(count);
      lcd.clear();
      lcd.print("0 bis 99 (#):");
      lcd.setCursor(0, 1);
      count = 0;
    }
  }

  return num;
}
