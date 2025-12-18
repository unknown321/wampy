# Making of FM radio

There is no FM radio on Walkman One. Let's fix that.

You need FM chip on board, so ZX300, WM1A/Z and DMP-1Z are not eligible. These devices are shipped with
mock `libTunerPlayerService.so` (you can see that from service startup log). There is no code, so no radio; forcefully
bringing up standard radio interface results in error.

Even though there is no chip, all devices are shipped with fm driver - `radio-si4708icx.ko` for SI4708 chip. Loading
that driver successfully creates `/dev/radio0` device file; this is where chipless devices fail.

Here are the steps so far:

- bring valid `libTunerPlayerService.so` to the device
- load driver

After these two steps you can invoke FM radio interface and hear some static. However, there is an issue: Walkman One  
identifies itself as radioless model (WM1A/Z), so no GUI is loaded; all you can see is a blank screen with toolbar at
the bottom. There might be a workaround by changing "isAvailableFmRadio" value somewhere in standard interface, but this
is unreliable and tedious. You can control radio via `libTunerPlayerService` and Wampy has some space for "FM" button,
so here it is, blocky interface with full FM range (76-108 MHz) and unlimited presets (30 in standard).

This is the point where this document should end, but guess what? Audio stops about 5 seconds after hitting power
button; this was discovered after writing documentation and packing the release. Since we are enabling radio without
standard player, it thinks that there is nothing to play, changes [power state][1] to `mem` and disables audio input.
Both of those immediately kill radio.

First issue is resolved by creating wakeup source object (see power doc above); second is much harder. On power press
standard player sets a timer with variable duration; on expiration one of the services will turn `analog input device`
ALSA simple control from `tuner` to `off`. You can list all controls and values by using `amixer contents`. There are
three approaches:

- disable that timer in standard application
- prevent unknown service from disabling input
- turn it back on ourselves

As usual, first option sucks. Standard application is complex and hard to navigate without source code; even if that
timer is found, app also must be aware of Wampy-activated fm radio which. In my opinion, sharing state both ways is not
a good idea.

Second option is tedious - must find that service, inject code in it, communicate with it...

Third option looks easy. There is code for monitoring ALSA events - [monitor.c][2]. You can see it in action by
running `amixer sevents`. Except you won't see it in action on the device. This code works by finding control
device (`/dev/snd/controlC0`), getting its file descriptor and watching for filesystem events (like writing into it)
using `poll/epoll`. In our case there are no filesystem events; communication is performed using `ioctl`. You can see it
yourself by running `strace amixer cset name='master volume' 10`. I've spent embarrassing amount of time figuring that
out. That leaves us with polling for input status value every second instead of being notified. I think that one of the
sound libraries is aware of mixer changes, but there is no time to do it.

Finally, radio works just like it supposed to.

Or not, A30 Walkman One fails to load all drivers I provided.

#### Recording audio

Implementation is straightforward, but chip doesn't support [compress offloading][3];
raw pcm data is encoded to mp3 using [shine][4] library.

[1]: https://www.kernel.org/doc/Documentation/ABI/testing/sysfs-power

[2]: https://git.alsa-project.org/?p=alsa-utils.git;a=blob;f=alsactl/monitor.c;hb=HEAD.

[3]: https://docs.kernel.org/sound/designs/compress-offload.html

[4]: https://github.com/toots/shine