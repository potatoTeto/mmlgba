#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <gba_dma.h>
#include <gba_sound.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>
#include "directsound.h"
#include "musMonchi.h"
#include "smpPiano.h"
#include "bigKick.h"

typedef unsigned char UBYTE, UINT8;
typedef unsigned short UWORD;

#define VU8 volatile UBYTE *
#define VU16 volatile UWORD *

//#define	GBA_REG_BASE	0x04000000
//#define CPU_FREQ 16777216

//#define REG_DMA1CNT_H   	(*(volatile u16*)0x40000C6)

/*

// this causes the DMA destination to be the same each time rather than increment
#define DMA_DEST_FIXED 0x400000

// this causes the DMA to repeat the transfer automatically on some interval
#define DMA_REPEAT 0x2000000
#define DMA_ENABLE 0x80000000
#define DMA_16 0x00000000
#define DMA_32 0x04000000
#define DMA_MODE_FIFO 0x0011000000000000

*/
//#define REG_DMA1CNT_H	*(vu16*)(REG_BASE + 0x0c6)
//#define REG_WAITCNT *(vu16*)(REG_BASE + 0x204)
//#define CST_ROM0_1ST_3WAIT 0x0004
//#define CST_ROM0_2ND_1WAIT 0x0010
#define REG_DMA1CNT	*(vu32*)(REG_BASE + 0x0c4)

//#define DMA_SRC_FIXED		BIT(8)
//#define DMA_WORD			BIT(10)
//#define DMA_ENABLE			BIT(15)


s32 i;

void DSSwapAudioBuffers()
{
	if(sndVars.activeBuffer == 1)	// buffer 1 just got over
	{
		// Start playing buffer 0
		//*(volatile u32*)0x40000C4 = 0;
		REG_DMA1CNT = 0;
		//*(volatile u32*)0x40000BC = (u32)sndVars.mixBufferBase;
		REG_DMA1SAD = (u32) sndVars.mixBufferBase; //(u32) &musMonchi; //
		//*(volatile u16*)0x40000C6 = BIT(6) | BIT(9) | BIT(10) | (BIT(12) | BIT(13)) | BIT(15); //DMA_DST_FIXED | DMA_REPEAT | DMA16 | DMA_MODE_FIFO | DMA_ENABLE;
		REG_DMA1CNT_H =	0xB640;//0xB640;//BIT(6) | BIT(9) | BIT(10) | (BIT(12) | BIT(13)) | BIT(15);//0xB640;

		// Set the current buffer pointer to the start of buffer 1
		sndVars.curMixBuffer = sndVars.mixBufferBase + sndVars.mixBufferSize;
		sndVars.activeBuffer = 0;
	}
	else	// buffer 0 just got over
	{
		// DMA points to buffer 1 already, so don't bother stopping and resetting it

		// Set the current buffer pointer to the start of buffer 0
		sndVars.curMixBuffer = sndVars.mixBufferBase;
		sndVars.activeBuffer = 1;
	}
}

int main(void) {

  irqInit();
	irqSet(IRQ_VBLANK, DSSwapAudioBuffers); // Swap audio buffers using DSSwapAudioBuffers() whenever VBlank occurs
  irqEnable(IRQ_VBLANK);

  consoleDemoInit();
	// Initialize the sound engine (in directsound.c)
	SndInit(SND_FREQ_11469);
	int sampleBitRate = 8363;
	int streamBitRate = 11469;

	// Initiate BGM "monchi.raw" converted as "monchi.h/c" on DirectSound Pseudo-Channel 3

  while (1) {
		int keys_pressed, keys_released;

		iprintf("\x1b[0;0HSound Test\n\nL: Unlooped Piano sound\nR: Looped Piano sound\nB: Halt all sound\nStart: Play BGM");
		//\n\n\nSong Pos: %lu\nSong Len: %lu%lu", sndChannel[3].pos, sndChannel[3].length);

		VBlankIntrWait();

		scanKeys();

		keys_pressed = keysDown();
		keys_released = keysUp();

		if (keys_pressed & KEY_START) {
				sndChannel[3].data = 0;
				sndChannel[3].pos = 0;
				sndChannel[3].inc = (12000<<12)/streamBitRate;
				sndChannel[3].vol = 64;
				sndChannel[3].length = 720000 << 12;//0xF0000000;//1089420 << 12;   0x427E30   983040  0x7FFFFFFF    0xF0000000
				sndChannel[3].loopLength = -1;
				sndChannel[3].data = (s8*) musMonchi;
		}

    if (keys_pressed & KEY_L) {
      // make sure the channel is disabled first
      sndChannel[0].data = 0;

      // set up channel vars

      // Start at the start
      sndChannel[0].pos = 0;

      // Calculate the increment. The piano sample was
      // originally 8363Hz, so I've hardcoded it in here.
      // Later we'll want to make a table of info on samples
      // so it can be done automatically.
      // also, it is 12-bit fixed-point, so shift up before the divide
      sndChannel[0].inc			= (10000<<12)/streamBitRate;

      // Set the volume to maximum
      sndChannel[0].vol = 64;

      // The length of the original sample (also 12-bit fixed-point)
      // This will go into our sample info table too
      sndChannel[0].length = 11396 << 12;
      // Set the loop length to the special no-loop marker value
      sndChannel[0].loopLength = 0;

      // Set the data. This will actually start the channel up so
      // it will be processed next time StepDirectSound() is called
      sndChannel[0].data = (s8*) bigKick;
    }

    if (keys_pressed & KEY_R) {
      // make sure the channel is disabled first
      sndChannel[1].data = 0;

      // set up channel vars
      sndChannel[1].pos = 0;

      // Play at 2* frequency. This is one octave above normal
      sndChannel[1].inc = (sampleBitRate*2<<12)/streamBitRate;

      // Same
      sndChannel[1].vol = 64;

      // Same
      sndChannel[1].length = 10516<<12;

      // This one will actually loop. Here, the loop start
      // position (which I checked in the MOD I ripped the
      // sample from) is 9176. So, we take length-loopStart
      // and convert to 12-bit fixed-point like everything else.
      sndChannel[1].loopLength = (10514-9176)<<12;

      // Same
      sndChannel[1].data = (s8*) smpPiano;
    }
		if (keys_released & KEY_R) {
			sndChannel[1].data = 0;
		}

    if (keys_pressed & KEY_B) {
      for (i = 0; i < SND_MAX_CHANNELS; i++) {
        // Disable all the channels by setting the special disable value
        sndChannel[i].data = 0;
      }
    }


		// Fill the next sound buffer. This can be done anywhere you like.
		// It takes a while if you have many channels running,
		// so it's best to do it after you're done with everything else.
		// If you think you could get missed frames, you might want to
		// do this in your VBlank function, just so you know for sure
		// it will happen every frame.
    StepDirectSound();
  }
}
