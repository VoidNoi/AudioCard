#include <FS.h>
#include <Wire.h>
#include <SD.h>
#include "icons.h"
#include <SPI.h>
#include <M5Cardputer.h>
#include "Audio.h"

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
String currentDir = "root";
int pathLen = 0;

int fileAmount;
int dirAmount;

int mainCursor = 0;
int fileCursor = 0;

String sdFiles[maxFiles];
int fileType[maxFiles];

int letterHeight = 16;
int letterWidth = 12;

int cursorPosX, cursorPosY, screenPosX, screenPosY = 0;

bool isPlaying = true;
bool isStopped = false;

bool pathChanged = true;

int volume = 10;

int textMenuPosY = 60;

int currentAudioNum = 0;

TaskHandle_t handleAudioTask = NULL;

M5Canvas menuBackgroundSprite(&display);
M5Canvas menuSprite(&display);

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

void audioTask(void *pvParameters);
void handleFolders();
void handleMenus(int options, void (*executeFunction)(), int& cursor, String* strings, bool addIcons);

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
  audio.setVolume(volume);
  audio.setTone(1, 2, -10);
  
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

void displayTopBar() {
  M5Canvas topBar(&display);
  
  topBar.createSprite(display.width(), letterHeight/2+6);
  
  currentDir = pathArray[pathLen];

  topBar.drawRect(0,0, display.width()-1, letterHeight/2+6, PURPLE);

  String prevPath = pathLen >= 1 ? pathArray[pathLen-1] : pathArray[0];
  topBar.setTextSize(1);
  topBar.setTextColor(BLACK);
  topBar.drawString(prevPath, 2, 2);
  topBar.setTextColor(PURPLE);
  topBar.drawString(pathArray[pathLen], 2, 4);
  
  float battery = M5.Power.getBatteryLevel();
  
  topBar.drawRect(display.width()-34, 3, 26, 8);
  int batteryPercent = battery*22/100;
  topBar.fillRect(display.width()-32, 5, batteryPercent, 4);
  topBar.fillRect(display.width()-8, 5, 2, 4);  

  topBar.pushSprite(&menuBackgroundSprite, 0, 0);
}

void printMenu(int cursor, String* strings, int stringsAmount, int screenDirection, bool addIcons) {
  int textPosX = addIcons ? 40 : 20;

  menuSprite.drawRect(0,display.height()/2-letterHeight/2-2,menuSprite.width()-1, letterHeight+6, PURPLE);

  for (int i = 0; i <= stringsAmount; i++) {
    int prevString = i-screenDirection;
    
    if (screenDirection != 0) {
      if (addIcons){
        menuSprite.setSwapBytes(true);
        menuSprite.fillRect(20, textMenuPosY+i*20, 16, 16, BLACK);
        menuSprite.pushImage(20, textMenuPosY+i*20, 16, 16, (uint16_t *)icons[fileType[i]]);
      }

      if (i == 0 && screenDirection == -1) {
        if (addIcons) menuSprite.fillRect(20, textMenuPosY-20, 16, 16, BLACK);
        menuSprite.setTextColor(BLACK);
        menuSprite.drawString(strings[i], textPosX, textMenuPosY-20);
      }

      if (i == stringsAmount && screenDirection == 1) {
        if (addIcons) menuSprite.fillRect(20, textMenuPosY + (stringsAmount+1) * 20, 16, 16, BLACK);
        menuSprite.setTextColor(BLACK);
        menuSprite.drawString(strings[i], textPosX, textMenuPosY + (stringsAmount+1) * 20);
      }
      
      menuSprite.setTextColor(BLACK);
      menuSprite.drawString(strings[prevString], textPosX, textMenuPosY+i*20);
      
      menuSprite.setTextColor(PURPLE);
      menuSprite.drawString(strings[i], textPosX, textMenuPosY+i*20);
    } else {
      if (addIcons){
        menuSprite.setSwapBytes(true);
        menuSprite.pushImage(20, textMenuPosY+i*20, 16, 16, (uint16_t *)icons[fileType[i]]);
      }
      menuSprite.setTextColor(PURPLE);
      menuSprite.drawString(strings[i], textPosX, textMenuPosY+i*20);
    }
  }

  menuSprite.pushSprite(&menuBackgroundSprite, 0, 0);
  displayTopBar();
  menuBackgroundSprite.pushSprite(0,0);
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
  dirAmount = tempDirsAmount;
}

void getCurrentPath() {
  path = "";
  for (int i = 0; i <= pathLen; i++) {
    path += pathArray[i];
  }
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

bool show = false;
unsigned long currentTime;
unsigned long startTime = millis();
unsigned long currentTimeText;
unsigned long startTimeText = millis();

void volumeBar(M5Canvas background) {
  //If volume changes, slide in a volume bar modal showing a vertical bar that gets filled depending on the volume value. After 2 seconds of no change, hide the modal by sliding out 
  M5Canvas volBarSprite(&display);
  
  volBarSprite.createSprite(24, 104);

  volBarSprite.fillSprite(BLACK);
  volBarSprite.setTextColor(PURPLE);
  volBarSprite.setTextSize(1);

  volBarSprite.setCursor(4, 2);
  volBarSprite.println("Vol");
  
  volBarSprite.drawRect(0,0,volBarSprite.width(), volBarSprite.height(), PURPLE);
  
  volBarSprite.drawRect(volBarSprite.width()/2-5, letterHeight/2+4, 10, 84, PURPLE);
  
  if (volume > 0) {
    volBarSprite.fillRect(volBarSprite.width()/2-4, letterHeight/2+4 + 84, 8, -volume *4, PURPLE);
  }
  
  volBarSprite.pushSprite(&background, display.width() - 25, display.height() / 2 - 52);

  if (currentTime - startTime >= 1000) {
    show = false;
    startTime = currentTime;
  }
}

void scrollText(M5Canvas sprite, String name, int& x, int y) {
  currentTimeText = millis();
  sprite.drawString(name, x, y);
  
  int namePixelLength = name.length()*letterWidth;
  int diff = display.width()-namePixelLength;

  if (diff < 1 && x > diff && currentTimeText - startTimeText >= 2000) {
    x = x - 2;
  } else if (currentTimeText - startTimeText >= 5000) {
    x = 1;
    startTimeText = currentTimeText;
  }
}

void audioPlayingScreen() {
  //M5Canvas backgroundSprite(&display);
  //M5Canvas menuSprite(&display);

  menuBackgroundSprite.createSprite(display.width(), display.height());
  menuSprite.createSprite(display.width(), display.height());
  
  int durationHour, durationMin, durationSec;
  char fixedFileDuration[50];
  char fixedFileCurrent[50];

  currentAudioNum = 0;
  int fileNameXPos = 1;

  startTimeText = millis();
  
  while(true) {
    String filePlayingName = sdFiles[mainCursor+currentAudioNum];
    uint32_t act = audio.getAudioCurrentTime();
    uint32_t afd = audio.getAudioFileDuration();
    if (afd >= 3600) {
      if (act > afd) {
        secondsToTime(act, fixedFileDuration, true);
      } else {
        secondsToTime(afd, fixedFileDuration, true);
      }
      secondsToTime(act, fixedFileCurrent, true);
    } else {
      if (act > afd) {
        secondsToTime(act, fixedFileDuration, false);
      }
      else {
        secondsToTime(afd, fixedFileDuration, false);
      }
      secondsToTime(act, fixedFileCurrent, false);
    }
    
    menuBackgroundSprite.fillSprite(BLACK);
    
    menuSprite.fillSprite(BLACK);
    menuSprite.setTextColor(PURPLE);

    menuSprite.setTextSize(2);

    scrollText(menuSprite, filePlayingName, fileNameXPos, display.height()/2-letterHeight);
    
    menuSprite.setTextSize(1);

    menuSprite.setCursor(display.width()-strlen(fixedFileDuration)*letterWidth/2-2, display.height()/2+letterHeight/2+4);
    menuSprite.println(fixedFileDuration);
    
    menuSprite.setCursor(0, display.height()/2+letterHeight/2+4);
    menuSprite.println(fixedFileCurrent);

    menuSprite.drawRect(0, display.height()/2, display.width()-2, 10, PURPLE);

    int barFillGauge = act * display.width()/afd;

    if (act > 0) {
      menuSprite.fillRect(1, display.height()/2 + 1, barFillGauge, 8, PURPLE);
    }

    menuSprite.pushSprite(&menuBackgroundSprite, 0, 0);

    currentTime = millis();

    if (kb.isChange()) {
      // pause/resume song
      if (kb.isKeyPressed(KEY_ENTER)) {
        audio.pauseResume(); 
      }
      // volume up
      if (kb.isKeyPressed(';') && volume < 21) {
        show = true;
        startTime = currentTime;
        volume++;
        audio.setVolume(volume);
      }
      // volume down
      if (kb.isKeyPressed('.') && volume > 0) {
        show = true;
        startTime = currentTime;
        volume--;
        audio.setVolume(volume);
      }
      // previous audio
      if (kb.isKeyPressed(',') && mainCursor + currentAudioNum > dirAmount) {
        fileNameXPos = 1;
        startTimeText = millis();
        currentAudioNum--;
        String fullFileName = path + "/" + sdFiles[mainCursor+currentAudioNum];
        audio.stopSong();
        audio.connecttoSD(fullFileName.c_str());
      }
      // next audio
      if (kb.isKeyPressed('/') && mainCursor + currentAudioNum < fileAmount-1) {
        fileNameXPos = 1;
        startTimeText = millis();
        currentAudioNum++;
        String fullFileName = path + "/" + sdFiles[mainCursor+currentAudioNum];
        audio.stopSong();
        audio.connecttoSD(fullFileName.c_str());
      }
      // If esc key is pressed stop song and go to menu
      if (kb.isKeyPressed('`')){
        audio.stopSong();
        delay(100);
        return;
      }

      Keyboard_Class::KeysState status = kb.keysState();
      // If del key is press go to menu
      if (status.del) {
        return;
      }
    }
    
    if (show) {
      volumeBar(menuBackgroundSprite);
    }
    
    displayTopBar();
    menuBackgroundSprite.pushSprite(0, 0);
    
    M5Cardputer.update();

    if (currentAudioNum > fileAmount - dirAmount) {
      return;
    }
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
  String fileName = path + "/" + sdFiles[mainCursor];
  switch (fileType[mainCursor]) {
  case 5: // File
    audio.connecttoSD(fileName.c_str());     // SD
    audioPlayingScreen();
    break;
  case 6: // Folder
    pathLen++;
    break;
  case 7: // Previous folder
    sdFiles[pathLen] = '\0';
    pathLen--;
    break;
  }
  return;
}

void handleFolders() {
  display.setTextSize(2);
  while (pathChanged) {
    if (pathArray[pathLen] != root) {
      fileAmount = 1;
      if (mainCursor >= 0 && fileType[fileCursor] < 8) {
        pathArray[pathLen] = "/" + sdFiles[mainCursor];
      }
      getCurrentPath();
      sdFiles[0] = "..";
      //sdFiles[1] = "NEW FOLDER";
      fileType[0] = 7;
      //fileType[1] = 2;
    } else {
      fileAmount = 0;
      path = root;
      pathLen = 0;
      
      //sdFiles[0] = "NEW FOLDER";
      //fileType[0] = 2;
    }
    
    getDirectory(path, fileAmount, sdFiles, fileType);
    // Empties an extra value at the end to prevent previous files from appearing in the menu
    sdFiles[fileAmount] = '\0';
    fileType[fileAmount] = '\0';
    
    mainCursor = 0;
    //pathChanged = false;
    handleMenus(fileAmount-1, mainOptions, mainCursor, sdFiles, true);
  }
}

void handleMenus(int options, void (*executeFunction)(), int& cursor, String* strings, bool addIcons) {

  menuBackgroundSprite.createSprite(display.width(), display.height());
  menuSprite.createSprite(display.width(), display.height());

  menuSprite.setTextSize(2);
  menuBackgroundSprite.fillScreen(BLACK);
  cursor = 0;
  textMenuPosY = 60;
  int screenDirection; // -1 = down | 0 = none | 1 = up;
  while (true) {
    M5Cardputer.update();
    if (kb.isChange()) {
      if (screenPosY == 0) {
        screenDirection = 0;
      }
      menuSprite.setTextColor(BLACK);
      int drawCursor = cursor*20;
        
      menuSprite.drawString(">", 5, drawCursor);

      if (kb.isKeyPressed(';') && cursor > 0){
        cursor--;

        //if (screenPosY > 0 && cursor > 0) {
          screenPosY--;
          screenDirection = -1;
          textMenuPosY += 20;
          //}
      } else if (kb.isKeyPressed('.') && cursor < options) {
        cursor++;

        //if (60+cursor * 20 >= display.height() - 20) {
          screenPosY++;
          screenDirection = 1;
          textMenuPosY -= 20;
          //}
      }
      
      //drawCursor = cursor*20;
      //if (60 + cursor * 20 > display.height()-20) {
      //drawCursor = (display.height() - 20) - 15;
      //}

      menuSprite.setTextColor(PURPLE);

      menuSprite.drawString(">", 5, 3*20+1);

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

void audio_eof_mp3(const char *info){  //end of file
    Serial.print("audio_info: "); Serial.println(info);
    currentAudioNum++;
    String fileName = path + "/" + sdFiles[mainCursor+currentAudioNum];
    audio.connecttoSD(fileName.c_str());
}
