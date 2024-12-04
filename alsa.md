## Alsa notes

To visualize sound you need a sound stream to analyze. Device uses ALSA. Here is the issue: default output device does
not provide capture interface, only playback one.

Here are some notes on that:

#### Regular playback

Press "play" button:

```shell
# aplay -l | grep -B1 Subdevices
card 0: sonysoccard [sony-soc-card], device 0: cxd3778gf-hires-out DAI_CXD3778GF_DAC-0 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 1: cxd3778gf-standard DAI_CXD3778GF_STD-1 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 2: dsdenc DAI_CXD3778GF_ICX-2 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 3: cxd3778gf-dsd-out DAI_CXD3778GF_ICX-3 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 4: cxd3778gf-icx-lowpower DAI_CXD3778GF_ICX-4 []
  Subdevices: 0/1                                                      <---- here it is
--
card 0: sonysoccard [sony-soc-card], device 5: cxd3778gf-icx-lowpower_test DAI_CXD3778GF_ICX-5 []
  Subdevices: 1/1
```

#### Splitting into two?

```shell
# aplay -D hw:0,4  /contents/1.wav --dump-hw-params
Playing WAVE '/contents/1.wav' : Signed 16 bit Little Endian, Rate 44100 Hz, Stereo
HW Params of device "hw:0,4":
--------------------
ACCESS:  RW_INTERLEAVED
FORMAT:  S16_LE S32_LE DSD_U8 DSD_U16_LE
SUBFORMAT:  STD
SAMPLE_BITS: [8 32]
FRAME_BITS: [16 64]
CHANNELS: 2
RATE: [8000 384000]
PERIOD_TIME: [1000 1920000]
PERIOD_SIZE: [384 15360]
PERIOD_BYTES: [3072 30720]
PERIODS: [8 1024]
BUFFER_TIME: [8000 131072000]
BUFFER_SIZE: [3072 1048576]
BUFFER_BYTES: [6144 2097152]
TICK_TIME: ALL
```

Patch and build `snd_aloop` kernel module:

`CONFIG_SND_ALOOP=m`

```diff
--- sound/drivers/aloop.c	2018-07-18 10:30:55.000000000 +0300
+++ ../linux/sound/drivers/aloop.c	2024-09-30 10:32:54.270468455 +0300
@@ -426,18 +426,21 @@
 	.info =		(SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_MMAP |
 			 SNDRV_PCM_INFO_MMAP_VALID | SNDRV_PCM_INFO_PAUSE |
 			 SNDRV_PCM_INFO_RESUME),
-	.formats =		SNDRV_PCM_FMTBIT_S32_LE,
+    .formats = (SNDRV_PCM_FMTBIT_S16_LE
+				| SNDRV_PCM_FMTBIT_S32_LE
+				| SNDRV_PCM_FMTBIT_DSD_U8
+				| SNDRV_PCM_FMTBIT_DSD_U16_LE),
 #ifndef CONFIG_SND_ALOOP_RATE_48K_SERIES_ONLY_MODE
 	.rates =		SNDRV_PCM_RATE_44100 |
-			 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000,
+			 SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400,
 	.rate_min =		44100,
 #else
-	.rates =	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000,
+	.rates =	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_176400,
 	.rate_min =		48000,
 #endif
-	.rate_max =		96000,
-	.channels_min =		1,
-	.channels_max =		32,
+	.rate_max =		176400,
+	.channels_min =		2,
+	.channels_max =		2,
 	.buffer_bytes_max =	2 * 1024 * 1024,
 	.period_bytes_min =	64,
 	/* note check overflow in frac_pos() using pcm_rate_shift before

```

```shell
# pc
adb push sound/drivers/snd-aloop.ko /system/vendor/unknown321/

# device
insmod /system/vendor/unknown321/snd-aloop.ko
```

/etc/asound.conf (device):

```
pcm.snd_card { # my usual sound card
    type hw
    card 0
#    device 4
}

ctl.!default { # default control; alsamixer and such will use this
    type hw
    card 0
}

# software mixer for sound card
pcm.dmixer {
    type dmix
    ipc_key 1024
    ipc_perm 0666 # allow other users
    slave.pcm "snd_card"
    slave {
        period_time 0
        period_size 1024
        buffer_size 4096
        channels 2 # must match bindings
    }
    bindings {
        0 0
        1 1
    }
}

# software mixer for loopback device
pcm.dmixerloop {
    type dmix
    ipc_key 2048
    ipc_perm 0666 # allow other users
    slave.pcm "hw:Loopback,0,0"
    slave {
        period_time 0
        period_size 1024
        buffer_size 4096
        channels 2 # must match bindings
    }
    bindings {
        0 0
        1 1
    }
}

# allows multiple programs to capture simultaneously
pcm.dsnooper {
    type dsnoop
    ipc_key 2048
    ipc_perm 0666
    slave.pcm "snd_card"
    slave
    {
        period_time 0
        period_size 1024
        buffer_size 4096
        channels 2
    }
    bindings {
        0 0
        1 1
    }
}

pcm.!default {
    type asym
    playback.pcm "out"
    capture.pcm "dsnooper"
}

# Multi, splitting onto usual card and loopback
pcm.out {
    type plug
    slave.pcm {
        type multi
        slaves {
            a { channels 2 pcm "dmixer" }
            b { channels 2 pcm "dmixerloop" }
        }
        bindings {
            0 { slave a channel 0 }
            1 { slave a channel 1 }
            2 { slave b channel 0 }
            3 { slave b channel 1 }
        }
    }
    ttable [
        [ 1 0 1 0 ]   # left  -> a.left,  b.left
        [ 0 1 0 1 ]   # right -> a.right, b.right
    ]
}
```

```shell
# aplay /contents/1.wav --dump-hw-params
Playing WAVE '/contents/1.wav' : Signed 16 bit Little Endian, Rate 44100 Hz, Stereo
HW Params of device "out":
--------------------
ACCESS:  MMAP_INTERLEAVED MMAP_NONINTERLEAVED MMAP_COMPLEX RW_INTERLEAVED RW_NONINTERLEAVED
FORMAT:  S8 U8 S16_LE S16_BE U16_LE U16_BE S24_LE S24_BE U24_LE U24_BE S32_LE S32_BE U32_LE U32_BE FLOAT_LE FLOAT_BE FLOAT64_LE FLOAT64_BE MU_LAW A_LAW IMA_ADPCM S24_3LE S24_3BE U24_3LE U24_3BE S20_3LE S20_3BE U20_3LE U20_3BE S18_3LE S18_3BE U18_3LE U18_3BE
SUBFORMAT:  STD
SAMPLE_BITS: [4 64]
FRAME_BITS: [4 640000]
CHANNELS: [1 10000]
RATE: [4000 4294967295)
PERIOD_TIME: (21333 21334)
PERIOD_SIZE: (85 91628833)
PERIOD_BYTES: (42 4294967295)
PERIODS: (0 4311811)
BUFFER_TIME: [1 4294967295]
BUFFER_SIZE: [170 366503875]
BUFFER_BYTES: [85 4294967295]
TICK_TIME: ALL
--------------------


# aplay -l | grep -B1 Subdevices
card 0: sonysoccard [sony-soc-card], device 0: cxd3778gf-hires-out DAI_CXD3778GF_DAC-0 []
  Subdevices: 0/1                                                        <-------------
--
card 0: sonysoccard [sony-soc-card], device 1: cxd3778gf-standard DAI_CXD3778GF_STD-1 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 2: dsdenc DAI_CXD3778GF_ICX-2 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 3: cxd3778gf-dsd-out DAI_CXD3778GF_ICX-3 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 4: cxd3778gf-icx-lowpower DAI_CXD3778GF_ICX-4 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 5: cxd3778gf-icx-lowpower_test DAI_CXD3778GF_ICX-5 []
  Subdevices: 1/1
--
card 1: Loopback [Loopback], device 0: Loopback PCM [Loopback PCM]
  Subdevices: 7/8                                                        <-------------
--
card 1: Loopback [Loopback], device 1: Loopback PCM [Loopback PCM]
  Subdevices: 8/8
```

`cava` config:

```ini
[input]
method = alsa
source = hw:Loopback,1

[output]
method = raw
```

cava spills bytes onto screen = it works... for aplay

#### Redirect to loopback?

`/system/vendor/sony/lib/libaudiohal-adleralsa.so`, look for `hw:0,4`, replace with `hw:1,0`

```shell
chmod 0755 /system/vendor/sony/lib/libaudiohal-adleralsa.so
chown root:shell /system/vendor/sony/lib/libaudiohal-adleralsa.so
```

Now music player plays music through loopback device, so..?

```shell
# aplay -l | grep -B1 Subdevices
card 0: sonysoccard [sony-soc-card], device 0: cxd3778gf-hires-out DAI_CXD3778GF_DAC-0 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 1: cxd3778gf-standard DAI_CXD3778GF_STD-1 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 2: dsdenc DAI_CXD3778GF_ICX-2 []
  Subdevices: 1/1
--
card 0: sonysoccard [sony-soc-card], device 3: cxd3778gf-dsd-out DAI_CXD3778GF_ICX-3 []
  Subdevices: 1/1

card 0: sonysoccard [sony-soc-card], device 4: cxd3778gf-icx-lowpower DAI_CXD3778GF_ICX-4 []
  Subdevices: 1/1                                                      <---- :(
--
card 0: sonysoccard [sony-soc-card], device 5: cxd3778gf-icx-lowpower_test DAI_CXD3778GF_ICX-5 []
  Subdevices: 1/1
--
card 1: Loopback [Loopback], device 0: Loopback PCM [Loopback PCM]
  Subdevices: 7/8                                                      <---- ok
--
card 1: Loopback [Loopback], device 1: Loopback PCM [Loopback PCM]
  Subdevices: 8/8
```

libaudiohal uses device directly bypassing `asound.conf`?
