// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAEnv.h: interface for the CSAAEnv class.
//
// Applies an envelope from its two inputs, generating two outputs
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef SAAENV_H_INCLUDED
#define SAAENV_H_INCLUDED

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

class CSAAEnv  
{
private:
	stereolevel m_nLevel;
	bool m_bEnabled;
	bool m_bInvertRightChannel;
	int m_nPhase;
	int m_nPhasePosition;
	bool m_bEnvelopeEnded;
	int m_nWaveform;
	int m_nPhaseAdd[2];
	int m_nCurrentPhaseAdd;
	bool m_bLooping;
	int m_nNumberOfPhases;
	int m_nResolution;
	int m_nInitialLevel;
	int m_nTempE4;
	bool m_bNewData;
	int m_nNextData;
	bool m_bOkForNewData;
	bool m_bClockExternally;
	static const ENVDATA cs_EnvData[8];

	void Tick(void);
	void SetLevels(void);
	void SetNewEnvData(int nData);

public:
	CSAAEnv();
	~CSAAEnv();

	void InternalClock(void);
	void ExternalClock(void);
	void SetEnvControl(BYTE nData);
	int LeftLevel(void) const;
	int RightLevel(void) const;
	bool IsActive(void) const;

};


#endif	// SAAENV_H_INCLUDED
