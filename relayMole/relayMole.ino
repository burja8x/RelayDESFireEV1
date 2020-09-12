#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532.h>

PN532_SPI pn532spi(SPI, 5);
PN532 nfc(pn532spi);

void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello!");
  Serial.println(D1);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.println("Didn't find PN53x board");
    //while (1); // halt
  }

  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  nfc.setPassiveActivationRetries(0xFF);
  //nfc.setPassiveActivationRetries(10);

  // configure board to read RFID tags
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A cardAA");
  Serial.println("\n-----------\n");
}

uint8_t response[50];
uint8_t responseLength = 50;

String readCMD;
uint8_t cmdData[100];
bool startCmd;
bool endCmd;
uint32_t cmdLength;

uint8_t uidLength;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
uint8_t uidX[] = { 0x04, 0x11, 0x22, 0x33, 0x44, 0x88, 0xaa };


//uint8_t cmdSelectApp[] = {0x90, 0x5a, 0x00, 0x00, 0x03, 0x49, 0x4e, 0x55, 0x00};
//uint8_t cmdSelectApp[] = {0x5a, 0x49, 0x4e, 0x55};
//uint8_t cmdSelectApp[] = {0x90, 0x5a, 0x00, 0x00, 0x03, 0x19, 0x04, 0x1d, 0x00};
uint8_t cmdSelectApp[] = {0x5a, 0x19, 0x04, 0x1d};

//uint8_t cmd1[] = {0x90, 0xaa, 0x00, 0x00, 0x01, 0x01, 0x00};
uint8_t cmd1[] = {0xaa, 0x01};

//uint8_t cmd2[] = {0x90, 0xaf, 0x00, 0x00, 0x20, 0x7a, 0x4e, 0x6c, 0x1c, 0x32, 0xcf, 0xc0, 0x87, 0xc0, 0xc2, 0x99, 0x9e, 0x6d, 0x47, 0xc9, 0xec, 0xc7, 0xe8, 0x3c, 0x31, 0x6e, 0x38, 0x0f, 0x10, 0x97, 0x57, 0x76, 0x5f, 0xdb, 0xa0, 0xdc, 0x28, 0x00};
uint8_t cmd2[] = {0xaf, 0x7a, 0x4e, 0x6c, 0x1c, 0x32, 0xcf, 0xc0, 0x87, 0xc0, 0xc2, 0x99, 0x9e, 0x6d, 0x47, 0xc9, 0xec, 0xc7, 0xe8, 0x3c, 0x31, 0x6e, 0x38, 0x0f, 0x10, 0x97, 0x57, 0x76, 0x5f, 0xdb, 0xa0, 0xdc, 0x28};


bool exitLoop = false;
bool entireCMD = false;

bool prevCmdAASucsessful = false;

unsigned long startTime;

void loop(void) { 
  boolean success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength, 1000, true);

  if (success) {
    prevCmdAASucsessful = false;  
    Serial.print("UID:");
    PrintHexT(uidX, (uint32_t)uidLength);

    // Select application
    bool statusX = nfc.inDataExchange(cmdSelectApp, sizeof(cmdSelectApp), &response[0], &responseLength);
    if(statusX){
        
        if(response[0] == 0x00 || (response[0] == 0x91 && response[1] == 0x00 )){
            Serial.println("APPLICATION SELECTED");  
            CleanResponse();
        }else{
            Serial.println("ERROR WHEN SELECTING APP. NOT OK");
            PrintHexT(response, (uint32_t)responseLength); 
            CleanResponse();
  /*
            for(int g = 0; g < 10; g++){
                bool cc1m = nfc.inDataExchange(cmd1, sizeof(cmd1), &response[0], &responseLength);
                if(cc1m){
                  if(response[0] == 0xAF){
                      Serial.println("BEGIN AUTH OK");
                      PrintHexH(response, (uint32_t)responseLength);
                  }else{
                      PrintHexT(response, (uint32_t)responseLength);
                  }
                }else{
                  Serial.println("ERROR AUTH");
                }
                CleanResponse();
            }
            */
        }
        
        // Wait for CMD.
        
        exitLoop = false;
        entireCMD = false;        
        startTime = millis();
        while( ! exitLoop){
            if(millis() - startTime > 5000){
                exitLoop = true;
                Serial.println("----- EXIT 5s ------");
            }

            waitCommand();

            if(entireCMD){
                entireCMD = false;
                startTime = millis();
                executeCmd();    
            }
        }
    }else{
        Serial.println("ERROR WHEN SELECTING APP. wrong status");
    }
/*
    delay(500);
    // wait until the card is taken away
    while (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength)){
        yield();
    } 
*/    
    Serial.println("\n############################################################################\n");
  }
  else yield();
}


void CleanResponse(){
  responseLength = 50;
  for(int i = 0; i < 50; i++){
    response[i] = 0x00;
  }  
}
void PrintHexT(const uint8_t *data, const uint32_t numBytes){
    for (uint8_t i = 0; i < numBytes; i++) {
        if (data[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(data[i], HEX);
    }
    Serial.println("");  
}

void PrintHexH(const uint8_t *data, const uint32_t numBytes){
    Serial.print(">");
    for (uint8_t i = 0; i < numBytes; i++) {
        if (data[i] < 0x10) {
            Serial.print("0");
        }
        Serial.print(data[i], HEX);
    }
    Serial.println("<");  
}


void executeCmd(){
    if(cmdLength == 0){
        Serial.println("ERROR len( cmd ) == 0");
    }
    bool cc = nfc.inDataExchange(cmdData, cmdLength / 2, &response[0], &responseLength);
    if(cc){
        Serial.print(">");
        Serial.print(cmdData[0], HEX);
        PrintHexH(response, (uint32_t)responseLength);
        if(response[0] == 0x91 && response[1] == 0xCA){
            prevCmdAASucsessful = false;  
        }else{
            if(responseLength > 10){
                prevCmdAASucsessful = true;  
            }else{
                prevCmdAASucsessful = false;  
            }
        }
    }else{
        exitLoop = true;
    }
    entireCMD = false;
    CleanResponse();
    cmdLength = 0;
}

void waitCommand(){
    while (Serial.available()) {
       char c = Serial.read();
       
       if(c == '+'){ // start
          startCmd = true;
          endCmd = false;
          readCMD = "";
          cmdLength = 0;
          // clean cmdData
          for(int i = 0; i < 100; i++){
            cmdData[i] = 0x00;
          }
       }else if(c == '*'){ // end
          endCmd = true;
          if(startCmd && endCmd){ // we have entire CMD !
              startCmd = false;  
              endCmd = false;
              entireCMD = true;
              PrintHexT(cmdData, cmdLength/2);
              //break;
          }
       }else if(c == 'X'){
          exitLoop = true;
          Serial.println("EXIT");
       }else if(c == '\n' || c == '\r'){
          /*if(!startCmd){
            startCmd = true;
            endCmd = true;
            if(prevCmdAASucsessful){
              cmdLength = 0;
              for(int i = 0; i < sizeof(cmd2); i++){
                cmdData[i] = cmd2[i];
                cmdLength += 2;
              }
            }else{*/
              /*
              cmdLength = 14;
              cmdData[0] = 0x90;
              cmdData[1] = 0xaa;
              cmdData[2] = 0x00;
              cmdData[3] = 0x00;
              cmdData[4] = 0x01;
              cmdData[5] = 0x01;
              cmdData[6] = 0x00;
              */
            /*  cmdLength = 4;
              cmdData[0] = 0xaa;
              cmdData[1] = 0x01;
            }
          }*/
       }else{
          if(! entireCMD){
              readCMD += c;
              cmdLength++;
              if(cmdLength%2 == 0){
                cmdData[(cmdLength/2)-1] = char2int(readCMD[cmdLength-2])*16 + char2int(c);
              }
          }else{
            Serial.println(c);
            Serial.println("e char !");
          }
       }
   }
}

int char2int(char input)
{
  if(input >= '0' && input <= '9')
    return input - '0';
  if(input >= 'A' && input <= 'F')
    return input - 'A' + 10;
  if(input >= 'a' && input <= 'f')
    return input - 'a' + 10;
  Serial.println("ERROR char2int");
}
