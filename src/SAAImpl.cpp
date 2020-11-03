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
#include "SAAImpl.h"
#include "defns.h"


//////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////

// TODO: Support DEBUGSAA via runtime flag / config file
#ifdef DEBUGSAA
#include <ios>
#include <iostream>
#include <fstream>
std::ofstream dbgfile, pcmfile;
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAASoundInternal::CSAASoundInternal()
	:
m_nClockRate(EXTERNAL_CLK_HZ),
m_bHighpass(false),
m_nSampleRate(SAMPLE_RATE_HZ),
m_nOversample(DEFAULT_OVERSAMPLE),
m_uParam(0),
m_uParamRate(0),
m_chip()
{
	#ifdef DEBUGSAA
	dbgfile.open(DEBUG_SAA_REGISTER_LOG, std::ios_base::out);
	pcmfile.open(DEBUG_SAA_PCM_LOG, std::ios_base::out|std::ios_base::binary);
	m_nDebugSample = 0;
	#endif
	// set parameters
	// TODO support defaults and overrides from config file
	// m_chip.SetSoundParameters(SAAP_FILTER | SAAP_11025 | SAAP_8BIT | SAAP_MONO);
	// reset the virtual SAA
	// m_chip.Clear();

	m_chip._SetClockRate(m_nClockRate);
	m_chip._SetOversample(m_nOversample);
}

CSAASoundInternal::~CSAASoundInternal()
{
#ifdef DEBUGSAA
	dbgfile.close();
	pcmfile.close();
#endif
}

//////////////////////////////////////////////////////////////////////
// CSAASound members
//////////////////////////////////////////////////////////////////////

void CSAASoundInternal::SetClockRate(unsigned int nClockRate)
{
	m_nClockRate = nClockRate;
	m_chip._SetClockRate(m_nClockRate);
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
	m_chip._WriteData(nData);
#ifdef DEBUGSAA
	dbgfile << m_nDebugSample << " " << (int)m_chip._ReadAddress() << ":" << (int)nData << std::endl;
#endif
}

void CSAASoundInternal::WriteAddress(BYTE nReg)
{
	// originated from an OUT 511,r call
	m_chip._WriteAddress(nReg);
#ifdef DEBUGSAA
	dbgfile << m_nDebugSample << " " << (int)nReg << ":";
	if (nReg==24)
	{
		dbgfile << "<!ENVO!>";
	}
	else if (nReg==25)
	{
		dbgfile << "<!ENV1!>";
	}
	dbgfile << std::endl;
#endif
}

void CSAASoundInternal::WriteAddressData(BYTE nReg, BYTE nData)
{
	// performs WriteAddress(nReg) followed by WriteData(nData)
	m_chip._WriteAddress(nReg);
	m_chip._WriteData(nData);
}

#ifdef DEBUG
BYTE CSAASoundInternal::ReadAddress(void)
{
	// Not a real hardware function of the SAA-1099, which is write-only
	return(m_chip._ReadAddress());
}
#else
BYTE CSAASoundInternal::ReadAddress(void)
{
	// Not a real hardware function of the SAA-1099, which is write-only
	return(0);
}
#endif

void CSAASoundInternal::SetSoundParameters(SAAPARAM uParam)
{
	// set samplerate properties from uParam (deprecated but still supported)
	unsigned int nSampleRate = m_nSampleRate;
	switch (uParam & SAAP_MASK_SAMPLERATE)
	{
	case SAAP_44100:
		nSampleRate = 44100;
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_44100;
		break;
	case SAAP_22050:
		nSampleRate = 22050;
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_22050;
		break;
	case SAAP_11025:
		nSampleRate = 11025;
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_11025;
		break;
	case 0:// change nothing!
	default:
		break;
	}
	
	if (nSampleRate != m_nSampleRate)
	{
		m_nSampleRate = nSampleRate;
		m_chip._SetSampleRate(m_nSampleRate);
	}

	// set filter properties from uParam
	m_uParam = (m_uParam & ~SAAP_MASK_FILTER) | (uParam & SAAP_MASK_FILTER);
	 
	m_bHighpass=true;
}

void CSAASoundInternal::SetSampleRate(unsigned int nSampleRate)
{
	if (nSampleRate != m_nSampleRate)
	{
		m_nSampleRate = nSampleRate;
		m_chip._SetSampleRate(m_nSampleRate);
	}
}

void CSAASoundInternal::SetOversample(unsigned int nOversample)
{
	if (nOversample != m_nOversample)
	{
		m_nOversample = nOversample;
		m_chip._SetOversample(m_nOversample);
	}
}

SAAPARAM CSAASoundInternal::GetCurrentSoundParameters(void)
{
	return m_uParam | m_uParamRate;
}

unsigned short CSAASoundInternal::GetCurrentBytesPerSample(void)
{
	// 16 bit stereo => 4 bytes per sample
	return 4;
}

/*static*/ unsigned short CSAASound::GetBytesPerSample(SAAPARAM uParam)
{
	// 16 bit stereo => 4 bytes per sample
	switch (uParam & (SAAP_MASK_CHANNELS | SAAP_MASK_BITDEPTH))
	{
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
	unsigned int temp_left, temp_right;
	double f_left, f_right;
	static double filterout_z1_left = 0, filterout_z1_right = 0;
	signed short left, right; // output only - 16 bit result

#ifdef DEBUGSAA
	BYTE* pBufferStart = pBuffer;
	unsigned long nTotalSamples = nSamples;
#endif

	// for now, this is the only case that has full support and tested
	// accumulate and mix at the same time
	// Of course, this means we can't separately filter and generate
	// per-channel outputs (see notes at end of file)
	while (nSamples--)
	{
		m_chip._TickAndOutputStereo(temp_left, temp_right);
		f_left = (double)temp_left;
		f_right = (double)temp_right;
		f_left /= (double)(1 << m_nOversample);
		f_right /= (double)(1 << m_nOversample);

		// scale output into good range
		f_left *= 10;
		f_right *= 10;

		if (m_bHighpass)
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

		left = (signed short)f_left;
		right = (signed short)f_right;

		*pBuffer++ = left & 0x00ff;
		*pBuffer++ = (left >> 8) & 0x00ff;
		*pBuffer++ = right & 0x00ff;
		*pBuffer++ = (right >> 8) & 0x00ff;
	}

#ifdef DEBUGSAA
	pcmfile.write((const char *)pBufferStart, nTotalSamples * (unsigned long)GetCurrentBytesPerSample());
	m_nDebugSample += nTotalSamples;
#endif
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
