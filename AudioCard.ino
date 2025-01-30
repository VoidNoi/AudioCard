#include <FS.h>
#include <Wire.h>
#include <SD.h>
#include "icons.h"
#include <SPI.h>
#include <M5Cardputer.h>
#include "src/Audio/src/Audio.h"

Audio audio;

#define I2S_DOUT      42
#define I2S_BCLK      41
#define I2S_LRC       43

#define display M5Cardputer.Display
#define kb M5Cardputer.Keyboard

const int maxFiles = 200;

File myFile;
String root = "/AudioCard";
String path = root;
String pathArray[maxFiles] = {root};
int pathLen = 0;

int fileAmount;

int mainCursor = 0;
int fileCursor = 0;

String sdFiles[maxFiles] = {"NEW FOLDER"};
int fileType[maxFiles] = {2};

const int fileOptionsAmount = 2;
String fileMenuOptions[fileOptionsAmount] = {"Play song", "Delete song"};

int letterHeight = 16;
int letterWidth = 12;

int cursorPosX, cursorPosY, screenPosX, screenPosY = 0;

bool isPlaying = true;
bool isStopped = false;

bool deletingFolder = false;
bool pathChanged = true;

TaskHandle_t handleAudioTask = NULL;

void bootLogo(){
  
  M5Canvas backgroundSprite(&display);
  M5Canvas versionSprite(&display);
  M5Canvas logoSprite(&display);
  M5Canvas fwNameSprite(&display);
  M5Canvas pressKeySprite(&display);

  unsigned long t;

  t = millis();

  char version[] = "v1.0.0";
  int versionLength = sizeof(version)-1;
  int versionWidth = versionLength*letterWidth/2+3;
  int versionHeight = letterHeight/2+2;

  char fwName[] = "AudioCard";
  int fwNameLength = sizeof(fwName)-1;
  double fwNamePosX = fwNameLength % 2 == 1 ? (display.width()/2-letterWidth/2)-(fwNameLength/2)*letterWidth : display.width()/2-(fwNameLength/2)*letterWidth;
  //int fwNamePosY = display.height()/2 - 50;
  int fwNamePosY = letterHeight/2;
  
  char pressKeyText[] = "Press any key...";
  int pressKeyTextLength = sizeof(pressKeyText)-1;
  int pressKeyPosX = display.width()/2 - 95;
  //int pressKeyPosY = display.height()/2 + 40;
  int pressKeyPosY = display.height() - letterHeight-4;
  
  int offset = 8;
  
  backgroundSprite.createSprite(display.width()+letterWidth*2, display.height());
  versionSprite.createSprite(versionWidth, versionHeight);
  logoSprite.createSprite(display.width()/2 + 70, display.height()/2-10);
  fwNameSprite.createSprite(fwNameLength*letterWidth+10, letterHeight+10);
  pressKeySprite.createSprite(pressKeyTextLength*letterWidth+10, letterHeight+10);

  int move = 0;
  int mosaicSpriteSize = letterWidth*2.5; // Change the number to adjust the distance between the mosaic's sprites
    
  while(true) {
    //Background animation loop
    backgroundSprite.setTextSize(2);
    backgroundSprite.fillSprite(BLACK);
    for(int x = 0; x < 15; x++) {
      for (int y = 0; y < 15; y++) {
        //Use a character
        //backgroundSprite.setCursor(x*letterWidth*1.5-move, y*letterWidth*1.5-move);
        //backgroundSprite.setTextColor(0x5c0a5cU);
        //backgroundSprite.println("+");

        //Or use an image
        backgroundSprite.setSwapBytes(true);
        backgroundSprite.pushImage(x*mosaicSpriteSize-move, y*mosaicSpriteSize-move, 16, 16, (uint16_t *)icons[8]);
      }
    }
    for (int x = 0; x <= display.width()+letterWidth; x++) {
      for (int y = 0; y <= display.height(); y++) {
        if (x % 2 == 0 || y % 2 == 0) {
          backgroundSprite.drawPixel(x, y, BLACK);
        }
      }
    }

    if (move >= mosaicSpriteSize) {
      move = 0;
    }
    //Slow down animation
    if (millis() -t >= 80) {
      move++;
      t = millis();
    }

    //Prints logo
    logoSprite.setTextSize(1);
    logoSprite.setTextColor(PURPLE);
    /* logoSprite.drawLine(1, 1, 1, display.height()/2-12, PURPLE); */
    /* logoSprite.drawLine(1, display.height()/2-12, display.width()/2+60+offset, display.height()/2-12, PURPLE); */
    /* logoSprite.drawLine(display.width()/2+60 + offset, display.height()/2-12, display.width()/2+68, 1, PURPLE); */
    /* logoSprite.drawLine(display.width()/2+60 + offset, 1, 1, 1, PURPLE); */
    logoSprite.setCursor(offset, 2);
    logoSprite.println(" __   __     ______     __    ");
    logoSprite.setCursor(offset, 12);
    logoSprite.println("/\\\ \"-.\\ \\   /\\  __ \\   /\\ \\   ");
    logoSprite.setCursor(offset, 22);
    logoSprite.println("\\ \\ \\-.  \\  \\ \\ \\/\\ \\  \\ \\ \\  ");
    logoSprite.setCursor(offset, 32);
    logoSprite.println(" \\ \\_\\\\\"\\_\\  \\ \\_____\\  \\ \\_\\ ");
    logoSprite.setCursor(offset, 42);
    logoSprite.println("  \\/_/ \\/_/   \\/_____/   \\/_/ ");
    
    //Prints version
    versionSprite.setTextSize(1);
    versionSprite.setTextColor(PURPLE);
    versionSprite.setCursor(0, 0);
    versionSprite.drawLine(0, versionHeight-2, versionWidth-2, versionHeight-2, PURPLE);
    versionSprite.drawLine(versionWidth-2, versionHeight-2, versionWidth-2, 0, PURPLE);
    versionSprite.println(version);
    
    //Prints firmware name
    fwNameSprite.setTextSize(2);
    fwNameSprite.setTextColor(PURPLE);
    fwNameSprite.setCursor(offset-2, offset-2);
    /* fwNameSprite.drawLine(1, 1, 1, letterHeight+offset, PURPLE); */
    fwNameSprite.drawLine(1, letterHeight+offset, fwNameLength*letterWidth+offset, letterHeight+offset, PURPLE);
    /* fwNameSprite.drawLine(fwNameLength*letterWidth+offset, letterHeight+offset, fwNameLength*letterWidth+offset, 1, PURPLE); */
    /* fwNameSprite.drawLine(fwNameLength*letterWidth+offset, 1, 1, 1, PURPLE); */
    fwNameSprite.println(fwName);
    
    //Prints press any key
    pressKeySprite.setTextSize(2);
    pressKeySprite.setTextColor(PURPLE);
    pressKeySprite.setCursor(offset-2, offset-2);
    pressKeySprite.drawLine(1, 1, 1, letterHeight+offset, PURPLE);
    pressKeySprite.drawLine(1, letterHeight+offset, pressKeyTextLength*letterWidth+offset, letterHeight+offset, PURPLE);
    pressKeySprite.drawLine(pressKeyTextLength*letterWidth+offset, letterHeight+offset, pressKeyTextLength*letterWidth+offset, 1, PURPLE);
    pressKeySprite.drawLine(pressKeyTextLength*letterWidth+offset, 1, 1, 1, PURPLE);
    pressKeySprite.println(pressKeyText);
    
    // Pushes the sprites onto the canvas
    versionSprite.pushSprite(&backgroundSprite, letterWidth, 0);
    logoSprite.pushSprite(&backgroundSprite, display.width()/2 - 95 + letterWidth, display.height()/2 - 30);
    fwNameSprite.pushSprite(&backgroundSprite, fwNamePosX + letterWidth-5, fwNamePosY-10);
    pressKeySprite.pushSprite(&backgroundSprite, pressKeyPosX + letterWidth-5, pressKeyPosY-5);
    backgroundSprite.pushSprite(-letterWidth, 0);
    
    M5Cardputer.update();
    
    // If any key is pressed go to main menu
    if (kb.isChange()) {
      delay(100);
      return;
    }
  }
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  SPI.begin(
    M5.getPin(m5::pin_name_t::sd_spi_sclk),
    M5.getPin(m5::pin_name_t::sd_spi_miso),
    M5.getPin(m5::pin_name_t::sd_spi_mosi),
    M5.getPin(m5::pin_name_t::sd_spi_ss));
  
  while (false == SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI)) {
    delay(1);
  }

  if (!SD.exists(root)) {
    SD.mkdir(root);
  }

  display.setRotation(1);
  display.setTextColor(PURPLE);
  
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(10);
  
  //Manually assigning the audio to a core allows audio and video to be played at the same time
  xTaskCreatePinnedToCore(audioTask, "Audio Task", 10240, NULL, 3, &handleAudioTask, 1);
  bootLogo();
  handleFolders();
}

void audioTask(void *pvParameters) {
    while (true) {
        if (isPlaying) {
            while (isPlaying) {
                if(!isStopped)
                audio.loop();
                vTaskDelay(1);
            }
        } else {
            isPlaying=true;
        }
    }
}

void printMenu(int cursor, String* strings, int stringsAmount, int screenDirection, bool addIcons) {
  int textPosX = addIcons ? 40 : 20;
  for (int i = 0; i <= stringsAmount; i++) {
    int prevString = i-screenDirection;
    
    if (screenDirection != 0) {
      if (addIcons){
        display.setSwapBytes(true);
        display.fillRect(20, i*20, 16, 16, BLACK);
        display.pushImage(20, i*20, 16, 16, (uint16_t *)icons[fileType[i+screenPosY]]);
      }
      display.setTextColor(BLACK);
      display.drawString(strings[prevString+screenPosY], textPosX, i*20);
          
      display.setTextColor(PURPLE);
      display.drawString(strings[i+screenPosY], textPosX, i*20);
    } else {
      if (addIcons){
        display.setSwapBytes(true);
        display.pushImage(20, i*20, 16, 16, (uint16_t *)icons[fileType[i]]);
      }
      display.setTextColor(PURPLE);
      display.drawString(strings[i], textPosX, i*20);
    }
  }
}

void getDirectory(String directory, int &amount, String* fileArray, int* types) {
  File dir = SD.open(directory);

  String tempFilesArray[maxFiles];
  int tempFilesAmount = 0;
  int tempDirsAmount = amount;

  while (true) {
    File entry =  dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }

    String fileName = entry.name();

    if (entry.isDirectory()) {
      fileArray[tempDirsAmount] = fileName; // Store directories first
      types[tempDirsAmount] = 6; // Set type directory
      tempDirsAmount++;
    } else {
      tempFilesArray[tempFilesAmount] = fileName; // Store files in a temporary array
      tempFilesAmount++;
    }
    amount++;
    
    if (amount > maxFiles) {
      display.fillScreen(BLACK);
      display.println("Can't store any more files");
    }
  }
  int filesCount = 0;
  // Set files after directories and set the type to file
  for(int i = tempDirsAmount; i < amount; i++) {
    if (tempFilesAmount > 0) {
      fileArray[i] = tempFilesArray[filesCount];
      types[i] = 5;
      filesCount++;
    }
  }
}

void getCurrentPath() {
  path = "";
  for (int i = 0; i <= pathLen; i++) {
    path += pathArray[i];
  }
}

void deleteFolderMenu() {
  int deleteCursor = 1;
  int deleteCursorPosX = display.width()/2+2*letterWidth/2+letterWidth;
  //Displays the delete folder menu
  display.fillScreen(BLACK); 
  display.setCursor(display.width()/2-15*letterWidth/2, display.height()/2-letterHeight*3);
  display.println("Deleting folder");
  display.setCursor(display.width()/2-12*letterWidth/2, display.height()/2-letterHeight/2*3);
  display.println("Are you sure?");
  display.setCursor(display.width()/2-10*letterWidth/2, display.height()/2);
  display.println("Yes     No");
  // Handles menu controls
  while (true) {
    M5Cardputer.update();
    if (kb.isChange()) {
      display.setTextColor(BLACK);
      display.drawString(">", deleteCursorPosX, display.height()/2);
      // "," is pressed, select yes
      if (kb.isKeyPressed(',')) {
        deleteCursor = 0;
        
        deleteCursorPosX = display.width()/2-10*letterWidth/2-letterWidth;
      }
      // "/" is pressed, select no
      if (kb.isKeyPressed('/')) {
        deleteCursor = 1;

        deleteCursorPosX = display.width()/2+2*letterWidth/2+letterWidth;
      }
      
      display.setTextColor(PURPLE);
      display.drawString(">", deleteCursorPosX, display.height()/2);

      // Cursor is on yes ("0"), attempt to delete folder
      if (kb.isKeyPressed(KEY_ENTER)) {
        if (deleteCursor == 0) {
          String folderPath = path + "/" + sdFiles[mainCursor];
          File dir = SD.open(folderPath);
          // The folders have files inside, try to delete them
          if (!SD.rmdir(folderPath)){
            while (true) {
              File folderFile =  dir.openNextFile();
              if (!folderFile) { // No more files
                if (SD.rmdir(folderPath)) { // All files have been removed, delete folder
                  display.fillScreen(BLACK);
                  display.setCursor(1,1);
                  display.println("Folder successfully deleted");
                } else { // The folder couldn't be removed for some unknown reason
                  display.fillScreen(BLACK);
                  display.setCursor(1,1);
                  display.println("Folder couldn't be deleted");
                }

                mainCursor = 0;
                break;
              }
              
              String folderFileName = folderFile.name();
              // If the file is a directory stop the function and let the user remove it manually
              // This is done because I haven't implemented recursive file/folder deletion
              if (folderFile.isDirectory()) {
                display.fillScreen(BLACK);
                display.setCursor(1,1);
                display.println("Remove folders first");
                mainCursor = 0;
                break;
              } else { // The file couldn't be removed for some unknown reason
                if (!SD.remove(folderPath + "/" + folderFileName)) {
                  display.fillScreen(BLACK);
                  display.setCursor(1,1);
                  display.println("Couldn't remove a file");
                }
              }
            }
          } else { // If the folder doesn't have any files inside, delete it
            display.fillScreen(BLACK);
            display.setCursor(1,1);
            display.println("Folder successfully deleted");
          }
          dir.close();
          delay(1500);
        }
        // Return to current folder
        handleFolders();
        break;
      }
    }
  }
}

void deleteFile() {
  String fileName = path + "/" + sdFiles[mainCursor];
  
  display.fillScreen(BLACK);
  display.setCursor(1,1);
  display.println("Deleting file...");
  if (SD.remove(fileName)) {
    display.println("File deleted");
    sdFiles[fileAmount] = '\0';
    fileAmount--;
    delay(1000);
  } else {
    display.println("File couldn't be deleted");
    delay(1000);
  }
  return;
}

void secondsToTime(unsigned long totalSeconds, char *buffer, bool hourLong) {
  unsigned long hours = totalSeconds / 3600UL;
  unsigned long remaining = totalSeconds % 3600UL;
  unsigned long minutes = remaining / 60UL;
  unsigned long seconds = remaining % 60UL;
  if (hourLong) {   
    sprintf(buffer, "%02lu:%02lu:%02lu", hours, minutes, seconds);
  } else {
    sprintf(buffer, "%02lu:%02lu", minutes, seconds);
  }
}

void playSong() {
  M5Canvas backgroundSprite(&display);
  M5Canvas playingSprite(&display);
  backgroundSprite.createSprite(display.width(), display.height());
  playingSprite.createSprite(display.width(), letterHeight*2);
  
  String fileName = path + "/" + sdFiles[mainCursor];
  audio.connecttoFS(SD, fileName.c_str());     // SD
  int durationHour, durationMin, durationSec;
  char fixedFileDuration[50];
  char fixedFileCurrent[50];

  while(true) {
    if (audio.getAudioFileDuration() >= 3600) {
      secondsToTime(audio.getAudioFileDuration(), fixedFileDuration, true);
      secondsToTime(audio.getAudioCurrentTime(), fixedFileCurrent, true);
    } else {
      secondsToTime(audio.getAudioFileDuration(), fixedFileDuration, false);
      secondsToTime(audio.getAudioCurrentTime(), fixedFileCurrent, false);
    }
    backgroundSprite.fillSprite(BLACK);
    playingSprite.fillSprite(BLACK);
    playingSprite.setTextColor(PURPLE);
    playingSprite.setTextSize(1);
    playingSprite.setCursor(0, 0);
    
    playingSprite.println(fixedFileDuration);
    playingSprite.setCursor(0, letterHeight);
    playingSprite.println(fixedFileCurrent);
    playingSprite.pushSprite(&backgroundSprite, 10, 0);
    backgroundSprite.pushSprite(0, 0);

    M5Cardputer.update();
    
    // If any key is pressed go to main menu
    if (kb.isChange()) {
      if (kb.isKeyPressed('`')){
        delay(100);
        return;
      }
    }
  }
}

void fileOptions() {
  if (fileCursor == 0) {
    playSong();
    return;
  }
  else if (fileCursor == 1) {
    deleteFile();
    return;
  }
}

void newFolder() {
  String fileName = "";

  display.fillScreen(BLACK);

  display.setCursor(display.width()/2-(12/2)*letterWidth, 0);
  display.println("Folder Name:");
  display.drawString(fileName, display.width()/2-(fileName.length()/2)*letterWidth, letterHeight);
  
  while(true) {
    M5Cardputer.update();
    if (kb.isChange()) {
      if (kb.isPressed()) {
        Keyboard_Class::KeysState status = kb.keysState();
        
        for (auto i : status.word) {
          fileName += i;
        }
        
        if (status.del) {
          fileName.remove(fileName.length() - 1);  
        }
        
        display.fillScreen(BLACK);
        
        String newFileName = "Folder Name:";
        
        display.setCursor(display.width()/2-(newFileName.length()/2)*letterWidth, 0);
        display.println(newFileName);
        display.drawString(fileName, display.width()/2-(fileName.length()/2)*letterWidth, letterHeight);
        
        if (status.enter) {
          //Make folder
          getCurrentPath();
          SD.mkdir(path + "/" + fileName);
          return;
        }
      }
    }
  }
  return;
}

void mainOptions() {
  switch (fileType[mainCursor]) {
  case 2: // New Folder
    delay(200);
    newFolder();
    break;
  case 5: // File
    fileType[0] = 8;
    fileType[1] = 9;
    fileType[2] = 10;
    handleMenus(fileOptionsAmount-1, fileOptions, fileCursor, fileMenuOptions, true);
    break;
  case 6: // Folder
    // If G0 button is pressed show delete folder menu
    if (digitalRead(0)==0) {
      deletingFolder = true;
      deleteFolderMenu();
    } else {
      pathLen++;
      //pathChanged = true;
    }
    break;
  case 7: // Previous folder
    sdFiles[pathLen] = '\0';
    pathLen--;
    //pathChanged = true;
    break;
  }
  return;
}

void handleFolders() {
  display.setTextSize(2);
  while (pathChanged) {
    if (pathArray[pathLen] != root) {
      fileAmount = 2;
      if (mainCursor > 0 && fileType[fileCursor] < 8 && !deletingFolder) {
        pathArray[pathLen] = "/" + sdFiles[mainCursor];
      }
      getCurrentPath();
      sdFiles[0] = "..";
      sdFiles[1] = "NEW FOLDER";
      fileType[0] = 7;
      fileType[1] = 2;
    } else {
      fileAmount = 1;
      path = root;
      pathLen = 0;
      
      sdFiles[0] = "NEW FOLDER";
      fileType[0] = 2;
    }
    
    getDirectory(path, fileAmount, sdFiles, fileType);
    // Empties an extra value at the end to prevent previous files from appearing in the menu
    sdFiles[fileAmount] = '\0';
    fileType[fileAmount] = '\0';
    
    mainCursor = 0;
    deletingFolder = false;
    //pathChanged = false;
    handleMenus(fileAmount-1, mainOptions, mainCursor, sdFiles, true);
  }
}

void handleMenus(int options, void (*executeFunction)(), int& cursor, String* strings, bool addIcons) {
  display.fillScreen(BLACK);
  cursor = 0;
  int screenDirection; // -1 = down | 0 = none | 1 = up;
  while (true) {
    M5Cardputer.update();
    if (kb.isChange()) {
      if (screenPosY == 0) {
        screenDirection = 0;
      }
      display.setTextColor(BLACK);
      int drawCursor = cursor*20;
        
      display.drawString(">", 5, drawCursor);

      if (kb.isKeyPressed(';') && cursor > 0){
        cursor--;

        if (screenPosY > 0 && cursor > 0) {
          screenPosY--;
          screenDirection = -1;
        }
      } else if (kb.isKeyPressed('.') && cursor < options) {
        cursor++;

        if (cursor * 20 >= display.height() - 20) {
          screenPosY++;
          screenDirection = 1;
        }
      }
      
      drawCursor = cursor*20;
      if (cursor * 20 > display.height()-20) {
        drawCursor = (display.height() - 20) - 15;
      }

      display.setTextColor(PURPLE);

      display.drawString(">", 5, drawCursor);

      printMenu(cursor, strings, options, screenDirection, addIcons);

      if (kb.isKeyPressed(KEY_ENTER)) {
        screenPosY = 0;
        delay(100);

        executeFunction();
        screenPosY = 0;
        return;
      }
    }
  }
}

void loop() {
}
