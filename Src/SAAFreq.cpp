// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAFreq.cpp: implementation of the CSAAFreq class.
// only 7-bit fractional accuracy on oscillator periods. I may consider fixing that.
//
// Version 3.01.0 (10 Jan 2001)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#include "windows.h"
#include "types.h"
#include "SAANoise.h"
#include "SAAEnv.h"
#include "SAAFreq.h"
#ifdef _DEBUG
#include <stdio.h>
#endif
// 'load in' the data for the static frequency lookup table:
const unsigned long CSAAFreq::m_FreqTable[2048] =
{
#include "SAAFreqTable.dat"
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAAFreq::CSAAFreq(CSAANoise * const NoiseGenerator, CSAAEnv * const EnvGenerator)
:
m_nConnectedMode((NoiseGenerator == NULL) ? ((EnvGenerator == NULL) ? 0 : 1) : 2),
m_pcConnectedNoiseGenerator(NoiseGenerator),
m_pcConnectedEnvGenerator(EnvGenerator),
m_nLevel(2), m_nCounter(0), m_nAdd(0),
m_nCurrentOffset(0), m_nCurrentOctave(0), m_nNextOffset(0), m_nNextOctave(0),
m_bNewData(false), m_bIgnoreOffsetData(false),
m_bSync(false),
m_nSampleRateMode(2), m_nSampleRateTimes4K(11025<<12)
{
	SetAdd(); // current octave, current offset
}

CSAAFreq::~CSAAFreq()
{
	// Nothing to do
}

void CSAAFreq::SetFreqOffset(BYTE nOffset)
{
	// nOffset between 0 and 255

	if (!m_bSync)
	{
		m_nNextOffset = nOffset;
		m_bNewData=true;
		if (m_nNextOctave==m_nCurrentOctave)
		{
			// According to Philips, if you send the SAA-1099
			// new Octave data and then new Offset data in that
			// order, on the next half-cycle of the current frequency
			// generator, ONLY the octave data is acted upon.
			// The offset data will be acted upon next time.
			m_bIgnoreOffsetData=true;
		}
	}
	else
	{
		// updates straightaway if m_bSync
		m_bNewData=false;
		m_nCurrentOffset = nOffset;
		m_nCurrentOctave = m_nNextOctave;
		SetAdd();
	}

}

void CSAAFreq::SetFreqOctave(BYTE nOctave)
{
	// nOctave between 0 and 7

	if (!m_bSync)
	{
		m_nNextOctave = nOctave;
		m_bNewData=true;
		m_bIgnoreOffsetData = false;
	}
	else
	{
		// updates straightaway if m_bSync
		m_bNewData=false;
		m_nCurrentOctave = nOctave;
		m_nCurrentOffset = m_nNextOffset;
		SetAdd();
	}
}

void CSAAFreq::UpdateOctaveOffsetData(void)
{
	// loads the buffered new octave and new offset data into the current registers
	// and sets up the new frequency for this frequency generator (i.e. sets up m_nAdd)
	// - called during Sync, and called when waveform half-cycle completes

	// How the SAA-1099 really treats new data:
	// if only new octave data is present,
	// then set new period based on just the octave data
	// Otherwise, if only new offset data is present,
	// then set new period based on just the offset data
	// Otherwise, if new octave data is present, and new offset data is present,
	// and the offset data was set BEFORE the octave data,
	// then set new period based on both the octave and offset data
	// Else, if the offset data came AFTER the new octave data
	// then set new period based on JUST THE OCTAVE DATA, and continue
	// signalling the offset data as 'new', so it will be acted upon
	// next half-cycle
	// 
	// Weird, I know. But that's how it works. Philips even documented as much.

	if (!m_bNewData)
	{
		// optimise for the most common case! No new data!
		return;
	}
		
	m_nCurrentOctave=m_nNextOctave;
	if (!m_bIgnoreOffsetData)
	{
		m_nCurrentOffset=m_nNextOffset;
		m_bNewData=false;
	}
	m_bIgnoreOffsetData=false;

	SetAdd();
}

void CSAAFreq::SetSampleRateMode(int nSampleRateMode)
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
	m_nSampleRateTimes4K = 44100 << (12-nSampleRateMode);
}

unsigned short CSAAFreq::Level(void) const
{
	return GetLevel(m_nLevel);
}

/*static*/ inline unsigned short CSAAFreq::GetLevel(unsigned short nLevel)
{
	return nLevel;
}


unsigned short CSAAFreq::Tick(void)
{
	// set to the absolute level (0 or 2)
	if (!m_bSync)
	{

		m_nCounter+=m_nAdd;

		if (m_nCounter >= m_nSampleRateTimes4K)
		{
			// period elapsed for one half-cycle of
			// current frequency
			// reset counter to zero (or thereabouts, taking into account
			// the fractional part in the lower 12 bits)
			while (m_nCounter >= m_nSampleRateTimes4K)
			{
				m_nCounter-=m_nSampleRateTimes4K;
				// flip state - from 0 to -2 or vice versa
				m_nLevel=2-m_nLevel;
					
				// trigger any connected devices
				switch (m_nConnectedMode)
				{
				case 1:
					// env trigger
					m_pcConnectedEnvGenerator->InternalClock();
					break;
			
				case 2:
					// noise trigger
					m_pcConnectedNoiseGenerator->Trigger();
					break;
				
				default:
					// do nothing
					break;
				}
				
			}

			// get new frequency (set period length m_nAdd) if new data is waiting:
			UpdateOctaveOffsetData();
		}
	
	}
	return GetLevel(m_nLevel);
}


void CSAAFreq::SetAdd(void)
{
	// nOctave between 0 and 7; nOffset between 0 and 255

	// Used to be:
	// m_nAdd = (15625 << nOctave) / (511 - nOffset);
	// Now just table lookup:
	m_nAdd = m_FreqTable[m_nCurrentOctave<<8 | m_nCurrentOffset];
}

void CSAAFreq::Sync(bool bSync)
{
	m_bSync = bSync;

	// update straightaway if m_bSync
	if (m_bSync)
	{
		m_nCounter = 0;
		m_nLevel=2;
		m_nCurrentOctave=m_nNextOctave;
		m_nCurrentOffset=m_nNextOffset;
		SetAdd();
	}
}