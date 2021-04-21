#include "mmlgba.h"
#include <string.h>
#include <gba_console.h>
#include <gba_timers.h>
#include "music.h"
#include "notes.h"
#include "freq.h"
#include "noisefreq.h"
#include "vib.h"
#include "directsound.h"
#include "../data/samples/smpPiano.h"
#include "../data/samples/bigKick.h"

#define MAX_REPEATS 4

UBYTE mus_paused;
UBYTE* mus_song;

UBYTE mus_enabled1, mus_enabled4;
UBYTE mus_done1, mus_done2, mus_done3, mus_done4;
UWORD mus_freq1, mus_freq2, mus_freq3;
UBYTE mus_freq4;
UBYTE* mus_data1, * mus_data2, * mus_data3, * mus_data4, * ds_mus_data[SND_MAX_DS_CHANNELS];
UBYTE* mus_data1_bak, * mus_data2_bak, * mus_data3_bak, * mus_data4_bak, * ds_mus_data_bak[SND_MAX_DS_CHANNELS];
UBYTE* mus_wave;
UBYTE* mus_loop1, * mus_loop2, * mus_loop3, * mus_loop4, * ds_mus_loop[SND_MAX_DS_CHANNELS];
UBYTE mus_octave1, mus_octave2, mus_octave3, mus_octave4;
UBYTE mus_length1, mus_length2, mus_length3, mus_length4;
UBYTE mus_volume1, mus_volume2, mus_volume3, mus_volume4;
UBYTE mus_env1, mus_env2, mus_env4;
UBYTE mus_pan1, mus_pan2, mus_pan3, mus_pan4;
UBYTE mus_duty1, mus_duty2;
UBYTE mus_wait1, mus_wait2, mus_wait3, mus_wait4;
UWORD mus_target1, mus_target2, mus_target4;
UBYTE mus_slide1, mus_slide2, mus_slide4;
UBYTE mus_vib_speed1, mus_vib_speed2;
UBYTE* mus_vib_table1, * mus_vib_table2;
UBYTE mus_vib_pos1, mus_vib_pos2;
UBYTE mus_vib_delay1, mus_vib_delay2;
UBYTE mus_noise_step;
UBYTE mus_po1, mus_po2, mus_po3;
UBYTE mus_macro1, mus_macro2, mus_macro3, mus_macro4;

UBYTE* mus_rep1[MAX_REPEATS], * mus_rep2[MAX_REPEATS], * mus_rep3[MAX_REPEATS], * mus_rep4[MAX_REPEATS];
UBYTE mus_repeats1[MAX_REPEATS], mus_repeats2[MAX_REPEATS], mus_repeats3[MAX_REPEATS], mus_repeats4[MAX_REPEATS];
UBYTE mus_rep_depth1, mus_rep_depth2, mus_rep_depth3, mus_rep_depth4;
u16 periodTable[12*5];

// On GB this is 255 - value
UWORD getTMAValue(UBYTE value)
{
	return 0xFFFF - (UWORD)value;
	//return 0xFF - (UBYTE)value;//0xFFFF - (UWORD)value;
}

void mus_init(UBYTE* song_data) {
	UBYTE i, j;


	// 5 octaves' worth of space. Create the DirectSound periodTable so we can play music notes at correct frequencies

	u8 octave, note;

	for(octave = 0; octave < 5; octave++)
	{
	   for(note = 0; note < 12; note++)
	   {
	         // *2 to get us into octave 0, then divide by 2 for each octave up from there
	      periodTable[octave*12 + note] = (octave1Period[note]*2) >> octave; // octave1Period is from freq.c
	   }
	}

	/*NR52_REG = 0x80U; // Enable sound
	NR51_REG = 0xFFU;
	NR50_REG = 0xFFU;*/

	// GBA Init
	SND_STAT = SNDSTAT_ENABLE |
		SNDSTAT_SQR1 | SNDSTAT_SQR2 |
		SNDSTAT_TRI | SNDSTAT_NOISE;

	DMGSNDCTRL = DMGSNDCTRL_LSQR1 | DMGSNDCTRL_RSQR1 |
		DMGSNDCTRL_LSQR2 | DMGSNDCTRL_RSQR2 |
		DMGSNDCTRL_LTRI | DMGSNDCTRL_RTRI |
		DMGSNDCTRL_LNOISE | DMGSNDCTRL_RNOISE;

	// Kill sound
	/*NR10_REG = 0U;
	NR12_REG = 0U;
	NR22_REG = 0U;
	NR30_REG = 0U;
	NR32_REG = 0U;
	NR42_REG = 0U;

	// Setup timer
	//TAC_REG = 0x04U; // TAC clock = 4096 Hz
	//REG_TM3CNT_L = 255U - 51U; // Default to ~150 bpm*/

	// Setup data
	mus_paused = 0U;

	mus_song = song_data;
	mus_data1 = mus_loop1 = song_data + ((UWORD*)song_data)[0];
	mus_data2 = mus_loop2 = song_data + ((UWORD*)song_data)[1];
	mus_data3 = mus_loop3 = song_data + ((UWORD*)song_data)[2];
	mus_data4 = mus_loop4 = song_data + ((UWORD*)song_data)[3];
	mus_wave = song_data + ((UWORD*)song_data)[4 + SND_MAX_DS_CHANNELS];

	mus_enabled1 = mus_enabled4 = 1U;
	mus_done1 = mus_done2 = mus_done3 = mus_done4 = 0U;
	mus_wait1 = mus_wait2 = mus_wait3 = mus_wait4 = 0U;
	mus_octave1 = mus_octave2 = mus_octave3 = mus_octave4 = 4U;
	mus_length1 = mus_length2 = mus_length3 = mus_length4 = 48U;
	mus_volume1 = mus_volume2 = mus_volume4 = 0xF0U;
	mus_volume3 = 32U;
	mus_env1 = mus_env2 = mus_env4 = 3U;
	mus_rep_depth1 = mus_rep_depth2 = mus_rep_depth3 = mus_rep_depth4 = 255U;
	mus_slide1 = mus_slide2 = mus_slide4 = 0U;
	mus_vib_speed1 = mus_vib_speed2 = 0U;
	mus_noise_step = 0U;
	mus_po1 = mus_po2 = mus_po3 = 128U;
	mus_pan1 = mus_pan2 = mus_pan3 = mus_pan4 = 0x11U;
	mus_macro1 = mus_macro2 = mus_macro3 = mus_macro4 = 0U;

	for (i = 0U; i != MAX_REPEATS; ++i) {
		mus_repeats1[i] = 0U;
		mus_repeats2[i] = 0U;
		mus_repeats3[i] = 0U;
		mus_repeats4[i] = 0U;
	}

	// Initialize all the DirectSound pseudo-channels
	for (i = 0U; i != SND_MAX_DS_CHANNELS; ++i) {
		ds_mus_data[i] = ds_mus_loop[i] = song_data + ((UWORD*)song_data)[4 + i];
		sndChannel[i].mus_enabled = 1U;
		sndChannel[i].mus_done = 0U;
		sndChannel[i].mus_wait = 0U;
		sndChannel[i].mus_octave = 4U;		// o4
		sndChannel[i].mus_length = 48U;		// quarter note
		sndChannel[i].mus_volume = 0xFF;
		sndChannel[i].mus_adsr = 0xF0F0; // Attack, Decay, Sustain, Release. In order. 0-F
		sndChannel[i].mus_curr_adsr = 0x0;
		sndChannel[i].mus_rep_depth = 255U;
		sndChannel[i].mus_slide = 0U;
		sndChannel[i].mus_vib_speed = 0U;
		sndChannel[i].mus_po = 128U;			// pitch offset = centered
		sndChannel[i].mus_pan = 0x11U;		// center pan
		sndChannel[i].mus_macro = 0U;			// no macros

		for (j = 0U; j != MAX_REPEATS; ++j) {
		  sndChannel[i].mus_repeats[j] = 0U;
		}
		dirSndNoteOff(i);
	}
}

void mus_setPaused(UBYTE p) {
	mus_paused = p;

	if (mus_paused) {
		NR12_REG = NR22_REG = NR32_REG = NR42_REG = 0U;
		NR14_REG = NR24_REG = NR34_REG = NR44_REG = 0x80U;
	}
}

void mus_togglePaused() {
	mus_setPaused(mus_paused ^ 1U);
}

void mus_disable1() {
	mus_enabled1 = 0U;
	NR13_REG = 0U;
	NR14_REG = 0x80U;
}

void mus_disable4() {
	mus_enabled4 = 0U;
	NR43_REG = 0U;
	NR44_REG = 0x80U;
}

void mus_restore1() {
	mus_enabled1 = 2U;
	NR51_REG = (NR51_REG & 0xEEU) | mus_pan1;
	NR11_REG = mus_duty1;
	NR12_REG = 0U;
	NR14_REG = 0x80U;
}

void mus_restore4() {
	mus_enabled4 = 2U;
	NR51_REG = (NR51_REG & 0x77U) | (mus_pan4 << 3);
	NR42_REG = 0U;
	NR44_REG = 0x80U;
}

void dirSndNoteOff(s8 channel) {
		sndChannel[channel].mus_note_on = 0;
		// Kill the sound (or initialize it)
		//sndChannel[channel].sampleData = 0;
		// set up channel vars
		//sndChannel[channel].samplePos = 0;
		sndChannel[channel].mus_curr_adsr = 0x0;
}

void dirSndNoteOn(s8 channel) {
		sndChannel[channel].mus_note_on = 1;
		// make sure the channel is disabled first
		sndChannel[channel].sampleData = 0;
		// Start at the start
		sndChannel[channel].samplePos = 0;
		// Play at 2* frequency. This is one octave above normal
		//int this = 3579546 / (sndChannel[channel].sampleLength * 428);//sndChannel[channel].output_freq);

		sndChannel[channel].sampleInc = (sndChannel[channel].mus_freq * sampleBitRate)/streamBitRate;//((sndChannel[channel].output_freq)<<12)/streamBitRate;
//sampleBitRate *

		// Same
		sndChannel[channel].mus_curr_adsr = 0xFF; // TODO: calculate adsr here

		// Same
		sndChannel[channel].sampleLength = 10516<<12;

		// This one will actually loop. Here, the loop start
		// position (which I checked in the MOD I ripped the
		// sample from) is 9176. So, we take length-loopStart
		// and convert to 12-bit fixed-point like everything else.
		sndChannel[channel].sampleLoopLength = (10514-9176)<<12;

		// Same
		if (channel == 1)
			sndChannel[channel].sampleData = (s8*) bigKick;
		else
			sndChannel[channel].sampleData = (s8*) smpPiano;
}

UBYTE mus_is_done() {
	return mus_paused || (mus_done1 && mus_done2 && mus_done3 && mus_done4
	 && sndChannel[0].mus_done && sndChannel[1].mus_done && sndChannel[2].mus_done
  && sndChannel[3].mus_done && sndChannel[4].mus_done && sndChannel[5].mus_done);
}

void musicSequencerUpdate() {
	if (mus_paused) return;

	dirSndMusChannelsUpdate();
	gbPulse1Update();
	//gbPulse2Update();
	gbWaveUpdate();
	gbNoiseUpdate();
}

void gbPulse1Update() {
	UBYTE note;
	UWORD tmp_freq;

	if (mus_slide1) {
		if (mus_target1 > mus_freq1) {
			mus_freq1 += mus_slide1;
			if (mus_freq1 > mus_target1) {
				mus_freq1 = mus_target1;
			}
		}
		else if (mus_target1 < mus_freq1) {
			mus_freq1 -= mus_slide1;
			if (mus_freq1 < mus_target1) {
				mus_freq1 = mus_target1;
			}
		}
		if (mus_enabled1) {
			NR13_REG = (UBYTE)mus_freq1;
			NR14_REG = mus_freq1 >> 8;
		}
	}

	if (mus_vib_delay1) {
		mus_vib_delay1--;
	}
	else if (mus_vib_speed1) {
		mus_vib_pos1 = (mus_vib_pos1 + mus_vib_speed1) & 63U;
		tmp_freq = mus_freq1 - *mus_vib_table1 + mus_vib_table1[mus_vib_pos1];

		if (mus_enabled1) {
			NR13_REG = (UBYTE)tmp_freq;
			NR14_REG = tmp_freq >> 8;
		}
	}

	if (mus_wait1) {
		mus_wait1--;
		if (mus_wait1) return;
	}

	while (1U) {
		note = *mus_data1++;
		if (note >= MUS_FIRST_NOTE) {
			if (note & MUS_HAS_LENGTH) {
				note ^= MUS_HAS_LENGTH;
				mus_wait1 = *mus_data1++;
			}
			else {
				mus_wait1 = mus_length1;
			}
			if (note == T_WAIT) {
				return;
			}
			else if (note == T_REST) {
				mus_freq1 = 0U;
				if (mus_enabled1) NR12_REG = 0U;
			}
			else {
				tmp_freq = freq[(mus_octave1 << 4) + note - MUS_FIRST_NOTE] + mus_po1 - 128U;
				if (mus_slide1) {
					mus_target1 = tmp_freq;
				}
				else {
					mus_freq1 = tmp_freq;
				}
				if (mus_enabled1) NR12_REG = mus_volume1 | mus_env1;
			}
			if (mus_enabled1) {
				if (mus_enabled1 == 2U) {
					mus_enabled1--;
					NR12_REG = mus_volume1 | mus_env1;
				}
				NR13_REG = (UBYTE)mus_freq1;
				NR14_REG = (mus_freq1 >> 8) | 0x80U;
			}
			return;
		}
		switch (note) {
		case T_LENGTH:
			mus_length1 = *mus_data1++;
			break;
		case T_OCTAVE:
			mus_octave1 = *mus_data1++;
			break;
		case T_OCT_UP:
			mus_octave1++;
			break;
		case T_OCT_DOWN:
			mus_octave1--;
			break;
		case T_VOL:
			mus_volume1 = *mus_data1++;
			if (mus_enabled1) NR12_REG = mus_volume1 | mus_env1;
			break;
		case T_ENV:
			mus_env1 = *mus_data1++;
			if (mus_enabled1) NR12_REG = mus_volume1 | mus_env1;
			break;
		case T_WAVEDUTY:
			mus_duty1 = *mus_data1++;
			if (mus_enabled1) NR11_REG = mus_duty1;
			break;
		case T_PAN:
			mus_pan1 = *mus_data1++;
			if (mus_enabled1) NR51_REG = (NR51_REG & 0xEEU) | mus_pan1;
			break;
		case T_PORTAMENTO:
			mus_slide1 = *mus_data1++;
			break;
		case T_VIBRATO:
			mus_vib_pos1 = 0U;
			note = *mus_data1++;
			mus_vib_speed1 = note & 15U;
			note = note >> 4;
			if (note == 1U) mus_vib_table1 = vib1;
			else if (note == 2U) mus_vib_table1 = vib2;
			else if (note == 3U) mus_vib_table1 = vib3;
			else mus_vib_table1 = vib4;
			mus_vib_delay1 = 0U;
			break;
		case T_VIBRATO_DELAY:
			mus_vib_pos1 = 0U;
			note = *mus_data1++;
			mus_vib_speed1 = note & 15U;
			note = note >> 4;
			if (note == 1U) mus_vib_table1 = vib1;
			else if (note == 2U) mus_vib_table1 = vib2;
			else if (note == 3U) mus_vib_table1 = vib3;
			else mus_vib_table1 = vib4;
			mus_vib_delay1 = *mus_data1++;
			break;
		case T_REP_START:
			mus_rep_depth1++;
			mus_rep1[mus_rep_depth1] = mus_data1;
			break;
		case T_REP_END:
			note = *mus_data1++;
			if (!mus_repeats1[mus_rep_depth1]) {
				mus_repeats1[mus_rep_depth1] = note;
				mus_data1 = mus_rep1[mus_rep_depth1];
			}
			mus_repeats1[mus_rep_depth1]--;
			if (mus_repeats1[mus_rep_depth1]) {
				mus_data1 = mus_rep1[mus_rep_depth1];
			}
			else {
				mus_rep_depth1--;
			}
			break;
		case T_LOOP:
			mus_loop1 = mus_data1;
			break;
		case T_PITCH_OFFSET:
			mus_po1 = *mus_data1++;
			break;
		case T_TEMPO:
			REG_TM3CNT_L = getTMAValue(*mus_data1++);
			break;
		case T_NOISE_STEP:
			break;
		case T_WAVE:
			break;
		case T_MACRO:
			note = *mus_data1++;
			mus_data1_bak = mus_data1;
			mus_data1 = mus_song + ((UWORD*)mus_song)[5 + SND_MAX_DS_CHANNELS + note];
			mus_macro1 = 1U;
			break;
		case T_EOF:
			if (mus_macro1) {
				mus_data1 = mus_data1_bak;
				mus_macro1 = 0U;
			}
			else {
				mus_data1 = mus_loop1;
				mus_done1 = 1U;
				if (*mus_data1 == T_EOF) {
					mus_wait1 = 255U;
					return;
				}
			}
			break;
		}
	}
}

void gbPulse2Update() {
	UBYTE note;
	UWORD tmp_freq;

	if (mus_slide2) {
		if (mus_target2 > mus_freq2) {
			mus_freq2 += mus_slide2;
			if (mus_freq2 > mus_target2) {
				mus_freq2 = mus_target2;
			}
		}
		else if (mus_target2 < mus_freq2) {
			mus_freq2 -= mus_slide2;
			if (mus_freq2 < mus_target2) {
				mus_freq2 = mus_target2;
			}
		}
		NR23_REG = (UBYTE)mus_freq2;
		NR24_REG = mus_freq2 >> 8;
	}

	if (mus_vib_delay2) {
		mus_vib_delay2--;
	}
	else if (mus_vib_speed2) {
		mus_vib_pos2 = (mus_vib_pos2 + mus_vib_speed2) & 63U;
		tmp_freq = mus_freq2 - *mus_vib_table2 + mus_vib_table2[mus_vib_pos2];

		NR23_REG = (UBYTE)tmp_freq;
		NR24_REG = tmp_freq >> 8;
	}

	if (mus_wait2) {
		mus_wait2--;
		if (mus_wait2) return;
	}

	while (1U) {
		note = *mus_data2++;
		if (note >= MUS_FIRST_NOTE) {
			if (note & MUS_HAS_LENGTH) {
				note ^= MUS_HAS_LENGTH;
				mus_wait2 = *mus_data2++;
			}
			else {
				mus_wait2 = mus_length2;
			}
			if (note == T_WAIT) {
				return;
			}
			else if (note == T_REST) {
				mus_freq2 = 0U;
				NR22_REG = 0U;
			}
			else {
				tmp_freq = freq[(mus_octave2 << 4) + note - MUS_FIRST_NOTE] + mus_po2 - 128U;
				if (mus_slide2) {
					mus_target2 = tmp_freq;
				}
				else {
					mus_freq2 = tmp_freq;
				}
				NR22_REG = mus_volume2 | mus_env2;
			}
			NR23_REG = (UBYTE)mus_freq2;
			NR24_REG = 0x80U | (mus_freq2 >> 8);
			return;
		}
		switch (note) {
		case T_LENGTH:
			mus_length2 = *mus_data2++;
			break;
		case T_OCTAVE:
			mus_octave2 = *mus_data2++;
			break;
		case T_OCT_UP:
			mus_octave2++;
			break;
		case T_OCT_DOWN:
			mus_octave2--;
			break;
		case T_VOL:
			mus_volume2 = *mus_data2++;
			NR22_REG = mus_volume2 | mus_env2;
			break;
		case T_ENV:
			mus_env2 = *mus_data2++;
			NR22_REG = mus_volume2 | mus_env2;
			break;
		case T_WAVEDUTY:
			mus_duty2 = *mus_data2++;
			NR21_REG = mus_duty2;
			break;
		case T_PAN:
			mus_pan2 = *mus_data2++;
			NR51_REG = (NR51_REG & 0xDDU) | (mus_pan2 << 1);
			break;
		case T_PORTAMENTO:
			mus_slide2 = *mus_data2++;
			break;
		case T_VIBRATO:
			mus_vib_pos2 = 0U;
			note = *mus_data2++;
			mus_vib_speed2 = note & 15U; // 0b1111
			note = note >> 4;
			if (note == 1U) mus_vib_table2 = vib1;
			else if (note == 2U) mus_vib_table2 = vib2;
			else if (note == 3U) mus_vib_table2 = vib3;
			else mus_vib_table2 = vib4;
			mus_vib_delay2 = 0U;
			break;
		case T_VIBRATO_DELAY:
			mus_vib_pos2 = 0U;
			note = *mus_data2++;
			mus_vib_speed2 = note & 15U;
			note = note >> 4;
			if (note == 1U) mus_vib_table2 = vib1;
			else if (note == 2U) mus_vib_table2 = vib2;
			else if (note == 3U) mus_vib_table2 = vib3;
			else mus_vib_table2 = vib4;
			mus_vib_delay2 = *mus_data2++;
			break;
		case T_REP_START:
			mus_rep_depth2++;
			mus_rep2[mus_rep_depth2] = mus_data2;
			break;
		case T_REP_END:
			note = *mus_data2++;
			if (!mus_repeats2[mus_rep_depth2]) {
				mus_repeats2[mus_rep_depth2] = note;
				mus_data2 = mus_rep2[mus_rep_depth2];
			}
			mus_repeats2[mus_rep_depth2]--;
			if (mus_repeats2[mus_rep_depth2]) {
				mus_data2 = mus_rep2[mus_rep_depth2];
			}
			else {
				mus_rep_depth2--;
			}
			break;
		case T_LOOP:
			mus_loop2 = mus_data2;
			break;
		case T_PITCH_OFFSET:
			mus_po2 = *mus_data2++;
			break;
		case T_TEMPO:
			REG_TM3CNT_L = getTMAValue(*mus_data2++);
			break;
		case T_NOISE_STEP:
			break;
		case T_WAVE:
			break;
		case T_MACRO:
			note = *mus_data2++;
			mus_data2_bak = mus_data2;
			mus_data2 = mus_song + ((UWORD*)mus_song)[5 + SND_MAX_DS_CHANNELS + note];
			mus_macro2 = 1U;
			break;
		case T_EOF:
			if (mus_macro2) {
				mus_data2 = mus_data2_bak;
				mus_macro2 = 0U;
			}
			else {
				mus_data2 = mus_loop2;
				mus_done2 = 1U;
				if (*mus_data2 == T_EOF) {
					mus_wait2 = 255U;
					return;
				}
			}
			break;
		}
	}
}

void gbWaveUpdate() {
	UBYTE note;

	if (mus_wait3) {
		mus_wait3--;
		if (mus_wait3) return;
	}

	while (1U) {
		note = *mus_data3++;
		if (note >= MUS_FIRST_NOTE) {
			if (note & MUS_HAS_LENGTH) {
				note ^= MUS_HAS_LENGTH;
				mus_wait3 = *mus_data3++;
			}
			else {
				mus_wait3 = mus_length3;
			}
			if (note == T_WAIT) {
				return;
			}
			else if (note == T_REST) {
				mus_freq3 = 0U;
				NR32_REG = 0U;
			}
			else {
				mus_freq3 = freq[(mus_octave3 << 4) + note - MUS_FIRST_NOTE] + mus_po3 - 128U;
				NR32_REG = mus_volume3;
			}
			NR30_REG = 0x0U;
			NR30_REG = 0xC0U;//NR30_REG = 0x80U;
			NR33_REG = (UBYTE)mus_freq3;
			NR34_REG = 0x80U | (mus_freq3 >> 8);
			return;
		}
		switch (note) {
		case T_LENGTH:
			mus_length3 = *mus_data3++;
			break;
		case T_OCTAVE:
			mus_octave3 = *mus_data3++;
			break;
		case T_OCT_UP:
			mus_octave3++;
			break;
		case T_OCT_DOWN:
			mus_octave3--;
			break;
		case T_VOL:
			mus_volume3 = *mus_data3++;
			break;
		case T_ENV:
			break;
		case T_WAVEDUTY:
			break;
		case T_PAN:
			mus_pan3 = *mus_data3++;
			NR51_REG = (NR51_REG & 0xBBU) | (mus_pan3 << 2);
			break;
		case T_PORTAMENTO:
			break;
		case T_VIBRATO:
			break;
		case T_VIBRATO_DELAY:
			break;
		case T_REP_START:
			mus_rep_depth3++;
			mus_rep3[mus_rep_depth3] = mus_data3;
			break;
		case T_REP_END:
			note = *mus_data3++;
			if (!mus_repeats3[mus_rep_depth3]) {
				mus_repeats3[mus_rep_depth3] = note;
				mus_data3 = mus_rep3[mus_rep_depth3];
			}
			mus_repeats3[mus_rep_depth3]--;
			if (mus_repeats3[mus_rep_depth3]) {
				mus_data3 = mus_rep3[mus_rep_depth3];
			}
			else {
				mus_rep_depth3--;
			}
			break;
		case T_LOOP:
			mus_loop3 = mus_data3;
			break;
		case T_PITCH_OFFSET:
			mus_po3 = *mus_data3++;
			break;
		case T_TEMPO:
			REG_TM3CNT_L = getTMAValue(*mus_data3++);
			break;
		case T_NOISE_STEP:
			break;
		case T_WAVE:
			note = *mus_data3++;
			NR30_REG = 0x0U;
			memcpy(WAVE_RAM, mus_wave + (note << 4), 16U);
			NR30_REG = 0xC0U; //NR30_REG = 0x80U;
			break;
		case T_MACRO:
			note = *mus_data3++;
			mus_data3_bak = mus_data3;
			mus_data3 = mus_song + ((UWORD*)mus_song)[5 + SND_MAX_DS_CHANNELS + note];
			mus_macro3 = 1U;
			break;
		case T_EOF:
			if (mus_macro3) {
				mus_data3 = mus_data3_bak;
				mus_macro3 = 0U;
			}
			else {
				mus_data3 = mus_loop3;
				mus_done3 = 1U;
				if (*mus_data3 == T_EOF) {
					mus_wait3 = 255U;
					return;
				}
			}
			break;
		}
	}
}

void gbNoiseUpdate() {
	UBYTE note;

	if (mus_slide4) {
		if (mus_target4 > mus_freq4) {
			mus_freq4 += mus_slide4;
			if (mus_freq4 > mus_target4) {
				mus_freq4 = mus_target4;
			}
		}
		else if (mus_target4 < mus_freq4) {
			mus_freq4 -= mus_slide4;
			if (mus_freq4 < mus_target4) {
				mus_freq4 = mus_target4;
			}
		}
		if (mus_enabled4) NR43_REG = mus_freq4 | mus_noise_step;
	}

	if (mus_wait4) {
		mus_wait4--;
		if (mus_wait4) return;
	}

	while (1U) {
		note = *mus_data4++;
		if (note >= MUS_FIRST_NOTE) {
			if (note & MUS_HAS_LENGTH) {
				note ^= MUS_HAS_LENGTH;
				mus_wait4 = *mus_data4++;
			}
			else {
				mus_wait4 = mus_length4;
			}
			if (note == T_WAIT) {
				return;
			}
			else if (note == T_REST) {
				mus_freq4 = 0U;
				if (mus_enabled4) NR42_REG = 0U;
			}
			else {
				if (mus_slide4) {
					mus_target4 = noise_freq[((mus_octave4 - MUS_NOISE_FIRST_OCTAVE) << 4) + note - MUS_FIRST_NOTE];
				}
				else {
					mus_freq4 = noise_freq[((mus_octave4 - MUS_NOISE_FIRST_OCTAVE) << 4) + note - MUS_FIRST_NOTE];
				}
				if (mus_enabled4) NR42_REG = mus_volume4 | mus_env4;
			}
			if (mus_enabled4) {
				if (mus_enabled4 == 2U) {
					mus_enabled4--;
					NR42_REG = mus_volume4 | mus_env4;
				}
				NR43_REG = mus_freq4 | mus_noise_step;
				NR44_REG = 0x80U;
			}
			return;
		}
		switch (note) {
		case T_LENGTH:
			mus_length4 = *mus_data4++;
			break;
		case T_OCTAVE:
			mus_octave4 = *mus_data4++;
			break;
		case T_OCT_UP:
			mus_octave4++;
			break;
		case T_OCT_DOWN:
			mus_octave4--;
			break;
		case T_VOL:
			mus_volume4 = *mus_data4++;
			if (mus_enabled4) NR42_REG = mus_volume4 | mus_env4;
			break;
		case T_ENV:
			mus_env4 = *mus_data4++;
			if (mus_enabled4) NR42_REG = mus_volume4 | mus_env4;
			break;
		case T_WAVEDUTY:
			break;
		case T_PAN:
			mus_pan4 = *mus_data4++;
			if (mus_enabled4) NR51_REG = (NR51_REG & 0x77U) | (mus_pan4 << 3);
			break;
		case T_PORTAMENTO:
			mus_slide4 = *mus_data4++;
			break;
		case T_VIBRATO:
			break;
		case T_VIBRATO_DELAY:
			break;
		case T_REP_START:
			mus_rep_depth4++;
			mus_rep4[mus_rep_depth4] = mus_data4;
			break;
		case T_REP_END:
			note = *mus_data4++;
			if (!mus_repeats4[mus_rep_depth4]) {
				mus_repeats4[mus_rep_depth4] = note;
				mus_data4 = mus_rep4[mus_rep_depth4];
			}
			mus_repeats4[mus_rep_depth4]--;
			if (mus_repeats4[mus_rep_depth4]) {
				mus_data4 = mus_rep4[mus_rep_depth4];
			}
			else {
				mus_rep_depth4--;
			}
			break;
		case T_LOOP:
			mus_loop4 = mus_data4;
			break;
		case T_PITCH_OFFSET:
			break;
		case T_TEMPO:
			REG_TM3CNT_L = getTMAValue(*mus_data4++);
			break;
		case T_NOISE_STEP:
			mus_noise_step = (*mus_data4++) << 3;
			break;
		case T_WAVE:
			break;
		case T_MACRO:
			note = *mus_data4++;
			mus_data4_bak = mus_data4;
			mus_data4 = mus_song + ((UWORD*)mus_song)[5 + SND_MAX_DS_CHANNELS + note];
			mus_macro4 = 1U;
			break;
		case T_EOF:
			if (mus_macro4) {
				mus_data4 = mus_data4_bak;
				mus_macro4 = 0U;
			}
			else {
				mus_data4 = mus_loop4;
				mus_done4 = 1U;
				if (*mus_data4 == T_EOF) {
					mus_wait4 = 255U;
					return;
				}
			}
			break;
		}
	}
}


void dirSndMusChannelsUpdate() {
	// Update all the DirectSound pseudo-channels' states based on the sequence data
	UBYTE i;
	for (i = 0U; i != SND_MAX_DS_CHANNELS; ++i) {
		UBYTE note;
		UWORD tmp_freq;

		/*
		if (sndChannel[i].mus_slide) {
			if (sndChannel[i].mus_target > sndChannel[i].mus_freq) {
				sndChannel[i].mus_freq += sndChannel[i].mus_slide;
				if (sndChannel[i].mus_freq > sndChannel[i].mus_target) {
					sndChannel[i].mus_freq = sndChannel[i].mus_target;
				}
			}
			else if (sndChannel[i].mus_target < sndChannel[i].mus_freq) {
				sndChannel[i].mus_freq -= sndChannel[i].mus_slide;
				if (sndChannel[i].mus_freq < sndChannel[i].mus_target) {
					sndChannel[i].mus_freq = sndChannel[i].mus_target;
				}
			}
			if (sndChannel[i].mus_enabled) {
				sndChannel[i].output_freq = ((UBYTE)sndChannel[i].mus_freq) << 8 | (sndChannel[i].mus_freq >> 8);
				//NR13_REG = (UBYTE)sndChannel[i].mus_freq;
				//NR14_REG = sndChannel[i].mus_freq >> 8;
			}
		}

		if (sndChannel[i].mus_vib_delay) {
			sndChannel[i].mus_vib_delay--;
		}
		else if (sndChannel[i].mus_vib_speed) {
			sndChannel[i].mus_vib_pos = (sndChannel[i].mus_vib_pos + sndChannel[i].mus_vib_speed) & 63U;
			tmp_freq = sndChannel[i].mus_freq - *sndChannel[i].mus_vib_table + sndChannel[i].mus_vib_table[sndChannel[i].mus_vib_pos];

			if (sndChannel[i].mus_enabled) {
				sndChannel[i].output_freq = ((UBYTE)tmp_freq) << 8 | (tmp_freq >> 8);
				//NR13_REG = (UBYTE)tmp_freq;
				//NR14_REG = tmp_freq >> 8;
			}
		}
*/


		if (sndChannel[i].mus_wait) {
			sndChannel[i].mus_wait--;
			if (sndChannel[i].mus_wait) continue;
		}

		while (1U) {
			note = *ds_mus_data[i]++;
			if (note >= MUS_FIRST_NOTE) {
				if (note & MUS_HAS_LENGTH) {
					note ^= MUS_HAS_LENGTH;
					sndChannel[i].mus_wait = *ds_mus_data[i]++;	// the sequence data has specified an actual note length. Wait for that long
				}
				else {
					sndChannel[i].mus_wait = sndChannel[i].mus_length; // wait for lXX steps, where XX is defined in the mml (or l4 by default)
				}
				if (note == T_WAIT) {
					return;
				}
				else if (note == T_REST) { // kill the sound
					sndChannel[i].mus_freq = 0U;
					if (sndChannel[i].mus_enabled) dirSndNoteOff(i);//NR12_REG = 0U; // kill the adsr too
				}
				else { ///
					tmp_freq = freq[(sndChannel[i].mus_octave << 4) + note - MUS_FIRST_NOTE] + sndChannel[i].mus_po - 128U;

					if (sndChannel[i].mus_slide) {
						sndChannel[i].mus_target = tmp_freq;
					}
					else {
						sndChannel[i].mus_freq = tmp_freq;
					}

					// Play the tone at the designated frequency
					if (sndChannel[i].mus_enabled)
						dirSndNoteOn(i); //NR12_REG = sndChannel[i].mus_volume | sndChannel[i].mus_env; perhaps ADSR could be defined here too?
				}
				if (sndChannel[i].mus_enabled) {
					/*
					// Decay the sound over time
					if (sndChannel[i].mus_enabled == 2U) {
						sndChannel[i].mus_enabled--;
						NR12_REG = sndChannel[i].mus_volume | sndChannel[i].mus_env;
					}
					*/

					// Set the sound frequency
					sndChannel[i].output_freq = ((UBYTE)sndChannel[i].mus_freq) << 8 | ((sndChannel[i].mus_freq >> 8) | 0x80U);
					//NR13_REG = (UBYTE)sndChannel[i].mus_freq;
					//NR14_REG = (sndChannel[i].mus_freq >> 8) | 0x80U;
				}
				return;
			}
			switch (note) {
			case T_LENGTH:
				sndChannel[i].mus_length = *ds_mus_data[i]++;
				break;
			case T_OCTAVE:
				sndChannel[i].mus_octave = *ds_mus_data[i]++;
				break;
			case T_OCT_UP:
				sndChannel[i].mus_octave++;
				break;
			case T_OCT_DOWN:
				sndChannel[i].mus_octave--;
				break;
			case T_VOL:
				//(u16)chnPtr->mus_volume * (u16)chnPtr->mus_curr_adsr
				sndChannel[i].mus_volume = *ds_mus_data[i]++;
				//if (sndChannel[i].mus_enabled) NR12_REG = sndChannel[i].mus_volume | sndChannel[i].mus_env; // Step the ADSR envelope here too
				break;
			case T_ENV:
				// Update the ADSR
				//sndChannel[i].mus_env = *ds_mus_data[i]++;
				//if (sndChannel[i].mus_enabled) NR12_REG = sndChannel[i].mus_volume | sndChannel[i].mus_env; // Step the ADSR envelope here too
				break;
			case T_WAVEDUTY:
				// Could probably force a sample change to an actual pulse duty wave sound?
				//mus_duty1 = *ds_mus_data[i]++;
				//if (sndChannel[i].mus_enabled) NR11_REG = mus_duty1;
				break;
			case T_PAN:
				// No pan for now; engine's DS is currently global hardpan only.
				//sndChannel[i].mus_pan = *ds_mus_data[i]++;
				//if (sndChannel[i].mus_enabled) NR51_REG = (NR51_REG & 0xEEU) | sndChannel[i].mus_pan;
				break;
			case T_PORTAMENTO:
				sndChannel[i].mus_slide = *ds_mus_data[i]++;
				break;
			case T_VIBRATO:
				sndChannel[i].mus_vib_pos = 0U;
				note = *ds_mus_data[i]++;
				sndChannel[i].mus_vib_speed = note & 15U;
				note = note >> 4;
				if (note == 1U) sndChannel[i].mus_vib_table = vib1;
				else if (note == 2U) sndChannel[i].mus_vib_table = vib2;
				else if (note == 3U) sndChannel[i].mus_vib_table = vib3;
				else sndChannel[i].mus_vib_table = vib4;
				sndChannel[i].mus_vib_delay = 0U;
				break;
			case T_VIBRATO_DELAY:
				sndChannel[i].mus_vib_pos = 0U;
				note = *ds_mus_data[i]++;
				sndChannel[i].mus_vib_speed = note & 15U;
				note = note >> 4;
				if (note == 1U) sndChannel[i].mus_vib_table = vib1;
				else if (note == 2U) sndChannel[i].mus_vib_table = vib2;
				else if (note == 3U) sndChannel[i].mus_vib_table = vib3;
				else sndChannel[i].mus_vib_table = vib4;
				sndChannel[i].mus_vib_delay = *ds_mus_data[i]++;
				break;

			case T_REP_START:
				sndChannel[i].mus_rep_depth++;
				sndChannel[i].ds_mus_rep[sndChannel[i].mus_rep_depth] = ds_mus_data[i];
				break;
			case T_REP_END:
				note = *ds_mus_data[i]++;
				if (!sndChannel[i].mus_repeats[sndChannel[i].mus_rep_depth]) {
					sndChannel[i].mus_repeats[sndChannel[i].mus_rep_depth] = note;
					ds_mus_data[i] = sndChannel[i].ds_mus_rep[sndChannel[i].mus_rep_depth];
				}
				sndChannel[i].mus_repeats[sndChannel[i].mus_rep_depth]--;
				if (sndChannel[i].mus_repeats[sndChannel[i].mus_rep_depth]) {
					ds_mus_data[i] = sndChannel[i].ds_mus_rep[sndChannel[i].mus_rep_depth];
				}
				else {
					sndChannel[i].mus_rep_depth--;
				}
				break;


			case T_LOOP:
				ds_mus_loop[i] = ds_mus_data[i];
				break;
			case T_PITCH_OFFSET:
				sndChannel[i].mus_po = *ds_mus_data[i]++;
				break;
			case T_TEMPO:
				REG_TM3CNT_L = getTMAValue(*ds_mus_data[i]++);
				break;
			case T_NOISE_STEP:
				break;
			case T_WAVE:
				break;
			case T_MACRO:
				note = *ds_mus_data[i]++;
				ds_mus_data_bak[i] = ds_mus_data[i];
				ds_mus_data[i] = mus_song + ((UWORD*)mus_song)[5 + SND_MAX_DS_CHANNELS + note];
				sndChannel[i].mus_macro = 1U;
				break;
			case T_EOF:
				if (sndChannel[i].mus_macro) {
					ds_mus_data[i] = ds_mus_data_bak[i];
					sndChannel[i].mus_macro = 0U;
				}
				else {
					ds_mus_data[i] = ds_mus_loop[i];
					sndChannel[i].mus_done = 1U;
					if (*ds_mus_data[i] == T_EOF) {
						sndChannel[i].mus_wait = 255U;
						return;
					}
				}
				break;
			}
		}

	}
}
