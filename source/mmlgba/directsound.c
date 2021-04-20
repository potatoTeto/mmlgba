#include "directsound.h"

#include <gba_console.h>
#include <gba_timers.h>
#include <gba_sound.h>
#include <gba_dma.h>
#include <stdio.h>
#include <stdlib.h>

// structure to store all the frequency settings we allow
typedef struct _FREQ_TABLE
{
	u16 timer;
	u16 freq;
	u16 bufSize;

} FREQ_TABLE;

static const FREQ_TABLE freqTable[SND_FREQ_NUM] =
{
	// timer, frequency, and buffer size for frequencies
	// that time out perfectly with VBlank.
	// These are in the order of the SND_FREQ enum in Sound.h.
  /*
  	{ 62610, 5734, 96 },
  	{ 63940, 10512, 176 },
  	{ 64282, 13379, 224 },
  	{ 64612, 18157, 304 },
  	{ 64738, 21024, 352 },
  	{ 64909, 26758, 448 },
  	{ 65004, 31536, 528 },
  	{ 65073, 36314, 608 },
  	{ 65118, 40137, 672 },
  	{ 65137, 42048, 704 },
  	{ 65154, 43959, 736 }
  */
  { 62611, 5735, 96 },
  { 63688, 9079, 151 },
  { 63940, 10513, 176 }, //
  { 64073, 11469, 192 },
  { 64282, 13380, 224 },
  { 64472, 15769, 263 }, // not good
  { 64612, 18158, 304 },
  { 64738, 21025, 350 },
  { 64909, 26760, 447 },
  { 65004, 31537, 526 },
  { 65074, 36316, 605 },
  { 65118, 40138, 669 },
  { 65137, 42049, 701 }
};
// Globals, as seen in tutorial day 2. In the tutorial, I hardcoded
// SOUND_MAX_CHANNELS to 4, but it's the same ffect either way
SOUND_CHANNEL sndChannel[SND_MAX_DS_CHANNELS];
SOUND_VARS sndVars;

// This is the actual double buffer memory. The size is taken from
// the highest entry in the frequency table above.
s8 sndMixBuffer[736*2] IN_EWRAM;

// Declare bitrate here
int sampleBitRate = 8363;
int streamBitRate = 11469;

// Call this once at startup
void dirSndInit(SND_FREQ freq)
{
	s32 i;

  // enable sound
  REG_SOUNDCNT_H = BIT(1) | BIT(2) | BIT(3) | BIT(8) | BIT(9) | BIT(11);//0x0604;//SOUNDA_VOLUME_100 | SOUNDA_LOUT | SOUNDA_ROUT | SOUNDA_FIFORESET;   // = 0x0604
  REG_SOUNDCNT_X = 0x80;
  //REG_SOUNDCNT_H =	SNDA_L_ENABLE | SNDA_R_ENABLE | SNDA_RESET_FIFO | SNDA_VOL_100;
  //REG_SOUNDCNT_X = 0x80;//SOUND_ENABLE;

  // clear the whole buffer area
	i = 0;
	Dma3(sndMixBuffer, &i, 736*2/4, DMA_MEMSET32);

	// initialize main sound variables
	sndVars.mixBufferSize	= freqTable[freq].bufSize;
	sndVars.mixBufferBase	= sndMixBuffer;
	sndVars.curMixBuffer	= sndVars.mixBufferBase;
	sndVars.activeBuffer	= 1;	// 1 so first swap will start DMA

		// initialize channel structures
	for(i = 0; i < SND_MAX_DS_CHANNELS; i++)
	{
		sndChannel[i].sampleData			= 0;
		sndChannel[i].samplePos			= 0;
		sndChannel[i].sampleInc			= 0;
		sndChannel[i].sampleVol			= 0;
		sndChannel[i].sampleLength		= 0;
		sndChannel[i].sampleLoopLength	= 0;

		sndChannel[i].mus_enabled = 1U;
		sndChannel[i].mus_done = 0U;
		sndChannel[i].mus_wait = 0U;
		sndChannel[i].mus_octave = 4U;
		sndChannel[i].mus_length = 48U;
		sndChannel[i].mus_volume = 0xF0U;
		sndChannel[i].mus_env = 3U;
		sndChannel[i].mus_rep_depth = 255U;
		//sndChannel[i].mus_target = 0U;
		sndChannel[i].mus_slide = 0U;
		sndChannel[i].mus_vib_speed = 0U;
		sndChannel[i].mus_po = 128U;
		sndChannel[i].mus_pan = 0x11U;
		sndChannel[i].mus_macro = 0U;

	}

  // start up the timer we will be using
	REG_TM0CNT_L = freqTable[freq].timer;
	REG_TM0CNT_H = 0x80;//TIMER_START;

  // set up the DMA settings, but let the VBlank interrupt
  // actually start it up, so the timing is right
	REG_DMA1CNT = 0;
	REG_DMA1DAD = (u32) &(REG_FIFO_A);//DSOUND_FIFOA);

}	// SndInit

	// Call this every frame to fill the buffer. It can be
	// called anywhere as long as it happens once per frame.



void StepDirectSound()
{
	s32 i, curChn, saturation;

		// If you want to use a higher frequency than 18157,
		// you'll need to make this bigger.
		// To be safe, it would be best to set it to the buffer
		// size of the highest frequency we allow in freqTable
	s16 tempBuffer[736*2];
  saturation = 2.5;// deliberate clipping to get extra crunch

		// zero the buffer
	i = 0;
	Dma3(tempBuffer, &i, sndVars.mixBufferSize*sizeof(s16)/4, DMA_MEMSET32);

	for(curChn = 0; curChn < SND_MAX_DS_CHANNELS; curChn++)
	{
		SOUND_CHANNEL *chnPtr = &sndChannel[curChn];

			// check special active flag value
		if(chnPtr->sampleData != 0)
		{
				// this channel is active, so mix its data into the intermediate buffer
			for(i = 0; i < sndVars.mixBufferSize; i++)
			{
					// mix a sample into the intermediate buffer
				tempBuffer[i] += chnPtr->sampleData[ chnPtr->samplePos>>12 ] * chnPtr->sampleVol;
				chnPtr->samplePos += chnPtr->sampleInc;

					// loop the sound if it hits the end
				if(chnPtr->samplePos >= chnPtr->sampleLength)
				{
					// check special loop on/off flag value
					if(chnPtr->sampleLoopLength == 0)
					{
							// disable the channel and break from the i loop
						chnPtr->sampleData = 0;
						i = sndVars.mixBufferSize;
					}
          else if(chnPtr->sampleLoopLength == -1) // Loop infinitely if -1
  					{
              // loop back
  						while(chnPtr->samplePos >= chnPtr->sampleLength)
  						{
  							chnPtr->samplePos -= chnPtr->sampleLength;
  						}
  					}
					else
					{
							// loop back
						while(chnPtr->samplePos >= chnPtr->sampleLength)
						{
							chnPtr->samplePos -= chnPtr->sampleLoopLength;
						}
					}
				}
			}	// end for i = 0 to bufSize
		}	// end data != 0
	}	// end channel loop

		// now downsample the 16-bit buffer and copy it into the actual playing buffer
	for(i = 0; i < sndVars.mixBufferSize; i++)
	{
			// >>6 to divide off the volume, >>2 to divide by 4 channels
			// to prevent overflow. Could make a define for this up with
			// SND_MAX_CHANNELS, but I'll hardcode it for now
		sndVars.curMixBuffer[i] = (tempBuffer[i] >> 8) * saturation;
	}

}	// SndMix
