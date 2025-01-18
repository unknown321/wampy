#general

- [ ] make llusbdac build everywhere, not just on my machine
- [ ] apply tunings on boot
- [ ] mtk backup is broken on nw-a40, update docs
- [x] export bookmarks
- [x] current volume on volume table graphs
- [x] <s>read skins from sd card</s> a lot of code for like what, 10 mb of skins on sd card? just use internal storage
- [x] cassette default tape text pos bug on first start
- [x] <s>digital clock compressed</s> quality drops no matter what
- [x] cassette compressed
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

- [ ] compressed winamp (increased loading time (bad), better memory usage (good), reduced quality(maybe?))
- [ ] winamp: reduce texture amount (but increase memory usage?)
- [ ] visualization from NW-WM1Z spectrum analyzer
- [ ] eq button custom action - too small?
- [ ] single connection over socket?
- [ ] enqueue?
- [x] walkman One settings in wampy settings
- [x] simple clock skin
- [x] region.txt (winamp)
- [x] <s>"restart wampy" button in settings - use case?</s> no use
- [x] <s>clickable lyrics icon (hagoromo)</s> huge cover hides it, no one in their mind would use small cover after huge
  one

# hard to solve

- [ ] time/cover features are not applied on device boot, only after first toggle
- [ ] blink/marquee timers are not precise
- [ ] fps limit works after application restart, because you need to reset `lasttime`; don't want to pass it through
  everything.
- [ ] first clock update is not visible because it is immediately wiped by volume change (which is needed to get first
  volume reading)
- [x] font atlases are too big
- [x] cassette tape combo is hidden by screen edge on hagoromo (culling and rotation)
- [x] <s>winamp playlist flicker sometimes (or is it?)</s> not anymore

