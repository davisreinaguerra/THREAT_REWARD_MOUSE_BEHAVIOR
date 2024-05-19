// Adafruit protomatter definition
#include <Adafruit_Protomatter.h>
// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0 
#define WHITE    0xFFFF

uint8_t rgbPins[]  = {7, 8, 9, 10, 11, 12};
uint8_t addrPins[] = {17, 18, 19, 20, 21};
uint8_t clockPin   = 14;
uint8_t latchPin   = 15;
uint8_t oePin      = 16;

// adafruit LED matrix object
Adafruit_Protomatter matrix(
  64,          // Width of matrix (or matrix chain) in pixels
  4,           // Bit depth, 1-6
  1, rgbPins,  // # of matrix chains, array of 6 RGB pins for each
  5, addrPins, // # of address pins (height is inferred), array of pins
  clockPin, latchPin, oePin, // Other matrix control pins
  false);      // No double-buffering here (see "doublebuffer" example)

// input pin
// Looming specific global vars
#define trig_pin 25 //A3
bool trig;

void setup(void) {
  Serial.begin(9600);
  pinMode(trig_pin, INPUT);

  // Initialize matrix...
  ProtomatterStatus status = matrix.begin();
  Serial.print("Protomatter begin() status: ");
  Serial.println((int)status);
  if(status != PROTOMATTER_OK) {
    for(;;);
  }

  matrix.fillScreen(WHITE);
  matrix.show();
}

void loop(void) {

  trig = digitalRead(trig_pin);

  if (trig == 0) {
    matrix.fillScreen(WHITE);
    matrix.show();
    delay(5);
  }

  if (trig == 1) {
    for (int i = 0; i <= 50; i++) {
      matrix.fillScreen(WHITE);
      matrix.fillCircle(32, 32, i, BLACK);
      matrix.show();
      delay(i/5);
    }
  }
}
