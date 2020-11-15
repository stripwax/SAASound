// Part of SAASound copyright 2020 Dave Hooper <dave@beermex.com>
//
// SAAConfig.cpp: configuration file handler class
//
//////////////////////////////////////////////////////////////////////

#include "defns.h"
#ifdef USE_CONFIG_FILE

#include "SAAConfig.h"
#define INI_READONLY
#define INI_ANSIONLY
#include "minIni/minIni.h"
#include <codecvt>

SAAConfig::SAAConfig()
:
m_bHasReadConfig(false),
m_bGenerateRegisterLogs(false),
m_bGeneratePcmLogs(false),
m_minIni(_T(CONFIG_FILE_PATH))
{
}

void SAAConfig::ReadConfig()
{
	// Assume (i.e. require) that the config file is always in UTF-8 .
	// These days, I think that's a good assumption to want to make.
	// It's also easy for people to create UTF-8 configs.
	// Define a helper to let us read from UTF-8 and convert to system locale
	// across platforms (and assume this will be a no-op on *nix)

#ifdef WIN32
	// Only support UNICODE WIN32
	// convert config file contents from utf8 to wchar_t
	// minIni has been compiled to use plain char for file read-write operations
	// (which supports UTF8) and wchar_t for filenames (which supports unicode filenames
	// on win32)
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
#define wrapped_gets(_mstring, _section, _element, _default) do{ \
		std::string _temp_u8 = m_minIni.gets(u8"" _section "", u8"" _element "", u8"" _default ""); \
		_mstring = converter.from_bytes(_temp_u8.c_str()); \
	} \
	while(0)
#else
	// For *nix I think I'm supposed to convert the text from the
	// file encoding (which I'm assuming is utf8) to system/user locale (if different)
	// which I think that requires converting first to wchar_t and then to current locale
	// For now I'm just going to assume you'using UTF8, and I will do no conversion.
	// minIni has been compiled to use plain char for everything,
	// which is really just a utf8 passthrough assumption.

#define wrapped_gets(_mstring, _section, _element, _default) \
	_mstring = m_minIni.gets(u8"" _section "", u8"" _element "", u8"" _default "");
#endif

#define _u8ify(x) ( u8"" x "" )
	m_bGenerateRegisterLogs = m_minIni.getbool(u8"Debug", u8"WriteRegisterLog", false);
	if (m_bGenerateRegisterLogs)
	{
		wrapped_gets(m_strRegisterLogPath, "Debug", "RegisterLogPath", DEBUG_SAA_REGISTER_LOG);
	}

	m_bGeneratePcmLogs = m_minIni.getbool(u8"Debug", u8"WritePCMOutput", false);
	if (m_bGeneratePcmLogs)
	{
		wrapped_gets(m_strPcmOutputPath, "Debug", "PCMOutputPath", DEBUG_SAA_PCM_LOG);
	}
}

#endif // USE_CONFIG_FILE