
#ifndef BEENHERE
#include "SDT.h"
#endif

/*****
  Purpose: Determine which UI button was pressed

  Parameter list:
    int valPin            the ADC value from analogRead()

  Return value;
    int                   -1 if not valid push button, index of push button if valid
*****/
int ProcessButtonPress(int valPin) {
#if defined(G0ORX_FRONTPANEL) || defined(G0ORX_FRONTPANEL_2)
  return valPin;
#else
  int switchIndex;

  if (valPin == BOGUS_PIN_READ) {  // Not valid press
    return -1;
  }
  if (valPin == MENU_OPTION_SELECT && menuStatus == NO_MENUS_ACTIVE) {
    NoActiveMenu();
    return -1;
  }
  for (switchIndex = 0; switchIndex < NUMBER_OF_SWITCHES; switchIndex++) {
    if (abs(valPin - EEPROMData.switchValues[switchIndex]) < WIGGLE_ROOM)  // ...because ADC does return exact values every time
    {
      return switchIndex;
    }
  }
  return -1;  // Really should never do this
#endif
}

/*****
  Purpose: Check for UI button press. If pressed, return the ADC value

  Parameter list:
    int vsl               the value from analogRead in loop()\

  Return value;
    int                   -1 if not valid push button, ADC value if valid
*****/
int ReadSelectedPushButton() {
#if defined(G0ORX_FRONTPANEL) || defined(G0ORX_FRONTPANEL_2)
  __disable_irq();
#ifdef G0ORX_FRONTPANEL_2
  getKeypad();
#endif
  int i=G0ORXButtonPressed;
  G0ORXButtonPressed=-1;
  __enable_irq();
  return i;
#else
  minPinRead = 0;
  int buttonReadOld = 1023;

  while (abs(minPinRead - buttonReadOld) > 3) {  // do averaging to smooth out the button response
    minPinRead = analogRead(BUSY_ANALOG_PIN);

    buttonRead    = .1 * minPinRead + (1 - .1) * buttonReadOld;  // See expected values in next function.
    buttonReadOld = buttonRead;
  }
  if (buttonRead > EEPROMData.switchValues[0] + WIGGLE_ROOM) {  //AFP 10-29-22 per Jack Wilson
    return -1;
  }
  minPinRead = buttonRead;
  MyDelay(100L);
  return minPinRead;
#endif
}

/*****
  Purpose: Function is designed to route program control to the proper execution point in response to
           a button press.

  Parameter list:
    int vsl               the value from analogRead in loop()

  Return value;
    void
*****/
void ExecuteButtonPress(int val) 
{
    
  switch (val) {

    case MAIN_MENU_UP:                      // 11/16/23 JJP
      tft.setFontScale((enum RA8875tsize) 1);
      DrawMenuDisplay();                    // Draw selection box and primary menu
      SetPrimaryMenuIndex();                // Scroll through primary indexes and select one
      SetSecondaryMenuIndex();              // Use the primary index selection to redraw the secondary menu and set its index
      secondaryMenuChoiceMade = functionPtr[mainMenuIndex](); 

      tft.fillRect(1, SPECTRUM_TOP_Y + 1, 513, 379, RA8875_BLACK);          // Erase Menu box
      DrawSpectrumDisplayContainer();
      EraseMenus();
      break;

    case BAND_UP:  // 2 Now calls ProcessIQData and Encoders calls
      EraseMenus();
      if(currentBand < 5) digitalWrite(bandswitchPins[currentBand], LOW);  // Added if so unused GPOs will not be touched.  KF5N October 16, 2023.
      ButtonBandIncrease();
      if(currentBand < 5) digitalWrite(bandswitchPins[currentBand], HIGH);
      BandInformation();
      NCOFreq = 0L;
      DrawBandWidthIndicatorBar();  // AFP 10-20-22
      SetFreq();
      ShowSpectrum();
      break;

    case ZOOM:  // 3
      menuStatus = PRIMARY_MENU_ACTIVE;
      EraseMenus();
      ButtonZoom();
      break;

    case MAIN_MENU_DN:  // 4
      ButtonMenuDecrease();
      if (menuStatus != NO_MENUS_ACTIVE) {  // Doing primary menu
        ShowMenu(&topMenus[mainMenuIndex], PRIMARY_MENU);
      }
      break;

    case BAND_DN:  // 5
      EraseMenus();
      ShowSpectrum();  //Now calls ProcessIQData and Encoders calls
      if(currentBand < 5) digitalWrite(bandswitchPins[currentBand], LOW);
      ButtonBandDecrease();
      if(currentBand < 5) digitalWrite(bandswitchPins[currentBand], HIGH);
      BandInformation();
      NCOFreq = 0L;
      DrawBandWidthIndicatorBar();  //AFP 10-20-22
      break;

    case FILTER:  // 6
      EraseMenus();
      ButtonFilter();
      break;

    case DEMODULATION:  // 7
      EraseMenus();
      ButtonDemodMode();
      break;

    case SET_MODE:  // 8
      ButtonMode();
      ShowSpectrumdBScale();
      break;

    case NOISE_REDUCTION:  // 9
      ButtonNR();
      break;

    case NOTCH_FILTER:  // 10
      ButtonNotchFilter();
      UpdateNotchField();
      break;

    case NOISE_FLOOR:  // 11
      ButtonSetNoiseFloor();
      break;

    case FINE_TUNE_INCREMENT:  // 12
      UpdateIncrementField();
      break;

    case DECODER_TOGGLE:  // 13
      decoderFlag = !decoderFlag;
      UpdateDecoderField();
      break;

    case MAIN_TUNE_INCREMENT:  // 14
      ButtonFreqIncrement();
      UpdateEEPROMSyncIndicator(0);
      break;

    case RESET_TUNING:  // 15   AFP 10-11-22
      ResetTuning();    // AFP 10-11-22
      break;            // AFP 10-11-22

    case UNUSED_1:  // 16
#ifdef G0ORX_VFO
      // Copy VFOA to VFOB
      currentFreqB = currentFreqA;
      currentBandB = currentBandA;
      if(activeVFO == VFO_A) {
        ShowFrequency();
      } else {
        NCOFreq = 0L;
        centerFreq = TxRxFreq = currentFreqB;
        SetBand();           // KF5N July 12, 2023
        SetBandRelay(HIGH);  // Required when switching VFOs. KF5N July 12, 2023
        SetFreq();
        RedrawDisplayScreen();
        BandInformation();
        ShowBandwidth();
        FilterBandwidth();

        tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);  // delete old digit
        tft.fillRect(FREQUENCY_X, FREQUENCY_Y - 12, VFOA_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);        // delete old digit  tft.setFontScale( (enum RA8875tsize) 0);
        ShowFrequency();
        // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
        tft.writeTo(L2);
        tft.clearMemory();
        if (xmtMode == CW_MODE) BandInformation();
        DrawBandWidthIndicatorBar();
        DrawFrequencyBarValue();
      }
#else
      if (calOnFlag == 0) {
        ButtonFrequencyEntry();
      }
#endif
      break;

    case BEARING:  // 17  // AFP 10-11-22
#ifdef G0ORX_VFO
      // Copy VFOB to VFOA
      currentFreqA = currentFreqB;
      currentBandA = currentBandB;
      if(activeVFO == VFO_B) {
        // just need to update display
        ShowFrequency();
      } else {
        NCOFreq = 0L;
        centerFreq = TxRxFreq = currentFreqA;
        SetBand();           // KF5N July 12, 2023
        SetBandRelay(HIGH);  // Required when switching VFOs. KF5N July 12, 2023
        SetFreq();
        RedrawDisplayScreen();
        BandInformation();
        ShowBandwidth();
        FilterBandwidth();

        tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);  // delete old digit
        tft.fillRect(FREQUENCY_X, FREQUENCY_Y - 12, VFOA_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK);        // delete old digit  tft.setFontScale( (enum RA8875tsize) 0);
        ShowFrequency();
        // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
        tft.writeTo(L2);
        tft.clearMemory();
        if (xmtMode == CW_MODE) BandInformation();
        DrawBandWidthIndicatorBar();
        DrawFrequencyBarValue();
      }
#else
      int buttonIndex, doneViewing, valPin;
      float retVal;
      
      tft.clearScreen(RA8875_BLACK);

      DrawKeyboard();
      CaptureKeystrokes();
      retVal = BearingHeading(keyboardBuffer);

     
      if (retVal != -1.0) {                           // We have valid country
        bmpDraw((char *)myMapFiles[selectedMapIndex].mapNames, IMAGE_CORNER_X, IMAGE_CORNER_Y);
        doneViewing = false;
      } else {
        tft.setTextColor(RA8875_RED);
        tft.setCursor(380 - (17 * tft.getFontWidth(0)) / 2, 240);   // Center message
        tft.print("Country not found");  
        tft.setTextColor(RA8875_WHITE);        
      }
      while (true) {
        valPin = ReadSelectedPushButton();            // Poll UI push buttons
        MyDelay(100L);
        if (valPin != BOGUS_PIN_READ) {               // If a button was pushed...
          buttonIndex = ProcessButtonPress(valPin);   // Winner, winner...chicken dinner!
          switch (buttonIndex) {
            case BEARING:                             // Pressed puchbutton 18
              doneViewing = true;
              break;
            default:
              break;
          }
        }

        if (doneViewing == true) {
            //tft.clearMemory();          // Need to clear overlay too
            //tft.writeTo(L2);
            //tft.fillWindow();
          break;
        }
      }
      RedrawDisplayScreen();
      ShowFrequency();
      DrawFrequencyBarValue();
#endif
      break;

#if (defined(G0ORX_FRONTPANEL) || defined(G0ORX_FRONTPANEL_2))
    case 18:                                                      // 18 - Encoder 1 SW (Volume)
      switch(volumeFunction) {
        case AUDIO_VOLUME:
          volumeFunction=AGC_GAIN;
          break;
        case AGC_GAIN:
          volumeFunction=MIC_GAIN;
          break;
        case MIC_GAIN:
          volumeFunction=SIDETONE_VOLUME;
          break;
        case SIDETONE_VOLUME:
          volumeFunction=NOISE_FLOOR_LEVEL;
          break;
        case NOISE_FLOOR_LEVEL:
          volumeFunction=FMSQUELCH_LEVEL;
          break;
        case FMSQUELCH_LEVEL:
          volumeFunction=AUDIO_VOLUME;
          break;
      }
      volumeChangeFlag = true;
      break;

    case 19:                                                      // 19 - Encoder 2 SW (Filter/Menu)
      // Temp use to step through AGC
      AGCMode++;
      if(AGCMode>4) {
        AGCMode = 0;
      }

      AGCLoadValues();

      EEPROMData.AGCMode = AGCMode;                               // Store in EEPROM and...
      EEPROM.put(EEPROM_BASE_ADDRESS, EEPROMData);                // ...save it
      UpdateAGCField();
      break;

    case 20:                                                      // 20 - Encoder 3 SW (Center Tune)
      // switch between VFO-A and VFO-B
      if (activeVFO == VFO_A) {
        centerFreq = TxRxFreq = currentFreqB;
        activeVFO = VFO_B;
        currentBand = currentBandB;
        tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);      // Erase split message
        splitOn = 0;
      } else { // Must be VFO-B
        centerFreq = TxRxFreq = currentFreqA;
        activeVFO = VFO_A;
        currentBand = currentBandA;
        tft.fillRect(FILTER_PARAMETERS_X + 180, FILTER_PARAMETERS_Y, 150, 20, RA8875_BLACK);      // Erase split message
        splitOn = 0;
      }
      bands[currentBand].freq = TxRxFreq;
      SetBand();  // KF5N July 12, 2023
      SetBandRelay(HIGH);  // Required when switching VFOs. KF5N July 12, 2023
      SetFreq();
      RedrawDisplayScreen();
      BandInformation();
      ShowBandwidth();
      FilterBandwidth();
      EEPROMData.activeVFO = activeVFO;

      tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK); // delete old digit
      tft.fillRect(FREQUENCY_X,       FREQUENCY_Y - 12, VFOA_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK); // delete old digit  tft.setFontScale( (enum RA8875tsize) 0);
      ShowFrequency();
      // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
      tft.writeTo(L2);
      tft.clearMemory();
      if(xmtMode == CW_MODE) BandInformation();
      DrawBandWidthIndicatorBar();
      DrawFrequencyBarValue();
      break;

    case 21:                                                      // 21 - Encoder 4 SW (Fine Tune)
      // swap VFO A and B
      long tempFreq = currentFreqA;
      int tempBand = currentBandA;
      currentFreqA = currentFreqB;
      currentBandA = currentBandB;
      currentFreqB = tempFreq;
      currentBandB = tempBand;
      if (activeVFO == VFO_A) {
        centerFreq = TxRxFreq = currentFreqA;
        currentBand = currentBandA;
      } else { // must be VFO-B
        centerFreq = TxRxFreq = currentFreqB;
        currentBand = currentBandB;
      }
      bands[currentBand].freq = TxRxFreq;
      SetBand();  // KF5N July 12, 2023
      SetBandRelay(HIGH);  // Required when switching VFOs. KF5N July 12, 2023
      SetFreq();
      RedrawDisplayScreen();
      BandInformation();
      ShowBandwidth();
      FilterBandwidth();
      EEPROMData.activeVFO = activeVFO;

      tft.fillRect(FREQUENCY_X_SPLIT, FREQUENCY_Y - 12, VFOB_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK); // delete old digit
      tft.fillRect(FREQUENCY_X,       FREQUENCY_Y - 12, VFOA_PIXEL_LENGTH, FREQUENCY_PIXEL_HI, RA8875_BLACK); // delete old digit  tft.setFontScale( (enum RA8875tsize) 0);
      ShowFrequency();
      // Draw or not draw CW filter graphics to audio spectrum area.  KF5N July 30, 2023
      tft.writeTo(L2);
      tft.clearMemory();
      if(xmtMode == CW_MODE) BandInformation();
      DrawBandWidthIndicatorBar();
      DrawFrequencyBarValue();
      break;
#endif
  }
}


/*****
  Purpose: To process a band decrease button push

  Parameter list:
    void

  Return value:
    void
*****/
void ButtonFreqIncrement() {
  tuneIndex--;
  if (tuneIndex < 0)
    tuneIndex = MAX_FREQ_INDEX - 1;
  freqIncrement = incrementValues[tuneIndex];
  //UpdateEEPROMSyncIndicator(0);   //  JJP 7/25/23
  DisplayIncrementField();
}


/*****
  Purpose: Error message if Select button pressed with no Menu active

  Parameter list:
    void

  Return value;
    void
*****/
void NoActiveMenu() {
  tft.setFontScale((enum RA8875tsize)1);
  tft.setTextColor(RA8875_RED);
  tft.setCursor(10, 0);
  tft.print("No menu selected");

  menuStatus = NO_MENUS_ACTIVE;
  mainMenuIndex = 0;
  secondaryMenuIndex = 0;
}
