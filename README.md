# clickfix
Some mice (especially from Logitech) tend to double click on their own after some time. This is due to a worn out spring.

If you don't feel like opening the mouse and tensing the spring yourself, this simple program fixes the problem by ignoring left button events that happen too close together (faster than what a human can do).
## Usage
For 64-bit computers, you can download the precompiled executable and launch manually or add to your system's autostart. Or you can compile it yourself.

## Compiling
You'll need flags for c++11 or later and for the experimental/filesystem library:

`g++ -std=c++11 -O3 clickfix.cpp -o clickfix -lstdc++fs`

## Todo
 * There is a bug where some left clicks are ignored even though they are not double clicks.
