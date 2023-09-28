# Diag_LED
This library provides functions to allow easy implementation of a diagnostic LED in projects.
It needs to be given an Arduino pin number to use. You can then set the activity to get the desired output:
 
### LED states:
 * `idle`         -   Outputs a static light.
 * `blink`        -   Blinks given LED at a given rate set by *.setBlinkRate() (default 500ms).
 * `heartbeat`    -   Blinks in a heartbeat like method.
 * `manual`       -   Is set when using the `.set(bool)` function, allows the main program to directly control light.
 * `error`        -   Is set when `.setErrorCode(code)` is used. LED blinks out a two digit error code.

### Usage
To begin using this library, first include the header file in your code:
```c++
#include <diag_led.h>
```
Then use one of the following constructors depending on what board you are using:
#### Constructor:
```c++
Diagnostic_LED diagLED();                           // For Arduino and most compatible boards
Diagnostic_LED diagLED( ledPin_number );            // For other boards
Diagnostic_LED diagLED( ledPin, DL_INVERT_OUTPUT ); // Optionally can invert the output if your LED needs low side switching.
```
Then put the `update();` function in your `void loop()`:
```c++
void loop(){
    diagLED.update();
    // Your code here
}
```

And now you can use your diag light! By default it will use the `idle` state to display a steady light. If you want to change this, then all you need to do is call `.setActivity(activity_name);` to change the behavoir to one of the modes above.

If you want to revert back to the `idle` state, you can use either `.setActivity(activity_name);` or `.resetActivity();`.

And if you want to control the LED directly without the library interefering, you can use `.set(bool);` which will set the LED to the state given, which can be a boolean value or you can use `DL_ON` and `DL_OFF`. 
*Note: When you use `.set(bool);`, you must set the activity to another state or use `.resetActivity();` to exit out of manual mode.*

### Error codes
Error codes must be two hexadecimal digits and should only be two whole numbers, i.e. 0x23 or 0x57 and cannot go above 9 per digit, i.e. 0x1F will not be a valid error code.
There is no technical reason why this library cannot use 0x01 or 0x10 but as these will be confusing to the end user, these types of error codes will fail to set. If you must use these codes, you can uncomment out the `#define IGNORE_CONFUSING_ERRORS` to enable support.

When writing out your documents, it's suggested to get your users to count the blinks between pauses to make the two digit code. For example:

* 2 blinks
* 4 second pause
* 5 blinks
* 6 second pause

Would result in error code 0x25.

If your program needs to read an error code out, use the `.setErrorCode(code);` function. This will return the error code you passed to it and enable the error state which will read out the code given. If an invalid error code is given and you have not disabled the error code supression, then the library will not change the LED state.

With invalid error codes enabled, your error codes will read 1-15 per digit. This means your user, following the above advise, could get an error code such as error 1015. **It is \*strongly\* advised to ignore error codes like 0x01, 0x02, 0x03, 0x10, 0x20, 0x30 e.t.c as the user will not be able to tell between them.**

If you need to clear the error code, use `.clearErrorCode();`.

You can also read the currently set error code with `.readErrorCode();`.
