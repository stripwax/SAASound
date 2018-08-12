// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAASoundImplementation.cpp: implementation of the CSAASound class.
// the bones of the 'virtual SAA-1099' emulation
//
// the actual sound generation is carried out in the other classes;
// this class provides the output stage and the external interface only
//
// As part of the output stage, a one-bit 'Click Click' sound can be generated
// by using the ClickClick(bool) function. This is not part of the SAA-1099
// specification (or indeed part of the SAA-1099 emulation algorithm) but is
// handy if (for example) the SAA-1099 emulation were to be used in conjunction
// with a 1-bit speaker emulator (an 8-bit micro such as the Sam Coupe, for 
// instance, uses both an SAA-1099 soundchip and a 1-bit speaker driver)
//
// Combining one-bit clickclick output into this class means postmixing of
// the sound buffer is not required to 'mix in' this very simple additional
// sound output.
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#include "assert.h"
#include "windows.h"
#include "types.h"
#include "SAAEnv.h"
#include "SAANoise.h"
#include "SAAFreq.h"
#include "SAAAmp.h"
#include "SAASound.h"
#include "SAASoundImplementation.h"
#include "stdio.h" // for sprintf

//////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
FILE * dbgfile = fopen("debugsaa.txt","wt");
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSAASoundInternal::CSAASoundInternal()
:
m_bOutputEnabled(false),
m_bSync(false),
m_nCurrentSaaReg(0),
m_uParam(0),
m_uParamRate(0),
m_bClickClick(0),
m_bClickClicktimes256(0)
{
	// Create and link up the objects that make up the emulator
	Noise[0] = new CSAANoise(0x14af5209); // Create and seed a noise generator
	Noise[1] = new CSAANoise(0x76a9b11e); // Create and seed a noise generator
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
	// noise generators and envelope controlloers
	Amp[0] = new CSAAAmp(Osc[0], Noise[0], NULL),
	Amp[1] = new CSAAAmp(Osc[1], Noise[0], NULL),
	Amp[2] = new CSAAAmp(Osc[2], Noise[0], Env[0]),
	Amp[3] = new CSAAAmp(Osc[3], Noise[1], NULL),
	Amp[4] = new CSAAAmp(Osc[4], Noise[1], NULL),
	Amp[5] = new CSAAAmp(Osc[5], Noise[1], Env[1]);
	for (i=5; i>=0; i--)
	{
		assert (Amp[i] != NULL);
	}


	// set parameters
	SetSoundParameters(SAAP_NOFILTER | SAAP_11025 | SAAP_8BIT | SAAP_MONO);
	// reset the virtual SAA
	Clear();
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
}


//////////////////////////////////////////////////////////////////////
// CSAASound members
//////////////////////////////////////////////////////////////////////

void CSAASoundInternal::Clear(void)
{
	// reinitialises virtual SAA:
	// sets reg 28 to 0x02;
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

#ifdef _DEBUG
	fprintf(dbgfile, "%02d:%02x\n",m_nCurrentSaaReg,nData);
#endif

	// route nData to the appropriate place
	switch (m_nCurrentSaaReg)
	{
	case 0:
		// Amplitude data (==> Amp)
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

	case 8:
		// Freq data (==> Osc)
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

	case 16:
		// Freq octave data (==> Osc) for channels 0,1
		Osc[0]->SetFreqOctave(nData & 0x07);
		Osc[1]->SetFreqOctave((nData >> 4) & 0x07);
		break;

	case 17:
		// Freq octave data (==> Osc) for channels 2,3
		Osc[2]->SetFreqOctave(nData & 0x07);
		Osc[3]->SetFreqOctave((nData >> 4) & 0x07);
		break;

	case 18:
		// Freq octave data (==> Osc) for channels 4,5
		Osc[4]->SetFreqOctave(nData & 0x07);
		Osc[5]->SetFreqOctave((nData >> 4) & 0x07);
		break;

	case 20:
		// Tone mixer control (==> Amp)
		Amp[0]->SetToneMixer(nData & 0x01);
		Amp[1]->SetToneMixer(nData & 0x02);
		Amp[2]->SetToneMixer(nData & 0x04);
		Amp[3]->SetToneMixer(nData & 0x08);
		Amp[4]->SetToneMixer(nData & 0x10);
		Amp[5]->SetToneMixer(nData & 0x20);
		break;

	case 21:
		// Noise mixer control (==> Amp)
		Amp[0]->SetNoiseMixer(nData & 0x01);
		Amp[1]->SetNoiseMixer(nData & 0x02);
		Amp[2]->SetNoiseMixer(nData & 0x04);
		Amp[3]->SetNoiseMixer(nData & 0x08);
		Amp[4]->SetNoiseMixer(nData & 0x10);
		Amp[5]->SetNoiseMixer(nData & 0x20);
		break;

	case 22:
		// Noise frequency/source control (==> Noise)
		Noise[0]->SetSource(nData & 0x03);
		Noise[1]->SetSource((nData >> 4) & 0x03);
		break;

	case 24:
		// Envelope control data (==> Env) for envelope controller #0
		Env[0]->SetEnvControl(nData);
		break;

	case 25:
		// Envelope control data (==> Env) for envelope controller #1
		Env[1]->SetEnvControl(nData);
		break;

	case 28:
		if (!m_bSync && (nData & 0x02))
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
			m_bSync = true;
		}
		else if (m_bSync && !(nData & 0x02))
		{
			// Unsync all devices
			Osc[0]->Sync(false);
			Osc[1]->Sync(false);
			Osc[2]->Sync(false);
			Osc[3]->Sync(false);
			Osc[4]->Sync(false);
			Osc[5]->Sync(false);
			Noise[0]->Sync(false);
			Noise[1]->Sync(false);
			m_bSync = false;
		}

		if (!m_bOutputEnabled && (nData & 0x01))
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
		else if (m_bOutputEnabled && !(nData & 0x01))
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
#ifdef _DEBUG
	fprintf(dbgfile,"%02d:",nReg);
#endif
	m_nCurrentSaaReg = nReg & 31;
	if (m_nCurrentSaaReg==24)
	{
		Env[0]->ExternalClock();
#ifdef _DEBUG
		fprintf(dbgfile, "<!ENVO!>");
#endif
	}
	else if (m_nCurrentSaaReg==25)
	{
		Env[1]->ExternalClock();
#ifdef _DEBUG
		fprintf(dbgfile, "<!ENV1!>");
#endif
	}
#ifdef _DEBUG
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
	// can't remember if this is actually supported by the real
	// SAA-1099 hardware - but hey, sometimes it's useful, right?
	return(m_nCurrentSaaReg);
}


void CSAASoundInternal::SetSoundParameters(SAAPARAM uParam)
{
	switch (uParam & SAAP_MASK_FILTER)
	{
	case SAAP_NOFILTER: // disable filter
		m_uParam = (m_uParam & ~SAAP_MASK_FILTER) | SAAP_NOFILTER;
		break;
	case SAAP_FILTER: // enable filter
//		m_uParam = (m_uParam & ~SAAP_MASK_FILTER) | SAAP_FILTER;
		//FILTERING IS CURRENTLY TOTALLY DISABLED
		m_uParam = (m_uParam & ~SAAP_MASK_FILTER) | SAAP_NOFILTER;
		break;
	case 0:// change nothing!
	default:
		break;
	}

	switch (uParam & SAAP_MASK_SAMPLERATE)
	{
	case SAAP_44100:
		Osc[0]->SetSampleRateMode(0);
		Osc[1]->SetSampleRateMode(0);
		Osc[2]->SetSampleRateMode(0);
		Osc[3]->SetSampleRateMode(0);
		Osc[4]->SetSampleRateMode(0);
		Osc[5]->SetSampleRateMode(0);
		Noise[0]->SetSampleRateMode(0);
		Noise[1]->SetSampleRateMode(0);
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_44100;
		break;
	case SAAP_22050:
		Osc[0]->SetSampleRateMode(1);
		Osc[1]->SetSampleRateMode(1);
		Osc[2]->SetSampleRateMode(1);
		Osc[3]->SetSampleRateMode(1);
		Osc[4]->SetSampleRateMode(1);
		Osc[5]->SetSampleRateMode(1);
		Noise[0]->SetSampleRateMode(1);
		Noise[1]->SetSampleRateMode(1);
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_22050;
		break;
	case SAAP_11025:
		Osc[0]->SetSampleRateMode(2);
		Osc[1]->SetSampleRateMode(2);
		Osc[2]->SetSampleRateMode(2);
		Osc[3]->SetSampleRateMode(2);
		Osc[4]->SetSampleRateMode(2);
		Osc[5]->SetSampleRateMode(2);
		Noise[0]->SetSampleRateMode(2);
		Noise[1]->SetSampleRateMode(2);
		m_uParamRate = (m_uParamRate & ~SAAP_MASK_SAMPLERATE) | SAAP_11025;
		break;
	case 0:// change nothing!
	default:
		break;
	}

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
	switch(uParam & (SAAP_MASK_CHANNELS | SAAP_MASK_BITDEPTH))
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
	switch(uParam & SAAP_MASK_SAMPLERATE)
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

void CSAASoundInternal::GenerateMany(BYTE * pBuffer, unsigned long nSamples)
{
	unsigned short o1,o2;

	switch(m_uParam)
	{
	case SAAP_NOFILTER | SAAP_MONO | SAAP_8BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();
			Amp[0]->Tick();
			Amp[1]->Tick();
			Amp[2]->Tick();
			Amp[3]->Tick();
			Amp[4]->Tick();
			Amp[5]->Tick();

			// force output into the range 0<=x<=255
			// (strictly, the following gives us 0<=x<=247)
			o1 = 	Amp[0]->MonoOutput() +
					Amp[1]->MonoOutput() +
					Amp[2]->MonoOutput() +
					Amp[3]->MonoOutput() +
					Amp[4]->MonoOutput() +
					Amp[5]->MonoOutput() ;
			*pBuffer++ = 0x80+((o1 + (o1>>2) + (o1>>3))>>6)-(m_bClickClick);
		}
		break;
	
	case SAAP_NOFILTER | SAAP_MONO | SAAP_16BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();
			Amp[0]->Tick();
			Amp[1]->Tick();
			Amp[2]->Tick();
			Amp[3]->Tick();
			Amp[4]->Tick();
			Amp[5]->Tick();

			o1 = 	Amp[0]->MonoOutput() +
					Amp[1]->MonoOutput() +
					Amp[2]->MonoOutput() +
					Amp[3]->MonoOutput() +
					Amp[4]->MonoOutput() +
					Amp[5]->MonoOutput() ;
			// force output into the range 0<=x<=65535
			// (strictly, the following gives us 0<=x<=63360)
			o1 = ((o1>>1)+(o1)+(o1<<2))-(m_bClickClicktimes256);
			*pBuffer++ = (o1 & 0x00ff);
			*pBuffer++ = (o1 >> 8);
		}
		break;
	
	case SAAP_NOFILTER | SAAP_STEREO | SAAP_8BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();
			Amp[0]->Tick();
			Amp[1]->Tick();
			Amp[2]->Tick();
			Amp[3]->Tick();
			Amp[4]->Tick();
			Amp[5]->Tick();

			o1 =	Amp[0]->LeftOutput() +
					Amp[1]->LeftOutput() +
					Amp[2]->LeftOutput() +
					Amp[3]->LeftOutput() +
					Amp[4]->LeftOutput() +
					Amp[5]->LeftOutput() ;
			o2 = 	Amp[0]->RightOutput() +
					Amp[1]->RightOutput() +
					Amp[2]->RightOutput() +
					Amp[3]->RightOutput() +
					Amp[4]->RightOutput() +
					Amp[5]->RightOutput() ;
			// force output into the range 0<=x<=255
			// (strictly, the following gives us 0<=x<=247)
			*pBuffer++ = 0x80+((o1 + (o1>>2) + (o1>>3)) >> 5)-(m_bClickClick);
			*pBuffer++ = 0x80+((o2 + (o2>>2) + (o2>>3)) >> 5)-(m_bClickClick);
		}
		break;
			
	
	case SAAP_NOFILTER | SAAP_STEREO | SAAP_16BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();
			Amp[0]->Tick();
			Amp[1]->Tick();
			Amp[2]->Tick();
			Amp[3]->Tick();
			Amp[4]->Tick();
			Amp[5]->Tick();

			o1 =	Amp[0]->LeftOutput() +
					Amp[1]->LeftOutput() +
					Amp[2]->LeftOutput() +
					Amp[3]->LeftOutput() +
					Amp[4]->LeftOutput() +
					Amp[5]->LeftOutput() ;
			o2 = 	Amp[0]->RightOutput() +
					Amp[1]->RightOutput() +
					Amp[2]->RightOutput() +
					Amp[3]->RightOutput() +
					Amp[4]->RightOutput() +
					Amp[5]->RightOutput() ;
			// force output into the range 0<=x<=65535
			// (strictly, the following gives us 0<=x<=63360)
			o1 = ((o1)+(o1<<1)+(o1<<3))-(m_bClickClicktimes256);
			o2 = ((o2)+(o2<<1)+(o2<<3))-(m_bClickClicktimes256);
			*pBuffer++ = (o1 & 0x00ff);
			*pBuffer++ = (o1 >> 8);
			*pBuffer++ = (o2 & 0x00ff);
			*pBuffer++ = (o2 >> 8);
		}
		break;
	
	

	// FILTER (lowpass):
	
	case SAAP_FILTER | SAAP_MONO | SAAP_8BIT:
	case SAAP_FILTER | SAAP_MONO | SAAP_16BIT:
	case SAAP_FILTER | SAAP_STEREO | SAAP_8BIT:
	case SAAP_FILTER | SAAP_STEREO | SAAP_16BIT:
	default: // ie - the m_uParam contains modes not implemented yet
		{
#ifdef _DEBUG
		char error[256];
		sprintf(error,"not implemented: uParam=%#L.8x\n",m_uParam);
		OutputDebugString(error);
#endif
		}
	}
}




int CSAASoundInternal::SendCommand(SAACMD nCommandID, long nData)
{
	/********************/
	/* to be completed! */
	/********************/
	switch (nCommandID)
	{
	case SAACMD_SetSampleRate: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_GetSampleRate: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_SetVolumeBoost: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_GetVolumeBoost: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_SetFilterMode: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_GetFilterMode: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_SetBitDepth: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_GetBitDepth: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_SetNumChannels: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	case SAACMD_GetNumChannels: return SAASENDCOMMAND_FEATURE_NOT_YET_IMPLEMENTED;
	
	default: return SAASENDCOMMAND_UNKNOWN_INVALID_COMMAND;
	}
}


void CSAASoundInternal::ClickClick(int bValue)
{
	if (bValue)
	{
		m_bClickClick = CLICKCLICKLEVEL;
		m_bClickClicktimes256 = CLICKCLICKLEVELTIMES256;
	}
	else
	{
		m_bClickClick = 0;
		m_bClickClicktimes256 = 0;
	}
}



unsigned long CSAASound::Generate(void)
{
	// obsolete - DON'T BOTHER USING THIS NOW THAT I'VE OBSOLETED IT!
	return 0;
}

///////////////////////////////////////////////////////

LPCSAASOUND __stdcall CreateCSAASound(void)
{
	return (new CSAASoundInternal);
}

void __stdcall DestroyCSAASound(LPCSAASOUND object)
{
	delete (object);
}

///////////////////////////////////////////////////////



CSAASound::CSAASound()
{
	// Nothing. I know for a fact the only CSAASound objects that can be created
	// are CSAASoundInternal objects and therefore handled by the constructor for
	// CSAASoundInternal objects. There is no base-level object initialisation.
}

CSAASound::~CSAASound()
{
	// Nothing
}
