// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAAmp.h: interface for the CSAAAmp class.
// This class handles Tone/Noise mixing, Envelope application and
// amplification.
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef SAAAMP_H_INCLUDED
#define SAAAMP_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSAAAmp  
{
private:
	unsigned int leftlevel,leftleveltimes16,leftleveltimes32;
	unsigned int rightlevel,rightleveltimes16,rightleveltimes32;
	unsigned int monolevel,monoleveltimes16,monoleveltimes32;
	unsigned short m_nOutputIntermediate;
	int m_nMixMode;
	CSAAFreq * const m_pcConnectedToneGenerator; // not const because amp calls ->Tick()
	const CSAANoise * const m_pcConnectedNoiseGenerator;
	const CSAAEnv * const m_pcConnectedEnvGenerator;
	const bool m_bUseEnvelope;
	bool m_bMute;

public:
	CSAAAmp(CSAAFreq * const ToneGenerator, const CSAANoise * const NoiseGenerator, const CSAAEnv * const EnvGenerator);
	~CSAAAmp();

	void SetAmpLevel(BYTE level);
	void SetToneMixer(char bEnabled);
	void SetNoiseMixer(char bEnabled);
	unsigned short LeftOutput(void) const;
	unsigned short RightOutput(void) const;
	unsigned short MonoOutput(void) const;
	void Mute(bool bMute);
	void Tick(void);

};

#endif	// SAAAMP_H_INCLUDED
