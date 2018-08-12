// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAFreq.cpp: implementation of the CSAAFreq class.
// only 7-bit fractional accuracy on oscillator periods. I may consider fixing that.
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
m_nLevel(0), m_nCounter(0), m_nAdd(0),
m_nCurrentOffset(0), m_nCurrentOctave(0), m_nNextOffset(0), m_nNextOctave(0),
m_bNewOffsetData(false), m_bNewOctaveData(false), m_bIgnoreOffsetData(false),
m_bSync(false),
m_nSampleRateMode(2), m_nSampleRateTimes128(11025*128)
{
	SetAdd(0,0);
}

CSAAFreq::~CSAAFreq()
{
	// Nothing to do
}

void CSAAFreq::SetFreqOffset(BYTE nOffset)
{
	m_nNextOffset = nOffset;
	m_bNewOffsetData = true;
	if (m_bNewOctaveData) m_bIgnoreOffsetData = true;
}

void CSAAFreq::SetFreqOctave(BYTE nOctave)
{
	m_nNextOctave = nOctave;
	m_bNewOctaveData = true;
	if (m_bNewOffsetData) m_bIgnoreOffsetData = false;
}

void CSAAFreq::SetSampleRateMode(BYTE nSampleRateMode)
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
	m_nSampleRateTimes128 = (44100 << 7) >> nSampleRateMode;
}

BYTE CSAAFreq::Level(void) const
{
	return m_nLevel;
}


BYTE CSAAFreq::Tick(void)
{
	if (!m_bSync)
	{
		m_nCounter+=m_nAdd;
		if (m_nCounter >= m_nSampleRateTimes128)
		{
			// period elapsed for one half-cycle of
			// current frequency
			// reset counter to zero (or thereabouts, taking into account
			// the fractional part in the lower 8 bits)
			while (m_nCounter >= m_nSampleRateTimes128)
			{
				m_nCounter-=m_nSampleRateTimes128;
		
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
			m_nLevel ^= 0x02;
	
			// get new frequency (set period length m_nAdd) if new data is waiting:
			
			// if new octave data is present, and new offset data is present,
			// and the offset data came AFTER the new octave data
			// then set new period based on just the octave data, and continue
			// signalling the offset data as 'new', so it will be acted upon
			// next half-cycle
			// Otherwise, if both new octave and offset data are present,
			// and the offset data was set BEFORE the octave data,
			// then set new period based on both the octave and offset data
			// Otherwise, if only new octave data is present,
			// then set new period based on just the octave data
			// Otherwise, if only new offset data is present,
			// then set new period based on just the offset data
			// Otherwise, if no new data is present, do nothing.
			if (m_bNewOctaveData)
			{
				if (m_bNewOffsetData)
				{
					if (m_bIgnoreOffsetData)
					{
						m_bNewOctaveData=false;
						m_bNewOffsetData=true;
						m_nCurrentOctave=m_nNextOctave;
						SetAdd(m_nCurrentOctave, m_nCurrentOffset);
					}
					else
					{
						m_bNewOctaveData=false;
						m_bNewOffsetData=false;
						m_nCurrentOctave=m_nNextOctave;
						m_nCurrentOffset=m_nNextOffset;
						SetAdd(m_nCurrentOctave, m_nCurrentOffset);
					}
				}
				else
				{
					m_bNewOctaveData=false;
					m_nCurrentOctave=m_nNextOctave;
					SetAdd(m_nCurrentOctave, m_nCurrentOffset);
				}
			}
			else if (m_bNewOffsetData)
			{
				m_bNewOffsetData=false;
				m_nCurrentOffset=m_nNextOffset;
				SetAdd(m_nCurrentOctave, m_nCurrentOffset);
			}
		}
	}

	return m_nLevel;
}


void CSAAFreq::SetAdd(BYTE nOctave, BYTE nOffset)
{
	// Used to be:
	// m_nAdd = (15625 << (nOctave + 8)) / (511 - nOffset);
	// Now just lookup:
	m_nAdd = m_FreqTable[nOctave<<8 | nOffset];
}

void CSAAFreq::Sync(bool bSync)
{
	if (bSync)
	{
		m_nCounter = 0;
	}
	m_bSync = bSync;
}