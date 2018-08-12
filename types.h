// Part of SAASound copyright 1998-2000 dave hooper <no-brain@mindless.com>
//
// handy typedefs
//
// Version 3.01.0 (10 Jan 2001)
// (c) 1998-2001 dave @ spc       <no-brain@mindless.com>
//
//////////////////////////////////////////////////////////////////////

#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

typedef union
{
	struct {
		unsigned short Left;
		unsigned short Right;
	} sep;
	unsigned long dword;
} stereolevel;

typedef struct
{
	int nNumberOfPhases;
	bool bLooping;
	unsigned short nLevels[2][2][16]; // [Resolution][Phase][Withinphase]
} ENVDATA;

#ifndef BYTE
#define BYTE unsigned char
#endif

#endif