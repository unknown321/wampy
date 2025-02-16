# Making of visualizer

Winamp has 2 modes of built-in visualization - oscilloscope and spectrum analyzer. There is a spectrum analyzer
interface in Walkman, WM1A/Z and ZX300 models, looks like a low-hanging fruit for me.

Spectrum data comes from `libAudioAnalyzerService.so`, which is the same on all devices (stock A50 and WM1Z).

As usual, there are different approaches:

- intercept data between standard application and service
- connect to the service as a legitimate client

Since there is no documentation, it makes sense to stand between app and service first to figure out the correct chain
of commands and reproduce it later as a client (if possible). This is achieved by using `LD_PRELOAD` mechanism that
was already mentioned in previous development stories. However, not all functions are
interceptable - [virtual functions][1] are not overridable, because they are called not by symbol name, but by vtable
address + offset. Some of the `AudioAnalyzerService` functions were unavailable, so I had to guess (or decompile) which
parameters are sent.

So how does setup looks like? Standard application sets mode (1 to enable), sets bands to analyze (50, 100, 160, 250,
500, 750, 1000, 2000, 4000, 8000, 16000, 28000), sets mean(?) value for each band (456 for all, 406 for 28000 - where do
these values come from?) and tells service to send notifications on spectrum value change. I tried to grab data directly
from service after spectrum changed, but it was very laggy for whatever reason (standard app was fine), meaning that
Wampy must become a proper client, not a man-in-the-middle. After some trial and error I got a list of rapidly changing
numbers (12 per batch) ranging from 40k to millions. These values are audio amplitudes, which must be converted to sound
pressure levels. That process involves using `"math.h"` header and `clang` failed on me, telling that some math
functions are unavailable because... uhh, just because. The compiler must be explicitly told about processor features,
in my case it was

```shell
--mfpu=vfp \
--mfloat-abi=hard \
```

C/C++ compiler errors are *horrible*, there was not a single hint about the error source. Later it failed in the same
way and refused to work even with these options, because `__CORRECT_ISO_CPP11_MATH_H_PROTO_FP` was defined in another
system header and compiler decided to leave me without float comparison functions. I "solved" that problem by moving
code from client code (pstserver) into Wampy, which is built with `gcc`. That code belonged to Wampy anyway, but these
issues came out during "will it work" development stage when code was changed everywhere.

Here is another problem: standard player is not aware of other AudioAnalyzerService clients and throws popups with "An
error has occurred, please reboot" messages if another client connected to the service before it. Thus,
AudioAnalyzerService library is patched during Wampy installation to ignore duplicate start calls
(`pst::services::audioanalyzerservice::AudioAnalyzerServiceServiceImpl::DoStart` function).

After that procedure I finally got the same values as standard app.

Guess what, they look **bad** compared to Winamp.

Video:

[Winamp, desktop](./images/winamp-spectrum.mp4)

[Walkman, A50 with Walkman One](./images/walkman-spectrum.mp4)

Sample audio files: [link][2].

Winamp does 76 measurements with step of 150Hz (see winamp source code, SA.cpp). Values are then averaged in groups of
4, making that 19 bands. Webamp displays same behaviour, but Walkman is special. It makes only 12 measurements (FFT?)
without an option to increase that amount. Unfortunately decompiled code is not so clear to me, and I am not good at
audio processing to recognize the process. Anyway, the result looks horrible; there is not enough data. Is there
anything we can do about it?

Here is what we can do:

- parse audio packet ourselves and make as many measurements as needed
- try to match Winamp settings

First option is complicated, audio packet changes constantly; it's format is unknown - will take too much dev time.

Second option is more or less achievable, so let's try that. Bands can be changed to be in even intervals; this option
is called "Visualizer Winamp mode" in Wampy. Obviously, it leaves behind half of the existing spectrum, but the result
looks much closer to what is heard. Missing bands can be interpolated from their neighbours, which also looks more or
less fine. Values are on a linear scale, so half of the display is almost always empty. Wampy applies `arctan` to them,
making changes in lower range more significant; you can increase the threshold in settings. Winamp also has refresh rate
and peak falloff options. I was unable to replicate them exactly due to the lack of time, difference in input refresh
rate, lack of input data and poor readability of Winamp's code.

Final issue: bands don't fit into band rectangle because of upscaling (800/275 = 2.909...). So a couple of bands are
narrower than others. Band positioning is also a little off to compensate scaling.

Still, the result looks pretty close to the real thing; hope you like it too.

[1]: https://en.wikipedia.org/wiki/Virtual_method_table

[2]: https://www.dr-lex.be/cgi-bin/download.pl?f=software/download/mp3sweeps-CA.zip