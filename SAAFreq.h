// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAFreq.h: interface for the CSAAFreq class.
// Note about Samplerates: 0=44100, 1=22050; 2=11025
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef SAAFREQ_H_INCLUDE
#define SAAFREQ_H_INCLUDE

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSAAFreq  
{
private:
	static const unsigned long m_FreqTable[2048];

	unsigned long m_nCounter;
	unsigned long m_nAdd;
	BYTE m_nLevel;

	BYTE m_nCurrentOffset;
	BYTE m_nCurrentOctave;
	BYTE m_nNextOffset;
	BYTE m_nNextOctave;
	bool m_bNewOctaveData;
	bool m_bNewOffsetData;
	bool m_bIgnoreOffsetData;
	bool m_bSync;

	BYTE m_nSampleRateMode;
	unsigned long m_nSampleRateTimes128;
	CSAANoise * const m_pcConnectedNoiseGenerator;
	CSAAEnv * const m_pcConnectedEnvGenerator;
	const BYTE m_nConnectedMode; // 0 = nothing; 1 = envgenerator; 2 = noisegenerator

	void SetAdd(BYTE nOctave, BYTE nOffset);

public:
	CSAAFreq(CSAANoise * const pcNoiseGenerator, CSAAEnv * const pcEnvGenerator);
	~CSAAFreq();
	void SetFreqOffset(BYTE nOffset);
	void SetFreqOctave(BYTE nOctave);
	void SetSampleRateMode(BYTE nSampleRateMode);
	void Sync(bool bSync);
	BYTE Tick(void);
	BYTE Level(void) const;

};

#endif	// SAAFREQ_H_INCLUDE
