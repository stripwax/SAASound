// Part of SAASound copyright 1998-2018 Dave Hooper <dave@beermex.com>
//
// SAAEnv.cpp: implementation of the CSAAEnv class.
//
//////////////////////////////////////////////////////////////////////

#include "SAASound.h"
#include "types.h"
#include "SAAEnv.h"


//////////////////////////////////////////////////////////////////////
// Static member initialisation
//////////////////////////////////////////////////////////////////////

const ENVDATA CSAAEnv::cs_EnvData[8] =
{
	{1,false,	{	{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},					{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
					{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},					{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}},
	{1,true,	{	{{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15},{15,15,15,15,15,15,15,15,15,15,15,15,15,15,15,15}},
					{{14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14},{14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14}}}},
	{1,false,	{	{{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
					{{14,14,12,12,10,10,8,8,6,6,4,4,2,2,0,0},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}},
	{1,true,	{	{{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
					{{14,14,12,12,10,10,8,8,6,6,4,4,2,2,0,0},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}},
	{2,false,	{	{{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},			{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0}},
					{{0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14},			{14,14,12,12,10,10,8,8,6,6,4,4,2,2,0,0}}}},
	{2,true,	{	{{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},			{15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0}},
					{{0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14},			{14,14,12,12,10,10,8,8,6,6,4,4,2,2,0,0}}}},
	{1,false,	{	{{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
					{{0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}},
	{1,true,	{	{{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}},
					{{0,0,2,2,4,4,6,6,8,8,10,10,12,12,14,14},			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}}}}
};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAAEnv::CSAAEnv()
:
m_bEnabled(false),
m_bNewData(false),
m_nNextData(0),
m_bOkForNewData(false)
{
	// initialise itself with the value 'zero'
	SetNewEnvData(0);
}

CSAAEnv::~CSAAEnv()
{
	// Nothing to do
}

void CSAAEnv::InternalClock(void)
{
	// will only do something if envelope clock mode is set to internal
	// and the env control is enabled
	if (m_bEnabled && (!m_bClockExternally)) Tick();
}

void CSAAEnv::ExternalClock(void)
{
	// will only do something if envelope clock mode is set to external
	// and the env control is enabled
	if (m_bClockExternally && m_bEnabled) Tick();
}

void CSAAEnv::SetEnvControl(int nData)
{
	// process immediate stuff first:
	// start with the Enabled flag.  if env is disabled,
	// there's not much to do
	bool bEnabled = ((nData & 0x80)==0x80);
	if (!bEnabled && !m_bEnabled)
		return;
	m_bEnabled = bEnabled;
	if (!m_bEnabled)
	{
		// env control was enabled, and now disabled
		// Any subsequent env control changes are immediate.
		m_bEnvelopeEnded = true;
		m_bOkForNewData = true;
		return;
	}

	// Resolution (3bit/4bit) is also immediately processed
	m_nResolution = ((nData & 0x10) == 0x10) ? 2 : 1;

	// now buffered stuff: but only if it's ok to, and only if the
	// envgenerator is not disabled. otherwise it just stays buffered until
	// the Tick() function sets okfornewdata to true and realises there is
	// already some new data waiting
	if (m_bOkForNewData)
	{
		SetNewEnvData(nData); // also does the SetLevels() call for us.
		m_bNewData=false;
		// m_bOkForNewData=false;
		// is this right?
		// NO.  See test case EnvExt_34 (setting data multiple times
		// when m_bOkForNewData is already true, should not reset
		// m_bOkForNewData)
	}
	else
	{
		// since the 'next resolution' changes arrive unbuffered, we
		// may need to change the current level because of this:
		SetLevels();
	
		// store current new data, and set the newdata flag:
		m_bNewData = true;
		m_nNextData = nData;
	}
			
}
	
unsigned short CSAAEnv::LeftLevel(void) const
{
	return m_nLeftLevel;
}

unsigned short CSAAEnv::RightLevel(void) const
{
	return m_nRightLevel;
}

inline void CSAAEnv::Tick(void)
{
	// if disabled, do nothing
	if (!m_bEnabled) // m_bEnabled is set directly, not buffered, so this is ok
	{
		// for sanity, reset stuff:
		m_bEnvelopeEnded = true;
		m_nPhase = 0;
		m_nPhasePosition = 0;
		m_bOkForNewData = true;
		return;
	}

	// else : m_bEnabled


	if (m_bEnvelopeEnded)
	{
		// do nothing 
		// (specifically, don't change the values of m_bEnvelopeEnded,
		//  m_nPhase and m_nPhasePosition, as these will still be needed
		//  by SetLevels() should it be called again)
		
		return;
	}


	// else : !m_bEnvelopeEnded
	// Continue playing the same envelope ... 
	// increments the phaseposition within an envelope.
	// also handles looping and resolution appropriately.
	// Changes the level of the envelope accordingly
	// through calling SetLevels() . This must be called after making
	// any changes that will affect the output levels of the env controller!!
	// SetLevels also handles left-right channel inverting

	// increment phase position
	// NOTE: additional test case required: If I change resolution mid-waveform
	// is the result consistent with the below?  for example:
	// phase = 0
	// tick with 4-bit resolution: phase = 1  ; should sound different when flipping resolution between 3/4
	// tick with 3-bit resolution: phase = 3  ; should sound different when flipping resolution between 3/4
	// tick with 4-bit resolution: phase = 4  ; should sound identical when flipping resolution between 3/4
	// tick with 3-bit resolution: phase = 6  ; should sound identical when flipping resolution between 3/4
	// tick with 4-bit resolution: phase = 5  ; should sound different , .. etc
	m_nPhasePosition += m_nResolution;

	// if this means we've gone past 16 (the end of a phase)
	// then change phase, and if necessary, loop
	if (m_nPhasePosition >= 16)
	{
		m_nPhase++;
		m_nPhasePosition-=16;
			
		// if we should loop, then do so - and we've reached position (4)
		// otherwise, if we shouldn't loop,
		// then we've reached position (3) and so we say that
		// we're ok for new data.
		if (m_nPhase == m_nNumberOfPhases)
		{
			// at position (3) or (4)
			m_bOkForNewData = true;

			if (!m_bLooping)
			{
				// position (3) only
				// note that it seems that the sustain level is ALWAYS zero
				// in the case of non-looping waveforms
				m_bEnvelopeEnded = true;
				m_bOkForNewData = true;
			}
			else
			{
				// position (4) only
				m_bEnvelopeEnded = false;
				// set phase pointer to start of envelope for loop
				m_nPhase=0;
			}
		}
		else // (m_nPhase < m_nNumberOfPhases)
		{
			// not at position (3) or (4) ... 
			// (i.e., we're in the middle of an envelope with
			//  more than one phase. Specifically, we're in
			//  the middle of envelope 4 or 5 - the
			//  triangle envelopes - but that's not important)

			// any commands sent to this envelope controller
			// will be buffered. Set the flag to indicate this.
			m_bOkForNewData = false;
		}
	}
	else // (m_nPhasePosition < 16)
	{
		// still within the same phase;
		// but, importantly, we are no longer at the start of the phase ...
		// so new data cannot be acted on immediately, and must
		// be buffered
		m_bOkForNewData = false;
		// Phase and PhasePosition have already been updated.
		// SetLevels() will need to be called to actually calculate
		// the output 'level' of this envelope controller
	}

	
	// if we have new (buffered) data, now is the time to act on it
	if (m_bNewData && m_bOkForNewData)
	{
		m_bNewData = false;
		// m_bOkForNewData=false;
		// is this right? do we need to reset OkForNewData?
		// NO.  See test case EnvExt_34
		// If we do set m_bOkForNewData=false, then we cannot change the envelope
		// values multiple times at point (4)
		// test case required: can we send two (or more) changes when we're at point (3)
		// or (4) in envelope waveform, prior to receiving the next tick?
		// Another test case required:  what happens if we're at point (4) in the first
		// cycle of a repetitive decay or repetitive attack, and then change waveform to
		// triangle  -  does our point (4) move and now we can NOT change waveform again
		// until the NEXT point (4)?
		SetNewEnvData(m_nNextData);
	}
	else
	{
		// ok, we didn't have any new buffered date to act on,
		// so we just call SetLevels() to calculate the output level
		// for whatever the current envelope is
		SetLevels();
	}

}

inline void CSAAEnv::SetLevels(void)
{
	// sets m_nLeftLevel 
	// Also sets m_nRightLevel in terms of m_nLeftLevel
	//								   and m_bInvertRightChannel

	// m_nResolution: 1 means 4-bit resolution; 2 means 3-bit resolution. Resolution of envelope waveform.

	// Note that this is handled 'immediately', and doesn't wait for synchronisation of
	// the envelope waveform (this is important, see test case EnvExt_imm)
	// It is therefore possible to switch between 4-bit and 3-bit resolution in the middle of
	// an envelope waveform.  if you are at an 'odd' phase position, you would be able to hear
	// the difference.  if you are at an 'even' phase position, the volume level for 4-bit
	// and 3-bit would be the same.
	// NOTE: additional test cases are required.

	switch (m_nResolution)
	{
		case 1: // 4 bit res waveforms
		default:
			{
				// special case: if envelope is not a looping one, and we're at the end
				// then our level should be zero (all of the non-looping waveforms have
				// a sustain level of zero):
				if (m_bEnvelopeEnded && !m_bLooping)
					m_nLeftLevel = 0;
				else
					m_nLeftLevel = m_pEnvData->nLevels[0][m_nPhase][m_nPhasePosition];

				if (m_bInvertRightChannel)
					m_nRightLevel = 15-m_nLeftLevel;
				else
					m_nRightLevel = m_nLeftLevel;
				break;
			}
		case 2: // 3 bit res waveforms
			{
				// special case: if envelope is not a looping one, and we're at the end
				// then our level should be zero (all of the non-looping waveforms have
				// a sustain level of zero):
				if (m_bEnvelopeEnded && !m_bLooping)
					m_nLeftLevel = 0;
				else
					m_nLeftLevel = m_pEnvData->nLevels[1][m_nPhase][m_nPhasePosition];
				if (m_bInvertRightChannel)
					m_nRightLevel = 14-m_nLeftLevel;
				else
					m_nRightLevel = m_nLeftLevel;
				break;
			}
	}
}


inline void CSAAEnv::SetNewEnvData(int nData)
{
	// loads envgenerator's registers according to the bits set
	// in nData

	m_nPhase = 0;
	m_nPhasePosition = 0;
	m_pEnvData = &(cs_EnvData[(nData >> 1) & 0x07]);
	m_bInvertRightChannel = ((nData & 0x01) == 0x01);
	m_bClockExternally = ((nData & 0x20) == 0x20);
	m_nNumberOfPhases = m_pEnvData->nNumberOfPhases;
	m_bLooping = m_pEnvData->bLooping;
	m_nResolution = (((nData & 0x10)==0x10) ? 2 : 1);
	m_bEnabled = ((nData & 0x80) == 0x80);
	if (m_bEnabled)
	{
		m_bEnvelopeEnded = false;
	}
	else
	{
		// DISABLED - so set stuff accordingly
		m_bEnvelopeEnded = true;
		m_nPhase = 0;
		m_nPhasePosition = 0;
		m_bOkForNewData = true;
	}
	
	SetLevels();

}


bool CSAAEnv::IsActive(void) const
{
	return m_bEnabled;
}
