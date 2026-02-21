Other stuff
===========

## Apple jack

Apple headphones have their mini jack shorter than other headphones, which means that some devices will ignore them
(like my Sony phone). NW-A50 accepts them, but you need to make sure jack is fully in. Headphones also have analog
volume control buttons and mic on them. My Samsung tablet is aware of these buttons and reacts to them properly. iPod,
obviously, works. NW-A50 ignores these buttons, why? Perhaps there is no support for them in the driver? Grepping Linux
source for `CTIA/OMTP` returns only Realtek drivers. Or is it hardware? Want a definitive answer.

https://news.ycombinator.com/item?id=37603776

https://superuser.com/questions/1780812/how-to-check-if-a-jack-port-is-ctia-or-omtp

`regmon cxd3778gf:*` show no differences between Apple and non-Apple (without controls) jacks.

## Brightness

NW-A50 is bright as a sun even on lowest brightness setting. There is a `/sys/class/leds/lcd-backlight/brightness` file,
accepting values from 0 (screen off) to 255. 1 is still too bright in a dark room. Value is restored to 22 on power
button press. There is also a `/sys/class/leds/lcd-backlight/duty` file which looks like it has something to do with...
refresh rate? Don't want to mess up the display, but really want to dim it further.
