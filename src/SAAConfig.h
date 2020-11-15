// Part of SAASound copyright 2020 Dave Hooper <dave@beermex.com>
//
// SAAConfig.h: configuration file handler class
//
//////////////////////////////////////////////////////////////////////

#include "defns.h"
#ifdef USE_CONFIG_FILE

#ifndef SAA_CONFIG_H_INCLUDED
#define SAA_CONFIG_H_INCLUDED

#define INI_READONLY
#define INI_ANSIONLY  /*nb not really 'ANSI', this just forces all read/write to use 8-bit char*/
#include "minIni/minIni.h"

class SAAConfig
{
private:
	minIni m_minIni;
	bool m_bHasReadConfig;

public:
	bool m_bGenerateRegisterLogs;
	bool m_bGeneratePcmLogs;
	std::wstring m_strRegisterLogPath;
	std::wstring m_strPcmOutputPath;

	SAAConfig();
	void ReadConfig();
};

#endif  // SAA_CONFIG_H_INCLUDED

#endif // USE_CONFIG_FILE