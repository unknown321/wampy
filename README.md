Wampy
=====

Interface addon for NW-A50 / ZX300 / WM1A / WM1Z series WALKMAN® players.

Also works on Walkman One for [NW-A50][1] / [A40][2] / [A30][3] / [ZX300][4] / [WM1A/Z][5].

[1]: https://www.mrwalkman.com/p/sony-nw-a50series-custom-firmware.html

[2]: https://www.mrwalkman.com/p/walkman-one-for-nw-a40series.html

[3]: https://www.mrwalkman.com/p/walkman-one-nw-a30series.html

[4]: https://www.mrwalkman.com/p/walkman-one-zx300series.html

[5]: https://www.mrwalkman.com/p/walkman-one-for-wm1az.html

<img src="images/promo.png">

## Features:

- Winamp 2 skin support
- Custom cassette skins
- On-the-fly skin change
- Volume table editor
- Per-song audio options
- All audio filters are available regardless of firmware
- Default player enhancements (add clock and increase cover art size)
- Digital clock skin (pretty!)
- [Low latency USB DAC module](https://github.com/zhangboyang/llusbdac)
- FM radio on devices with FM chip and Walkman One (A30/40/50)

See [USAGE.md](./USAGE.md).

## Device support

| Device          | Stock | Walkman One | Notes                                                    |
|-----------------|-------|-------------|----------------------------------------------------------|
| NW-A50          | ✅     | ✅           |                                                          |
| └──A50Z mod     | ✅     |             | mod is unavailable                                       |
| NW-A40          | ❌     | ✅           | stock fw is broken, outdated GPU driver; use Walkman One |
| └──[A50 mod][6] | ✅     |             | confirmed by community                                   |
| NW-A30          | ❌     | ✅           | stock fw is broken, outdated GPU driver; use Walkman One |
| NW-ZX300        | ?     | ✅           | looking for tester [#12][7]                              |
| NW-WM1A         | ✅     | ✅           | UI doesn't fill the screen                               |
| NW-WM1Z         | ✅     | ✅           | UI doesn't fill the screen                               |
| DMP-Z1          | ?     |             | looking for the bravest tester of them all               |

[6]: https://www.mrwalkman.com/p/nw-a40-stock-update.html

[7]: https://github.com/unknown321/wampy/issues/12

[8]: https://github.com/unknown321/wampy/issues/13

## Install

### Pre-install

It is recommended to make a backup. See [BACKUP.md](./BACKUP.md).
You should also read [USAGE.md](./USAGE.md) beforehand to get acquainted with quirks and bugs.

Check [Device support](#device-support) table, find your device and select supported version for currently installed
firmware.

### Windows

Download exe from [releases](https://github.com/unknown321/wampy/releases), run and follow instructions.

Device will restart twice.

### Linux/OSX

See [INSTALL.md](./INSTALL.md)

## Upgrade

Just install new version, no need to uninstall previous version.

## Uninstall

Run installer, select your model, installed firmware version and check "Uninstall" action.

## Build from source

See [BUILD.md](./BUILD.md)

## Other projects for these DAPs

[fix-coverart](https://github.com/unknown321/fix-coverart) - fixes missing cover art.

[Scrobbler](https://github.com/unknown321/scrobbler) - keeps tracks of played songs in .scrobbler.log for further
submission to Last.fm.

## Development stories

[Making of](./MAKING_OF.md)

[Making of volume tables](./MAKING_OF_VOLUME_TABLES.md)

[Making of equalizer per song](./MAKING_OF_EQUALIZER_PER_SONG.md)

[Making of equalizer / audio filters](./MAKING_OF_EQUALIZER_FILTERS.md)

## Support me

https://boosty.to/unknown321/donate
