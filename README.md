# mmlgba
An MML parser and DevkitPro sound driver for the Nintendo Game Boy Advance. This engine, coded primarily for homebrew use, is heavily based on two sound engines:
* The Gameboy (DMG) sound engine, mmlgb: https://github.com/SimonLarsen/mmlgb
* Deku's interfacing framework for AGB DirectSound: https://deku.gbadev.org/program/sound1.html

# Dependencies
The only thing required is <a href="https://devkitpro.org/wiki/Getting_Started">DevkitPro with libgba.</a> Just configure the Makefile(s) to point to your $(DEVKITPRO) and $(DEVKITARM), then type "make" in terminal

# Special Thanks
<a href="https://github.com/SimonLarsen">@SimonLarsen</a> - Created <a href="https://github.com/SimonLarsen/mmlgb">the original mmlgb engine.</a><br>
<a href="https://github.com/hexundev">@hexundev</a> - Provided helpful pointers on getting started with this project, and porting mmlgb to the GBA.<br>
<a href="https://deku.gbadev.org/program/sound1.html">Deku</a> - Created not only the base DirectSound engine but also wrote <a href="https://deku.gbadev.org/program/sound1.html">an immensely helpful, in-depth tutorial on the hardware.</a>
