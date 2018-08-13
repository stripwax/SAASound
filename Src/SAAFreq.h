// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAFreq.h: interface for the CSAAFreq class.
// Note about Samplerates: 0=44100, 1=22050; 2=11025
//
// Version 3.01.0 (10 Jan 2001)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef SAAFREQ_H_INCLUDE
#define SAAFREQ_H_INCLUDE

class CSAAFreq  
{
private:
	static const unsigned long m_FreqTable[2048];
	static inline unsigned short GetLevel(unsigned short nLevel);

	unsigned long m_nCounter;
	unsigned long m_nAdd;
	unsigned short m_nLevel;

	int m_nCurrentOffset;
	int m_nCurrentOctave;
	int m_nNextOffset;
	int m_nNextOctave;
	bool m_bIgnoreOffsetData;
	bool m_bNewData;
	bool m_bSync;

	int m_nSampleRateMode;
	unsigned long m_nSampleRateTimes4K;
	CSAANoise * const m_pcConnectedNoiseGenerator;
	CSAAEnv * const m_pcConnectedEnvGenerator;
	const int m_nConnectedMode; // 0 = nothing; 1 = envgenerator; 2 = noisegenerator

	void UpdateOctaveOffsetData(void);
	void SetAdd(void);

public:
	CSAAFreq(CSAANoise * const pcNoiseGenerator, CSAAEnv * const pcEnvGenerator);
	~CSAAFreq();
	void SetFreqOffset(BYTE nOffset);
	void SetFreqOctave(BYTE nOctave);
	void SetSampleRateMode(int nSampleRateMode);
	void Sync(bool bSync);
	unsigned short Tick(void);
	unsigned short Level(void) const;

};

#endif	// SAAFREQ_H_INCLUDE
