# RGB_SpectrumAnalyzer
Battery powered 12-band spectrum analyzer with an RGB matrix display. https://imgur.com/0Q14NK4

The sound signal is taken from an electret microphone, amplified with an op-amp, and lastly processed using an FFT algorithm.
In addition to the display, data is also outputed through serial. Pushing the black button puts the esp32 into deep sleep and disconnects the display to save power. Pushing it again wakes the device back up.

Lots of credit to https://github.com/s-marley/ESP32_FFT_VU , the code used in this project is based on s-marley's code.
![bild 1](https://github.com/AlexwithanH/RGB_SpectrumAnalyzer/blob/main/Images/20220606_151129.jpg)
