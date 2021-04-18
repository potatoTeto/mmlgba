# mmlgba
A MML parser and DevkitPro sound driver for the Nintendo Game Boy Advance. This engine is heavily based on the Gameboy (DMG) sound engine, mmlgb (https://github.com/SimonLarsen/mmlgb) and Deku's implementation of DirectSound (https://deku.gbadev.org/program/sound1.html)

# Dependencies
The only thing required is DevkitPro with libgba. Just configure the Makefile(s) to point to your $(DEVKITPRO) and $(DEVKITARM), then type "make" in terminal

# Special Thanks
@SimonLarsen - Created the original mmlgb engine: https://github.com/SimonLarsen/mmlgb
@hexundev - Provided helpful pointers on getting started with this project, and porting mmlgb to the GBA.
Deku - Created not only the base DirectSound engine but also an immensely helpful tutorial for it (https://deku.gbadev.org/program/sound1.html)
