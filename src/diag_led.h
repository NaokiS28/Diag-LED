/*
 *  Activity/Diagnostic Light Class
 *  ===============================
 *  NaokiS / NaokiSRC (2023)
 * 
 *  This library provides functions to allow easy implementation of a diagnostic LED in projects.
 *  It needs to be given an Arduino pin number to use. You can then set the activity to get the desired
 *  output:
 *  
 *  LED states:
 *   * idle         =   Outputs a static light.
 *   * blink        =   Blinks given LED at a given rate set by *.setBlinkRate() (default 500ms).
 *   * heartbeat    =   Blinks in a heartbeat like method.
 *   * manual       =   Is set when using the .set() command, allows main program to directly control light.
 *   * error        =   Is set when .setErrorCode() is used. LED blinks out a two digit error code.
 * 
 *  Constructor:
 *      Diagnostic_LED diagLED();                           = For Arduino and compatible boards
 *      Diagnostic_LED diagLED( ledPin_number );            = For other boards
 *      Diagnostic_LED diagLED( ledPin, DL_INVERT_OUTPUT ); = Optionally can invert the output if your LED needs low side switching.
 * 
 *  Error codes:
 *      Error codes must be two hexadecimal digits and should only be two whole numbers, ie 0x23 or 0x57.
 *      There is no technical reason why this library cannot use 0x01 or 0x10 but as these will be confusing
 *      to the end user, these types of error codes will fail to set. If you must use these codes, you can
 *      uncomment out the #define IGNORE_CONFUSING_ERRORS to enable support.
 * 
 */

#pragma once
#include <Arduino.h>

#define DL_OFF 0
#define DL_ON 1

#define DL_INVERT_OUTPUT true

#define ACTIVITY_MAX 1000 // If no updates are received in 1 second, reset to idle

// Before uncommenting this line, know that the user will not know the difference between 0x01 and 0x10 etc.
// #define IGNORE_CONFUSING_ERRORS

enum ActivityStates {
    none,
    idle,
    blink,
    heartbeat,
    manual,
    error
};

class Diagnostic_LED {
    public:
    Diagnostic_LED(uint8_t _p = LED_BUILTIN, bool _i = false) {
        _outputPin = _p;
        _invertOutput = _i;

        pinMode(_outputPin, OUTPUT);
        this->setDiagLED(DL_ON);
        this->setActivity(idle);
    }

    void update() {
        this->updateActivity(lastClockTime);
    }

    void setActivity(ActivityStates s) {
        pendingActivity = s;
    }

    uint8_t readErrorCode() {
        return errorCode;
    }

    uint8_t setErrorCode(uint8_t _ec) {
        // Only set error code if the error can be mapped from 10-99. 
        //  There's no technical reason it cannot be higher, but it's easier on the user to use normal, non-hex numbers.
#ifndef IGNORE_CONFUSING_ERRORS
        if (
            (((_ec >> 4) < 9) && (_ec >> 4) > 0) // If high nyble is between 1 and 9
            &&
            (((_ec & 0xF) < 9) && ((_ec & 0xF) > 0)) //  and low nyble is between 1 and 9.
        ) {
#elif
        if (_ec != 0x00) {
#endif
            errorCode = _ec;
            this->setActivity(error);

            return errorCode;
        } else {
            // Requirements not met
            return 0;
        }
    }

    void clearErrorCode() {
        errorCode = 0;
        this->setActivity(idle);
    }

    void set(bool s){
        activity = manual;
        setDiagLED(s);
    }

    void resetActivity(){
        activity = idle;
        this->update();
    }

    void setBlinkRate(uint16_t _ms){
        blinkRate = _ms;
    }

    private:

    bool _invertOutput = false;
    uint8_t _outputPin = 0; // Arduino default LED

    // Current diag LED activity indicator
    ActivityStates activity = idle;
    ActivityStates pendingActivity = none;

    bool diagLight = DL_OFF; // When using debugging, this specifies the pin's current state.
    uint32_t activityLastCall = 0;
    uint32_t lastClockTime = 0;
    uint32_t errorReadoutTime = 0;
    uint8_t errorReadoutCount = 0;
    uint8_t errorReadoutStep = 0;
    uint8_t errorCode = 0x00; // If an error occurs, the activity light will be used to flash out a HEX error code.

    uint8_t heartbeatStep = 0;
    uint32_t heartbeatTime = 0;
    
    uint16_t blinkRate = 500;   // in milliseconds

    void updateActivityState() {
        if (pendingActivity != none && (activity != error || (activity == error && errorReadoutStep == 3))) {
            activityLastCall = 0;
            errorReadoutCount = 0;
            errorReadoutStep = 0;
            errorReadoutTime = 0;

            this->setDiagLED(DL_OFF);

            activity = pendingActivity;
            pendingActivity = none;
        }
    }

    void setDiagLED(bool s) {
        diagLight = s;
        if (_invertOutput) {
            digitalWrite(_outputPin, !diagLight);
        } else {
            digitalWrite(_outputPin, diagLight);
        }
    }

    void toggleDiagLED() {
        diagLight = !diagLight;
        digitalWrite(_outputPin, diagLight);
    }

    void updateActivity(uint32_t _lastClockTime) {
        uint32_t currentTime = millis();

        this->updateActivityState();

        switch (activity) {
        case idle: {
            if (diagLight != DL_ON) {
                setDiagLED(DL_ON);
            }

            /*if ((currentTime - _lastClockTime) < ACTIVITY_MAX) {
                this->setActivity(blink);
            }*/
            break;
        }
        case blink: {
            // Blink at 250ms rate until input clock has been idle long enough that we've gone idle
            if ((currentTime - activityLastCall) >= blinkRate) {
                activityLastCall = currentTime;
                /*if ((currentTime - _lastClockTime) >= ACTIVITY_MAX) {
                    this->setActivity(idle);
                } else {*/
                    this->toggleDiagLED();
                //}
            }
            break;
        }
        case heartbeat: {
            // Do heartbeat blink
            if ((currentTime - activityLastCall) >= 75) {
                activityLastCall = currentTime;
                switch (heartbeatStep)
                {
                case 0:
                    // On and delay
                    this->setDiagLED(DL_ON);
                    heartbeatStep++;
                    break;
                case 1:
                    // Off and delay one cycle
                    this->setDiagLED(DL_OFF);
                    heartbeatStep++;
                    heartbeatTime = currentTime;
                    break;
                case 2:
                    // 200 millisecond pause
                    if (currentTime - heartbeatTime >= 200) {
                        heartbeatTime = currentTime;
                        heartbeatStep++;
                    }
                    break;
                case 3:
                    // On and delay
                    this->setDiagLED(DL_ON);
                    heartbeatStep++;
                    break;
                case 4:
                    // On and delay
                    this->setDiagLED(DL_OFF);
                    heartbeatTime = currentTime;
                    heartbeatStep++;
                    break;
                default:
                    // 2 second pause
                    if (currentTime - heartbeatTime >= 2000) {
                        heartbeatTime = currentTime;
                        heartbeatStep = 0;
                    }
                    break;
                }
            }
            break;
        }
        case manual: {
            // Do nothing here. External source controlling light.
            break;
        }
        case error: {
            if ((currentTime - activityLastCall) >= 500) {
                activityLastCall = currentTime;

                switch (errorReadoutStep) {
                case 0: {
                    // 1 second pause
                    if (currentTime - errorReadoutTime >= 1000) {
                        errorReadoutTime = currentTime;
                        errorReadoutStep++;
                    }
                    break;
                }
                case 1: {
                    // High nybble of error code
                    if (diagLight == DL_OFF) {
                        if (errorReadoutCount < (errorCode >> 4)) {
                            this->setDiagLED(DL_ON);
                        } else {
                            // Go to pause
                            errorReadoutTime = 0;
                            errorReadoutCount = 0;
                            errorReadoutStep++;
                        }
                    } else {
                        this->setDiagLED(DL_OFF);
                        errorReadoutCount++;
                    }
                    break;
                }
                case 2: {
                    // 4 second pause
                    if (currentTime - errorReadoutTime >= 4000) {
                        errorReadoutTime = currentTime;
                        errorReadoutStep++;
                        errorReadoutCount = 0;
                    }
                    break;
                }
                case 3: {
                    // Low nybble of error code
                    if (diagLight == DL_OFF) {
                        if (errorReadoutCount < (errorCode & 0xF)) {
                            this->setDiagLED(DL_ON);
                        } else {
                            // Go to pause
                            errorReadoutTime = currentTime;
                            errorReadoutStep++;
                            errorReadoutCount = 0;
                        }
                    } else {
                        this->setDiagLED(DL_OFF);
                        errorReadoutCount++;
                    }
                    break;
                }
                case 4: {
                    // 5 second pause (+1 second when looping)
                    if (currentTime - errorReadoutTime >= 5000) {
                        errorReadoutTime = currentTime;
                        errorReadoutStep = 0;
                        errorReadoutCount = 0;
                    }
                    break;
                }
                default: {
                    errorReadoutStep = 0;
                    errorReadoutCount = 0;
                    errorReadoutTime = currentTime;
                    break;
                }
                }
            }
            break;
        }
        default:
            // Error to idle
            this->setActivity(idle);
            break;
        }
    }
};
