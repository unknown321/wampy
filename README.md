wampy
=====

Interface addon for NW-A50/ZX300/WM1A/WM1Z series Walkman players.

Also works on Walkman One [NW-A50][1]/[A40][2]/[A30][3]/[ZX300][4]/[WM1A/Z][5].

[1]: https://www.mrwalkman.com/p/sony-nw-a50series-custom-firmware.html

[2]: https://www.mrwalkman.com/p/walkman-one-for-nw-a40series.html

[3]: https://www.mrwalkman.com/p/walkman-one-nw-a30series.html

[4]: https://www.mrwalkman.com/p/walkman-one-zx300series.html

[5]: https://www.mrwalkman.com/p/walkman-one-for-wm1az.html

<p align="center"><img src="images/promo-a50.png" alt="winamp">&nbsp;<img src="images/promo-a40.png" alt="cassette"></p>
<p align="center"><img src="images/promo-a30.png">&nbsp;<img src="images/promo-zx300.png">&nbsp;<img src="images/promo-wm1z.png"></p>

## Features:

- Winamp 2 skin support
- Custom cassette skins
- On-the-fly skin change
- Volume table editor
- Per-song audio options
- Default player enhancements (add clock and increase cover art size)
- Digital clock skin (pretty!)
- [Low latency USB DAC module](https://github.com/zhangboyang/llusbdac)

See [USAGE.md](./USAGE.md).

## Device support

| Device          | Stock | Walkman One | Notes                                                    |
|-----------------|-------|-------------|----------------------------------------------------------|
| NW-A50          | ✅     | ✅           |                                                          |
| └──A50Z mod     | ✅     | ---         | mod is unavailable                                       |
| NW-A40          | ❌     | ✅           | stock fw is broken, outdated GPU driver; use Walkman One |
| └──[A50 mod][6] | ✅     | ---         | confirmed by community                                   |
| NW-A30          | ❌     | ✅           | stock fw is broken, outdated GPU driver; use Walkman One |
| NW-ZX300        | ?     | ✅           | looking for tester [#12][7]                              |
| NW-WM1A/Z       | ?     | ?           | looking for tester [#13][8]                              |
| DMP-Z1          | ?     | ?           | looking for tester                                       |

[6]: https://www.mrwalkman.com/p/nw-a40-stock-update.html

[7]: https://github.com/unknown321/wampy/issues/12

[8]: https://github.com/unknown321/wampy/issues/13

## Install

### Pre-install

It is recommended to make a backup. See [BACKUP.md](./BACKUP.md).
You should also read [USAGE.md](./USAGE.md) beforehand to get acquainted with quirks and bugs.

### Windows

Download exe from [releases](https://github.com/unknown321/wampy/releases), run and follow instructions.

Device will restart twice.

### Linux/OSX

See [INSTALL.md](./INSTALL.md)

## Build from source

See [BUILD.md](./BUILD.md)

## See also

[Making of](./MAKING_OF.md)

[Making of sound settings](./MAKING_OF_SOUND_SETTINGS.md)

[Making of equalizer](./MAKING_OF_EQUALIZER.md)

[Scrobbler](https://github.com/unknown321/scrobbler)

## Support me

https://boosty.to/unknown321/donate
