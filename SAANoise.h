// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAANoise.h: interface for the CSAANoise class.
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef SAANOISE_H_INCLUDED
#define SAANOISE_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000



class CSAANoise  
{
private:
	bool m_bUseFreqGen; // true to indicate using freqgen; false to indicate using counter/add/samplerate
	unsigned long m_nCounter;
	unsigned long m_nAdd;
	bool m_bSync; // see description of "SYNC" bit of register 28
	BYTE m_nSampleRateMode; // 0=44100, 1=22050; 2=11025
	unsigned long m_nSampleRateTimes256; // = (44100*256) when RateMode=0, for example
#ifndef NEW_RAND
	/* old code: */
	BYTE m_nLevel;
#endif
	BYTE m_nSourceMode;
	static const unsigned long cs_nAddBase; // nAdd for 31.25 kHz noise at 44.1 kHz samplerate

	// pseudo-random number generator
	unsigned long m_nRand;


	void ChangeLevel(void);
	

public:
	CSAANoise();
	CSAANoise(unsigned long seed);
	~CSAANoise();

	void SetSource(BYTE nSource);
	void Trigger(void);
	void SetSampleRateMode(BYTE nSampleRateMode);
	void Seed(unsigned long seed);

	BYTE Tick(void);
	BYTE Level(void) const;
	void Sync(bool bSync);

};



#endif	// SAANOISE_H_INCLUDED
