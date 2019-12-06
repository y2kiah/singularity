//	----==== BITFIELD.H ====----
//
//	Author:			Jeffrey Kiah
//					y2kiah@hotmail.com
//	Version:		1
//	Date:			1/04
//	Description:	Replaces the bool primitive in situations where storing many boolean
//					values is needed
//----------------------------------------------------------------------------------------



#ifndef BITFIELD_H
#define BITFIELD_H

/*------------------
---- STRUCTURES ----
------------------*/

//----------------------------------------------------------------------------------------
//	**class BitField**
//
//	The C++ bool type is inadequate for storage of flag data since it uses 32 bits of
//	space to contain 1 bit of data. All bits of this type can be used to store flag
//	information. The objects containing bit fields are responsible for defining and
//	maintaining the data that each bit represents.
//----------------------------------------------------------------------------------------
template <class T>
class BitField {
	private:
		T		bits;

	public:
		///// Operators
		operator T() const { return bits; }
		__inline BitField &	operator=(const BitField &bf);
		__inline BitField &	operator=(const T set);
		__inline bool		operator&(const T test);
		bool				operator==(const BitField &bf) const { return (bits == bf.bits); }
		bool				operator!=(const BitField &bf) const { return (bits != bf.bits); }
		
		///// Mutators
		void		set(T set) { bits = set; }
		void		clear() { bits = 0; }
		void		setBit(const int bit) { bits |= (1<<bit); }
		void		clearBit(const int bit) { bits &= ~(1<<bit); }
		__inline void setBit(const int bit, const bool setting = true);

		///// Accessors
		T			getBits() const { return bits; }
		bool		isEmpty() const { return (bits == 0); }
		__inline bool	testBit(const int bit) const;	// test a single bit
		__inline bool	testFlags(const T test) const;	// test if multiple bits are same
		__inline bool	testAny(const T test) const;	// test if any of the bits are true in both

		///// Info
		__inline int	totalBits() const;	// return number of bits for this type
		__inline int	totalSet() const;	// return number of bits set to true

		///// Constructors / Destructor
		explicit BitField() : bits(0) {}
		explicit BitField(const T set) : bits(set) {}
		explicit BitField(const BitField &bf) : bits(bf.bits) {}
		~BitField() {}
};


/*------------------------
---- INLINE FUNCTIONS ----
------------------------*/

////////// class BitField //////////

// Operators

template <class T>
__inline BitField<T> & BitField<T>::operator=(const BitField<T> &bf)
{
	bits = bf.bits;
	return (*this);
}


template <class T>
__inline BitField<T> & BitField<T>::operator=(const T set)
{
	bits = set;
	return (*this);
}

// Mutators

template <class T>
__inline void BitField<T>::setBit(const int bit, const bool setting)
{
	if (setting) {
		setBit(bit);
	} else {
		clearBit(bit);
	}
}

// Accessors

template <class T>
__inline bool BitField<T>::testBit(const int bit) const
{
	return(bits & (1<<bit));
}

template <class T>
__inline bool BitField<T>::testFlags(const T test) const
{
	return ((value & test) == test);
}

template <class T>
__inline bool BitField<T>::testAny(const T test) const
{
	return (value & test);
}

// Info

template <class T>
__inline int BitField<T>::totalBits() const
{
	return (sizeof(T)<<3);
}

template <class T>
__inline int BitField<T>::totalSet() const
{
	int count = 0;
	int total = totalBits();
	T TestValue = value;
	
	for (int i = total; i > 0; i--) {
		count += (TestValue & 1);
		TestValue >>= 1;
	}

	return count;
}

#endif