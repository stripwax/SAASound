// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// Thanks to this file (and associated header file) you can now
// use CSAASound from within a standard 'C' program
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
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

SAASND newSAASND(void)
{
	return (SAASND)(new CSAASoundInternal());
}

void deleteSAASND(SAASND object)
{
	delete (LPCSAASOUND)(object);
}

void SAASNDSetSoundParameters(SAASND object, SAAPARAM uParam)
{
	((LPCSAASOUND)(object))->SetSoundParameters(uParam);
}

void SAASNDWriteAddress(SAASND object, BYTE nReg)
{
	((LPCSAASOUND)(object))->WriteAddress(nReg);
}

void SAASNDWriteData(SAASND object, BYTE nData)
{
	((LPCSAASOUND)(object))->WriteData(nData);
}

void SAASNDWriteAddressData(SAASND object, BYTE nReg, BYTE nData)
{
	((LPCSAASOUND)(object))->WriteAddressData(nReg, nData);
}

void SAASNDClear(SAASND object)
{
	((LPCSAASOUND)(object))->Clear();
}

BYTE SAASNDReadAddress(SAASND object)
{
	return ((LPCSAASOUND)(object))->ReadAddress();
}

SAAPARAM SAASNDGetCurrentSoundParameters(SAASND object)
{
	return ((LPCSAASOUND)(object))->GetCurrentSoundParameters();
}

unsigned short SAASNDGetCurrentBytesPerSample(SAASND object)
{
	return ((LPCSAASOUND)(object))->GetCurrentBytesPerSample();
}

unsigned short SAASNDGetBytesPerSample(SAAPARAM uParam)
{
	
	return CSAASound::GetBytesPerSample(uParam);
}

unsigned long SAASNDGetCurrentSampleRate(SAASND object)
{
	return ((LPCSAASOUND)(object))->GetCurrentSampleRate();
}

unsigned long SAASNDGetSampleRate(SAAPARAM uParam)
{
	return CSAASound::GetSampleRate(uParam);
}


void SAASNDGenerateMany(SAASND object, BYTE * pBuffer, unsigned long nSamples)
{
	((LPCSAASOUND)(object))->GenerateMany(pBuffer, nSamples);
}


int SAASNDSendCommand(SAASND object, SAACMD nCommandID, long nData)
{
	return ((LPCSAASOUND)(object))->SendCommand(nCommandID, nData);
}

void SAASNDClickClick(SAASND object, bool bValue)
{
	((LPCSAASOUND)(object))->ClickClick(bValue);
}



unsigned long SAASNDGenerate(void)
{
	// obsolete - DON'T BOTHER USING THIS NOW THAT I'VE OBSOLETED IT!
	return 0;
}
