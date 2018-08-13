// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// Thanks to this file (and associated header file) you can now
// use CSAASound from within a standard 'C' program
//
// Version 3.01.0 (10 Jan 2000)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#include "types.h"
#include "SAAEnv.h"
#include "SAANoise.h"
#include "SAAFreq.h"
#include "SAAAmp.h"
#include "SAASound.h"
#include "SAASoundImplementation.h"
#include "SAASndC.h"

SAASND EXTAPI newSAASND(void)
{
	return (SAASND)(new CSAASoundInternal());
}

void EXTAPI deleteSAASND(SAASND object)
{
	delete (LPCSAASOUND)(object);
}

void EXTAPI SAASNDSetSoundParameters(SAASND object, SAAPARAM uParam)
{
	((LPCSAASOUND)(object))->SetSoundParameters(uParam);
}

void EXTAPI SAASNDWriteAddress(SAASND object, BYTE nReg)
{
	((LPCSAASOUND)(object))->WriteAddress(nReg);
}

void EXTAPI SAASNDWriteData(SAASND object, BYTE nData)
{
	((LPCSAASOUND)(object))->WriteData(nData);
}

void EXTAPI SAASNDWriteAddressData(SAASND object, BYTE nReg, BYTE nData)
{
	((LPCSAASOUND)(object))->WriteAddressData(nReg, nData);
}

void EXTAPI SAASNDClear(SAASND object)
{
	((LPCSAASOUND)(object))->Clear();
}

BYTE EXTAPI SAASNDReadAddress(SAASND object)
{
	return ((LPCSAASOUND)(object))->ReadAddress();
}

SAAPARAM EXTAPI SAASNDGetCurrentSoundParameters(SAASND object)
{
	return ((LPCSAASOUND)(object))->GetCurrentSoundParameters();
}

unsigned short EXTAPI SAASNDGetCurrentBytesPerSample(SAASND object)
{
	return ((LPCSAASOUND)(object))->GetCurrentBytesPerSample();
}

unsigned short EXTAPI SAASNDGetBytesPerSample(SAAPARAM uParam)
{
	
	return CSAASound::GetBytesPerSample(uParam);
}

unsigned long EXTAPI SAASNDGetCurrentSampleRate(SAASND object)
{
	return ((LPCSAASOUND)(object))->GetCurrentSampleRate();
}

unsigned long EXTAPI SAASNDGetSampleRate(SAAPARAM uParam)
{
	return CSAASound::GetSampleRate(uParam);
}


void EXTAPI SAASNDGenerateMany(SAASND object, BYTE * pBuffer, unsigned long nSamples)
{
	((LPCSAASOUND)(object))->GenerateMany(pBuffer, nSamples);
}


int EXTAPI SAASNDSendCommand(SAASND object, SAACMD nCommandID, long nData)
{
	return ((LPCSAASOUND)(object))->SendCommand(nCommandID, nData);
}

void EXTAPI SAASNDClickClick(SAASND object, bool bValue)
{
	// removed from library - does nothing
}



unsigned long EXTAPI SAASNDGenerate(void)
{
	// obsolete - DON'T BOTHER USING THIS NOW THAT I'VE OBSOLETED IT!
	return 0;
}
