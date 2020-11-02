// Part of SAASound copyright 2020 Dave Hooper <dave@beermex.com>
//
// SAADevice.cpp: connecting the subcomponents of the SAA1099 together.
// This class handles device inputs and outputs (clocking, data and
// address bus, and simulated output)
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include <assert.h>
#else
#define assert(exp)	((void)0)
#endif

#include "SAASound.h"

#include "types.h"
#include "SAAEnv.h"
#include "SAANoise.h"
#include "SAAFreq.h"
#include "SAAAmp.h"
#include "SAASound.h"
#include "SAAImpl.h"
#include "defns.h"


//////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////

// TODO: Support DEBUGSAA via runtime flag / config file
#ifdef DEBUGSAA
#include <stdio.h>	// for sprintf
FILE* dbgfile = NULL;
FILE* pcmfile = NULL;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAADevice::CSAADevice()
	:
	m_nCurrentSaaReg(0),
	m_bOutputEnabled(false),
	m_bSync(false),
	m_bHighpass(true),
	m_Noise0(0xffffffff),
	m_Noise1(0xffffffff),
	m_Env0(),
	m_Env1(),
	m_Osc0(&m_Noise0, NULL),
	m_Osc1(NULL, &m_Env0),
	m_Osc2(NULL, NULL),
	m_Osc3(&m_Noise1, NULL),
	m_Osc4(NULL, &m_Env1),
	m_Osc5(NULL, NULL),
	m_Amp0(&m_Osc0, &m_Noise0, NULL),
	m_Amp1(&m_Osc1, &m_Noise0, NULL),
	m_Amp2(&m_Osc2, &m_Noise0, &m_Env0),
	m_Amp3(&m_Osc3, &m_Noise1, NULL),
	m_Amp4(&m_Osc4, &m_Noise1, NULL),
	m_Amp5(&m_Osc5, &m_Noise1, &m_Env1)
{
	// Create and link up the objects that make up the emulator
	Noise[0] = &m_Noise0;
	Noise[1] = &m_Noise1;
	Env[0] = &m_Env0;
	Env[1] = &m_Env1;

	// Create oscillators (tone generators) and link to noise generators and
	// envelope controllers
	Osc[0] = &m_Osc0;
	Osc[1] = &m_Osc1;
	Osc[2] = &m_Osc2;
	Osc[3] = &m_Osc3;
	Osc[4] = &m_Osc4;
	Osc[5] = &m_Osc5;

	// Create amplification/mixing stages and link to appropriate oscillators,
	// noise generators and envelope controllers
	Amp[0] = &m_Amp0;
	Amp[1] = &m_Amp1;
	Amp[2] = &m_Amp2;
	Amp[3] = &m_Amp3;
	Amp[4] = &m_Amp4;
	Amp[5] = &m_Amp5;

	_SetClockRate(EXTERNAL_CLK_HZ);
}

CSAADevice::~CSAADevice()
{
}

//////////////////////////////////////////////////////////////////////
// CSAASound members
//////////////////////////////////////////////////////////////////////

void CSAADevice::_SetClockRate(unsigned int nClockRate)
{
	m_Osc0.SetClockRate(nClockRate);
	m_Osc1.SetClockRate(nClockRate);
	m_Osc2.SetClockRate(nClockRate);
	m_Osc3.SetClockRate(nClockRate);
	m_Osc4.SetClockRate(nClockRate);
	m_Osc5.SetClockRate(nClockRate);
	m_Noise0.SetClockRate(nClockRate);
	m_Noise1.SetClockRate(nClockRate);
}

void CSAADevice::_SetOversample(unsigned int nOversample)
{
	m_Osc0.SetOversample(nOversample);
	m_Osc1.SetOversample(nOversample);
	m_Osc2.SetOversample(nOversample);
	m_Osc3.SetOversample(nOversample);
	m_Osc4.SetOversample(nOversample);
	m_Osc5.SetOversample(nOversample);
	m_Noise0.SetOversample(nOversample);
	m_Noise1.SetOversample(nOversample);
}

void CSAADevice::_WriteData(BYTE nData)
{
#ifdef DEBUG
	m_reg[m_nCurrentSaaReg] = nData;
#endif

	// route nData to the appropriate place
	switch (m_nCurrentSaaReg)
	{
		// Amplitude data (==> Amp)
	case 0:
		m_Amp0.SetAmpLevel(nData);
		break;
	case 1:
		m_Amp1.SetAmpLevel(nData);
		break;
	case 2:
		m_Amp2.SetAmpLevel(nData);
		break;
	case 3:
		m_Amp3.SetAmpLevel(nData);
		break;
	case 4:
		m_Amp4.SetAmpLevel(nData);
		break;
	case 5:
		m_Amp5.SetAmpLevel(nData);
		break;

		// Freq data (==> Osc)
	case 8:
		m_Osc0.SetFreqOffset(nData);
		break;
	case 9:
		m_Osc1.SetFreqOffset(nData);
		break;
	case 10:
		m_Osc2.SetFreqOffset(nData);
		break;
	case 11:
		m_Osc3.SetFreqOffset(nData);
		break;
	case 12:
		m_Osc4.SetFreqOffset(nData);
		break;
	case 13:
		m_Osc5.SetFreqOffset(nData);
		break;

		// Freq octave data (==> Osc) for channels 0,1
	case 16:
		m_Osc0.SetFreqOctave(nData & 0x07);
		m_Osc1.SetFreqOctave((nData >> 4) & 0x07);
		break;

		// Freq octave data (==> Osc) for channels 2,3
	case 17:
		m_Osc2.SetFreqOctave(nData & 0x07);
		m_Osc3.SetFreqOctave((nData >> 4) & 0x07);
		break;

		// Freq octave data (==> Osc) for channels 4,5
	case 18:
		m_Osc4.SetFreqOctave(nData & 0x07);
		m_Osc5.SetFreqOctave((nData >> 4) & 0x07);
		break;

		// Tone mixer control (==> Amp)
	case 20:
		m_Amp0.SetToneMixer(nData & 0x01);
		m_Amp1.SetToneMixer(nData & 0x02);
		m_Amp2.SetToneMixer(nData & 0x04);
		m_Amp3.SetToneMixer(nData & 0x08);
		m_Amp4.SetToneMixer(nData & 0x10);
		m_Amp5.SetToneMixer(nData & 0x20);
		break;

		// Noise mixer control (==> Amp)
	case 21:
		m_Amp0.SetNoiseMixer(nData & 0x01);
		m_Amp1.SetNoiseMixer(nData & 0x02);
		m_Amp2.SetNoiseMixer(nData & 0x04);
		m_Amp3.SetNoiseMixer(nData & 0x08);
		m_Amp4.SetNoiseMixer(nData & 0x10);
		m_Amp5.SetNoiseMixer(nData & 0x20);
		break;

		// Noise frequency/source control (==> Noise)
	case 22:
		m_Noise0.SetSource(nData & 0x03);
		m_Noise1.SetSource((nData >> 4) & 0x03);
		break;

		// Envelope control data (==> Env) for envelope controller #0
	case 24:
		m_Env0.SetEnvControl(nData);
		break;

		// Envelope control data (==> Env) for envelope controller #1
	case 25:
		m_Env1.SetEnvControl(nData);
		break;

		// Global enable and reset (sync) controls
	case 28:
		{
			// Reset (sync) bit
			bool bSync = bool(nData & 0x02);
			if (bSync != m_bSync)
			{
				// Sync all devices
				// This amounts to telling them all to reset to a
				// known state, which is also a state that doesn't change
				// (i.e. no audio output, although there are some exceptions)
				// bSync=true => all devices are sync (aka reset);
				// bSync=false => all devices are allowed to run and generate changing output
				m_Osc0.Sync(bSync);
				m_Osc1.Sync(bSync);
				m_Osc2.Sync(bSync);
				m_Osc3.Sync(bSync);
				m_Osc4.Sync(bSync);
				m_Osc5.Sync(bSync);
				m_Noise0.Sync(bSync);
				m_Noise1.Sync(bSync);
				m_Amp0.Sync(bSync);
				m_Amp1.Sync(bSync);
				m_Amp2.Sync(bSync);
				m_Amp3.Sync(bSync);
				m_Amp4.Sync(bSync);
				m_Amp5.Sync(bSync);
				m_bSync = bSync;
			}

			// Global mute bit
			bool bOutputEnabled = bool(nData & 0x01);
			if (bOutputEnabled != m_bOutputEnabled)
			{
				// unmute all amps - sound 'enabled'
				m_Amp0.Mute(!bOutputEnabled);
				m_Amp1.Mute(!bOutputEnabled);
				m_Amp2.Mute(!bOutputEnabled);
				m_Amp3.Mute(!bOutputEnabled);
				m_Amp4.Mute(!bOutputEnabled);
				m_Amp5.Mute(!bOutputEnabled);
				m_bOutputEnabled = bOutputEnabled;
			}
		}
		break;

	default:
		// anything else means data is being written to a register
		// that is not used within the SAA-1099 architecture
		// hence, we ignore it.
	{}
	}
}

void CSAADevice::_WriteAddress(BYTE nReg)
{
	m_nCurrentSaaReg = nReg & 31;
	if (m_nCurrentSaaReg == 24)
	{
		m_Env0.ExternalClock();
	}
	else if (m_nCurrentSaaReg == 25)
	{
		m_Env1.ExternalClock();
	}
}

#ifdef DEBUG
BYTE CSAADevice::_ReadAddress(void)
{
	// Not a real hardware function of the SAA-1099, which is write-only
	return(m_nCurrentSaaReg);
}

BYTE CSAADevice::_ReadData(void)
{
	// Not a real hardware function of the SAA-1099, which is write-only
	return(m_Reg[m_nCurrentSaaReg]);
}
#endif

void CSAADevice::_TickAndOutputStereo(unsigned int& left, unsigned int& right)
{
	unsigned int temp_left, temp_right;
	unsigned int accum_left = 0, accum_right = 0;
	for (int i = 1 << m_nOversample; i > 0; i--)
	{
		m_Noise0.Tick();
		m_Noise1.Tick();
		m_Amp0.TickAndOutputStereo(temp_left, temp_right);
		accum_left += temp_left;
		accum_right += temp_right;
		m_Amp1.TickAndOutputStereo(temp_left, temp_right);
		accum_left += temp_left;
		accum_right += temp_right;
		m_Amp2.TickAndOutputStereo(temp_left, temp_right);
		accum_left += temp_left;
		accum_right += temp_right;
		m_Amp3.TickAndOutputStereo(temp_left, temp_right);
		accum_left += temp_left;
		accum_right += temp_right;
		m_Amp4.TickAndOutputStereo(temp_left, temp_right);
		accum_left += temp_left;
		accum_right += temp_right;
		m_Amp5.TickAndOutputStereo(temp_left, temp_right);
		accum_left += temp_left;
		accum_right += temp_right;
	}
	left = accum_left;
	right = accum_right;
}