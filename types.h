// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// handy typedefs
//
// Version 3.00.0 (23 March 2000)
// (c) 1998-2000 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

typedef union
{
	struct {
		unsigned short left;
		unsigned short right;
	} sep;
	unsigned long dword;
} stereolevel;

typedef struct
{
	int nNumberOfPhases;
	bool bLooping;
	int nLevels[2][2][16]; // [Resolution][Phase][Withinphase]
} ENVDATA;

#ifndef BYTE
#define BYTE unsigned char
#endif

#endif