//	----==== LOOKUPMANAGER.CPP ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		3
//	Date:			2/04
//	Description:	This class manages all lookup tables used globally. Because
//					lookup tables cannot use floating point index values, a program
//					using this class will have to adopt a new degree system,
//					dependant on the table precision. For instance, if precision is
//					set to 80, then the tables will be accurate to 1/80th of a
//					degree and there will be 28800 values in each lookup table. So
//					the program must use a 28800 degree system instead of a 360
//					degree system, where every 80 degrees in the new system is equal
//					to 1 degree in the original. There are functions provided for
//					converting intuitive values based on 360 degrees to the actual
//					values needed for the lookup tables.
//	--------------------------------------------------------------------------------


#include <cmath>
#include "lookupmanager.h"
#include "..\UTILITYCODE\msgassert.h"

////////// class LookupManager //////////


//-----------------------------------------------------------------------
//	There is no error correction available for the precision arguement
//	since all const values in the class are based on it so it is up to
//	the programmer to supply a legal value. Anything under 10 or over 100
//	should be considered out of the norm. In a debug build the program
//	asserts the value of precision.
//-----------------------------------------------------------------------
LookupManager::LookupManager(unsigned int precision) : Singleton<LookupManager>(*this),
	ANGLE360(precision*360),
	ANGLE315(precision*315),
	ANGLE271(precision*271),
	ANGLE270(precision*270),
	ANGLE225(precision*225),
	ANGLE180(precision*180),
	ANGLE135(precision*135),
	ANGLE90(precision*90),
	ANGLE89(precision*89),
	ANGLE60(precision*60),
	ANGLE45(precision*45),
	ANGLE40(precision*40),
	ANGLE30(precision*30),
	ANGLE20(precision*20),
	ANGLE15(precision*15),
	ANGLE10(precision*10),
	ANGLE5(precision*5),
	ANGLE2(precision*2),
	ANGLE1(precision)
{
	msgAssert(precision >= 1, "LookupManager precision must be >= 1");

	anglePrecision = precision;
	angleIncrement = 1.0f / (float)precision;

	// set up the lookup tables
	sinTable = new float[ANGLE360];
	cosTable = new float[ANGLE360];
	tanTable = new float[ANGLE360];

	float angleFix = angleIncrement * DEGTORAD;
	for (int a = 0; a < ANGLE360; a++) {

		sinTable[a] = sinf((float)a * angleFix);
		cosTable[a] = cosf((float)a * angleFix);

		if (a == ANGLE90 || a == ANGLE270) tanTable[a] = INFINITY;
		else tanTable[a] = tanf((float)a * angleFix);
	}
}


LookupManager::~LookupManager()
{
	delete [] sinTable;
	delete [] cosTable;
	delete [] tanTable;
}