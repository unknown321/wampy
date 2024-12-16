```shell
adb pull /install_update_script/resource/fwup_bg.tgz
tar xf fwup_bg.tgz
dd if=fwup_bg.rgb of=fwup_bg_cut.rgb count=1536000 bs=1
convert  -size 480x800 -depth 8 rgba:fwup_bg_cut.rgb out.png
```
