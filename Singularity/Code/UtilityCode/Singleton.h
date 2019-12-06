//	----==== SINGLETON.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Credit:			Thanks to Greg Snook, "Real-Time 3D Terrain Engines Using C++ and
//					DirectX 9" for supplying a description and code for this class
//
//	Description:	The Singleton is a type of class of which only one instance can exist.
//					It is ideal for system resource managers, texture managers, or any
//					class meant for global access. Along with guaranteeing only one
//					instance, it provides global access to all other objects through
//					static member functions. A class need only inherit from the Singleton
//					class and construct the Singleton in its constructor with (*this)
//					passed as the argument.
//----------------------------------------------------------------------------------------



#ifndef SINGLETON_H
#define SINGLETON_H

// Disable the warning regarding 'this' pointers being used in 
// base member initializer list. Singletons rely on this action
#pragma warning(disable : 4355)

#include "msgassert.h"

/*------------------
---- STRUCTURES ----
------------------*/

template <class T>
class Singleton {
	private:
		
		///// Attributes
		static T *	inst;

		//--------------------------------------------------------------------------------
		// Make copy constructor and equality operator private since they should not exist
		// for this type
		//--------------------------------------------------------------------------------
		Singleton(const Singleton & s);
		Singleton& operator=(const Singleton & s);

	public:

		///// Accessors
		static T & instance()
		{
			msgAssert(inst,"Instance of singleton not available");

			return (*inst);
		}

		///// Constructors / Destructor

		//--------------------------------------------------------------------------------
		//	A Singleton object must be created with a reference to the controlled object
		//--------------------------------------------------------------------------------
		explicit Singleton(T & object)
		{
			// quit if an object of this class is already instantiated
			msgAssert(!inst,"Singleton object already instantiated");

			inst = &object;
		}

		~Singleton()
		{
			msgAssert(inst,"Instance of singleton not available");

			inst = 0;
		}
};


/*----------------------
---- STATIC MEMBERS ----
----------------------*/

////////// class Singleton //////////

template <class T> T * Singleton<T>::inst = 0;

#endif