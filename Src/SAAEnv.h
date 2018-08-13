//kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib 
// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// SAAEnv.h: interface for the CSAAEnv class.
//
// Applies an envelope from its two inputs, generating two outputs
//
// Version 3.01.0 (10 Jan 2001)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef SAAENV_H_INCLUDED
#define SAAENV_H_INCLUDED

class CSAAEnv  
{
private:
	unsigned short m_nLeftLevel, m_nRightLevel;
	ENVDATA const * m_pEnvData;

	bool m_bEnabled;
	bool m_bInvertRightChannel;
	char m_nPhase;
	char m_nPhasePosition;
	bool m_bEnvelopeEnded;
	char m_nPhaseAdd[2];
	char m_nCurrentPhaseAdd;
	bool m_bLooping;
	char m_nNumberOfPhases;
	char m_nResolution;
	char m_nInitialLevel;
	bool m_bNewData;
	BYTE m_nNextData;
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
	void SetEnvControl(int nData); // really just a BYTE
	unsigned short LeftLevel(void) const;
	unsigned short RightLevel(void) const;
	bool IsActive(void) const;

};

#endif	// SAAENV_H_INCLUDED
