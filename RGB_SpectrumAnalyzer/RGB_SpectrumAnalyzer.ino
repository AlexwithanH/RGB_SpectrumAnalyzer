// (Heavily) adapted from https://github.com/G6EJD/ESP32-8266-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_02.ino
// Adjusted to allow brightness changes on press+hold, Auto-cycle for 3 button presses within 2 seconds
// Edited to add Neomatrix support for easier compatibility with different layouts.

#include <arduinoFFT.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_DotStarMatrix.h>
#include <Adafruit_DotStar.h>

#define SAMPLES         1024          // Must be a power of 2
#define SAMPLING_FREQ   40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define AMPLITUDE       6000          // Depending on your audio source level, you may need to alter this value. Can be used as a 'sensitivity' control.
#define AUDIO_IN_PIN    A0            // Signal in on this pin
#define DATAPIN         27            // LED strip data
#define CLOCKPIN        13             // LED strip clock
#define SHIFTDELAY 100
//#define BRIGHTNESS 20
#define NUM_BANDS       12            // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
#define NOISE           500           // Used as a crude noise filter, values below this are ignored
const uint8_t kMatrixWidth = 12;                          // Matrix width
const uint8_t kMatrixHeight = 6;                         // Matrix height
#define NUM_LEDS       (kMatrixWidth * kMatrixHeight)     // Total number of LEDs
#define BAR_WIDTH      (1)  // If width >= 8 light 1 LED width per bar, >= 16 light 2 LEDs width bar etc
#define TOP            (kMatrixHeight - 0)                // Don't allow the bars to go offscreen

// Sampling and FFT stuff
unsigned int sampling_period_us;
byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= NUM_BANDS
int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
double vReal[SAMPLES];
double vImag[SAMPLES];
unsigned long newTime;

arduinoFFT FFT = arduinoFFT(vReal, vImag, SAMPLES, SAMPLING_FREQ);

Adafruit_DotStarMatrix matrix = Adafruit_DotStarMatrix(
                                  12, 6, DATAPIN, CLOCKPIN,
                                  DS_MATRIX_BOTTOM     + DS_MATRIX_LEFT +
                                  DS_MATRIX_ROWS + DS_MATRIX_PROGRESSIVE,
                                  DOTSTAR_BGR);

const uint16_t primaryColors[] = {
  matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255)
};

const uint16_t rainbowColors[] = {
  matrix.Color(255, 0, 0),   //A red
  matrix.Color(255, 125, 0), //D orange
  matrix.Color(200, 255, 0), 
  matrix.Color(100, 255, 0), //A yellowish
  matrix.Color(0, 255, 0),   //F green
  matrix.Color(0, 255, 225),
  matrix.Color(0, 0, 225), //R blue
  matrix.Color(100, 0, 255),
  matrix.Color(150, 0, 255), //U purple
  matrix.Color(255, 0, 220), //I pink
  matrix.Color(255, 65, 0),  //T reddish
  matrix.Color(255, 220, 0)  //! orange/yellow
};

  const uint16_t sunsetColors[] = {
  matrix.Color(158, 37, 143),
  matrix.Color(253, 68, 112),
  matrix.Color(255, 130, 82), 
  matrix.Color(253, 176, 70),
  matrix.Color(253, 205, 80),
  matrix.Color(253, 231, 103)
};

void setup() {

  pinMode(36, INPUT_PULLUP);
  pinMode(12, OUTPUT);
  digitalWrite(12, HIGH);

  Serial.begin(115200);
  delay (500);
  Serial.println("\nDotstar Matrix Wing");
  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(10);
  
  matrix.clear();
  for (int i = 0; i < 6; i++){
    for (int xpos = 0; xpos < 6; xpos++) {
      if (i % 2 == 0)
            matrix.drawPixel(2*xpos, 5-i, sunsetColors[i]);
          else
            matrix.drawPixel(2*xpos + 1, 5-i, sunsetColors[i]);
    }
    delay(200);
    matrix.show();
  }

  bool repeat = true;
  while (repeat == true){

    if (digitalRead(36) == LOW){
      matrix.clear();
      for (int i = 0; i < 6; i++){
        for (int xpos = 0; xpos < 6; xpos++) {
          if (i % 2 == 0)
            matrix.drawPixel(2*xpos, 5-i, sunsetColors[i]);
          else
            matrix.drawPixel(2*xpos + 1, 5-i, sunsetColors[i]);
        }
      }
      matrix.show();
      for (int i = 0; i < 6; i++){
        for (int xpos = 0; xpos < 12; xpos++) {
          matrix.drawPixel(xpos, i, matrix.Color(0, 0, 0));
        }
        delay(200);
        matrix.show();
      }
      matrix.clear();
      matrix.show();
      delay (200);
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_36,0); //1 = High, 0 = Low
      esp_deep_sleep_start();
    }

      // Reset bandValues[]
    for (int i = 0; i<NUM_BANDS; i++){
      bandValues[i] = 0;
    }
  
    // Sample the audio pin
    for (int i = 0; i < SAMPLES; i++) {
      newTime = micros();
      vReal[i] = analogRead(AUDIO_IN_PIN); // A conversion takes about 9.7uS on an ESP32
      vImag[i] = 0;
      while ((micros() - newTime) < sampling_period_us) { /* chill */ }
    }
  
    // Compute FFT
    FFT.DCRemoval();
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(FFT_FORWARD);
    FFT.ComplexToMagnitude();
  
    // Analyse FFT results
    for (int i = 2; i < (SAMPLES/2); i++){       // Don't use sample 0 and only first SAMPLES/2 are usable. Each array element represents a frequency bin and its value the amplitude.
      if (vReal[i] > NOISE) {                    // Add a crude noise filter
  
       //12 bands, 16kHz top band
        if (i<=2 )           bandValues[0]  += (int)vReal[i];
        if (i>2   && i<=3  ) bandValues[1]  += (int)vReal[i];
        if (i>3   && i<=5  ) bandValues[2]  += (int)vReal[i];
        if (i>5   && i<=8  ) bandValues[3]  += (int)vReal[i];
        if (i>8   && i<=14  ) bandValues[4]  += (int)vReal[i];
        if (i>14   && i<=24 ) bandValues[5]  += (int)vReal[i];
        if (i>24  && i<=40 ) bandValues[6]  += (int)vReal[i];
        if (i>40  && i<=68 ) bandValues[7]  += (int)vReal[i];
        if (i>68  && i<=114 ) bandValues[8]  += (int)vReal[i];
        if (i>114  && i<=193 ) bandValues[9]  += (int)vReal[i];
        if (i>193  && i<=326 ) bandValues[10] += (int)vReal[i];
        if (i>326  && i<=551 ) bandValues[11] += (int)vReal[i];
        
      }
    }

    /*int maxBandValue = 0;
    int maxBand = 0;
    for (int i = 0; i < bandValues.length; i++){
      if (bandValues[i] > maxBandValue){
        maxBandValue = bandValues;
        maxBand = i;
      }
    }
    Serial.println(maxBand + " - " + maxBandValue);*/
    
    // Process the FFT data into bar heights
    for (byte band = 0; band < NUM_BANDS; band++) {
  
      // Scale the bars for the display
      //Serial.println(bandValues[band]);
      int barHeight = (bandValues[band] / AMPLITUDE);
      if (barHeight > TOP) barHeight = TOP;
  
      // Small amount of averaging between frames
      //barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;
  
      drawBars(band, barHeight);
      if (Serial)
        Serial.println(barHeight);
    }
    Serial.println(0);
    Serial.println(0);
    Serial.println(0);
    Serial.println(0);
    Serial.println(0);
    Serial.println(0);
    
    matrix.show();
    matrix.clear();
    
  }
}


void loop() {

}

void drawBars (int band, int barHeight){
  
  for (int ypos = 0; ypos < barHeight; ypos++) {
    matrix.drawPixel(band, 5 - ypos, sunsetColors[ypos]);
  }
}
  
