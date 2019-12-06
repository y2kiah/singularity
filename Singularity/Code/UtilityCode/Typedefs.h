//	----==== TYPEDEFS.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Description:	Defines variable types that are easier to read and more meaningful,
//					and constants for reference throughout the engine
//----------------------------------------------------------------------------------------



#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include "bitfield.h"

/*----------------
---- TYPEDEFS ----
----------------*/

typedef unsigned char		uchar;
typedef unsigned short		ushort;
typedef unsigned int		uint;
typedef unsigned long		ulong;

typedef unsigned long		uint32;

typedef __int64				int64;
typedef unsigned __int64	uint64;

typedef	BitField<uchar>		u8Flags;
typedef	BitField<ushort>	u16Flags;
typedef	BitField<uint>		u32Flags;


/*---------------
---- DEFINES ----
---------------*/

#define EPSILON				0.000001f	// the minimum distance used for equality tests in 3d equations

#endif