// Part of SAASound copyright 1998-2018 Dave Hooper <dave@beermex.com>
//
// SAAImpl.cpp: implementation of the CSAASound class.
// the bones of the 'virtual SAA-1099' emulation
//
// the actual sound generation is carried out in the other classes;
// this class provides the output stage and the external interface only
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

//////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////

// TODO: Support DEBUGSAA via runtime flag / config file
#ifdef DEBUGSAA
#include <stdio.h>	// for sprintf
FILE * dbgfile = NULL;
FILE * pcmfile = NULL;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAASoundInternal::CSAASoundInternal()
:
m_nCurrentSaaReg(0),
m_bOutputEnabled(false),
m_bSync(false),
m_uParam(0),
m_uParamRate(0)
{
	prev_output_mono = 0;
	prev_output_left = prev_output_right = 0;

	#ifdef DEBUGSAA
	dbgfile = fopen("debugsaa.txt","wt");
	pcmfile = fopen("debugsaa.pcm","wb");
	#endif

	// Create and link up the objects that make up the emulator
	Noise[0] = new CSAANoise(0xffffffff); // Create and seed a noise generator
	Noise[1] = new CSAANoise(0xffffffff); // Create and seed a noise generator
	Env[0] = new CSAAEnv;
	Env[1] = new CSAAEnv;

	// (check that memory allocated succeeded this far)
	assert (Noise[0] != NULL);
	assert (Noise[1] != NULL);
	assert (Env[0] != NULL);
	assert (Env[1] != NULL);

	// Create oscillators (tone generators) and link to noise generators and
	// envelope controllers
	Osc[0] = new CSAAFreq(Noise[0], NULL);
	Osc[1] = new CSAAFreq(NULL, Env[0]);
	Osc[2] = new CSAAFreq(NULL, NULL);
	Osc[3] = new CSAAFreq(Noise[1], NULL);
	Osc[4] = new CSAAFreq(NULL, Env[1]);
	Osc[5] = new CSAAFreq(NULL, NULL);
	for (int i=5; i>=0; i--)
	{
		assert (Osc[i] != NULL);
	}

	// Create amplification/mixing stages and link to appropriate oscillators,
	// noise generators and envelope controllers
	Amp[0] = new CSAAAmp(Osc[0], Noise[0], NULL),
	Amp[1] = new CSAAAmp(Osc[1], Noise[0], NULL),
	Amp[2] = new CSAAAmp(Osc[2], Noise[0], Env[0]),
	Amp[3] = new CSAAAmp(Osc[3], Noise[1], NULL),
	Amp[4] = new CSAAAmp(Osc[4], Noise[1], NULL),
	Amp[5] = new CSAAAmp(Osc[5], Noise[1], Env[1]);
	for (int j=5; j>=0; j--)
	{
		assert (Amp[j] != NULL);
	}

	// set parameters
	// TODO support defaults and overrides from config file
	SetSoundParameters(SAAP_FILTER | SAAP_11025 | SAAP_8BIT | SAAP_MONO);
	// reset the virtual SAA
	Clear();

	SetClockRate(8000000);
}

CSAASoundInternal::~CSAASoundInternal()
{
	if (Noise[0]) delete Noise[0];
	if (Noise[1]) delete Noise[1];
	if (Env[0]) delete Env[0];
	if (Env[1]) delete Env[1];
	for (int i=5; i>=0; i--)
	{
		if (Osc[i]) delete Osc[i];
		if (Amp[i]) delete Amp[i];
	}
	
#ifdef DEBUGSAA
	if (dbgfile) fclose(dbgfile);
#endif
}

//////////////////////////////////////////////////////////////////////
// CSAASound members
//////////////////////////////////////////////////////////////////////

void CSAASoundInternal::SetClockRate(unsigned int nClockRate)
{
	for (int i = 0; i < 6; i++)
	{
		Osc[i]->SetClockRate(nClockRate);
	}
	Noise[0]->SetClockRate(nClockRate);
	Noise[1]->SetClockRate(nClockRate);
}

void CSAASoundInternal::Clear(void)
{
	// reinitialises virtual SAA:
	// sets reg 28 to 0x02; - sync and disabled
	// sets regs 00-31 (except 28) to 0x00;
	// sets reg 28 to 0x00;
	// sets current reg to 0
	WriteAddressData(28,2);
	for (int i=31; i>=0; i--)
	{
		if (i!=28) WriteAddressData(i,0);
	}
	WriteAddressData(28,0);
	WriteAddress(0);
}

void CSAASoundInternal::WriteData(BYTE nData)
{
	// originated from an OUT 255,d call

#ifdef DEBUGSAA
	fprintf(dbgfile, "%lu %02d:%02x\n", m_nDebugSample, m_nCurrentSaaReg, nData);
#endif

	// route nData to the appropriate place
	switch (m_nCurrentSaaReg)
	{
		// Amplitude data (==> Amp)
		case 0:
			Amp[0]->SetAmpLevel(nData);
			break;
		case 1:
			Amp[1]->SetAmpLevel(nData);
			break;
		case 2:
			Amp[2]->SetAmpLevel(nData);
			break;
		case 3:
			Amp[3]->SetAmpLevel(nData);
			break;
		case 4:
			Amp[4]->SetAmpLevel(nData);
			break;
		case 5:
			Amp[5]->SetAmpLevel(nData);
			break;

		// Freq data (==> Osc)
		case 8:
			Osc[0]->SetFreqOffset(nData);
			break;
		case 9:
			Osc[1]->SetFreqOffset(nData);
			break;
		case 10:
			Osc[2]->SetFreqOffset(nData);
			break;
		case 11:
			Osc[3]->SetFreqOffset(nData);
			break;
		case 12:
			Osc[4]->SetFreqOffset(nData);
			break;
		case 13:
			Osc[5]->SetFreqOffset(nData);
			break;

		// Freq octave data (==> Osc) for channels 0,1
		case 16:
			Osc[0]->SetFreqOctave(nData & 0x07);
			Osc[1]->SetFreqOctave((nData >> 4) & 0x07);
			break;

		// Freq octave data (==> Osc) for channels 2,3
		case 17:
			Osc[2]->SetFreqOctave(nData & 0x07);
			Osc[3]->SetFreqOctave((nData >> 4) & 0x07);
			break;

		// Freq octave data (==> Osc) for channels 4,5
		case 18:
			Osc[4]->SetFreqOctave(nData & 0x07);
			Osc[5]->SetFreqOctave((nData >> 4) & 0x07);
			break;

		// Tone mixer control (==> Amp)
		case 20:
			Amp[0]->SetToneMixer(nData & 0x01);
			Amp[1]->SetToneMixer(nData & 0x02);
			Amp[2]->SetToneMixer(nData & 0x04);
			Amp[3]->SetToneMixer(nData & 0x08);
			Amp[4]->SetToneMixer(nData & 0x10);
			Amp[5]->SetToneMixer(nData & 0x20);
			break;

		// Noise mixer control (==> Amp)
		case 21:
			Amp[0]->SetNoiseMixer(nData & 0x01);
			Amp[1]->SetNoiseMixer(nData & 0x02);
			Amp[2]->SetNoiseMixer(nData & 0x04);
			Amp[3]->SetNoiseMixer(nData & 0x08);
			Amp[4]->SetNoiseMixer(nData & 0x10);
			Amp[5]->SetNoiseMixer(nData & 0x20);
			break;

		// Noise frequency/source control (==> Noise)
		case 22:
			Noise[0]->SetSource(nData & 0x03);
			Noise[1]->SetSource((nData >> 4) & 0x03);
			break;

		// Envelope control data (==> Env) for envelope controller #0
		case 24:
			Env[0]->SetEnvControl(nData);
			break;

		// Envelope control data (==> Env) for envelope controller #1
		case 25:
			Env[1]->SetEnvControl(nData);
			break;

		// Global enable and reset (sync) controls
		case 28:
			// Reset (sync) bit
			if (nData & 0x02)
			{
				// Sync all devices
				// This amounts to telling them all to reset to a
				// known state.
				Osc[0]->Sync(true);
				Osc[1]->Sync(true);
				Osc[2]->Sync(true);
				Osc[3]->Sync(true);
				Osc[4]->Sync(true);
				Osc[5]->Sync(true);
				Noise[0]->Sync(true);
				Noise[1]->Sync(true);
				Amp[0]->Sync(true);
				Amp[1]->Sync(true);
				Amp[2]->Sync(true);
				Amp[3]->Sync(true);
				Amp[4]->Sync(true);
				Amp[5]->Sync(true);
				m_bSync = true;
			}
			else
			{
				// Unsync all devices i.e. run oscillators
				Osc[0]->Sync(false);
				Osc[1]->Sync(false);
				Osc[2]->Sync(false);
				Osc[3]->Sync(false);
				Osc[4]->Sync(false);
				Osc[5]->Sync(false);
				Noise[0]->Sync(false);
				Noise[1]->Sync(false);
				Amp[0]->Sync(false);
				Amp[1]->Sync(false);
				Amp[2]->Sync(false);
				Amp[3]->Sync(false);
				Amp[4]->Sync(false);
				Amp[5]->Sync(false);
				m_bSync = false;
			}

			// Global mute bit
			if (nData & 0x01)
			{
				// unmute all amps - sound 'enabled'
				Amp[0]->Mute(false);
				Amp[1]->Mute(false);
				Amp[2]->Mute(false);
				Amp[3]->Mute(false);
				Amp[4]->Mute(false);
				Amp[5]->Mute(false);
				m_bOutputEnabled = true;
			}
			else
			{
				// mute all amps
				Amp[0]->Mute(true);
				Amp[1]->Mute(true);
				Amp[2]->Mute(true);
				Amp[3]->Mute(true);
				Amp[4]->Mute(true);
				Amp[5]->Mute(true);
				m_bOutputEnabled = false;
			}

			break;

		default:
			// anything else means data is being written to a register
			// that is not used within the SAA-1099 architecture
			// hence, we ignore it.
			{}
	}
}

void CSAASoundInternal::WriteAddress(BYTE nReg)
{
	// originated from an OUT 511,r call
#ifdef DEBUGSAA
	fprintf(dbgfile, "%lu %02d:", m_nDebugSample, nReg);
#endif
	m_nCurrentSaaReg = nReg & 31;
	if (m_nCurrentSaaReg==24)
	{
		Env[0]->ExternalClock();
#ifdef DEBUGSAA
		fprintf(dbgfile, "<!ENVO!>");
#endif
	}
	else if (m_nCurrentSaaReg==25)
	{
		Env[1]->ExternalClock();
#ifdef DEBUGSAA
		fprintf(dbgfile, "<!ENV1!>");
#endif
	}
#ifdef DEBUGSAA
	fprintf(dbgfile,"\n");
#endif
}

void CSAASoundInternal::WriteAddressData(BYTE nReg, BYTE nData)
{
	// performs WriteAddress(nReg) followed by WriteData(nData)
	WriteAddress(nReg);
	WriteData(nData);
}

BYTE CSAASoundInternal::ReadAddress(void)
{
	// Not a real hardware function of the SAA-1099, which is write-only
	return(m_nCurrentSaaReg);
}

void CSAASoundInternal::SetSoundParameters(SAAPARAM uParam)
{
	int sampleratemode = CSAASound::GetSampleRate(m_uParamRate);

	// set samplerate properties from uParam
	switch (uParam & SAAP_MASK_SAMPLERATE)
	{
	case SAAP_44100:
		sampleratemode = 0;
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_44100;
		break;
	case SAAP_22050:
		sampleratemode = 1;
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_22050;
		break;
	case SAAP_11025:
		sampleratemode = 2;
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_11025;
		break;
	case 0:// change nothing!
	default:
		break;
	}

	Osc[0]->SetSampleRateMode(sampleratemode);
	Osc[1]->SetSampleRateMode(sampleratemode);
	Osc[2]->SetSampleRateMode(sampleratemode);
	Osc[3]->SetSampleRateMode(sampleratemode);
	Osc[4]->SetSampleRateMode(sampleratemode);
	Osc[5]->SetSampleRateMode(sampleratemode);
	Noise[0]->SetSampleRateMode(sampleratemode);
	Noise[1]->SetSampleRateMode(sampleratemode);

	// set filter properties from uParam
	m_uParam = (m_uParam & ~SAAP_MASK_FILTER) | (uParam & SAAP_MASK_FILTER);
	 
	m_nOversample = 0;

	// temporarily force OVERSAMPLE64x mode for testing
	//if ( (m_uParam & SAAP_MASK_FILTER_OVERSAMPLE) < SAAP_FILTER_OVERSAMPLE2x)
	//	m_uParam = (m_uParam & ~SAAP_MASK_FILTER_OVERSAMPLE) | SAAP_FILTER_OVERSAMPLE2x;
	if ( (m_uParam & SAAP_MASK_FILTER_OVERSAMPLE) < SAAP_FILTER_OVERSAMPLE64x)
		m_uParam = (m_uParam & ~SAAP_MASK_FILTER_OVERSAMPLE) | SAAP_FILTER_OVERSAMPLE64x;

	// temporarily force highpass filter on for testing
	m_uParam = (m_uParam & ~SAAP_MASK_FILTER_HIGHPASS) | SAAP_FILTER_HIGHPASS_SIMPLE;

	// Enabling the oversampling filter puts the oscillators and noise generators
	// into a higher sample rate via a scaling factor on the multilevel counter
	if ( (m_uParam & SAAP_MASK_FILTER_OVERSAMPLE) == SAAP_FILTER_OVERSAMPLE2x)
	{
		// i.e. 2^1
		m_nOversample = 1;
	}
	else if ((m_uParam & SAAP_MASK_FILTER_OVERSAMPLE) == SAAP_FILTER_OVERSAMPLE64x)
	{
		// i.e. 2^6
		m_nOversample = 6;
	}

	Osc[0]->SetOversample(m_nOversample);
	Osc[1]->SetOversample(m_nOversample);
	Osc[2]->SetOversample(m_nOversample);
	Osc[3]->SetOversample(m_nOversample);
	Osc[4]->SetOversample(m_nOversample);
	Osc[5]->SetOversample(m_nOversample);
	Noise[0]->SetOversample(m_nOversample);
	Noise[1]->SetOversample(m_nOversample);

	// set bit depth properties from uParam
	switch (uParam & SAAP_MASK_BITDEPTH)
	{
		case SAAP_8BIT: // set 8bit mode
			m_uParam = (m_uParam & ~SAAP_MASK_BITDEPTH) | SAAP_8BIT;
			break;
		case SAAP_16BIT: // set 16bit mode
			m_uParam = (m_uParam & ~SAAP_MASK_BITDEPTH) | SAAP_16BIT;
			break;
		case 0:// change nothing!
		default:
			break;
	}

	// set number of channels from uParam
	switch (uParam & SAAP_MASK_CHANNELS)
	{
		case SAAP_MONO: // set mono
			m_uParam = (m_uParam & ~SAAP_MASK_CHANNELS) | SAAP_MONO;
			break;
		case SAAP_STEREO: // set stereo
			m_uParam = (m_uParam & ~SAAP_MASK_CHANNELS) | SAAP_STEREO;
			break;
		case 0:// change nothing!
		default:
			break;
	}
}

SAAPARAM CSAASoundInternal::GetCurrentSoundParameters(void)
{
	return m_uParam | m_uParamRate;
}

unsigned short CSAASoundInternal::GetCurrentBytesPerSample(void)
{
	return CSAASound::GetBytesPerSample(m_uParam);
}

/*static*/ unsigned short CSAASound::GetBytesPerSample(SAAPARAM uParam)
{
	switch (uParam & (SAAP_MASK_CHANNELS | SAAP_MASK_BITDEPTH))
	{
		case SAAP_MONO | SAAP_8BIT:
			return 1;
		case SAAP_MONO | SAAP_16BIT:
		case SAAP_STEREO | SAAP_8BIT:
			return 2;
		case SAAP_STEREO | SAAP_16BIT:
			return 4;
		default:
			return 0;
	}
}

unsigned long CSAASoundInternal::GetCurrentSampleRate(void)
{
	return CSAASound::GetSampleRate(m_uParamRate);
}

/*static*/ unsigned long CSAASound::GetSampleRate(SAAPARAM uParam) // static member function
{
	switch (uParam & SAAP_MASK_SAMPLERATE)
	{
		case SAAP_11025:
			return 11025;
		case SAAP_22050:
			return 22050;
		case SAAP_44100:
			return 44100;
		default:
			return 0;
	}
}

void CSAASoundInternal::GenerateMany(BYTE* pBuffer, unsigned long nSamples)
{
	unsigned short mono1, mono2, mono;
	unsigned short left, right, temp_left, temp_right;

	unsigned short prev_mono = prev_output_mono;
	unsigned short prev_left, prev_right;
	prev_left = prev_output_left;
	prev_right = prev_output_right;

	static double filterout_z1_left = 0, filterout_z1_right = 0;

#ifdef DEBUGSAA
	BYTE* pBufferStart = pBuffer;
	unsigned long nTotalSamples = nSamples;
#endif

	if ( (m_uParam & SAAP_MASK_FILTER_OVERSAMPLE) == SAAP_NOFILTER)
	{
		// NO FILTER
		switch (m_uParam & (SAAP_MASK_CHANNELS + SAAP_MASK_BITDEPTH))
		{
		case SAAP_MONO | SAAP_8BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();

				mono = (Amp[0]->TickAndOutputMono() +
					Amp[1]->TickAndOutputMono() +
					Amp[2]->TickAndOutputMono() +
					Amp[3]->TickAndOutputMono() +
					Amp[4]->TickAndOutputMono() +
					Amp[5]->TickAndOutputMono());
				mono = mono * 5;
				mono = 0x80 + (mono >> 8);

				*pBuffer++ = (unsigned char)mono;
			}
			break;

		case SAAP_MONO | SAAP_16BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();

				mono = (Amp[0]->TickAndOutputMono() +
					Amp[1]->TickAndOutputMono() +
					Amp[2]->TickAndOutputMono() +
					Amp[3]->TickAndOutputMono() +
					Amp[4]->TickAndOutputMono() +
					Amp[5]->TickAndOutputMono());

				// force output into the range 0<=x<=65535
				// (strictly, the following gives us 0<=x<=63360)
				mono *= 5;

				*pBuffer++ = mono & 0x00ff;
				*pBuffer++ = mono >> 8;
			}
			break;

		case SAAP_STEREO | SAAP_8BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();

				Amp[0]->TickAndOutputStereo(left, right);
				Amp[1]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[2]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[3]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[4]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[5]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;

				// force output into the range 0<=x<=255
				left *= 10;
				right *= 10;

				*pBuffer++ = 0x80 + (left >> 8);
				*pBuffer++ = 0x80 + (right >> 8);
			}
			break;

		case SAAP_STEREO | SAAP_16BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();

				Amp[0]->TickAndOutputStereo(left, right);
				Amp[1]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[2]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[3]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[4]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[5]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;

				// force output into the range 0<=x<=65535
				// (strictly, the following gives us 0<=x<=63360)
				left *= 10;
				right *= 10;

				*pBuffer++ = left & 0x00ff;
				*pBuffer++ = left >> 8;
				*pBuffer++ = right & 0x00ff;
				*pBuffer++ = right >> 8;
			}
			break;
		}
	}
	else if ((m_uParam & SAAP_MASK_FILTER_OVERSAMPLE) == SAAP_FILTER_OVERSAMPLE2x)
	{
		bool highpass = (m_uParam & SAAP_MASK_FILTER_HIGHPASS);

		// FILTER : (high-quality mode + oversample filter + optional
		// highpass filter (not yet implemented) to remove very low frequency/DC)
		// For oversampling, tick everything n times and take an unweighted mean

		switch (m_uParam & (SAAP_MASK_CHANNELS + SAAP_MASK_BITDEPTH))
		{
		case SAAP_MONO | SAAP_8BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();
				mono1 = (Amp[0]->TickAndOutputMono() +
					Amp[1]->TickAndOutputMono() +
					Amp[2]->TickAndOutputMono() +
					Amp[3]->TickAndOutputMono() +
					Amp[4]->TickAndOutputMono() +
					Amp[5]->TickAndOutputMono());

				Noise[0]->Tick();
				Noise[1]->Tick();
				mono2 = (Amp[0]->TickAndOutputMono() +
					Amp[1]->TickAndOutputMono() +
					Amp[2]->TickAndOutputMono() +
					Amp[3]->TickAndOutputMono() +
					Amp[4]->TickAndOutputMono() +
					Amp[5]->TickAndOutputMono());

				// force output into the range 0<=x<=255
				mono = ((mono1 + mono2) * 5) >> 1;
				mono = 0x80 + (mono >> 8);

				if (highpass)
				{
					mono = (prev_mono + mono) >> 1;
					prev_mono = mono;
				}
				*pBuffer++ = (unsigned char)mono;
			}
			break;

		case SAAP_MONO | SAAP_16BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();
				mono1 = (Amp[0]->TickAndOutputMono() +
					Amp[1]->TickAndOutputMono() +
					Amp[2]->TickAndOutputMono() +
					Amp[3]->TickAndOutputMono() +
					Amp[4]->TickAndOutputMono() +
					Amp[5]->TickAndOutputMono());

				Noise[0]->Tick();
				Noise[1]->Tick();
				mono2 = (Amp[0]->TickAndOutputMono() +
					Amp[1]->TickAndOutputMono() +
					Amp[2]->TickAndOutputMono() +
					Amp[3]->TickAndOutputMono() +
					Amp[4]->TickAndOutputMono() +
					Amp[5]->TickAndOutputMono());

				// force output into the range 0<=x<=65535
				// (strictly, the following gives us 0<=x<=63360)
				mono1 *= 5;
				mono2 *= 5;
				mono = (mono1 + mono2) >> 1;

				if (highpass)
				{
					mono = (prev_mono + mono) >> 1;
					prev_mono = mono;
				}

				*pBuffer++ = (unsigned char)(mono & 0x00ff);
				*pBuffer++ = (unsigned char)(mono >> 8);
			}
			break;

		case SAAP_STEREO | SAAP_8BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();

				Amp[0]->TickAndOutputStereo(left, right);
				Amp[1]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[2]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[3]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[4]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[5]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;

				Noise[0]->Tick();
				Noise[1]->Tick();

				Amp[0]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[1]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[2]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[3]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[4]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[5]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;

				// force output into the range 0<=x<=255
				left = (left * 10) >> 1;
				right = (right * 10) >> 1;

				if (highpass)
				{
					left = (left + prev_left) >> 1;
					right = (right + prev_right) >> 1;
					prev_left = left;
					prev_right = right;
				}

				*pBuffer++ = 0x80 + (left >> 8);
				*pBuffer++ = 0x80 + (left >> 8);
			}
			break;

		case SAAP_STEREO | SAAP_16BIT:
			while (nSamples--)
			{
				Noise[0]->Tick();
				Noise[1]->Tick();

				Amp[0]->TickAndOutputStereo(left, right);
				Amp[1]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[2]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[3]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[4]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[5]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;

				Noise[0]->Tick();
				Noise[1]->Tick();

				Amp[0]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[1]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[2]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[3]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[4]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;
				Amp[5]->TickAndOutputStereo(temp_left, temp_right);
				left += temp_left; right += temp_right;

				// force output into the range 0<=x<=65535
				// (strictly, the following gives us 0<=x<=63360)
				left = (left * 10) >> 1;
				right = (right * 10) >> 1;

				if (highpass)
				{
					left = (left + prev_left) >> 1;
					right = (right + prev_right) >> 1;
					prev_left = left;
					prev_right = right;
				}

				*pBuffer++ = left & 0x00ff;
				*pBuffer++ = left >> 8;
				*pBuffer++ = right & 0x00ff;
				*pBuffer++ = right >> 8;
			}
			break;
		}
	}
	else if ((m_uParam & SAAP_MASK_FILTER_OVERSAMPLE) == SAAP_FILTER_OVERSAMPLE64x)
	{
		bool highpass = (m_uParam & SAAP_MASK_FILTER_HIGHPASS);

		// FILTER : (high-quality mode + oversample filter +
		// optional highpass filter (not yet implemented) to remove very low frequency/DC)
		// For oversampling, tick everything n times and take an unweighted mean

		switch (m_uParam & (SAAP_MASK_CHANNELS + SAAP_MASK_BITDEPTH))
		{
		case SAAP_MONO | SAAP_8BIT:
			while (nSamples--)
			{
				unsigned long chans[6] = { 0 };
				for (int i = 0; i < 1<<m_nOversample; i++)
				{
					Noise[0]->Tick();
					Noise[1]->Tick();
					for (int c = 0; c < 6; c++)
					{
						chans[c] += Amp[c]->TickAndOutputMono();
					}
				}

				mono = 0;
				for (int c = 0; c < 6; c++)
				{
					mono += (unsigned short)((((chans[c] * 5)+(1<<(m_nOversample-1))) >> m_nOversample));
				}

				// force output into the range 0<=x<=255
				mono = 0x80 + (mono >> 8);

				if (highpass)
				{
					mono = (prev_mono + mono) >> 1;
					prev_mono = mono;
				}

				*pBuffer++ = (unsigned char)mono;
				}
			break;

		case SAAP_MONO | SAAP_16BIT:
			while (nSamples--)
			{
				unsigned long chans[6] = { 0 };
				for (int i = 0; i < 1<<m_nOversample; i++)
				{
					Noise[0]->Tick();
					Noise[1]->Tick();
					for (int c = 0; c < 6; c++)
					{
						chans[c] += Amp[c]->TickAndOutputMono();
					}
				}

				mono = 0;
				for (int c = 0; c < 6; c++)
				{
					mono += (unsigned short)((((chans[c] * 5)+(1<<(m_nOversample-1))) >> m_nOversample));
				}

				// force output into the range 0<=x<=255
				mono = 0x80 + (mono >> 8);

				if (highpass)
				{
					mono = (prev_mono + mono) >> 1;
					prev_mono = mono;
				}

				*pBuffer++ = (unsigned char)(mono & 0x00ff);
				*pBuffer++ = (unsigned char)(mono >> 8);
			}
			break;

		case SAAP_STEREO | SAAP_8BIT:
			while (nSamples--)
			{
				unsigned short lefts[6] = { 0 };
				unsigned short rights[6] = { 0 };
				for (int i = 0; i < 1<<m_nOversample; i++)
				{
					Noise[0]->Tick();
					Noise[1]->Tick();
					for (int c = 0; c < 6; c++)
					{
//						chans[c].dword += (Amp[c]->TickAndOutputStereo()).dword;
						Amp[c]->TickAndOutputStereo(temp_left, temp_right);
						lefts[c] += temp_left;
						rights[c] += temp_right;
					}
				}

				// force output into the range 0<=x<=255
				left = right = 0;
				for (int c = 0; c < 6; c++)
				{
					left += (((lefts[c] * 10)+(1<<(m_nOversample-1))) >> m_nOversample);
					right += (((rights[c] * 10)+(1<<(m_nOversample-1))) >> m_nOversample);
				}

				if (highpass)
				{
					left = (left + prev_left) >> 1;
					right = (right + prev_right) >> 1;
					prev_left = left;
					prev_right = right;
				}

				*pBuffer++ = 0x80 + (left >> 8);
				*pBuffer++ = 0x80 + (right >> 8);
			}
			break;

		case SAAP_STEREO | SAAP_16BIT:
			// for now, this is the only case that has full support and tested
			// accumulate and mix at the same time
			// Of course, this means we can't separately filter and generate
			// per-channel outputs (see notes at end of file)
			while (nSamples--)
			{
				double f_left = 0.0, f_right = 0.0;
				for (int i = 0; i < 1<<m_nOversample; i++)
				{
					Noise[0]->Tick();
					Noise[1]->Tick();
					for (int c = 0; c < 6; c++)
					{
						Amp[c]->TickAndOutputStereo(temp_left, temp_right);
						f_left += (double)temp_left;
						f_right += (double)temp_right;
					}
				}

				f_left /= (double)(1 << m_nOversample);
				f_right /= (double)(1 << m_nOversample);

				// scale output into good range
				f_left *= 10;
				f_right *= 10;

				if (highpass)
				{
					/* cutoff = 5 Hz (say) 
						const double b1 = exp(-2.0 * M_PI * (Fc/Fs))
						const double a0 = 1.0 - b1;
					*/
					const double b1 = 0.99928787;
					const double a0 = 1.0 - b1;

					filterout_z1_left = f_left * a0 + filterout_z1_left * b1;
					filterout_z1_right = f_right * a0 + filterout_z1_right * b1;
					f_left -= filterout_z1_left;
					f_right -= filterout_z1_right;
				}

				left = (unsigned short)f_left; // hm this should be signed
				right = (unsigned short)f_right; // hm this should be signed

				*pBuffer++ = left & 0x00ff;
				*pBuffer++ = left >> 8; // if this is signed, won't this do the wrong thing?
				*pBuffer++ = right & 0x00ff;
				*pBuffer++ = right >> 8;
			}
			break;
		default: // ie - the m_uParam contains modes not implemented yet
			{
	#ifdef DEBUGSAA
				char error[256];
				sprintf(error, "not implemented: uParam=%#L.64x\n", m_uParam);
	#ifdef WIN32
				OutputDebugStringA(error);
	#else
				fprintf(stderr, error);
	#endif
	#endif
			}
		}
	}

#ifdef DEBUGSAA
	fwrite(pBufferStart, GetCurrentBytesPerSample(), nTotalSamples, pcmfile);
	m_nDebugSample += nTotalSamples;
#endif

	prev_output_mono = prev_mono;
	prev_output_left = prev_left;
	prev_output_right = prev_right;
}

///////////////////////////////////////////////////////

LPCSAASOUND SAAAPI CreateCSAASound(void)
{
	return (new CSAASoundInternal);
}

void SAAAPI DestroyCSAASound(LPCSAASOUND object)
{
	delete (object);
}


/* thoughts on lowpass filtering as part of oversampling.
I tried this and really it didn't seem to make a lot of (audible) difference.

// lowpass oversample filter adds complexity and not particularly audibly better than simple averaging.
// use_lowpass_oversample_filter_average_output adds an additional averaging step to the output of the oversample
// filter.  this seems critical, because without this, the raw output of the lowpass filter is full of aliases
// If use_lowpass_oversample_filter is False, then the _average_output flag is ignored.
// Default, use_lowpass_oversample_filter is False, it sounds just fine really.

//#define USE_LOWPASS_OVERSAMPLE_FILTER
#undef USE_LOWPASS_OVERSAMPLE_FILTER
//#define USE_LOWPASS_OVERSAMPLE_FILTER_AVERAGE_OUTPUT
#undef USE_LOWPASS_OVERSAMPLE_FILTER_AVERAGE_OUTPUT

#ifdef USE_LOWPASS_OVERSAMPLE_FILTER
static double oversample_lp_filterout_z1_left_stages[10] = { 0,0,0,0,0,0,0,0,0,0 };
static double oversample_lp_filterout_z1_right_stages[10] = { 0,0,0,0,0,0,0,0,0,0 };
double averaged_filterout_left = 0.0, averaged_filterout_right = 0.0;
const int nStages = 10;
for (int i = 0; i < 1 << m_nOversample; i++)
{
	Noise[0]->Tick();
	Noise[1]->Tick();
	f_left = f_right = 0;
	for (int c = 0; c < 6; c++)
	{
		Amp[c]->TickAndOutputStereo(temp_left, temp_right);
		f_left += (double)temp_left;
		f_right += (double)temp_right;
	}
	// apply lowpass here.
	// HACK: ASSUME m_nOversample is 64 (I was experimenting only using the 64x oversample anyway)
	// therefore Fs = 44100*64
	// let's set Fc = 10kHz
	// so Fc/Fs = 0.00354308390022675736961451247166
	// const double b1 = exp(-2.0 * M_PI * (Fc/Fs))
	// const double a0 = 1.0 - b1;
	// const double b1 = 0.9779841137335348363722276130195;
	const double b1 = 0.977;
	const double a0 = 1.0 - b1;

	oversample_lp_filterout_z1_left_stages[0] = f_left * a0 + oversample_lp_filterout_z1_left_stages[0] * b1;
	for (int stage = 1; stage < nStages; stage++)
		oversample_lp_filterout_z1_left_stages[stage] = oversample_lp_filterout_z1_left_stages[stage - 1] * a0 + oversample_lp_filterout_z1_left_stages[stage] * b1;
	oversample_lp_filterout_z1_right_stages[0] = f_right * a0 + oversample_lp_filterout_z1_right_stages[0] * b1;
	for (int stage = 1; stage < nStages; stage++)
		oversample_lp_filterout_z1_right_stages[stage] = oversample_lp_filterout_z1_right_stages[stage - 1] * a0 + oversample_lp_filterout_z1_right_stages[stage] * b1;

#ifdef USE_LOWPASS_OVERSAMPLE_FILTER_AVERAGE_OUTPUT
	averaged_filterout_left += oversample_lp_filterout_4z1_left;
	averaged_filterout_right += oversample_lp_filterout_4z1_right;
#endif
}

// by the end of this loop we will have computed the oversample lowpass filter m_nOversample times
// and yielded exactly ONE sample output.
#ifdef USE_LOWPASS_OVERSAMPLE_FILTER_AVERAGE_OUTPUT
f_left = averaged_filterout_left / (1 << m_nOversample);
f_right = averaged_filterout_right / (1 << m_nOversample);
#else
f_left = oversample_lp_filterout_z1_left_stages[nStages - 1];
f_right = oversample_lp_filterout_z1_right_stages[nStages - 1];
#endif

#else
	// do the simple 1/N averaging which is easier and sounds good enough

#endif

*/

/* notes about multiple channel output (not implemented)
* 
* I'm thinking I should just implement a Render stage to handle final mixing
* but I want to handle the following two cases:
* 
* 1.  If only mixed final stereo result is required, only need to perform filtering
*     on the mixed output
* 2.  If separate channel outputs are required, each needs to have filtering applied
*     (but then, mixing is trivial)
* 
* I want to optimise for 1, since that's what most people will actually be doing (i.e.
* listening to the output, rather than generating separate channels as separate outputs
* and doing their own analysis/mixing)
* 
* If I needed to do filtering for multiple outputs, I might do it like this:


				// if separate channel outputs:
				double f_left[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
				double f_right[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
				for (int i = 0; i < 1<<m_nOversample; i++)
				{
					Noise[0]->Tick();
					Noise[1]->Tick();
					for (int c = 0; c < 6; c++)
					{
						Amp[c]->TickAndOutputStereo(temp_left, temp_right);
						f_left[c] += (double)temp_left;
						f_right[c] += (double)temp_right;
					}
				}
				// ...
				// now downsample each of f_left[i], f_right[i]

*/


///////////////////////////////////////////////////////
