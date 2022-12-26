// LIBRARIES
#include <Keypad.h> //keypad
#include <LiquidCrystal.h> //LCD screen
#include <SD.h> // SD Card
#include <SPI.h> // RFID & SD Card
#include <MFRC522.h> //RFID

/* ------------------------------------------------------------------------------------------*/

// ACTIVE BUZZER
#define speakerPin 2

// LED
#define greenLight 49
#define redLight 48

// LCD SCREEN
#define RS 47
#define EN 45
#define D4 43
#define D5 41
#define D6 39
#define D7 37

// MICROSD CARD
#define sdCard 10

//RFID
#define SS_RFID 32
#define RST_RFID 33


//
/* ------------------------------------------------------------------------------------------*/

// LCD SCREEN
LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);

// KEYPAD
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'#', '0', '*', 'D'}
};

byte rowPins[ROWS] = {27, 28, 29, 30}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {23, 24, 25, 26}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

char code[4]; //code selectionné

// RFID
MFRC522 rfid(SS_RFID, RST_RFID);
byte nuidPICC[6]; //
byte existingLocker[6];

//MICROSD CARD
File myFile;

/* ------------------------------------------------------------------------------------------*/

void setup() {
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
  pinMode(greenLight, OUTPUT);
  pinMode(redLight, OUTPUT);
  pinMode(sdCard, OUTPUT);
  lcd.begin(16,2); // lcd : 16 colonnes et 2 lignes
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  Serial.println("Setup succeded");
}



void loop() {
  int compteurCode = 0;
  int affichageCode =0;
  char codeUser[] = "2023";
  lcdDefaut();
  for(int j=0;j<strlen(nuidPICC);j++){
    nuidPICC[j]=0;
  }


  //digitalWrite(SS_RFID, LOW); // Disable the RFID
  digitalWrite(sdCard, LOW);  

  if (!SD.begin(sdCard)) {
    Serial.println("initialization failed!");
  } else {
    Serial.println("initialization done.");
  }


  //digitalWrite(sdCard, HIGH);
  //digitalWrite(SS_RFID, LOW); // Enable the RFID

/*********************************
* Check if a card was read
*********************************/
  lcd.clear();
  //Wait to read a rfid card
  byte cardRead = 0;
  Serial.println("Wait for a card to be read");
  while(cardRead!=1){
    cardRead = readRFID(); //store the succes (or not) of the operation
    delay(200);
  }
  Serial.println("A card has been read"); 


/*
  Serial.println("nuidpicc : ");  
  for(int j=0;j<strlen(nuidPICC);j++){
    Serial.print(nuidPICC[j]);    
  }

  Serial.println("existing locker : ");
  for(int j=0;j<strlen(existingLocker);j++){
    Serial.print(existingLocker[j]);    
  }

  Serial.println(strlen(existingLocker));

  if(strcmp(nuidPICC,existingLocker)==0){
    Serial.println("sucess");
  } else{
    Serial.println("failed");
  }
  

  for(int j=0;j<strlen(nuidPICC);j++){
    Serial.print(nuidPICC[j]);    
  }

  strcpy(existingLocker,nuidPICC);
  Serial.println("existing locker : ");
  for(int j=0;j<strlen(existingLocker);j++){
    Serial.print(existingLocker[j]);    
  }

  if(strcmp(nuidPICC,existingLocker)==0){
    Serial.println("sucess");
  } else{
    Serial.println("failed");
  }*/

/************************************************
* Check if the rfid adress matches with a locker
************************************************/

/************************************************
* If not check if 
************************************************/


  // set the cursor to (0,0): TOP LEFT
  lcd.setCursor(0, 0);
  lcd.print("Veuillez saisir");
  
  // set the cursor to (0,1): BOTTOM LEFT
  lcd.setCursor(0, 1);
  lcd.print("votre code");
  Serial.println("code à insérer"); //!\ obligatoire sinon problème lors de l'execution /!\

  for (compteurCode = 0; compteurCode < 4; compteurCode++) { //compteur pour avoir les 4 premieres clés sélectionnées
    char key = keypad.waitForKey(); //wait for a key to be pressed
    if(compteurCode==0){
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("<CODE>");
      lcd.blink();
      lcd.cursor();
    }
     
    if (key != NO_KEY) {
      lcd.setCursor(6+compteurCode, 1);
      //lcd.print("*");
      lcd.print(key);            
      touche();
      code[compteurCode] = key;
      Serial.println(key);
    }
  }
  code[4] = 0; //caractère de fin de la chaine de caractère
  delay(400);
  
  if (strcmp(code, codeUser) == 0) {
    lcd.clear();
    lcdDefaut();
    lcd.setCursor(4, 0);
    lcd.print("CODE BON");
    codeBon();
  } else {
    lcd.clear();
    lcdDefaut();
    lcd.setCursor(4, 0);
    lcd.print("CODE FAUX");
    codeFaux();
  }


}



/* ------------------------------------------ ACTIVE BUZZER ------------------------------------------ */
void touche() {
  analogWrite (speakerPin, 255); // Signal haut sur broche 8
  delay (20); // pendant 20 ms
  analogWrite (speakerPin, 0); // Signal bas sur broche 8
}



void codeBon() {
  int i, j = 10;
  digitalWrite (greenLight, HIGH);
  for (i = 0; i < 2; i++) {
    analogWrite (speakerPin, j); // Signal haut sur broche 8
    delay (60); // pendant 60 ms
    analogWrite (speakerPin, 0); // Signal bas sur broche 8
    delay (100); // pendant 100 ms
    j += 10;
  }
  delay(200);
  digitalWrite (greenLight, LOW);
}



void codeFaux() {

  digitalWrite (redLight, HIGH);
  for (int i = 0; i < 5; i++) {
    analogWrite (speakerPin, 30); // Signal haut sur broche 8
    delay (50); // pendant 50 ms
    analogWrite (speakerPin, 0); // Signal bas sur broche 8
    delay (50); // pendant 50 ms
  }
  delay(200);
  digitalWrite (redLight, LOW);
}


/* ------------------------------------------ LCD SCREEN ------------------------------------------ */
void lcdDefaut(){
  lcd.noCursor();
  lcd.noBlink();
}

/* ------------------------------------------ RFID MODULE ------------------------------------------ */
int readRFID(){
  int flag=0;
       // Look for new card
       if (!rfid.PICC_IsNewCardPresent())
       return flag;
       // Verify if the NUID has been readed
       if (!rfid.PICC_ReadCardSerial())
       return flag;
      
     Serial.println(F("A new card has been detected."));
             // Store NUID into nuidPICC array
             for (byte i = 0; i < 4; i++) {
                nuidPICC[i] = rfid.uid.uidByte[i];
             }
         
     Serial.print(F("RFID tag in dec: "));
     printDec(rfid.uid.uidByte, rfid.uid.size);
     Serial.println();
     
     // Halt PICC
     rfid.PICC_HaltA();
     // Stop encryption on PCD
     rfid.PCD_StopCrypto1();
     flag++;
     //Serial.println(flag);
     return flag;
}

void printDec(byte *buffer, byte bufferSize) {
     for (byte i = 0; i < bufferSize; i++) {
             Serial.print(buffer[i] < 0x10 ? " 0" : " ");
             Serial.print(buffer[i], DEC);
     }
}
