// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAANoise.cpp: implementation of the CSAANoise class.
// One noise generator
//
// After construction, it's important to SetSampleRate before
// trying to use the generator.
// (Just because the CSAANoise object has a default samplerate
//  doesn't mean you should rely on it)
//
// Version 3.01.0 (10 Jan 2001)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "types.h"
#include "SAANoise.h"


//////////////////////////////////////////////////////////////////////
// static member initialisation
//////////////////////////////////////////////////////////////////////

const unsigned long CSAANoise::cs_nAddBase = 31250 << 12;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAANoise::CSAANoise()
:
#ifndef NEW_RAND
/* old code: */
m_nLevel(0),
m_nLevelTimesTwo(0),
#endif
m_bSync(false),
m_nCounter(0),
m_nAdd(cs_nAddBase),
m_nSampleRateMode(2),
m_nSampleRateTimes4K(11025<<12),
m_nSourceMode(0),
m_nRand(0x11111111) /* well hey, it's as good a seed as any */
{
	// That's all
}

CSAANoise::CSAANoise(unsigned long seed)
:
#ifndef NEW_RAND
/* old code: */
m_nLevel(0),
#endif
m_bSync(false),
m_nCounter(0),
m_nAdd(cs_nAddBase),
m_nSampleRateMode(2),
m_nSampleRateTimes4K(11025<<12),
m_nSourceMode(0),
m_nRand(seed)
{
	// That's all
}

CSAANoise::~CSAANoise()
{
	// Nothing to do
}

void CSAANoise::Seed(unsigned long seed)
{
	m_nRand = seed;
}

unsigned short CSAANoise::Level(void) const
{
#ifndef NEW_RAND
	/* old code: */
	return m_nLevel;
#else
	/* new code: */
	return m_nRand & 0x00000001;
#endif
}

unsigned short CSAANoise::LevelTimesTwo(void) const
{
#ifndef NEW_RAND
	/* old code: */
	return m_nLevelTimesTwo;
#else
	/* new code: */
	return (m_nRand & 0x00000001) << 1;
#endif
}

void CSAANoise::SetSource(int nSource)
{
	m_nSourceMode = nSource;
	m_nAdd = cs_nAddBase >> m_nSourceMode;
}


void CSAANoise::Trigger(void)
{
	// Trigger only does anything useful when we're
	// clocking from the frequency generator - i.e
	// if bUseFreqGen = true (i.e. SourceMode = 3)
	
	// So if we're clocking from the noise generator
	// clock (ie, SourceMode = 0, 1 or 2) then do nothing

//	No point actually checking m_bSync here ... because if sync is true,
//	then frequency generators won't actually be generating Trigger pulses
//	so we wouldn't even get here!
//	if ( (!m_bSync) && m_bUseFreqGen)
	if (m_nSourceMode == 3)
	{
		ChangeLevel();
	}
}

unsigned short CSAANoise::Tick(void)
{
	// Tick only does anything useful when we're
	// clocking from the noise generator clock
	// (ie, SourceMode = 0, 1 or 2)
	
	// So, if SourceMode = 3 (ie, we're clocking from a
	// frequency generator ==> bUseFreqGen = true)
	// then do nothing
	if ( (!m_bSync) && (m_nSourceMode!=3) )
	{
		m_nCounter+=m_nAdd;
		if (m_nCounter >= m_nSampleRateTimes4K)
		{
			while (m_nCounter >= m_nSampleRateTimes4K)
			{
				m_nCounter-=m_nSampleRateTimes4K;
				ChangeLevel();
			}
		}
	}

#ifndef NEW_RAND
	/* old code: */
	return m_nLevel;
#else
	/* new code: */
	return m_nRand & 0x00000001;
#endif
}

void CSAANoise::Sync(bool bSync)
{
	if (bSync)
	{
		m_nCounter = 0;
	}
	m_bSync = bSync;
}


void CSAANoise::SetSampleRateMode(int nSampleRateMode)
{
	// first, adjust the current value of the counter:
	if (nSampleRateMode < m_nSampleRateMode)
	{
		// samplerate has been increased; scale up counter value accordingly
		m_nCounter<<=(m_nSampleRateMode - nSampleRateMode);
	}
	else
	{
		// samplerate has been decreased (or maybe unchanged);
		// scale down counter value accordingly
		m_nCounter>>=(nSampleRateMode - m_nSampleRateMode);
	}
	m_nSampleRateMode = nSampleRateMode;
	m_nSampleRateTimes4K = 44100 << (12-m_nSampleRateMode);
}


inline void CSAANoise::ChangeLevel(void)
{
#ifndef NEW_RAND
	/* original routine : */
	m_nLevel = (m_nRand > 0x80000000) ? 1 : 0;
	m_nLevelTimesTwo = m_nLevel<<1;
	m_nRand = (m_nRand*110351245+12345); // ignore overflow. in fact, embrace it.
#else
	/* new routine (thanks to MASS) */
	/* which is quicker or better though? */
	if ( (m_nRand & 0x40000004) && ((m_nRand & 0x40000004) != 0x40000004) )
	{
		m_nRand = (m_nRand<<1)+1;
	}
	else
	{
		m_nRand<<=1;
	}
#endif
}
