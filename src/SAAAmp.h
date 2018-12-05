// Part of SAASound copyright 1998-2018 Dave Hooper <dave@beermex.com>
//
// SAAAmp.h: interface for the CSAAAmp class.
// This class handles Tone/Noise mixing, Envelope application and
// amplification.
//
//////////////////////////////////////////////////////////////////////

#ifndef SAAAMP_H_INCLUDED
#define SAAAMP_H_INCLUDED

class CSAAAmp  
{
private:
	unsigned short leftleveltimes16, leftleveltimes32, leftlevela0x0e, leftlevela0x0etimes2;
	unsigned short rightleveltimes16, rightleveltimes32, rightlevela0x0e, rightlevela0x0etimes2;
	unsigned short monoleveltimes16, monoleveltimes32;
	unsigned short m_nOutputIntermediate;
	unsigned int m_nMixMode;
	CSAAFreq * const m_pcConnectedToneGenerator; // not const because amp calls ->Tick()
	const CSAANoise * const m_pcConnectedNoiseGenerator;
	const CSAAEnv * const m_pcConnectedEnvGenerator;
	const bool m_bUseEnvelope;
	mutable bool m_bMute;
	mutable BYTE last_level_byte;
	mutable bool level_unchanged;
	mutable unsigned short last_leftlevel, last_rightlevel;
	mutable bool leftlevel_unchanged, rightlevel_unchanged;
	mutable unsigned short cached_last_leftoutput, cached_last_rightoutput;

	unsigned short EffectiveAmplitude(unsigned short amp, unsigned short env) const;

public:
	CSAAAmp(CSAAFreq * const ToneGenerator, const CSAANoise * const NoiseGenerator, const CSAAEnv * const EnvGenerator);
	~CSAAAmp();

	void SetAmpLevel(BYTE level_byte); // really just a BYTE
	void SetToneMixer(BYTE bEnabled);
	void SetNoiseMixer(BYTE bEnabled);
	unsigned short LeftOutput(void) const;
	unsigned short RightOutput(void) const;
	unsigned short MonoOutput(void) const;
	void Mute(bool bMute);
	void Tick(void);
	unsigned short TickAndOutputMono(void);
	stereolevel TickAndOutputStereo(void);

};

#endif	// SAAAMP_H_INCLUDED
