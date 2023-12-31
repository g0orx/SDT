#ifndef BEENHERE
#include "SDT.h"
#endif
/*
const char *topMenus[] = { "Bearing", "CW Options", "RF Set", "VFO Select",
                           "EEPROM", "AGC", "Spectrum Options",
                           "Noise Floor", "Mic Gain", "Mic Comp",
                           "EQ Rec Set", "EQ Xmt Set", "Calibrate" };

int (*functionPtr[])() = { &BearingMaps, &CWOptions, &RFOptions, &VFOSelect,
                           &EEPROMOptions, &AGCOptions, &SpectrumOptions,
                           &ButtonSetNoiseFloor, &MicGainSet, &MicOptions,
                           &EqualizerRecOptions, &EqualizerXmtOptions, &IQOptions
                        };
*/
/*****
  Purpose: void ShowMenu()

  Parameter list:
    char *menuItem          pointers to the menu
    int where               0 is a primary menu, 1 is a secondary menu

  Return value;
    void
*****/
void ShowMenu(const char *menu[], int where) {
  tft.setFontScale((enum RA8875tsize)1);

  if (menuStatus == NO_MENUS_ACTIVE)  // No menu selected??
    NoActiveMenu();

  if (where == PRIMARY_MENU) {                                             // Should print on left edge of top line
    tft.fillRect(PRIMARY_MENU_X, MENUS_Y, 300, CHAR_HEIGHT, RA8875_BLUE);  // Top-left of display
    tft.setCursor(5, 0);
    tft.setTextColor(RA8875_WHITE);
    tft.print(*menu);  // Primary Menu
  } else {
    tft.fillRect(SECONDARY_MENU_X, MENUS_Y, 300, CHAR_HEIGHT, RA8875_GREEN);  // Right of primary display
    tft.setCursor(35, 0);
    tft.setTextColor(RA8875_WHITE);
    tft.print(*menu);  // Secondary Menu
  }
  return;
}

/*****
  Purpose: To present the encoder-driven menu display

  Argument List;
    void

  Return value: index number for the selected menu

const char *secondaryChoices[][8] = {
  { "WPM", "Key Type", "CW Filter", "Paddle Flip", "Sidetone Vol", "Xmit Delay", "Cancel" },                              // CW Options
  { "Power level", "Gain", "Cancel" },                                                                                    // RF
  { "VFO A", "VFO B", "Split", "Cancel" },                                                                                // VFO
  { "Save Current", "Set Defaults", "Get Favorite", "Set Favorite", "EEPROM-->SD", "SD-->EEPROM", "SD Dump", "Cancel" },  // EEPROM
  { "Off", "Long", "Slow", "Medium", "Fast", "Cancel" },                                                                  // AGC
  { "20 dB/unit", "10 dB/unit", " 5 dB/unit", " 2 dB/unit", " 1 dB/unit", "Cancel" },                                     // Spectrum
  { "Set floor", "Cancel" },                                                                                              // Noise
  { "Set Mic Gain", "Cancel" },                                                                                           // Mic gain
  { "On", "Off", "Set Threshold", "Set Ratio", "Set Attack", "Set Decay", "Cancel" },                                     // Mic Compression
  { "On", "Off", "EQSet", "Cancel" },                                                                                     // Receiver equalizer
  { "On", "Off", "EQSet", "Cancel" },                                                                                     // Transmiiter equilizer
  { "Freq Cal", "CW PA Cal", "Rec Cal", "Xmit Cal", "SSB PA Cal", "Cancel" },                                             // Calibrate
  { "Set Prefix", "Cancel" }                                                                                              // Bearing
};
*****/
int DrawMenuDisplay() {
  int i;
  menuStatus = 0;  // No primary or secondary menu set
//  mainMenuIndex = 0;
  secondaryMenuIndex = 0;

  tft.writeTo(L2);  // Clear layer 2.  KF5N July 31, 2023
  tft.clearMemory();
  tft.writeTo(L1);
  tft.fillRect(1, SPECTRUM_TOP_Y + 1, 513, 379, RA8875_BLACK);  // Show Menu box
  tft.drawRect(1, SPECTRUM_TOP_Y + 1, 513, 378, RA8875_YELLOW);

  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_WHITE);
  for (i = 0; i < TOP_MENU_COUNT; i++) {  // Show primary menu list
    tft.setCursor(10, i * 25 + 115);
    tft.print(topMenus[i]);
  }
  tft.setTextColor(RA8875_GREEN);  // show currently active menu
  tft.setCursor(10, mainMenuIndex * 25 + 115);
  tft.print(topMenus[mainMenuIndex]);

  tft.setTextColor(DARKGREY);

  for (i = 0; i < secondaryMenuCounts[mainMenuIndex]; i++) {  // Show secondary choices
    tft.setCursor(300, i * 25 + 115);
    tft.print(secondaryChoices[mainMenuIndex][i]);
  }
  menuStatus = PRIMARY_MENU_ACTIVE;
  return 0;
}

static void unselectPrimaryMenuIndex(int index) {
  tft.setTextColor(RA8875_WHITE);  // Yep. Repaint the old choice
  tft.setCursor(10, index * 25 + 115);
  tft.print(topMenus[index]);
}

void selectPrimaryMenuIndex(int index) {
  tft.setTextColor(RA8875_GREEN);
  tft.setCursor(10, index * 25 + 115);
  tft.print(topMenus[index]);
  tft.fillRect(SECONDARY_MENU_X, SPECTRUM_TOP_Y + 5, 260, 279, RA8875_BLACK);  // Erase secondary menu list
  tft.setTextColor(DARKGREY);
  for (int i = 0; i < secondaryMenuCounts[index]; i++) {  // Have we read the last entry in secondary menu?
    tft.setCursor(300, i * 25 + 115);
    tft.print(secondaryChoices[index][i]);
  }
  tft.setCursor(300, 115);
  tft.print(secondaryChoices[index][0]);
}

void selectSecondaryMenuIndex(int index) {
  tft.setTextColor(RA8875_GREEN);  // Highlight the first menu option
  tft.setCursor(300, (index*25)+115);
  tft.print(secondaryChoices[mainMenuIndex][index]);
}

void unselectSecondaryMenuIndex(int index) {
  tft.setTextColor(DARKGREY);  // Yep. Repaint the old choice
  tft.setCursor(300, index * 25 + 115);
  tft.print(secondaryChoices[mainMenuIndex][index]);
}

/*****
  Purpose: To select the primary menu

  Argument List:
    void

  Return value: index number for the selected primary menu 
*****/

int SetPrimaryMenuIndex() {
  int i;
  int resultIndex;
  int val;

  i = 0;
  resultIndex = 0;
  //  mainMenuIndex = 0;

#ifdef G0ORX_FRONTPANEL_2
  calibrateFlag=1;
#endif
  while (true) {
    if(touchPrimaryMenuIndex>=0) {
      unselectPrimaryMenuIndex(mainMenuIndex);
      mainMenuIndex = touchPrimaryMenuIndex;
      touchPrimaryMenuIndex = -1;
      if (mainMenuIndex >= TOP_MENU_COUNT) {  // Did they go past the end of the primary menu list?
        mainMenuIndex = TOP_MENU_COUNT - 1;                    // Yep. Set to start of the list.
      }
      selectPrimaryMenuIndex(mainMenuIndex);
    } else if(touchSecondaryMenuIndex>=0) {
      break;
    } else if (filterEncoderMove != 0) {      // Did they move the encoder?
      unselectPrimaryMenuIndex(mainMenuIndex);
      mainMenuIndex += filterEncoderMove;     // Change the menu index to the new value
      if (mainMenuIndex >= TOP_MENU_COUNT) {  // Did they go past the end of the primary menu list?
        mainMenuIndex = 0;                    // Yep. Set to start of the list.
      } else {
        if (mainMenuIndex < 0) {               // Did they go past the start of the list?
          mainMenuIndex = TOP_MENU_COUNT - 1;  // Yep. Set to end of the list.
        }
      }
      selectPrimaryMenuIndex(mainMenuIndex);
      filterEncoderMove = 0;
    }
    val = ReadSelectedPushButton();  // Read the ladder value
    MyDelay(150L);
    if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {  // Did they press Select?
      val = ProcessButtonPress(val);                                      // Use ladder value to get menu choice
      if (val > -1) {                                                     // Valid choice?

        if (val == MENU_OPTION_SELECT) {  // They made a choice
                                          //          tft.setTextColor(RA8875_WHITE);
          break;
        }
        MyDelay(50L);
      }
    }

  }  // End while True

#ifdef G0ORX_FRONTPANEL_2
      calibrateFlag=0;
#endif

  tft.setTextColor(RA8875_WHITE);
  return mainMenuIndex;
}


/*****
  Purpose: To select the secondary menu

  Argument List;
    void

  Return value: index number for the selected primary menu 
*****/
int SetSecondaryMenuIndex() 
{
  int i = 0;
  int secondaryMenuCount = 0;
  int oldIndex = 0;
  int val;
  /*
  while (true) {                                                        // How many secondary menu options?
    if (strcmp(secondaryChoices[mainMenuIndex][i], "Cancel") != 0) {    // Have we read the last entry in secondary menu?
      i++;                                                              // Nope.  
    } else {
      secondaryMenuCount = i + 1;                                       // Add 1 because index starts with 0
      break;
    }
  }
*/
  secondaryMenuCount = secondaryMenuCounts[mainMenuIndex];
  secondaryMenuIndex = 0;  // Change the menu index to the new value
  filterEncoderMove = 0;

  //tft.setTextColor(RA8875_GREEN);  // Highlight the first menu option
  //tft.setCursor(300, 115);
  //tft.print(secondaryChoices[mainMenuIndex][0]);
  selectSecondaryMenuIndex(0);

  i = 0;
  oldIndex = 0;
#ifdef G0ORX_FRONTPANEL_2
  calibrateFlag=1;
#endif
  while (true) {

    if(touchSecondaryMenuIndex>=0) {
      unselectSecondaryMenuIndex(oldIndex);
      oldIndex = touchSecondaryMenuIndex;
      touchSecondaryMenuIndex = -1;
      if (oldIndex >= secondaryMenuCount) {  // Did they go past the end of the primary menu list?
        oldIndex = secondaryMenuCount - 1;                    // Yep. Set last in the list.
      }
      selectSecondaryMenuIndex(oldIndex);
    } else 
    if (filterEncoderMove != 0) {  // Did they move the encoder?
      //tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH + 30, CHAR_HEIGHT, RA8875_BLACK);
      //tft.setTextColor(DARKGREY);  // Yep. Repaint the old choice
      //tft.setCursor(300, oldIndex * 25 + 115);
      //tft.print(secondaryChoices[mainMenuIndex][oldIndex]);
      unselectSecondaryMenuIndex(oldIndex);
      i += filterEncoderMove;  // Change the menu index to the new value

      if (i == secondaryMenuCount) {  // Did they go past the end of the primary menu list?
        i = 0;                        // Yep. Set to start of the list.
      } else {
        if (i < 0) {                   // Did they go past the start of the list?
          i = secondaryMenuCount - 1;  // Yep. Set to end of the list.
        }
      }
      oldIndex = i;
      selectSecondaryMenuIndex(i);
      //tft.setTextColor(RA8875_GREEN);
      //tft.setCursor(300, i * 25 + 115);
      //tft.print(secondaryChoices[mainMenuIndex][i]);
      filterEncoderMove = 0;
    }
    val = ReadSelectedPushButton();  // Read the ladder value
    MyDelay(200L);
    if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);      // Use ladder value to get menu choice
      if (val > -1) {                     // Valid choice?
        if (val == MENU_OPTION_SELECT) {  // They made a choice
          tft.setTextColor(RA8875_WHITE);
          secondaryMenuIndex = oldIndex;
          break;
        }
        MyDelay(50L);
      }
    }
  }  // End while True
#ifdef G0ORX_FRONTPANEL_2
  calibrateFlag=0;
#endif
  return secondaryMenuIndex;
}


/*****
  Purpose: To select the secondary menu

  Argument List;
    char *choice[]                    // The options
    int count                         // Options in the list
    int menuDefault                   // The default or current choice

  Return value:                       // index number of option
*****/
int SecondarySubmenuString(char *options[], int count, int menuDefault) 
{
  int i = menuDefault;
  int val;
#ifdef G0ORX_FRONTPANEL_2
      calibrateFlag=1;
#endif
  while (true) {
    tft.fillRect(SECONDARY_MENU_X, MENUS_Y, EACH_MENU_WIDTH, CHAR_HEIGHT + 1, RA8875_MAGENTA);  // Erase menu choices
    tft.setCursor(SECONDARY_MENU_X, MENUS_Y);
    tft.setTextColor(RA8875_BLACK, RA8875_MAGENTA);
    tft.print(options[i]);

    i += filterEncoderMove;  // Change the menu index to the new value

    if (i == count) {  // Did they go past the end of the primary menu list?
      i = 0;           // Yep. Set to start of the list.
    } else {
      if (i < 0) {      // Did they go past the start of the list?
        i = count - 1;  // Yep. Set to end of the list.
      }
    }
    filterEncoderMove = 0;

    val = ReadSelectedPushButton();  // Read the ladder value
    MyDelay(200L);
    if (val != -1 && val < (EEPROMData.switchValues[0] + WIGGLE_ROOM)) {
      val = ProcessButtonPress(val);      // Use ladder value to get menu choice
      if (val > -1) {                     // Valid choice?
        if (val == MENU_OPTION_SELECT) {  // They made a choice
          keyType = i;
          break;
        }
        MyDelay(50L);
      }
    }
  }
#ifdef G0ORX_FRONTPANEL_2
  calibrateFlag=0;
#endif
  tft.setTextColor(RA8875_WHITE, RA8875_BLACK);

  return keyType;
}

void RenderBearing()
{
  
}