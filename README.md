Wampy
=====

Interface addon for NW-A50 / ZX300 / WM1A / WM1Z series WALKMAN® players.

Also works on Walkman One for [NW-A50][1] / [A40][2] / [A30][3] / [ZX300][4] / [WM1A/Z][5].

[1]: https://www.mrwalkman.com/p/sony-nw-a50series-custom-firmware.html

[2]: https://www.mrwalkman.com/p/walkman-one-for-nw-a40series.html

[3]: https://www.mrwalkman.com/p/walkman-one-nw-a30series.html

[4]: https://www.mrwalkman.com/p/walkman-one-zx300series.html

[5]: https://www.mrwalkman.com/p/walkman-one-for-wm1az.html

<img src="images/promo-github-dark-bg.png">

## Features:

- Winamp 2 skin support
- Custom cassette skins
- On-the-fly skin change
- Volume table editor
- Per-song audio options
- All audio filters are available (EQ 6/10 band, EQ Tone etc.)
- Standard interface enhancements (clock and cover art size)
- Digital clock skin (pretty!)
- [Low latency USB DAC module](https://github.com/zhangboyang/llusbdac)
- FM radio on devices with FM chip and Walkman One (A40/50 only)

See [USAGE.md](./USAGE.md).

## Device support

| Device\firmware | Stock | WalkmanOne | Notes                                                    |
|-----------------|-------|------------|----------------------------------------------------------|
| NW-A50          | ✅     | ✅          |                                                          |
| └──A50Z mod     | ✅     |            | mod is unavailable                                       |
| NW-A40          | ❌     | ✅          | stock fw is broken, outdated GPU driver; use Walkman One |
| └──[A50 mod][6] | ✅     |            | confirmed by community                                   |
| NW-A30          | ❌     | ✅          | stock fw is broken, outdated GPU driver; use Walkman One |
| NW-ZX300        | ✅     | ✅          |                                                          |
| NW-WM1A         | ✅     | ✅          | UI doesn't fill the screen                               |
| NW-WM1Z         | ✅     | ✅          | UI doesn't fill the screen                               |
| DMP-Z1          | ?     |            | looking for the bravest tester of them all               |

[6]: https://www.mrwalkman.com/p/nw-a40-stock-update.html

[7]: https://github.com/unknown321/wampy/issues/12

[8]: https://github.com/unknown321/wampy/issues/13

## Install

### Pre-install

You should read [USAGE.md](./USAGE.md) beforehand to get acquainted with quirks and bugs.

Check [Device support](#device-support) table (the one above), find your device and currently installed firmware. Make
sure you have at least 100MB free on internal storage.

### Windows

Download exe from [releases](https://github.com/unknown321/wampy/releases/latest), run and follow instructions. Make
sure that your internal storage is named "WALKMAN":

<img src="images/explorer.png">

Device will restart twice during installation.

### Linux/OSX

See [INSTALL.md](./INSTALL.md)

### Post-install

Toggle Hold switch to start Wampy after device fully loads (post "creating database" progressbar).

## Upgrade

Just install new version, no need to uninstall previous version.

## Uninstall

Run installer, select "Uninstall" action, follow instructions.

## Build from source

See [BUILD.md](./BUILD.md)

## Other projects for these DAPs

[fix-coverart](https://github.com/unknown321/fix-coverart) - fixes missing cover art.

[Scrobbler](https://github.com/unknown321/scrobbler) - keeps tracks of played songs in .scrobbler.log for further
submission to Last.fm.

[tpp-wampy](https://github.com/unknown321/tpp-wampy) - MGSV:TPP cassette skin

## Development stories

[Making of](./MAKING_OF.md)

[Making of volume tables](./MAKING_OF_VOLUME_TABLES.md)

[Making of equalizer per song](./MAKING_OF_EQUALIZER_PER_SONG.md)

[Making of equalizer / audio filters](./MAKING_OF_EQUALIZER_FILTERS.md)

[Making of FM radio](./MAKING_OF_FM.md)

[Making of visualizer](./MAKING_OF_VIS.md)

## Support me

https://boosty.to/unknown321/donate
