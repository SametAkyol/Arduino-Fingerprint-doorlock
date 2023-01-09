#include <Adafruit_Fingerprint.h>
#include <Key.h>
#include <Keypad.h>
//#include "Adafruit_Keypad.h"
#include<EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <avr/pgmspace.h>
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif
const char *message[] =
{
  "Sifre", "Dogru",
  "Lutfen", "Parmaginizi",
  "Okutun"
};
///////////////////////////////////////
LiquidCrystal_I2C lcd(0x27, 16, 2);
const char name_3[] PROGMEM = "Ozge";
const char name_0[] PROGMEM = "Serdar";
const char name_1[] PROGMEM = "Samet";
const char name_2[] PROGMEM = "Mert";
const char name_4[] PROGMEM = "Sinan";
const char name_5[] PROGMEM = "Omer";
const char *const name_table[] PROGMEM = {name_0, name_1, name_2, name_3, name_4, name_5};
char buffer[70];
const int rly = 12;
const int ldgr = 6;
const int ldrd = 8;
const int ldyl = 7;
const int buzzer = 5;
const int  C = 262;
const int D = 523;
///////////////////////////////////////////////////////////////////////
const byte ROWS = 4;
const byte COLS = 3;
char initial_password[4], new_password[4];
int i = 0;
int j = 0;
int a = 0;
int u = 0;
char password[4];
char key_pressed = 0;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9',},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {A3, A2, A1, A0};
byte colPins[COLS] = {10, 11, 13};
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Keypad keypad_key = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
void setup()
{
  lcd.begin();
  lcd.print("Sifreyi Giriniz");
  initialpassword();
  pinMode(ldgr, OUTPUT);
  pinMode(ldrd, OUTPUT);
  pinMode(ldyl, OUTPUT);
  pinMode(rly, OUTPUT);
  digitalWrite(rly, HIGH);
  digitalWrite(ldyl, HIGH);

  Serial.begin(9600);
  while (!Serial);
  delay(100);
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Sensor bulundu!");
  } else {
    Serial.println("Sensor bulunamadı. :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("Sensorde parmakizi kayitli degil.Lutfen enroll ornegini calıstırın.");
  }
  else {
    Serial.println("Parmak Bekleniyor..");
    Serial.print("Sensorde "); Serial.print(finger.templateCount); Serial.println("tane  ornek var.");
  }
}

void loop()
{
  getFingerprintID();
  lcd.setCursor(0, 0);
  key_pressed = keypad_key.getKey();
  if (key_pressed == '*')  change();
  if (key_pressed == '#') {
    key_pressed = 0;
    i = 0;
    u = 0;
    lcd.clear();
    lcd.print("Sifreyi Giriniz");

  }
  if (key_pressed) {
    password[i++] = key_pressed;
    lcd.setCursor(u, 1); u++;
    lcd.print(key_pressed);

    if (i == 4)
    {
      lcd.clear();
      delay(200);
      for (int j = 0; j < 4; j++)
        initial_password[j] = EEPROM.read(j);
      if (!(strncmp(password, initial_password, 4))) {
        a++;

        digitalWrite(ldyl, LOW);
        digitalWrite(ldgr, HIGH);
        delay(900);
        digitalWrite(ldgr, LOW);
        digitalWrite(ldyl, HIGH);
        msg();
        u = 0;
      }
      else {
        i = 0;
        digitalWrite(ldyl, LOW);
        digitalWrite(ldrd, HIGH);
        delay(900);
        digitalWrite(ldrd, LOW);
        digitalWrite(ldyl, HIGH);
        lcd.print("Sifre Yanlis");
        delay(700);
        lcd.clear();
        u = 0;
        lcd.print("Sifreyi Giriniz");
      }
    }
  }
}
uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    if (a >= 1) {
      lcd.clear();
      accepted();
      delay(1000);
      a = 0;
      i = 0;
    }
    else {
      denied();
    }
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    error();
    delay(1000);
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    denied();
    delay(300);
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return finger.fingerID;
}

int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)
    return -1;
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
void  accepted() {
  digitalWrite(ldyl, LOW);
  digitalWrite(ldgr, HIGH);
  tone(buzzer, C);
  delay(1000);
  noTone(buzzer);
  digitalWrite(ldgr, LOW);
  digitalWrite(rly, LOW);
  lcdok();
  delay(2000);
  digitalWrite(rly, HIGH);
  lcd.clear();
  lcd.print("Sifreyi Giriniz");
  digitalWrite(ldyl, HIGH);


}

void denied() {
  digitalWrite(ldyl, LOW);
  digitalWrite(ldrd, HIGH);
  tone(buzzer, D);
  delay(1000);
  digitalWrite(ldrd, LOW);
  noTone(buzzer);
  digitalWrite(ldyl, HIGH);
  a = 0;
  i = 0;
  lcd.clear();
  lcd.print("Parmak Eslesmedi");
  delay(1000);
  lcd.clear();
  lcd.print("Sifreyi Giriniz");
}

void error() {
  digitalWrite(ldyl, LOW);

  for (int c = 0; i < 4; c++) {
    digitalWrite(ldrd, HIGH);
    delay(100);
    digitalWrite(ldrd, LOW);
    delay(100);
  }
  digitalWrite(ldyl, HIGH);
  lcd.clear();

  lcd.print("Sifreyi Giriniz");
}
void initialpassword() {

  for (int j = 0; j < 4; j++)
    EEPROM.write(j, j + 49);
  for (int j = 0; j < 4; j++)
    initial_password[j] = EEPROM.read(j);
}

void change() {
  int j = 0;
  while (j < 4) {
    char key = keypad_key.getKey();
    if (key) {
      new_password[j++] = key;
    }
    key = 0;
  }
  delay(500);
  if ((strncmp(new_password, initial_password, 4))) {
  }
  else
  {
    j = 0;
    while (j < 4) {
      char key = keypad_key.getKey();
      if (key) {
        initial_password[j] = key;
        EEPROM.write(j, key);
        j++;
      }
    }
    delay(1000);
  }
  key_pressed = 0;
}
void lcdok() {
  lcd.setCursor(4, 0);
  lcd.print("Hosgeldin");
  lcd.setCursor(8 - (strlen(strcpy_P(buffer, (char *)pgm_read_word(&(name_table[finger.fingerID - 1]))))) / 2, 1);
  lcd.print(strcpy_P(buffer, (char *)pgm_read_word(&(name_table[finger.fingerID - 1]))));

}

void msg() {
  for (int m = 0; m < sizeof(message) / sizeof(message[0]); m += 2) {
    for (int line = 0; line < 2; line++) {
      lcd.setCursor( (16 / 2) - (strlen(message[m + line]) / 2), line);
      lcd.print(message[m + line]);
    }
    delay(1000);
    lcd.clear();
  }
}
