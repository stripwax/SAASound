// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAASoundImplementation.cpp: implementation of the CSAASound class.
// the bones of the 'virtual SAA-1099' emulation
//
// the actual sound generation is carried out in the other classes;
// this class provides the output stage and the external interface only
//
// Version 3.01.0 (10 Jan 2001)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
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

#ifdef DEBUGSAA
FILE * dbgfile = NULL;
FILE * pcmfile = NULL;
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
m_uParamRate(0)
{
	#ifdef DEBUGSAA
	dbgfile = fopen("debugsaa.txt","wt");
	pcmfile = fopen("debugsaa.pcm","wb");
	#endif

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
	
#ifdef DEBUGSAA
	if (dbgfile) fclose(dbgfile);
#endif
}


//////////////////////////////////////////////////////////////////////
// CSAASound members
//////////////////////////////////////////////////////////////////////

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
			m_bSync = true;
		}
		else 
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
	fprintf(dbgfile,"%02d:",nReg);
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
	// can't remember if this is actually supported by the real
	// SAA-1099 hardware - but hey, sometimes it's useful, right?
	return(m_nCurrentSaaReg);
}


void CSAASoundInternal::SetSoundParameters(SAAPARAM uParam)
{
	int sampleratemode = 0;

	switch (uParam & SAAP_MASK_FILTER)
	{
	case SAAP_NOFILTER: // disable filter
		m_uParam = (m_uParam & ~SAAP_MASK_FILTER) | SAAP_NOFILTER;
		break;
	case SAAP_FILTER: // enable filter
		m_uParam = (m_uParam & ~SAAP_MASK_FILTER) | SAAP_FILTER;
		break;
	case 0:// change nothing!
	default:
		break;
	}

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

	/* Enabling the filter automatically puts the oscillators and
	 * noise generators into an ultra-high-resolution mode of 88.2kHz */
	if ( (m_uParam & SAAP_MASK_FILTER) == SAAP_FILTER)
	{
		sampleratemode = -1;
	}


	Osc[0]->SetSampleRateMode(sampleratemode);
	Osc[1]->SetSampleRateMode(sampleratemode);
	Osc[2]->SetSampleRateMode(sampleratemode);
	Osc[3]->SetSampleRateMode(sampleratemode);
	Osc[4]->SetSampleRateMode(sampleratemode);
	Osc[5]->SetSampleRateMode(sampleratemode);
	Noise[0]->SetSampleRateMode(sampleratemode);
	Noise[1]->SetSampleRateMode(sampleratemode);

		
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
	unsigned short mono;
	stereolevel stereoval;

#ifdef DEBUGSAA
	BYTE * pBufferStart = pBuffer;
	unsigned long nTotalSamples = nSamples;
#endif

	switch(m_uParam)
	{
	case SAAP_NOFILTER | SAAP_MONO | SAAP_8BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();

			mono = (Amp[0]->TickAndOutputMono() +
				  Amp[1]->TickAndOutputMono() +
				  Amp[2]->TickAndOutputMono() +
				  Amp[3]->TickAndOutputMono() +
				  Amp[4]->TickAndOutputMono() +
				  Amp[5]->TickAndOutputMono() );

			// force output into the range 0<=x<=255
			mono *= 5;
			*pBuffer++ = 0x80+(mono>>8);
		}
		break;
	
	case SAAP_NOFILTER | SAAP_MONO | SAAP_16BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();
			
			mono = (Amp[0]->TickAndOutputMono() +
				  Amp[1]->TickAndOutputMono() +
				  Amp[2]->TickAndOutputMono() +
				  Amp[3]->TickAndOutputMono() +
				  Amp[4]->TickAndOutputMono() +
				  Amp[5]->TickAndOutputMono() );

			// force output into the range 0<=x<=65535
			// (strictly, the following gives us 0<=x<=63360)
			mono *= 5;
			*pBuffer++ = mono & 0x00ff;
			*pBuffer++ = mono >> 8;
		}
		break;
	
	case SAAP_NOFILTER | SAAP_STEREO | SAAP_8BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();

			
			stereoval.dword=(Amp[0]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[1]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[2]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[3]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[4]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[5]->TickAndOutputStereo()).dword;

			// force output into the range 0<=x<=255
			stereoval.sep.Left *= 10;
			stereoval.sep.Right *= 10;
			*pBuffer++ = 0x80+((stereoval.sep.Left)>>8);
			*pBuffer++ = 0x80+((stereoval.sep.Right)>>8);
		}
		break;
			
	
	case SAAP_NOFILTER | SAAP_STEREO | SAAP_16BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();

			stereoval.dword=(Amp[0]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[1]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[2]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[3]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[4]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[5]->TickAndOutputStereo()).dword;

			// force output into the range 0<=x<=65535
			// (strictly, the following gives us 0<=x<=63360)
			stereoval.sep.Left *= 10;
			stereoval.sep.Right *= 10;
			*pBuffer++ = stereoval.sep.Left & 0x00ff;
			*pBuffer++ = stereoval.sep.Left >> 8;
			*pBuffer++ = stereoval.sep.Right & 0x00ff;
			*pBuffer++ = stereoval.sep.Right >> 8;
		}
		break;
	

	// FILTER : (high-quality mode + bandpass filter)
	case SAAP_FILTER | SAAP_MONO | SAAP_8BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();

			mono = (Amp[0]->TickAndOutputMono() +
				  Amp[1]->TickAndOutputMono() +
				  Amp[2]->TickAndOutputMono() +
				  Amp[3]->TickAndOutputMono() +
				  Amp[4]->TickAndOutputMono() +
				  Amp[5]->TickAndOutputMono() );

			// force output into the range 0<=x<=255
			mono *= 5;
			*pBuffer++ = 0x80+(mono>>8);
		}
		break;
	
	case SAAP_FILTER | SAAP_MONO | SAAP_16BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();
			
			mono = (Amp[0]->TickAndOutputMono() +
				  Amp[1]->TickAndOutputMono() +
				  Amp[2]->TickAndOutputMono() +
				  Amp[3]->TickAndOutputMono() +
				  Amp[4]->TickAndOutputMono() +
				  Amp[5]->TickAndOutputMono() );


			// force output into the range 0<=x<=65535
			// (strictly, the following gives us 0<=x<=63360)
			mono *= 5;
			*pBuffer++ = mono & 0x00ff;
			*pBuffer++ = mono >> 8;
		}
		break;
	
	case SAAP_FILTER | SAAP_STEREO | SAAP_8BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();

			
			stereoval.dword=(Amp[0]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[1]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[2]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[3]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[4]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[5]->TickAndOutputStereo()).dword;

			// force output into the range 0<=x<=255
			stereoval.sep.Left *= 10;
			stereoval.sep.Right *= 10;
			*pBuffer++ = 0x80+(stereoval.sep.Left>>8);
			*pBuffer++ = 0x80+(stereoval.sep.Right>>8);
		}
		break;
			
	
	case SAAP_FILTER | SAAP_STEREO | SAAP_16BIT:
		while (nSamples--)
		{
			Noise[0]->Tick();
			Noise[1]->Tick();

			stereoval.dword=(Amp[0]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[1]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[2]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[3]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[4]->TickAndOutputStereo()).dword;
			stereoval.dword+=(Amp[5]->TickAndOutputStereo()).dword;

			// force output into the range 0<=x<=65535
			// (strictly, the following gives us 0<=x<=63360)
			stereoval.sep.Left *= 10;
			stereoval.sep.Right *= 10;
			*pBuffer++ = stereoval.sep.Left & 0x00ff;
			*pBuffer++ = stereoval.sep.Left >> 8;
			*pBuffer++ = stereoval.sep.Right & 0x00ff;
			*pBuffer++ = stereoval.sep.Right >> 8;
		}
		break;
	

	default: // ie - the m_uParam contains modes not implemented yet
		{
#ifdef DEBUGSAA
		char error[256];
		sprintf(error,"not implemented: uParam=%#L.8x\n",m_uParam);
		OutputDebugString(error);
#endif
		}
	}

#ifdef DEBUGSAA
	fwrite(pBufferStart, GetCurrentBytesPerSample(), nTotalSamples, pcmfile);
#endif
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
	// removed from library - does nothing
}



unsigned long CSAASound::Generate(void)
{
	// obsolete - DON'T BOTHER USING THIS NOW THAT I'VE OBSOLETED IT!
	return 0;
}

///////////////////////////////////////////////////////

LPCSAASOUND EXTAPI CreateCSAASound(void)
{
	return (new CSAASoundInternal);
}

void EXTAPI DestroyCSAASound(LPCSAASOUND object)
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
