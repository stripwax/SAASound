// Part of SAASound copyright 2020 Dave Hooper <dave@beermex.com>
//
// defns.h: compile-time configuration parameters
//
//////////////////////////////////////////////////////////////////////

#ifndef DEFNS_H_INCLUDED
#define DEFNS_H_INCLUDED

// initial default SAA1099 crystal clock rate in HZ (can be changed subsequently by calling SetClockRate)
#define EXTERNAL_CLK_HZ 8000000

// define SAAFREQ_FIXED_CLOCKRATE if the above external clock rate is the only supported clock rate
// i.e. only support a single compile-time clock rate (=> this also prevents using the SetClockRate method)
#undef SAAFREQ_FIXED_CLOCKRATE
// #define SAAFREQ_FIXED_CLOCKRATE

// initial default sample rate (audio samplerate)
#define SAMPLE_RATE_HZ 44100

// Whether to support a startup configuration file that is parsed at load time
// #undef USE_CONFIG_FILE
#define USE_CONFIG_FILE

// and if so, what is its location
#ifdef USE_CONFIG_FILE
#define CONFIG_FILE_PATH ""
#endif // USE_CONFIG_FILE

#endif //  DEFNS_H_INCLUDED
