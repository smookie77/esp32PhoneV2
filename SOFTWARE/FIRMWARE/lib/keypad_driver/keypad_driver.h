#ifndef KEYPAD_DRIVER_H
#define KEYPAD_DRIVER_H

#include <Arduino.h>
#include "TCA9555.h"

// Keypad definition: 7 rows, 3 columns (Nokia style)
#define KEYPAD_ROWS 7
#define KEYPAD_COLS 3

class KeypadDriver {
public:
    KeypadDriver(uint8_t address = 0x20);
    bool begin();
    char getKey(); // Returns the key pressed, or 0 if none

private:
    TCA9555 _gpio;
    
    // Mapping of TCA9555 pins to keypad rows/cols
    // Adjust these based on your actual wiring!
    // Example Assumption:
    // Rows 0-6 connected to TCA Port 0, pins 0-6
    // Cols 0-2 connected to TCA Port 1, pins 0-2
    // Row 0 = P00, Row 1 = P01 ... Row 6 = P06
    // Col 0 = P10, Col 1 = P11, Col 2 = P12
    
    uint8_t _rowPins[KEYPAD_ROWS] = {7, 8, 9, 10, 11, 12, 13}; // Using Port 0
    uint8_t _colPins[KEYPAD_COLS] = {6, 5, 4};            // Using Port 1 (pins 8-15 maps to P10-P17)

    // Key map layout (Standard phone style)
    const char _keyMap[KEYPAD_ROWS][KEYPAD_COLS] = {
        {'X', 'U', 'Y'},  // Aux buttons? (Adjust as needed)
        {'L', 'O', 'R'}, // Left, Up, Right (Softkeys / Navigation)
        {'C', 'D', 'E'}, // Call, Down, End
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}
    };
};

extern KeypadDriver keypad;

#endif
