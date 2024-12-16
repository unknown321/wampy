Angry about... XDA
==================

Usually XDA forums is the first Google link when you search for anything related to tinkering with phones.

> XDA Developers was founded by developers, for developers. It is now a valuable resource for people who want to make
> the most of their mobile devices, from customizing the look and feel to adding new functionality - XDA footer.

XDA is not a resource, it is a forum with all problems of a regular forum, including:

- impossible to find information
- karma
- no content quality check

I've visited XDA on different occasions trying to find information on different devices, such as Sony phones, Samsung
tablets, MediaTek tooling, Qualcomm platforms. And every time I failed.

### Basics

Let's try to figure out how Sony Xperia S boots by googling `site:xdaforums.com how does sony xperia s boot`. First four
threads are about bricked phones, fifth looks like a guide for
starters: https://xdaforums.com/t/xperia-s-info-ref-all-that-u-need-to-know-before-u-begin.1526866/

We are immediately introduced to 3 bootmodes, one of them is regular one and two others are for flashing, but one is an
official way of flashing and other is… unofficial? Then there is some blabber about booting into those modes, basebands,
fastboot drivers... Basically this guide throws random words at you without getting into details. After 4 or 5 posts
like that it all comes together into "hold buttons, swipe to flash and pray".

Perhaps XDA back in 2012 was a different place. What about more recent guides?

### Modern basics

February
2023, https://xdaforums.com/t/how-to-enable-volte-and-5g-without-permanent-root-on-xperia-5-iii-and-1-iii.4551847/

Nope, just like 2012, random program names are thrown at you. Install this exe, run it like this, next step. Now this
apk, don't ask what does it do, just execute orders.

First reply? `Amazing detailed guide. Thanks for sharing.`

### Asking easier questions

All right, maybe stuff like this IS complicated and explaining everything in one guide will take pages of text. Let's
ask something else, which should have a definitive answer. How about scatter file? From what I've gathered, this a file
with partition info used with various flashing tools, so you could flash only one partition instead of overwriting whole
memory device.

Here is the example:
<details>
<summary>MT8163 scatter file example</summary>

```
#########################################__WwR_MTK__########################################################
#
#  General Setting
#
#########################################__WwR_MTK__########################################################
- general: MTK_PLATFORM_CFG
  info: 
    - config_version: V1.1.2
      platform: MT8163
      project: Test
      storage: EMMC
      boot_channel: MSDC_0
      block_size: 0x20000
############################################################################################################
#
#  Layout Setting
#
############################################################################################################
- partition_index: SYS0
  partition_name: preloader
  file_name: preloader_tb8163p3_64_bsp.bin
  is_download: true
  type: SV5_BL_BIN
  linear_start_addr: 0x0
  physical_start_addr: 0x0
  partition_size: 0x40000
  region: EMMC_BOOT_1
  storage: HW_STORAGE_EMMC
  boundary_check: true
  is_reserved: false
  operation_type: BOOTLOADERS
  reserve: 0x0
  
- partition_index: SYS1
  partition_name: pgpt
  file_name: NONE
  is_download: false
  type: NORMAL_ROM
  linear_start_addr: 0x00
  physical_start_addr: 0x00
  partition_size: 0x80000
  region: EMMC_USER
  storage: HW_STORAGE_EMMC
  boundary_check: true
  is_reserved: false
  operation_type: INVISIBLE
  reserve: 0x0
...
```

</details>

Okay, but what exactly it is? Is there some doc describing the format?
Google: `site:xdaforums.com what is scatter file`.

Google responds with 9 links of questions about getting the file,
and [one (first one! could it be it?)](https://xdaforums.com/t/is-a-scatter-file-needed-for-building-a-custom-rom.4687546/)
that says right there:

> Scatter files are just used to point to the directories of images.

This is a post by `Forum Moderator` and `Staff member` named `fl0w` (1184 reactions score and 25026 XP points, sound
very reputable, right?) from 22 Aug 2024. As we can clearly see from MT8163 example, scatter file is much more than
simple `<partition:image file>` map file. This `Staff member` is either wrong or omitting a bunch of details.

Another search (`scatter file syntax`), another page of questions and step-by-step guides without answers. Is it me
asking wrong questions or there is no answer available online? By the way, there is no search form for unregistered
users, you have to join the community.

#### Why?

Why bother with scatter file details? NW-A50 is a MediaTek device, but regular tools (MTK Droid Tools, SP Flash Tool,
something else?) failed to dump partitions for reasons, asking for scatter
file. [mtkclient](https://github.com/bkerler/mtkclient) just works without any files, dumping whole ROM, but:

- some commands fail
- it takes ~20 minutes to fully reflash 16GB, when only one partition needs a reflash

So I decided to take a look into scatter files and here we are - not a lot of knowledge gained from
> valuable resource for people who want to make the most of their mobile devices

<details>
<summary>So what is it?</summary>

This is a file with a list of device partitions with following instructions:

- source file that will be used during flash
- name
- physical/logical addresses
- region in memory (?)
- and other options I have no idea about, like `is_reserved`

(That's right, I failed to gather a detailed answer from crops on the internet)

Partition table might be available in `/proc/mtd`, `/proc/emmc`, kernel source.

<details>
<summary>NW-A50 /proc/emmc file</summary>

```shell
bash-3.2# cat /proc/emmc               
partno:    start_sect   nr_sects  partition_name
emmc_p1: 00000400 00000002 "ebr1"
emmc_p2: 00002800 00001800 "pro_info"
emmc_p3: 00004000 00002800 "nvram"
emmc_p4: 00006800 00005000 "protect_f"
emmc_p5: 0000b800 00005000 "protect_s"
emmc_p6: 00010800 00000100 "seccfg"
emmc_p7: 00010900 00000300 "uboot"
emmc_p8: 00010c00 00008000 "bootimg"
emmc_p9: 00018c00 00008000 "recovery"
emmc_p10: 00020c00 00003000 "sec_ro"
emmc_p11: 00023c00 00000400 "misc"
emmc_p12: 00024000 00001800 "logo"
emmc_p13: 00025800 00005000 "expdb"
emmc_p14: 0002a800 00002800 "tee1"
emmc_p15: 0002d000 00002800 "tee2"
emmc_p16: 0002f800 00000800 "kb"
emmc_p17: 00030000 00000800 "dkb"
emmc_p18: 00030800 00000800 "xhrome"
emmc_p19: 00031000 00190000 "android"
emmc_p20: 001c1000 0000e000 "cache"
emmc_p21: 001cf000 00000400 "cm4"
emmc_p22: 001cf400 00007800 "nvp"
emmc_p23: 001d6c00 00037000 "var"
emmc_p24: 0020dc00 00032000 "db"
emmc_p25: 0023fc00 00004000 "option1"
emmc_p26: 00243c00 00040000 "option2"
emmc_p27: 00283c00 00200000 "option3"
emmc_p28: 00483c00 00014000 "usrdata"
emmc_p29: 00497c00 01886c00 "contents"
```

</details>

<details>
<summary>Excerpt from kernel, arch/arm/mach-mt8590/bx8590m1_emmc/common/partition_define_private.h</summary>

```c++
#include "partition_define.h"
#if 1
static const struct excel_info PartInfo_Private[PART_NUM]={
                        {"preloader",   262144,     0, EMMC, 0,EMMC_PART_BOOT1},
                        {"mbr",         524288,     0x0, EMMC, 0,EMMC_PART_USER},
                        {"ebr1",        524288,     0x80000, EMMC, 1,EMMC_PART_USER},
                        {"pro_info",    3145728,    0x100000, EMMC, 0,EMMC_PART_USER},
                        {"nvram",       5242880,    0x400000, EMMC, 0,EMMC_PART_USER},
                        {"protect_f",   10485760,   0x900000, EMMC, 2,EMMC_PART_USER},
                        {"protect_s",   10485760,   0x1300000, EMMC, 3,EMMC_PART_USER},
...
```

</details>

With these you *probably* can make a scatter file and feed it to tools, but I settled on `mtkclient`. There is also
a shortcut I discovered - you can flash ~10% (about 1GB) of previously dumped firmware to overwrite partitions that
matter and pull the cable out. Device *usually* boots fine.

Note that `bx8590m1_emmc` is not the same platform as actual device. According to `/proc/config.gz`, platform
is `BBDMP5_linux` (`CONFIG_ARCH_MTK_PROJECT`), but there are no partition definitions there. I suppose partitions
are taken from some other platform, but kernel sources are kinda hard to navigate, so I chose `bx8590m1_emmc` as an
example.

</details>

Just so you know, these are NOT cherry-picked threads to make people look bad, I literally used at least somewhat
relevant links on first page of Google Search.

---

I assume that there are two main categories of users on XDA.

- First, there is a newbie user, who wants to flash his phone (or already bricked it). He asks innocent questions, gets
  "this is a duplicate question, topic closed" from Senior Moderator, posts in another thread and gets banned - you
  know,
  regular forum experience.

- Second, power users. These have titles, lots of "experience points" (what is this), stars in their profiles and
  thousands of messages. They are the guys who throw random words at you, like Magisk, Shizuku, ODIN, LAHAINA, IMS, TWRP
  and others.

There is no "diving into details and telling others" user between them. No one makes guides explaining **what** every
step does, it's only **how to execute** that step. That makes sense, because newbies are not interested in details, all
they need is a shiny new firmware (camera doesn't work, 2G only) on their $40 phone. I am pretty sure that most of the
power users are not aware of inner workings of tools they use. It's just "unpack firmware with X.exe, do Y, try Z.exe if
X.exe fails" knowledge.

There is also a third group, which makes a couple of posts linking to their new shiny opensource tools replacing that
old garbage power users used to work with. These are the guys who usually know what are they doing and not afraid to
share their knowledge with others. GitHub issues for that project becomes a part of XDA with "please help
bricked" issues. Sooner or later author stops communicating, because hell is other people.
A great (and sad) example: https://github.com/phhusson/treble_experimentations

---

### Summary

In my opinion, XDA is not a place where you can learn technical knowledge and understand how your device works. This is
a place where step-by-step guides are born and bricked phones rest.