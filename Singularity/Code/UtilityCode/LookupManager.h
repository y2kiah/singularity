//	----==== LOOKUPMANAGER.H ====----
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


#ifndef LOOKUPMANAGER_H
#define LOOKUPMANAGER_H

#include "singleton.h"
#include "msgassert.h"

/*---------------
---- DEFINES ----
---------------*/

#define lManager		LookupManager::instance()	// used to access the LookupManager instance globally
#define math			LookupManager::instance()


#define PI				3.14159265358979f
#define DEGTORAD		0.01745329252f		// convert degrees to radians
#define INFINITY		2147483648.0f		// highest value for a 32 bit signed float


/*------------------
---- STRUCTURES ----
------------------*/

class LookupManager : public Singleton<LookupManager>
{
	private:

		///// Attributes

		int		anglePrecision;		// stores precision of lookup tables - will be (1/anglePrecision) of a degree
		float	angleIncrement;		// = (1 / anglePrecision), or the decimal increment of the lookup precision
		float	*sinTable;
		float	*cosTable;
		float	*tanTable;

	public:

		// the following are provided for quick conversion to some common angles
		const int	ANGLE360, ANGLE315, ANGLE271, ANGLE270, ANGLE225, ANGLE180, ANGLE135,
					ANGLE90, ANGLE89, ANGLE60, ANGLE45, ANGLE40, ANGLE30, ANGLE20, ANGLE15,
					ANGLE10, ANGLE5, ANGLE2, ANGLE1;

		///// Methods

		// Accessors

		const float &	getSinTable(void) const { return *sinTable; }
		const float &	getCosTable(void) const { return *cosTable; }
		const float &	getTanTable(void) const { return *tanTable; }

		int				getAnglePrec(void) const { return anglePrecision; }
		float			getAngleInc(void) const { return angleIncrement; }

		__inline float	getSin(int lookup) const;
		__inline float	getCos(int lookup) const;
		__inline float	getTan(int lookup) const;

		// Note that these functions do not recognize negative angles or angles > 360
		__inline int	degToIndex(float degree) const;
		__inline int	degToIndex(int degree) const;
		__inline float	indexToDeg(int index) const;

		explicit LookupManager(unsigned int precision);
		~LookupManager();
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/

////////// class LookupManager //////////


__inline float LookupManager::getSin(int lookup) const
{
	msgAssert(lookup >= 0 && lookup < ANGLE360, "LookupMgr: sinTable out of bounds");

	return sinTable[lookup];
}


__inline float LookupManager::getCos(int lookup) const
{
	msgAssert(lookup >= 0 && lookup < ANGLE360, "LookupMgr: cosTable out of bounds");

	return cosTable[lookup];
}


__inline float LookupManager::getTan(int lookup) const
{
	msgAssert(lookup >= 0 && lookup < ANGLE360, "LookupMgr: tanTable out of bounds");

	return tanTable[lookup];
}


__inline int LookupManager::degToIndex(int degree) const
{
	msgAssert(degree < 360 && degree >= 0, "LookupMgr: degree out of bounds");
	return degree * anglePrecision;
}


__inline int LookupManager::degToIndex(float degree) const
{
	msgAssert(degree < 360.0f && degree >= 0, "LookupMgr: degree out of bounds");
	return (int)(degree * anglePrecision);
}


__inline float LookupManager::indexToDeg(int index) const
{
	msgAssert(index < ANGLE360 && index >= 0, "LookupMgr: index out of bounds");
	msgAssert(anglePrecision != 0.0f, "LookupMgr: anglePrecision = 0.0, divide by 0 case");
	return (float)index * angleIncrement;
}

#endif