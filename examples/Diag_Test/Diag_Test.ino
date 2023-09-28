/*
 *  Diag_LED Example program
 *  ========================
 *  NaokiS / NaokiSRC (2023)
 * 
 *  This example just gives and example of the usage of the Diag_LED library by running
 *  through the various modes with a timer.
 * 
*/

#include <diag_led.h>   // For Diag_LED routines

Diagnostic_LED diagLED(0);

void setup() {

}

void loop() {
  // Call library to update LED
  diagLED.update();

  // Setup variables for looping demo
  static byte diagLED_routine = 0;
  static unsigned long lastCall = 0;
  unsigned long currentTime = millis();
  static unsigned long timerPeriod = 5000;

  // This timer is being used to change through the modes
  if((currentTime - lastCall) >= timerPeriod ){
    lastCall = currentTime;
    if(diagLED_routine <= 7) diagLED_routine++;
    else diagLED_routine = 0;

    switch(diagLED_routine){
    default:
    case 0:
        diagLED.setActivity(idle);      // Set to static on
        timerPeriod = 5000;             // for 5 seconds
        break;
    case 1:
        diagLED.setBlinkRate(500);      // Set the blink rate to 500ms
        diagLED.setActivity(blink);     // Tell Diag_LED to blink the LED
        timerPeriod = 5000;             // for 5 seconds
        break;
    case 2:
        diagLED.setBlinkRate(200);      // Set the blink rate to 200ms. The LED will blink faster
        timerPeriod = 5000;             // for 5 seconds
        break;
    case 3:
        diagLED.setActivity(heartbeat); // Use the heartbeat indicator
        timerPeriod = 10000;             // for 10 seconds
        break;
    case 4:
        diagLED.setErrorCode(0x23);     // Set an error code of 0x23
        timerPeriod = 10000;             // for 10 seconds
        break;
    case 5:
        diagLED.setErrorCode(0x57);     // Set a new error code of 0x57
        timerPeriod = 20000;             // for 20 seconds
        break;
    case 6:
        diagLED.clearErrorCode();       // Clear the error code
        diagLED.set(DL_ON);             // Set light to manually be on
        timerPeriod = 2000;             // for 2 seconds
    case 7:
        diagLED.set(DL_OFF);            // Set light to manually be off
        timerPeriod = 5000;             // for 5 seconds
        break;
    }
  } 
}
