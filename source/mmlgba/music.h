#ifndef MUSIC_H
#define MUSIC_H

/*
#define WAVE_OFFSET 8U

#define CHN1_OFFSET	0U
#define CHN2_OFFSET	1U
#define CHN3_OFFSET	2U
#define CHN4_OFFSET	3U
*/

extern UBYTE* wtf;

void mus_init(UBYTE *song);
void mus_setPaused(UBYTE p);
void mus_togglePaused();
void mus_disable1();
void mus_disable4();
void mus_restore1();
void mus_restore4();
void dirSndNoteOff(s8 channel);
void dirSndNoteOn(s8 channel);
//UBYTE* getDSMusDataAddress(s8 channel);
UBYTE mus_is_done();
void musicSequencerUpdate();
void gbPulse1Update();
void gbPulse2Update();
void gbWaveUpdate();
void gbNoiseUpdate();
void dirSndMusChannelsUpdate();

#define MUS_HAS_LENGTH 0x80U
#define MUS_FIRST_NOTE T_C

#endif
