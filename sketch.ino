// This #include statement was automatically added by the Particle IDE.
#include "FastLED.h"

FASTLED_USING_NAMESPACE;

#include "math.h"

// Derivative work to: http://pastebin.com/h95EgTkT
// Thanks again to Mark Kriegsman and FastLED

const uint8_t kCubeSize = 8;
const uint8_t kFracs = 16;

#define DATA_PIN 2
#define NUM_LEDS kCubeSize * kCubeSize * kCubeSize
#define MAX_POS (kCubeSize-1)*kFracs
#define CENTER MAX_POS/2
#define FILLED_NUCLEUS true
#define E_MIN_RAD 50
#define E_MAX_RAD 51
//50, 51
#define N_MAX_RAD 18
#define N_MIN_POS 32
#define N_MAX_POS 80
#define F88_PI 804
//#define F88_PI 3
#define MIN_THETA 0
#define MAX_THETA 2 * F88_PI
#define MIN_PHI 0
#define MAX_PHI F88_PI
//#define THETA_RES 65535
//#define PHI_RES 65535
#define THETA_RES 60
#define PHI_RES 30
#define LED_TYPE    WS2811
#define COLOR_ORDER GRB
#define MASTER_BRIGHTNESS   100
#define FADE 240
#define NUM_PIXELS 0
#define NUM_ELECTRONS 3
#define PI 3.14159
uint8_t hueelec = 0xff9a00;
CRGB leds[NUM_LEDS];

typedef struct {
  int     F16posX = 0; // x position of the pixel
  int     F16posY = 0; // y position of the pixel
  int     F16posZ = 0; // z position of the pixel
  uint8_t hue = 0;
  uint8_t xPosMod = 1;
  uint8_t yPosMod = 1;
  uint8_t zPosMod = 1;
  uint8_t posMin = N_MIN_POS;
  uint8_t posMax = N_MAX_POS;
} pixel;
pixel pixels[NUM_PIXELS];

typedef struct {
  int     F16posX = 0; // x position of the pixel
  int     F16posY = 0; // y position of the pixel
  int     F16posZ = 0; // z position of the pixel
  uint16_t radius = E_MAX_RAD;
  int theta = 0;
  int phi = 0;
  uint8_t thetaOrig = 0;
  int radiusMod = 1;
  bool thetaMod = false;
  bool phiMod = true;
  uint8_t hue = 0;
} electron;
electron electrons[NUM_ELECTRONS];

void initNucleusPixelsRandom()
{
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i].hue = random8();
    pixels[i].xPosMod = random8();
    pixels[i].yPosMod = random8();
    pixels[i].zPosMod = random8();
  }
}

void initElectronsRandom()
{
  int thetaOffset = 0;
  for (int i = 0; i < NUM_ELECTRONS; i++)
  {
    electrons[i].hue = i * 256 / NUM_ELECTRONS;
    electrons[i].radiusMod = 1;
    electrons[i].phiMod = (i % 2) ? true : false;
    electrons[i].phi = i * PHI_RES / NUM_ELECTRONS;
    electrons[i].thetaOrig = (i * THETA_RES / NUM_ELECTRONS) + thetaOffset;
  }
}

void printElectronPos()
{
  for (int i = 0; i < NUM_ELECTRONS; i++)
  {
    //Serial.print("Electron (");
    //Serial.print(i);
    //Serial.print(") [");
    //Serial.print(electrons[i].F16posX);
    //Serial.print(", ");
    //Serial.print(electrons[i].F16posY);
    //Serial.print(", ");
    //Serial.print(electrons[i].F16posZ);
    //Serial.print("] Theta ");
    //Serial.print(electrons[i].theta);
    //Serial.print(", Phi ");
    //Serial.print(electrons[i].phi);
    //Serial.print(", Radius ");
    //Serial.println(electrons[i].radius);
  }
}

void moveElectrons()
{
  for (int i = 0; i < NUM_ELECTRONS; i++)
  {
    if (electrons[i].phi == PHI_RES)
      electrons[i].phiMod = !electrons[i].phiMod;

    electrons[i].phi += (electrons[i].phiMod) ? 1 : -1;
    electrons[i].theta = (electrons[i].phiMod) ? electrons[i].thetaOrig : electrons[i].thetaOrig + (THETA_RES / 2);
    electrons[i].hue += 0;

    electrons[i].F16posX = CENTER + (electrons[i].radius * cos((float) PI * electrons[i].theta / THETA_RES)
                                     * sin((float) PI * electrons[i].phi / PHI_RES));

    electrons[i].F16posY = CENTER + (electrons[i].radius * sin((float) PI * electrons[i].theta / THETA_RES)
                                     * sin((float) PI * electrons[i].phi / PHI_RES));

    electrons[i].F16posZ = CENTER + (electrons[i].radius * cos((float) PI * electrons[i].phi / PHI_RES));

  }
}

void moveNucleusPixels()
{
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    pixels[i].F16posX = beatsin16(pixels[i].xPosMod, pixels[i].posMin, pixels[i].posMax);
    pixels[i].F16posY = beatsin16(pixels[i].yPosMod, pixels[i].posMin, pixels[i].posMax);
    pixels[i].F16posZ = beatsin16(pixels[i].zPosMod, pixels[i].posMin, pixels[i].posMax);
    pixels[i].hue += 0;
  }
}

//void fillNucleus()
//{
//  int res = 20;
//  if (! FILLED_NUCLEUS) return;
//  for (int theta=0; theta<res; theta+=2)
//    for (int phi=0; phi<res; phi+=2)
//    {
//      int x, y, z;
//        x = CENTER + (N_MAX_RAD * cos((float) 2 * PI * theta / res)
//          * sin((float) PI * phi / res));
//
//        y = CENTER + (N_MAX_RAD * sin((float) 2* PI * theta / res)
//          * sin((float) PI * phi / res));
//
//        z = CENTER + (N_MAX_RAD * cos((float) PI * phi / res));
//
//        paintPixels(x, y, z, nucleusHue);
//    }
//}

void paintPixels(int nx, int ny, int nz, uint8_t hue)
{
  uint8_t x = nx / kFracs; // convert from pos to raw pixel number
  uint8_t y = ny / kFracs; // convert from pos to raw pixel number
  uint8_t z = nz / kFracs; // convert from pos to raw pixel number
  uint8_t fracX = nx & 0x0F; // extract the 'factional' part of the position
  uint8_t fracY = ny & 0x0F; // extract the 'factional' part of the position
  uint8_t fracZ = nz & 0x0F; // extract the 'factional' part of the position

  uint8_t px = 255 - (fracX * kFracs);
  uint8_t py = 255 - (fracY * kFracs);
  uint8_t pz = 255 - (fracZ * kFracs);

  leds[XYZ(x, y, z)] = CHSV( hue, 255, scale8(px, scale8(py, pz)));
  leds[XYZ(x, y + 1, z)] = CHSV( hue, 255, scale8(px, scale8((255 - py), pz)));
  leds[XYZ(x, y, z + 1)] = CHSV( hue, 255, scale8(px, scale8(py, (255 - pz))));
  leds[XYZ(x, y + 1, z + 1)] = CHSV( hue, 255, scale8(px, scale8((255 - py), (255 - pz))));
  leds[XYZ(x + 1, y, z)] = CHSV( hue, 255, scale8((255 - px), scale8(py, pz)));
  leds[XYZ(x + 1, y + 1, z)] = CHSV( hue, 255, scale8((255 - px), scale8((255 - py), pz)));
  leds[XYZ(x + 1, y, z + 1)] = CHSV( hue, 255, scale8((255 - px), scale8(py, (255 - pz))));
  leds[XYZ(x + 1, y + 1, z + 1)] = CHSV( hue, 255, scale8((255 - px), scale8((255 - py), (255 - pz))));
}

void fillNucleus()
{
  paintNucleus(3, 3, 3);
  paintNucleus(3, 4, 3);
  paintNucleus(3, 3, 4);
  paintNucleus(3, 4, 4);
  paintNucleus(4, 3, 3);
  paintNucleus(4, 4, 3);
  paintNucleus(4, 3, 4);
  paintNucleus(4, 4, 4);
}

int nb = 32;
int nh = 0;
void paintNucleus(int x, int y, int z)
{
  nb = beatsin8(15, 32, 64);
  nh = beatsin8(7, 0, 255);
  //leds[XYZ(x,y,z)] += CHSV( nh, 255, nb);
  leds[XYZ(x, y, z)] = 0xFFFF00;
}

void setup() {
  Serial.begin(9600);
  delay(3000); // setup guard
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(MASTER_BRIGHTNESS);
  initElectronsRandom();
  initNucleusPixelsRandom();
}

uint16_t XYZ( uint8_t x, uint8_t y, uint8_t z)
{
  uint16_t i;

  i = kCubeSize * ((y * kCubeSize) + x) + z;

  if ((i >= 0) && (i <= 512))
    return i;
  else
    return 0;
}

// Draw a pixel on a matrix using fractions of light. Positions are measured in
// sixteenths of a pixel.  Fractional positions are
// rendered using 'anti-aliasing' of pixel brightness.
void drawPixels()
{
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    int hue = pixels[i].hue;

    uint8_t x = pixels[i].F16posX / kFracs; // convert from pos to raw pixel number
    uint8_t y = pixels[i].F16posY / kFracs; // convert from pos to raw pixel number
    uint8_t z = pixels[i].F16posZ / kFracs; // convert from pos to raw pixel number
    uint8_t fracX = pixels[i].F16posX & 0x0F; // extract the 'factional' part of the position
    uint8_t fracY = pixels[i].F16posY & 0x0F; // extract the 'factional' part of the position
    uint8_t fracZ = pixels[i].F16posZ & 0x0F; // extract the 'factional' part of the position

    uint8_t px = 255 - (fracX * kFracs);
    uint8_t py = 255 - (fracY * kFracs);
    uint8_t pz = 255 - (fracZ * kFracs);

    leds[XYZ(x, y, z)] += 0xFFFF00;
    leds[XYZ(x, y + 1, z)] += 0xFFFF00;
    leds[XYZ(x, y, z + 1)] += 0xFFFF00;
    leds[XYZ(x, y + 1, z + 1)] += 0xFFFF00;
    leds[XYZ(x + 1, y, z)] += 0xFFFF00;
    leds[XYZ(x + 1, y + 1, z)] += 0xFFFF00;
    leds[XYZ(x + 1, y, z + 1)] += 0xFFFF00;
    leds[XYZ(x + 1, y + 1, z + 1)] += 0xFFFF00;
  }
}

void drawElectrons()
{
  for (int i = 0; i < NUM_ELECTRONS; i++)
  {
    int hue = 0xFF9A00;

    uint8_t x = electrons[i].F16posX / kFracs; // convert from pos to raw pixel number
    uint8_t y = electrons[i].F16posY / kFracs; // convert from pos to raw pixel number
    uint8_t z = electrons[i].F16posZ / kFracs; // convert from pos to raw pixel number
    uint8_t fracX = electrons[i].F16posX & 0x0F; // extract the 'factional' part of the position
    uint8_t fracY = electrons[i].F16posY & 0x0F; // extract the 'factional' part of the position
    uint8_t fracZ = electrons[i].F16posZ & 0x0F; // extract the 'factional' part of the position

    uint8_t px = 255 - (fracX * kFracs);
    uint8_t py = 255 - (fracY * kFracs);
    uint8_t pz = 255 - (fracZ * kFracs);

    leds[XYZ(x, y, z)] = 0xFF9A00;
    leds[XYZ(x, y + 1, z)] = 0xFF9A00;
    leds[XYZ(x, y, z + 1)] = 0xFF9A00;
    leds[XYZ(x, y + 1, z + 1)] = 0xFF9A00;
    leds[XYZ(x + 1, y, z)] = 0xFF9A00;
    leds[XYZ(x + 1, y + 1, z)] = 0xFF9A00;
    leds[XYZ(x + 1, y, z + 1)] = 0xFF9A00;
    leds[XYZ(x + 1, y + 1, z + 1)] = 0xFF9A00;
  }
}
int go = 0;
void loop()
{

  // Dim everything
  nscale8(leds, NUM_LEDS, FADE);

  fif (Serial.available() > 0 || go > 0) {
    char data = Serial.read();
    Serial.println(go);
    if (go <= 0) {
      go = 75;
    }
    hueelec = 0xFF9A00;
    moveElectrons();
    drawPixels();
    drawElectrons();
    go --;
  }
  else {
    //hueelec -= 2;
  }

  moveNucleusPixels();

  fillNucleus();

  FastLED.show();

}
