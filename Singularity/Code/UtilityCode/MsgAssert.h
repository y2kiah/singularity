//	----==== MSGASSERT.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		2
//	Date:			6/06
//	Description:	A macro to replace the standard assert macro
//----------------------------------------------------------------------------------------

#ifndef MSGASSERT_H
#define MSGASSERT_H

#ifdef _DEBUG

#include <cassert>
#define msgAssert(a,b)	assert(a && b)

#else

#define msgAssert(a,b)	((void)0)

#endif
/*
void _msgAssert(bool condition, const char *msg);

#ifndef _DEBUG

#define msgAssert(_expression, msg)     ((void)0)

#else

#define msgAssert(_expression, msg)		_msgAssert(_expression, msg)

#endif // _DEBUG*/

#endif // MSGASSERT_H