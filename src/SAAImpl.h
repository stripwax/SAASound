// Part of SAASound copyright 1998-2018 Dave Hooper <dave@beermex.com>
//
// This is the internal implementation (header file) of the SAASound object.
// This is done so that the external interface to the object always stays the same
// (SAASound.h) even though the internal object can change
// .. Meaning future releases don't require relinking everyone elses code against
//    the updated saasound stuff
//
//////////////////////////////////////////////////////////////////////

#ifndef SAAIMPL_H_INCLUDED
#define SAAIMPL_H_INCLUDED

#include "SAASound.h"
#include "SAADevice.h"

class CSAASoundInternal : public CSAASound
{
private:
	CSAADevice m_chip;
	int m_uParam, m_uParamRate;
	unsigned int m_nClockRate; 
	unsigned int m_nSampleRate;
	unsigned int m_nOversample;
	bool m_bHighpass;

public:
	CSAASoundInternal();
	~CSAASoundInternal();

	void SetClockRate(unsigned int nClockRate);
	void SetSoundParameters(SAAPARAM uParam);
	void WriteAddress(BYTE nReg);
	void WriteData(BYTE nData);
	void WriteAddressData(BYTE nReg, BYTE nData);
	BYTE ReadAddress(void);
	void Clear(void);

	SAAPARAM GetCurrentSoundParameters(void);
	unsigned long GetCurrentSampleRate(void);
	static unsigned long GetSampleRate(SAAPARAM uParam);
	unsigned short GetCurrentBytesPerSample(void);
	static unsigned short GetBytesPerSample(SAAPARAM uParam);

	void GenerateMany(BYTE * pBuffer, unsigned long nSamples);

};

#endif // SAAIMPL_H_INCLUDED
