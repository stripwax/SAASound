// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAAmp.cpp: implementation of the CSAAAmp class.
// This class handles Tone/Noise mixing, Envelope application and
// amplification.
//
// Version 3.01.0 (10 Jan 2001)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
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
	m_nOutputIntermediate=0;
	last_level_byte=0;
	level_unchanged=false;
	last_leftlevel=0;
	last_rightlevel=0;
	leftlevel_unchanged=false;
	rightlevel_unchanged=false;
	cached_last_leftoutput=0;
	cached_last_rightoutput=0;
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
	if (m_bMute)
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
			return m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0etimes2;
		case 1:
			return m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0e;
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
	if (m_bMute)
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
			return m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0etimes2;
		case 1:
			return m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0e;
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
	if (m_bMute)
	{
		return 0;
	}

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
//		Implement 
//		return ( (m_pcConnectedEnvGenerator->RightLevel()*rightlevela0x0e) + (m_pcConnectedEnvGenerator->LeftLevel()*leftlevela0x0e) ) * (2.0f-m_nOutputIntermediate);
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
		case 0:
			return (m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0etimes2) + (m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0etimes2);
		case 1:
			return (m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0e) + (m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0e);
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

unsigned short CSAAAmp::TickAndOutputMono(void)
{
	// first, do the Tick:
	Tick();
	
	// now calculate the returned amplitude for this sample:
	////////////////////////////////////////////////////////

	if (m_bMute)
	{
		return 0;
	}

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
//		Implement 
//		return ( (m_pcConnectedEnvGenerator->RightLevel()*rightlevela0x0e) + (m_pcConnectedEnvGenerator->LeftLevel()*leftlevela0x0e) ) * (2.0f-m_nOutputIntermediate);
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
		case 0:
			return (m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0etimes2) + (m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0etimes2);
		case 1:
			return (m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0e) + (m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0e);
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



stereolevel CSAAAmp::TickAndOutputStereo(void)
{
	static stereolevel retval;
	static const stereolevel zeroval = { {0,0} };
	
	// first, do the Tick:
	Tick();
	
	// now calculate the returned amplitude for this sample:
	////////////////////////////////////////////////////////

	if (m_bMute)
	{
		return zeroval;
	}

	if (m_bUseEnvelope && m_pcConnectedEnvGenerator->IsActive())
	{
//		Implement 
//		return ( (m_pcConnectedEnvGenerator->RightLevel()*rightlevela0x0e) + (m_pcConnectedEnvGenerator->LeftLevel()*leftlevela0x0e) ) * (2.0f-m_nOutputIntermediate);
//		as a simple switch statement:
		switch (m_nOutputIntermediate)
		{
		case 0:
			retval.sep.Left=m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0etimes2;
			retval.sep.Right=m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0etimes2;
			return retval;
		case 1:
			retval.sep.Left=m_pcConnectedEnvGenerator->LeftLevel() * leftlevela0x0e;
			retval.sep.Right=m_pcConnectedEnvGenerator->RightLevel() * rightlevela0x0e;
			return retval;
		case 2:
		default:
			return zeroval;
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
			return zeroval;
		case 1:
			retval.sep.Left=leftleveltimes16;
			retval.sep.Right=rightleveltimes16;
			return retval;
		case 2:
			retval.sep.Left=leftleveltimes32;
			retval.sep.Right=rightleveltimes32;
			return retval;
		}
	}

}

