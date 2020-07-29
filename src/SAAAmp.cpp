// Part of SAASound copyright 1998-2018 Dave Hooper <dave@beermex.com>
//
// SAAAmp.cpp: implementation of the CSAAAmp class.
// This class handles Tone/Noise mixing, Envelope application and
// amplification.
//
//////////////////////////////////////////////////////////////////////

#include "SAASound.h"
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
m_pcConnectedToneGenerator(ToneGenerator),
m_pcConnectedNoiseGenerator(NoiseGenerator),
m_pcConnectedEnvGenerator(EnvGenerator),
m_bUseEnvelope(EnvGenerator != NULL)
{
	leftleveltimes32=leftleveltimes16=leftlevela0x0e=leftlevela0x0etimes2=0;
	rightleveltimes32=rightleveltimes16=rightlevela0x0e=rightlevela0x0etimes2=0;
	monoleveltimes32=monoleveltimes16=0;
	m_nMixMode = 0;
	m_bMute=true;
	m_bSync = false;
	m_nOutputIntermediate=0;
	last_level_byte=0;
	SetAmpLevel(0x00);

}

CSAAAmp::~CSAAAmp()
{
	// Nothing to do
}

void CSAAAmp::SetAmpLevel(BYTE level_byte)
{
	// if level unchanged since last call then do nothing
	if (level_byte != last_level_byte)
	{
		last_level_byte = level_byte;
		leftlevela0x0e = level_byte&0x0e;
		leftlevela0x0etimes2 = leftlevela0x0e<<1;
		leftleveltimes16 = (level_byte&0x0f) << 4;
		leftleveltimes32 = leftleveltimes16 << 1;
	
		rightlevela0x0e = (level_byte>>4) & 0x0e;
		rightlevela0x0etimes2 = rightlevela0x0e<<1;
		rightleveltimes16 = level_byte & 0xf0;
		rightleveltimes32 = rightleveltimes16 << 1;
	
		monoleveltimes16 = leftleveltimes16+rightleveltimes16;
		monoleveltimes32 = leftleveltimes32+rightleveltimes32;
	}

}

// output is between 0 and 480 per channel
// or between 0 and 960 for combined MONO channel

unsigned short CSAAAmp::LeftOutput(void) const
{
	if (m_bMute || m_bSync)
	{
		return 0;
	}

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
//		Implement 
//		return m_pcConnectedEnvGenerator->LeftLevel()*leftlevela0x0e*(2-m_nOutputIntermediate);
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
			case 0:
				return EffectiveAmplitude(m_pcConnectedEnvGenerator->LeftLevel(), leftlevela0x0e) * 2;
			case 1:
				return EffectiveAmplitude(m_pcConnectedEnvGenerator->LeftLevel(), leftlevela0x0e);
			case 2:
			default:
				return 0;
		}
	}
	else
	{
//		Implement 
//		return leftleveltimes16 * m_nOutputIntermediate;
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
			case 0:
			default:
				return 0;
			case 1:
				return leftleveltimes16;
			case 2:
				return leftleveltimes32;
		}
	}
}

unsigned short CSAAAmp::RightOutput(void) const
{
	if (m_bMute || m_bSync)
	{
		return 0;
	}

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
//		Implement 
//		return m_pcConnectedEnvGenerator->RightLevel()*rightlevela0x0e*(2-m_nOutputIntermediate);
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
			case 0:
				return EffectiveAmplitude(m_pcConnectedEnvGenerator->RightLevel(), rightlevela0x0e) * 2;
			case 1:
				return EffectiveAmplitude(m_pcConnectedEnvGenerator->RightLevel(), rightlevela0x0e);
			case 2:
			default:
				return 0;
		}
	}
	else
	{
//		Implement 
//		return rightleveltimes16 * m_nOutputIntermediate;
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
			case 0:
			default:
				return 0;
			case 1:
				return rightleveltimes16;
			case 2:
				return rightleveltimes32;
		}
	}
}

unsigned short CSAAAmp::MonoOutput(void) const
{
	if (m_bMute || m_bSync)
	{
		return 0;
	}

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
//		Implement something roughly equivalent to
//		return ( (m_pcConnectedEnvGenerator->RightLevel()*rightlevela0x0e) + (m_pcConnectedEnvGenerator->LeftLevel()*leftlevela0x0e) ) * (2.0f-m_nOutputIntermediate);
//		as a simple switch statement.  The nuance is in how the amplitude and envelope controls are combined (see EffectiveAmplitude)
		switch (m_nOutputIntermediate)
		{
			case 0:
				return (EffectiveAmplitude(m_pcConnectedEnvGenerator->RightLevel(), rightlevela0x0e) + EffectiveAmplitude(m_pcConnectedEnvGenerator->LeftLevel(), leftlevela0x0e)) * 2;
			case 1:
				return EffectiveAmplitude(m_pcConnectedEnvGenerator->RightLevel(), rightlevela0x0e) + EffectiveAmplitude(m_pcConnectedEnvGenerator->LeftLevel(), leftlevela0x0e);
			case 2:
			default:
				return 0;
		}
	}
	else
	{
//		Implement 
//		return monoleveltimes16 * m_nOutputIntermediate;
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
			case 0:
			default:
				return 0;
			case 1:
				return monoleveltimes16;
			case 2:
				return monoleveltimes32;
		}
	}
}

void CSAAAmp::SetToneMixer(BYTE bEnabled)
{
	if (bEnabled == 0)
	{
		// clear mixer bit
		m_nMixMode &= ~(0x01);
	}
	else
	{
		// set mixer bit
		m_nMixMode |= 0x01;
	}
}

void CSAAAmp::SetNoiseMixer(BYTE bEnabled)
{
	if (bEnabled == 0)
	{
		m_nMixMode &= ~(0x02);
	}
	else
	{
		m_nMixMode |= 0x02;
	}
}

void CSAAAmp::Mute(bool bMute)
{
	// m_bMute refers to the GLOBAL mute setting (sound 28,0)
	// NOT the per-channel mixer settings !!
	m_bMute = bMute;
}

void CSAAAmp::Sync(bool bSync)
{
	// m_bSync refers to the GLOBAL sync setting (sound 28,2)
	m_bSync = bSync;
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
			// NOTE: ConnectedToneGenerator returns either 0 or 2
			break;
		case 2:
			// noise only for this channel
			m_pcConnectedToneGenerator->Tick();
			m_nOutputIntermediate= m_pcConnectedNoiseGenerator->LevelTimesTwo();
			// NOTE: ConnectedNoiseFunction returns either 0 or 1 using ->Level()
			// and either 0 or 2 when using ->LevelTimesTwo();
			break;
		case 3:
			// tone+noise for this channel ... mixing algorithm :
			m_nOutputIntermediate = m_pcConnectedToneGenerator->Tick();
			if ( m_nOutputIntermediate==2 && (m_pcConnectedNoiseGenerator->Level())==1 )
			{
				m_nOutputIntermediate=1;
			}
			break;
	}
	// intermediate is between 0 and 2
}

unsigned short CSAAAmp::EffectiveAmplitude(unsigned short amp, unsigned short env) const
{
	// Return the effective amplitude of the low-pass-filtered result of the logical
	// AND of the amplitude PDM and envelope PDM patterns.  This is a more accurate
	// evaluation of the SAA than simply returning amp * env , based on how the SAA
	// implements pulse-density modulation.

	static const unsigned short pdm[16][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,2,2,2,2,2,2,2,2,4,4,4,4},
		{0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8},
		{0,1,1,2,4,5,5,6,6,7,7,8,10,11,11,12},
		{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
		{0,1,2,3,6,7,8,9,10,11,12,13,16,17,18,19},
		{0,2,3,5,6,8,9,11,12,14,15,17,18,20,21,23},
		{0,2,3,5,8,10,11,13,14,16,17,19,22,24,25,27},
		{0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30},
		{0,2,4,6,10,12,14,16,18,20,22,24,28,30,32,34},
		{0,3,5,8,10,13,15,18,20,23,25,28,30,33,35,38},
		{0,3,5,8,12,15,17,20,22,25,27,30,34,37,39,42},
		{0,3,6,9,12,15,18,21,24,27,30,33,36,39,42,45},
		{0,3,6,9,14,17,20,23,26,29,32,35,40,43,46,49},
		{0,4,7,11,14,18,21,25,28,32,35,39,42,46,49,53},
		{0,4,7,11,16,20,23,27,30,34,37,41,46,50,53,57}
	};
	return(pdm[amp][env] * 4);
}

unsigned short CSAAAmp::TickAndOutputMono(void)
{
	if (m_bSync)
		return 0;

	// first, do the Tick:
	Tick();
	return MonoOutput();
}

stereolevel CSAAAmp::TickAndOutputStereo(void)
{
	static stereolevel retval;
	static const stereolevel zeroval = { {0,0} };
	
	if (m_bSync)
		return zeroval;

	// first, do the Tick:
	Tick();
	
	// now calculate the returned amplitude for this sample:
	////////////////////////////////////////////////////////

	if (m_bMute)
	{
		retval = zeroval;
	}
	else if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
//		Implement 
//		return ( (m_pcConnectedEnvGenerator->RightLevel()*rightlevela0x0e) + (m_pcConnectedEnvGenerator->LeftLevel()*leftlevela0x0e) ) * (2.0f-m_nOutputIntermediate);
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
			case 0:
				retval.sep.Left = EffectiveAmplitude(m_pcConnectedEnvGenerator->LeftLevel(), leftlevela0x0e) * 2;
				retval.sep.Right = EffectiveAmplitude(m_pcConnectedEnvGenerator->RightLevel(), rightlevela0x0e) * 2;
				break;
			case 1:
				retval.sep.Left = EffectiveAmplitude(m_pcConnectedEnvGenerator->LeftLevel(), leftlevela0x0e);
				retval.sep.Right = EffectiveAmplitude(m_pcConnectedEnvGenerator->RightLevel(), rightlevela0x0e);
				break;
			case 2:
			default:
				retval = zeroval;
		}
	}
	else
	{
//		Implement 
//		return monoleveltimes16 * m_nOutputIntermediate;
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
			case 0:
			default:
				retval = zeroval;
				break;
			case 1:
				retval.sep.Left=leftleveltimes16;
				retval.sep.Right=rightleveltimes16;
				break;
			case 2:
				retval.sep.Left = leftleveltimes32;
				retval.sep.Right = rightleveltimes32;
				break;
		}
	}

	return retval;
}
