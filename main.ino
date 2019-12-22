#include <Adafruit_NeoPixel.h>
#include <avr/io.h>
#include <avr/eeprom.h>

#define LED_DATA_PIN          4   // Physically pin 3.
#define BRIGHTNESS_PIN        A1  // Physically pin 7.
#define COLOUR_DELAY          300 // In milliseconds
#define BRIGHTNESS_DELAY      25 // In milliseconds
#define LED_COUNT             5

int16_t* currentColour = new int16_t[3];
int16_t* targetColour = new int16_t[3];

uint32_t reseedRandomSeed EEMEM = 0xFFFFFFFF;

Adafruit_NeoPixel pixels(LED_COUNT, LED_DATA_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  reseedRandom( &reseedRandomSeed );
  
  pinMode(LED_DATA_PIN, OUTPUT);
  pinMode(BRIGHTNESS_PIN, INPUT);

  currentColour = generateColour(); 
  targetColour = generateColour();
  
  pixels.begin();
}

void loop() {

  // When target met, update target.
  if(currentColour[0] == targetColour[0] && currentColour[1] == targetColour[1] && currentColour[2] == targetColour[2]){
    targetColour = generateColour();
  } 

  // Update colour values one step at a time.
  for(uint8_t i = 0; i <= 2; i++){
    if(currentColour[i] < targetColour[i]){
      currentColour[i] += 1;
    } else if (currentColour[i] > targetColour[i]) {
      currentColour[i] -= 1;
    } else {
      currentColour[i] = targetColour[i];
    }
  }

  // For each pixel modifiy each HSL component (by pixel's address index)
  for(uint8_t pixel = 0; pixel < LED_COUNT; pixel++){
    uint16_t h = currentColour[0] + (pixel * 12);
    uint16_t s = currentColour[1] + (pixel * 10);
    uint16_t l = currentColour[2];

    if(h > 360){
      h = (h - 360);
    }

    if(s > 100){
      s = 100;
    }

    if(l > 40){
      l = 40;
    }

    pixels.setPixelColor(pixel, hsl(h, s, l));
    
  }

  // Allows for brightness to be updated between colour updates.
  for(int i = 0; i <= (COLOUR_DELAY / BRIGHTNESS_DELAY); i++){
    setBrightness();
    pixels.show();
    delay(BRIGHTNESS_DELAY);
  }
  
}

void setBrightness(){
  uint8_t brightness = analogRead(BRIGHTNESS_PIN) >> 2;
  pixels.setBrightness(brightness);
}

int16_t* generateColour() {
  int16_t* colour = new int16_t[3];
  colour[0] = random(0, 361);
  colour[1] = random(70, 101);
  colour[2] = random(30, 41);
  return colour;
}


/*
 * 
 *  Random Seeding.
 *  Source: https://forum.arduino.cc/index.php?topic=66206.45 (Reply #52)
 * 
 */


void reseedRandom( uint32_t* address )
{
  static const uint32_t HappyPrime = 127807 /*937*/;
  uint32_t raw;
  unsigned long seed;

  // Read the previous raw value from EEPROM
  raw = eeprom_read_dword( address );

  // Loop until a seed within the valid range is found
  do
  {
    // Incrementing by a prime (except 2) every possible raw value is visited
    raw += HappyPrime;

    // Park-Miller is only 31 bits so ignore the most significant bit
    seed = raw & 0x7FFFFFFF;
  }
  while ( (seed < 1) || (seed > 2147483646) );

  // Seed the random number generator with the next value in the sequence
  srandom( seed ); 

  // Save the new raw value for next time
  eeprom_write_dword( address, raw );
}

inline void reseedRandom( unsigned short address )
{
  reseedRandom( (uint32_t*)(address) );
}


void reseedRandomInit( uint32_t* address, uint32_t value )
{
  eeprom_write_dword( address, value );
}

inline void reseedRandomInit( unsigned short address, uint32_t value )
{
  reseedRandomInit( (uint32_t*)(address), value );
}


/*
 * 
 *  HSL -> RGB Conversion.
 *  Source: https://github.com/Mazaryk/neopixel-hsl
 * 
 */
 
uint32_t hsl(uint16_t ih, uint8_t is, uint8_t il) {

  float h, s, l, t1, t2, tr, tg, tb;
  uint8_t r, g, b;

  h = (ih % 360) / 360.0;
  s = constrain(is, 0, 100) / 100.0;
  l = constrain(il, 0, 100) / 100.0;

  if ( s == 0 ) {
    r = g = b = 255 * l;
    return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
  }

  if ( l < 0.5 ) t1 = l * (1.0 + s);
  else t1 = l + s - l * s;

  t2 = 2 * l - t1;
  tr = h + 1 / 3.0;
  tg = h;
  tb = h - 1 / 3.0;

  r = hsl_convert(tr, t1, t2);
  g = hsl_convert(tg, t1, t2);
  b = hsl_convert(tb, t1, t2);

  // NeoPixel packed RGB color
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

uint8_t hsl_convert(float c, float t1, float t2) {

  if ( c < 0 ) c += 1;
  else if ( c > 1 ) c -= 1;

  if ( 6 * c < 1 ) c = t2 + ( t1 - t2 ) * 6 * c;
  else if ( 2 * c < 1 ) c = t1;
  else if ( 3 * c < 2 ) c = t2 + ( t1 - t2 ) * ( 2 / 3.0 - c ) * 6;
  else c = t2;

  return (uint8_t)(c * 255);
}
