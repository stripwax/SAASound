// Part of SAASound copyright 2020 Dave Hooper <dave@beermex.com>
//
// defns.h: compile-time configuration parameters
//
//////////////////////////////////////////////////////////////////////

#ifndef DEFNS_H_INCLUDED
#define DEFNS_H_INCLUDED

// initial default SAA1099 crystal clock rate in HZ (can be changed subsequently by calling SetClockRate)
#define EXTERNAL_CLK_HZ 8000000

// initial default sample rate (audio samplerate)
#define SAMPLE_RATE_HZ 44100

// Whether to support a startup configuration file that is parsed at load time
#define USE_CONFIG_FILE false

// and if so, what is its location
#ifdef USE_CONFIG_FILE
#define CONFIG_FILE_PATH ""
#endif // USE_CONFIG_FILE


#endif //  DEFNS_H_INCLUDED
