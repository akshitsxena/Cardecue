#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#define RST_PIN 5    // RST-PIN for RC522 - RFID - SPI - Modul GPIO5 
#define SS_PIN 15    // SDA-PIN for RC522 - RFID - SPI - Modul GPIO15

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// Firebase credentials
#define FIREBASE_HOST "https://rfidtest-b74fd-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "vJQheR1WQA55hh1dj6GznRKCrESa658cLUBAqurQ"

const char* ssid = "No";
const char* password = "123456789";

FirebaseData firebaseData;

// Keypad configuration
const byte ROWS = 4;  // number of rows on the keypad
const byte COLS = 4;  // number of columns on the keypad

// Define the keypad layout
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {13, 12, 14, 27};     // connect to the row pinouts of the keypad
byte colPins[COLS] = {26, 25, 33, 32};     // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD display configuration
LiquidCrystal_I2C lcd(0x27,20,4);  // I2C address may vary, use the correct address

void setup() {
  Serial.begin(9600);
 
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Wi-Fi connected. IP address: ");
  Serial.println(WiFi.localIP());
 // Initialize serial communication.
  SPI.begin();           // Init SPI bus.
  mfrc522.PCD_Init();    // Init MFRC522 card.
  Serial.println("Ready to read RFID/NFC tags!");
  Serial.println("Please place the tag near the reader...");
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  lcd.init();                      // initialize the lcd 
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(0, 1);
  lcd.print("RFID Deduction");
  lcd.setCursor(0, 0);
  lcd.print("Ready");
}

void loop() {
  // Check for new RFID card

  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
     String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uid.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      uid.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    Serial.print("UID tag: ");
    Serial.println(uid);
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("UID:");
          lcd.setCursor(0, 1);
          lcd.print(uid);
          delay(2000);
    // Check if the UID exists in the database
    String path = "/users/" + uid;
    
    int espuserBalance = 0;
    String espPath = "/users/EspUser/balance";
    if(Firebase.getString(firebaseData, espPath))
     { 
      espuserBalance = firebaseData.stringData().toInt();
     }

    int pin = 0;
    if (Firebase.getString(firebaseData, path))
     {
      String balancePath = path + "/balance";
      String amountPath = path + "/amount";
      String pinPath = path + "/pin";
      if(Firebase.getString(firebaseData, pinPath))
     { 
      pin = firebaseData.stringData().toInt();
     }

      Serial.println("Your pin "+ pin);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter your Pin:");
      delay(2000);
      lcd.setCursor(0,1);
      int pininput = getEnteredAmount(); 

      if(pininput==pin)
     {
      
      // Retrieve the user's balance from the database
      if (Firebase.getString(firebaseData, balancePath))
       {
        int balance = firebaseData.stringData().toInt();
        // Get the amount from the keypad
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter Amount:");
        delay(2000);
        lcd.setCursor(0,1);

        int amount = getEnteredAmount();
        // Deduct the amount from the balance

        if (firebaseData.dataAvailable()) 
        {
          lcd.clear();
          lcd.print("Amount:");
          lcd.setCursor(0, 1);
          lcd.print(amount);
          int updatedBalance = balance - amount;
          espuserBalance = espuserBalance + amount;
        // Update the balance in the database

        Firebase.setInt(firebaseData, balancePath, updatedBalance);
        Firebase.setInt(firebaseData, amountPath, amount);
        Firebase.setInt(firebaseData, espPath, espuserBalance);

          Serial.println("Amount deducted successfully.");
          delay(2000);
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Amount deducted");
          lcd.setCursor(0,1);
          lcd.print("successfully");
          delay(2000);
          getprintednew();
        } 
        else {
          Serial.println("Failed to deduct amount.");
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Failed to");
          lcd.setCursor(0,1);
          lcd.print("deduct amount");
          delay(2000);
        }
      } 
      else {
        Serial.println("Failed to retrieve balance from the database.");
         lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Failed to ");
          lcd.setCursor(0,1);
          lcd.print("retrieve balance");
          delay(2000);
      }
     }
     else
     {
      Serial.println("Incorrect Pin Please try again");
       lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Incorrect Pin");
          lcd.setCursor(0,1);
          lcd.print("Try Again");
          delay(2000);
     }
     }
     else {
      Serial.println("UID not found in the database.");
       lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("UID not found");
          lcd.setCursor(0,1);
          lcd.print("Try Again");
          delay(2000);
    }
    getprintednew();

    // Halt PICC to stop further read attempts
    mfrc522.PICC_HaltA();

    // Reset the UID buffer
    mfrc522.uid.size = 0;
  }
}

void getprintednew()
{
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Place Your Tag");
  lcd.setCursor(0, 0);
  lcd.print("Ready");
}

  int getEnteredAmount() 
  {
  
  String amountString = "";
  char key = keypad.waitForKey();
  while (key != '#') {
    if (key >= '0' && key <= '9') {
      amountString += key;
      lcd.setCursor(amountString.length() - 1, 1);
      lcd.print(key);
      Serial.print(key);
    }
    key = keypad.waitForKey();
  }
  return amountString.toInt();
}
