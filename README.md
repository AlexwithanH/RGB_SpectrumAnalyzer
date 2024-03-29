# RGB_SpectrumAnalyzer
Battery powered 12-band spectrum analyzer with an RGB matrix display. https://imgur.com/0Q14NK4

The sound signal is taken from an electret microphone, amplified with an op-amp, and lastly processed using an FFT algorithm.
In addition to the display, data is also output through serial. Pushing the black button puts the esp32 into deep sleep and disconnects the display to save power. Pushing it again wakes the device back up.

Lots of credit to https://github.com/s-marley/ESP32_FFT_VU , the code used in this project is based on s-marley's code.

![Spectrum graph](https://github.com/AlexwithanH/RGB_SpectrumAnalyzer/blob/main/Images/20220608_155732.jpg)
Coloured spectrum graph

![device internals](https://github.com/AlexwithanH/RGB_SpectrumAnalyzer/blob/main/Images/20220606_173924.jpg)
Internals

![op-amp circuit](https://github.com/AlexwithanH/RGB_SpectrumAnalyzer/blob/main/Images/20220606_174016.jpg)
Op-amp circuit

![overview](https://github.com/AlexwithanH/RGB_SpectrumAnalyzer/blob/main/Images/20220608_155545.jpg)
![overview](https://github.com/AlexwithanH/RGB_SpectrumAnalyzer/blob/main/Images/20220608_155618.jpg)
Overview
