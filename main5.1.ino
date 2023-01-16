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
#define sdCard 53

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

char code[4]; //selected code with the keyboard

// RFID
MFRC522 rfid(SS_RFID, RST_RFID);
byte nuidPICC[6]; //Where the read RFID is stored

//MICROSD CARD
File myFile;
File lockerFile;

//ERROR STATUS
byte errorStatus=0; //if the variable is different from 0, there is an issue


//test
byte RFIDtag[6]; //to compare the rfid from files
byte existingLocker[6];

/* ------------------------------------------------------------------------------------------*/

void setup() {
  Serial.begin(9600);
  pinMode(speakerPin, OUTPUT);
  pinMode(greenLight, OUTPUT);
  pinMode(redLight, OUTPUT);
  pinMode(sdCard, OUTPUT);
  lcd.begin(16,2); // lcd : 16 columns et 2 rows
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  while(!Serial); 
  if(!SD.begin(sdCard)){
    Serial.println("sdCard initialisation failed!");
  } else {
    Serial.println("sdCard initialisation done.");
  }
  digitalWrite(sdCard, HIGH);
  digitalWrite(SS_RFID, HIGH);
  Serial.println("Setup succeded");
}



void loop() {

  int firstFreeLocker=0; // first available locker
  byte RFIDtag[6]; //to compare the rfid from files
  char codeUser[] = "1234"; // to get the size of a code, else add another caracter
  int lockerUser=0; //locker of the user
  byte openingAutorisation=0; //do not autorise the lockers to open
  
  lcdDefaut(); //reset the display of the lcd screen

  //Init the RFID tag arrays
  for(int j=0;j<strlen(nuidPICC);j++){
    nuidPICC[j]=0;
    RFIDtag[j]=0;
  }
  

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

  // nuidPICC is a byte array so we can't print it
  for(int j=0;j<strlen(nuidPICC);j++){
    Serial.print(nuidPICC[j]);
  }


  digitalWrite(sdCard, LOW);
  
  //Check if the SD cards is initialised
  if(!SD.begin(sdCard)){
    Serial.println("initialisation failed!");
  } else {
    Serial.println("initialisation done.");
  }

  /************************************************
  * Check if the rfid tag matches with a locker
  ************************************************/
  if(SD.exists("locker1.txt")){
  myFile = SD.open("locker1.txt");
    if (myFile) {
      Serial.println("compare locker1's RFID tag");
      //store the first line of the file in an array of byte RFIDtag
      for(int j=0;j<strlen(RFIDtag);j++){
        RFIDtag[j]=myFile.read();
      }
      if (strcmp(nuidPICC,RFIDtag)){ //compare the array with nuipPICC
        //store the second line of the file in an arry of int codeUser
        for(int j=0;j<strlen(codeUser);j++){
          codeUser[j]=myFile.read();
        } 
        lockerUser = 1;
      }
      myFile.close(); // close the file
      } else {
        Serial.println("Error opening locker1.txt");
      }
  }
  
  if(SD.exists("locker2.txt")){
  myFile = SD.open("locker2.txt");
    if (myFile) {
      Serial.println("compare locker2's RFID tag");
      //store the first line of the file in an array of byte RFIDtag
      for(int j=0;j<strlen(RFIDtag);j++){
        RFIDtag[j]=myFile.read();
      }
      if (strcmp(nuidPICC,RFIDtag)==0){ //compare the array with nuipPICC
      //store the second line of the file in an arry of int codeUser
        for(int j=0;j<strlen(codeUser);j++){
          codeUser[j]=myFile.read();
        } 
        lockerUser = 2;
      }
      myFile.close(); // close the file
      } else {
        Serial.println("Error opening locker2.txt");
      }
  }

  if(SD.exists("locker3.txt")){
  myFile = SD.open("locker3.txt");
    if (myFile) {
      Serial.println("compare locker3's RFID tag");
      //store the first line of the file in an array of byte RFIDtag
      for(int j=0;j<strlen(RFIDtag);j++){
        RFIDtag[j]=myFile.read();
      }
      if (strcmp(nuidPICC,RFIDtag)){ //compare the array with nuipPICC
      //store the second line of the file in an arry of int codeUser
        for(int j=0;j<strlen(codeUser);j++){
          codeUser[j]=myFile.read();
        } 
        lockerUser = 3;
      }
      myFile.close(); // close the file
      } else {
        Serial.println("Error opening locker3.txt");
      }
  }


  /************************************************
  * IF THE USER DOESN'T HAVE ANY LOCKER
  ************************************************/
  if(lockerUser==0){ 
    firstFreeLocker = locker_free(); //check the first free locker
    Serial.println(firstFreeLocker);
    if(firstFreeLocker==0){
      Serial.println("there is not any locker available");
      return;
    }
    char lockerFree[2];
    switch (firstFreeLocker){ //convert the int into a string
      case 1: 
        strcpy(lockerFree,"1");
        break;    
      case 2: 
        strcpy(lockerFree,"2");
        break; 
      case 3: 
        strcpy(lockerFree,"3");
        break; 
    }
    char file[11] = "locker";
    strcat(file,lockerFree);
    strcat(file,".txt"); // concatenate the file to lockerX.txt with X the first available locker 
    Serial.println(file);

    // set the cursor to (0,0): TOP LEFT
    lcd.setCursor(0, 0);
    lcd.print("Saisissez votre");
    
    // set the cursor to (0,1): BOTTOM LEFT
    lcd.setCursor(0, 1);
    lcd.print("nouveau code");
    Serial.println("Insert new code"); //!\ obligatoire sinon problème lors de l'execution /!\\ 

    for (int compteurCode = 0; compteurCode < 4; compteurCode++) { //compteur pour avoir les 4 premieres clés sélectionnées
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
        lcd.print(key);            
        touche();
        code[compteurCode] = key;
        Serial.println(key);
      }
    }
    code[4] = 0; //caractère de fin de la chaine de caractère

    lockerFile = SD.open(file, FILE_WRITE); //open the locker file to write in it
    Serial.println(lockerFile);
    if (lockerFile){
      //myFile.println(nuidPICC); //DO NOT WORK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      lockerFile.println(code); //DO NOT WORK !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      Serial.println("A locker has been assigned");
    } else {
      Serial.println("/!\\ Error to store the new user");
    }
    delay(400);
    lcd.clear();
    lockerUser = lockerFree; 
    openingAutorisation =1;
  } else { // The user already have a locker
      // set the cursor to (0,0): TOP LEFT
      Serial.println(lockerUser);
      lcd.setCursor(0, 0);
      lcd.print("Veuillez saisir");
      
      // set the cursor to (0,1): BOTTOM LEFT
      lcd.setCursor(0, 1);
      lcd.print("votre code");
      Serial.println("Insert code"); //!\ obligatoire sinon problème lors de l'execution /!\\ 

      for (int compteurCode = 0; compteurCode < 4; compteurCode++) { //compteur pour avoir les 4 premieres clés sélectionnées
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
          lcd.print("*");         
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
      openingAutorisation =1;
      } else {
        lcd.clear();
        lcdDefaut();
        lcd.setCursor(4, 0);
        lcd.print("CODE FAUX");
        codeFaux();
      }
    }


  if(openingAutorisation==1){
    //openLocker(lockerUser); //OPEN THE LOCKER OF THE USER
    Serial.println("A locker is opened");
  }

  digitalWrite(sdCard, HIGH);
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
  digitalWrite(SS_RFID, LOW);
  rfid.PCD_Init();
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
     digitalWrite(SS_RFID, HIGH);
     return flag;
}

void printDec(byte *buffer, byte bufferSize) {
     for (byte i = 0; i < bufferSize; i++) {
             Serial.print(buffer[i] < 0x10 ? " 0" : " ");
             Serial.print(buffer[i], DEC);
     }
}


/* ------------------------------------------ SD Card ------------------------------------------ */
int locker_free(){ //if a locker is not affiliated it returns the first number of a free locker
  if(!SD.exists("locker1.txt")){
    Serial.println("locker 1 is not affiliated");
    return 1;
  }else if(!SD.exists("locker2.txt")){
    Serial.println("locker 2 is not affiliated");
    return 2;
  }else if(!SD.exists("locker3.txt")){
    Serial.println("locker3 is not affiliated");
    return 3;
  }
  return 0;
}
