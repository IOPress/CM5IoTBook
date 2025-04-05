# Gpio5 For The Pi 5, P500 and CM5

The Gpio5 library provides functions to let you work with
the GPIO, PWM, I2C and SPI interfaces provided by the RP1
in the Raspberry Pi 5, P500 and CM5.

Where possible the functions are modelled after the functions
in the Pico SDK. In most cases you can take a Pico program and
run it on a Pi 5 with only minor changes.

No error trapping has been included in the functions. 
The reason is simply that API functions that check for
validity of parameters are inefficient. For example. none
of the GPIO functions check that the GPIO number supplied
is valid. The reason is that in most cases the GPIO number
to be used is known at compile time and checking for correctness
at runtime is a waste. 
