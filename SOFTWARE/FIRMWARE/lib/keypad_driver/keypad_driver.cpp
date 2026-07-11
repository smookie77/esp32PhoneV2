#include "keypad_driver.h"

KeypadDriver::KeypadDriver(uint8_t address) : _gpio(address) {
}

bool KeypadDriver::begin() {
    if (!_gpio.begin()) {
        return false;
    }

    // Initialize Columns as OUTPUT HIGH (inactive)
    for (int i = 0; i < KEYPAD_COLS; i++) {
        _gpio.pinMode1(_colPins[i], OUTPUT);
        _gpio.write1(_colPins[i], HIGH);
    }

    // Initialize Rows as INPUT
    // TCA9555 has no internal pull-ups for inputs in the traditional sense,
    // but the inputs are high-impedance. 
    // IMPORTANT: This driver assumes EXTERNAL PULL-UPS on the ROW pins.
    // If you don't have external pull-ups, the lines will float.
    // Alternatively, if you wire it so columns drive HIGH and rows have pull-downs, invert the logic.
    // Standard matrix: Drive Col LOW, Read Row (which is pulled HIGH). If Row reads LOW, key is pressed.
    
    for (int i = 0; i < KEYPAD_ROWS; i++) {
        _gpio.pinMode1(_rowPins[i], INPUT);
    }

    return true;
}

char KeypadDriver::getKey() {
    delay(20);
    for (int c = 0; c < KEYPAD_COLS; c++) {
        // Activate column (Active LOW)
        _gpio.write1(_colPins[c], LOW);
        
        // Scan rows
        for (int r = 0; r < KEYPAD_ROWS; r++) {
            // Read row pin. TCA9555 read1 returns state.
            // If row is LOW, button is pressed (assuming pull-up on row)
            if (_gpio.read1(_rowPins[r]) == LOW) {
                
                // Debounce / Wait for release could be handled here or at higher level
                // For simple scanning, we just return the key.
                
                // Deactivate column before returning
                _gpio.write1(_colPins[c], HIGH);
                return _keyMap[r][c];
            }
        }
        
        // Deactivate column
        _gpio.write1(_colPins[c], HIGH);
    }
    return 0; // No key pressed
}
