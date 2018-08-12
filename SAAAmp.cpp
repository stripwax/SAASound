// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAAmp.cpp: implementation of the CSAAAmp class.
// This class handles Tone/Noise mixing, Envelope application and
// amplification.
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "types.h"
#include "SAANoise.h"
#include "SAAEnv.h"
#include "SAAFreq.h"
#include "SAAAmp.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAAAmp::CSAAAmp(CSAAFreq * const ToneGenerator, const CSAANoise * const NoiseGenerator, const CSAAEnv * const EnvGenerator)
:
m_bUseEnvelope(EnvGenerator != NULL),
m_pcConnectedToneGenerator(ToneGenerator),
m_pcConnectedNoiseGenerator(NoiseGenerator),
m_pcConnectedEnvGenerator(EnvGenerator)
{
	rightlevel=rightleveltimes16=rightleveltimes32=0;
	leftlevel=leftleveltimes16=leftleveltimes32=0;
	monolevel=monoleveltimes16=monoleveltimes32=0;
	m_nMixMode = 0;
	m_bMute=true;
	m_nOutputIntermediate=0;
}

CSAAAmp::~CSAAAmp()
{
	// Nothing to do
}

void CSAAAmp::SetAmpLevel(BYTE level)
{
	leftlevel = level & 0x0f;
	leftleveltimes16 = leftlevel<<4;
	leftleveltimes32 = leftleveltimes16<<1;

	rightleveltimes16 = level&0xf0;
	rightlevel = rightleveltimes16 >> 4;
	rightleveltimes32 = rightleveltimes16<<1;

	monolevel = leftlevel+rightlevel;
	monoleveltimes16 = monolevel<<4;
	monoleveltimes32 = monoleveltimes16<<1;

}

// output is between 0 and 480 per channel
// or between 0 and 960 for combined MONO channel

unsigned short CSAAAmp::LeftOutput(void) const
{
	if (m_bMute) return 0;

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
		switch (m_nOutputIntermediate)
		{
		case 0:
//			Using the following line instead would (correctly) reduce amplitude
//			controller resolution to 3 bits when envelopes are enabled
			return (m_pcConnectedEnvGenerator->LeftLevel()*(leftlevel&0xee))<<1;
//			return (m_pcConnectedEnvGenerator->LeftLevel()*leftlevel)<<1;
		case 1:
			return m_pcConnectedEnvGenerator->LeftLevel()*(leftlevel&0xee);
//			return m_pcConnectedEnvGenerator->LeftLevel()*leftlevel;
		case 2:
			return 0;
		default:
			return 0;
		}
	}
	else
	{
		switch (m_nOutputIntermediate)
		{
		case 0:
			return 0;
		case 1:
			return leftleveltimes16;
		case 2:
			return leftleveltimes32;
		default:
			return 0;
		}
	}
}
	 

unsigned short CSAAAmp::RightOutput(void) const
{
	if (m_bMute) return 0;

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
		switch (m_nOutputIntermediate)
		{
		case 0:
//			Using the following line instead would (correctly) reduce amplitude
//			controller resolution to 3 bits when envelopes are enabled
			return (m_pcConnectedEnvGenerator->RightLevel()*(rightlevel&0xee))<<1;
//			return (m_pcConnectedEnvGenerator->RightLevel()*rightlevel)<<1;
		case 1:
			return m_pcConnectedEnvGenerator->RightLevel()*(rightlevel&0xee);
//			return m_pcConnectedEnvGenerator->RightLevel()*rightlevel;
		case 2:
			return 0;
		default:
			return 0;
		}
	}
	else
	{
		switch (m_nOutputIntermediate)
		{
		case 0:
			return 0;
		case 1:
			return rightleveltimes16;
		case 2:
			return rightleveltimes32;
		default:
			return 0;
		}
	}
}

unsigned short CSAAAmp::MonoOutput(void) const
{
	if (m_bMute) return 0;

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
		switch (m_nOutputIntermediate)
		{
		case 0:
//			Using the following line instead would (correctly) reduce amplitude
//			controller resolution to 3 bits when envelopes are enabled
			return ((m_pcConnectedEnvGenerator->RightLevel()*(rightlevel&0xee)) + (m_pcConnectedEnvGenerator->LeftLevel()*(leftlevel&0xee)))<<1;
//			return ((m_pcConnectedEnvGenerator->RightLevel()*rightlevel) + (m_pcConnectedEnvGenerator->LeftLevel()*leftlevel))<<1;
		case 1:
			return m_pcConnectedEnvGenerator->RightLevel()*(rightlevel&0xee) + m_pcConnectedEnvGenerator->LeftLevel()*(leftlevel&0xee);
//			return m_pcConnectedEnvGenerator->RightLevel()*rightlevel + m_pcConnectedEnvGenerator->LeftLevel()*leftlevel;
		case 2:
			return 0;
		default:
			return 0;
		}

	}
	else
	{
		switch (m_nOutputIntermediate)
		{
		case 0:
			return 0;
		case 1:
			return monoleveltimes16;
		case 2:
			return monoleveltimes32;
		default:
			return 0;
		}
	}
}

void CSAAAmp::SetToneMixer(char bEnabled)
{
	if (bEnabled)
	{
		m_nMixMode |= 0x01;
	}
	else
	{
		m_nMixMode &= ~(0x01);
	}
}

void CSAAAmp::SetNoiseMixer(char bEnabled)
{
	if (bEnabled)
	{
		m_nMixMode |= 0x02;
	}
	else
	{
		m_nMixMode &= ~(0x02);
	}
}


void CSAAAmp::Mute(bool bMute)
{
	// m_bMute refers to the GLOBAL mute setting (sound 28,0)
	// NOT the per-channel mixer settings !!
	m_bMute = bMute;
}


void CSAAAmp::Tick(void)
{
	switch (m_nMixMode)
	{
	case 0:
		// no tone or noise for this channel
		m_pcConnectedToneGenerator->Tick();
		m_nOutputIntermediate=0;
		break;
	case 1:
		// tone only for this channel
		m_nOutputIntermediate=(m_pcConnectedToneGenerator->Tick());
		// NOTE: ConnectedToneFunction returns value either 0 or *2*
		// not 0 or 1
		break;
	case 2:
		// noise only for this channel
		m_pcConnectedToneGenerator->Tick();
		m_nOutputIntermediate=(m_pcConnectedNoiseGenerator->Level())<<1;
		// NOTE: ConnectedNoiseFunction returns either 0 or 1
		// and so we shift left to get into the range 0 or 2
		break;
	case 3:
		// tone+noise for this channel ... mixing algorithm :
		if (m_pcConnectedToneGenerator->Tick())
		{
			m_nOutputIntermediate = 1+m_pcConnectedNoiseGenerator->Level();
		}
		else
		{
			m_nOutputIntermediate = 0;
		}
		break;
	}
	// intermediate is between 0 and 2
}
