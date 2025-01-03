# general

- [ ] read skins from sd card
  - [ ] inotify for directory change? init signal on card insert?
  - [ ] update skin list
  - [ ] error handling (current skin was deleted, what happens next?)
- [x] update winamp settings screenshot in usage (added clutterbar)
- [x] long cassette titles/artists are incorrectly formatted when changed and active screen is settings
- [x] refactor winamp skin loading with better error handling
- [x] remove title artist duplication on "huge cover art" toggle
- [x] winamp: play in playing state resets track to start and plays it again
- [x] winamp: play in paused state resumes playing
- [x] winamp: next in paused state switches track and plays it
- [x] winamp: pause in stopped state does nothing
- [x] winamp: prev in paused state starts playing previous track from the beginning
- [x] winamp: prev in playing state starts playing previous track from the beginning
- [x] winamp: prev/next in stopped state change tracks but do not start them, there is no seek button
- [x] winamp: marquee works in stopped state
- [x] Error in `/system/vendor/unknown321/bin/wampy': double free or corruption (out): 0xae900fc0
- [x] terminate called after throwing an instance of 'std::length_error' ; what():  basic_string::_ S_create
- [x] README
- [x] loaded winamp skin in settings dropdown
- [x] track sometimes disappears briefly (hagoromo only)
- [x] license in installer
- [x] about in settings
  - [x] commit info
  - [x] license
- [x] license in repo
- [x] fps limiter to config
- [x] remaining time to config
- [x] minus sign if nums_ex not present
- [x] namespace player
- [x] ignore volume/track buttons if render is off
- [x] unexpected bmp formats (windows98.wsz)
- [x] text color in cassette config
- [x] reboot loop protection
- [x] prettify server build
- [x] languages to config
- [x] russian letters
- [x] playlist broken colors?
- [x] balance.bmp can be replaced by volume.bmp
- [x] terminate called after throwing an instance of 'Magick::ErrorCorruptImage' what():  wampy: unexpected
  end-of-file '': No such file or directory @ error/bmp.c/ReadBMPImage/1606
- [x] winamp transparency color (see hp calc skin)
- [x] cassette config validate
- [x] cassette configs to ini
- [x] load tape/reel on change
- [x] cassette loads from config only
- [x] get rid of player, move to connector
- [x] player->connector is private
- [x] nested folder in zip (windows98 skin)
- [x] test with broken skins
- [x] pass mpd socket path in config
- [x] hagoromo volume max is 120
- [x] phantom inputs on toggle?
- [x] fps reduce
- [x] power flicker - ehh no
- [x] first load is empty
- [x] stop doesn't stop track on backend - seek is unpausing?
- [x] missing bitrate
- [x] swap next-prev buttons setting

# want

- [x] walkman One settings in wampy settings
- [x] simple clock skin
- [ ] visualization from NW-WM1Z spectrum analyzer
- [ ] regions.txt (winamp)
- [ ] clickable lyrics icon (hagoromo)
- [ ] eq button custom action - too small?
- [ ] "restart wampy" button in settings - use case?
- [ ] single connection over socket?

# hard to solve

- [x] cassette tape combo is hidden by screen edge on hagoromo (culling and rotation)
- [ ] winamp playlist flicker sometimes (or is it?)
- [ ] time/cover features are not applied on device boot, only after first toggle
- [ ] font atlases are too big
- [ ] blink/marquee timers are not precise
- [ ] fps limit works after application restart, because you need to reset `lasttime`; don't want to pass it through
  everything.
- [ ] first clock update is not visible because it is immediately wiped by volume change (which is needed to get first
  volume reading)
