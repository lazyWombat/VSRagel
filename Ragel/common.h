/*
 *  Copyright 2001-2006 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Ragel.
 *
 *  Ragel is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Ragel is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Ragel; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#ifndef _COMMON_H
#define _COMMON_H

#include <fstream>
#include <climits>
#include "dlist.h"

/* Location in an input file. */
struct InputLoc
{
	const wchar_t *fileName;
	long line;
	long col;
};


typedef unsigned long long Size;

struct Key
{
private:
	long key;

public:
	friend inline Key operator+(const Key key1, const Key key2);
	friend inline Key operator-(const Key key1, const Key key2);
	friend inline Key operator/(const Key key1, const Key key2);
	friend inline long operator&(const Key key1, const Key key2);

	friend inline bool operator<( const Key key1, const Key key2 );
	friend inline bool operator<=( const Key key1, const Key key2 );
	friend inline bool operator>( const Key key1, const Key key2 );
	friend inline bool operator>=( const Key key1, const Key key2 );
	friend inline bool operator==( const Key key1, const Key key2 );
	friend inline bool operator!=( const Key key1, const Key key2 );

	friend struct KeyOps;
	
	Key( ) {}
	Key( const Key &key ) : key(key.key) {}
	Key( long key ) : key(key) {}

	/* Returns the value used to represent the key. This value must be
	 * interpreted based on signedness. */
	long getVal() const { return key; };

	/* Returns the key casted to a long long. This form of the key does not
	 * require any signedness interpretation. */
	long long getLongLong() const;

	/* Returns the distance from the key value to the maximum value that the
	 * key implementation can hold. */
	Size availableSpace() const;

	bool isUpper() const { return ( L'A' <= key && key <= L'Z' ); }
	bool isLower() const { return ( L'a' <= key && key <= L'z' ); }
	bool isPrintable() const
	{
	    return ( 7 <= key && key <= 13 ) || ( 32 <= key && key < 127 );
	}

	Key toUpper() const
		{ return Key( L'A' + ( key - L'a' ) ); }
	Key toLower() const
		{ return Key( L'a' + ( key - L'A' ) ); }

	void operator+=( const Key other )
	{
		/* FIXME: must be made aware of isSigned. */
		key += other.key;
	}

	void operator-=( const Key other )
	{
		/* FIXME: must be made aware of isSigned. */
		key -= other.key;
	}

	void operator|=( const Key other )
	{
		/* FIXME: must be made aware of isSigned. */
		key |= other.key;
	}

	/* Decrement. Needed only for ranges. */
	inline void decrement();
	inline void increment();
};

struct HostType
{
	const wchar_t *data1;
	const wchar_t *data2;
	const wchar_t *internalName;
	bool isSigned;
	bool isOrd;
	bool isChar;
	long long minVal;
	long long maxVal;
	unsigned int size;
};

struct HostLang
{
	/* Target language. */
	enum Lang
	{
		C, D, D2, Go, Java, Ruby, CSharp, OCaml
	};

	Lang lang;
	HostType *hostTypes;
	int numHostTypes;
	HostType *defaultAlphType;
	bool explicitUnsigned;
};

extern HostLang *hostLang;

extern HostLang hostLangC;
extern HostLang hostLangD;
extern HostLang hostLangD2;
extern HostLang hostLangGo;
extern HostLang hostLangJava;
extern HostLang hostLangRuby;
extern HostLang hostLangCSharp;
extern HostLang hostLangOCaml;

HostType *findAlphType( const wchar_t *s1 );
HostType *findAlphType( const wchar_t *s1, const wchar_t *s2 );
HostType *findAlphTypeInternal( const wchar_t *s1 );

/* An abstraction of the key operators that manages key operations such as
 * comparison and increment according the signedness of the key. */
struct KeyOps
{
	/* Default to signed alphabet. */
	KeyOps() :
		isSigned(true),
		alphType(0)
	{}

	/* Default to signed alphabet. */
	KeyOps( bool isSigned ) 
		:isSigned(isSigned) {}

	bool isSigned;
	Key minKey, maxKey;
	HostType *alphType;

	void setAlphType( HostType *alphType )
	{
		this->alphType = alphType;
		isSigned = alphType->isSigned;
		if ( isSigned ) {
			minKey = (long) alphType->minVal;
			maxKey = (long) alphType->maxVal;
		}
		else {
			minKey = (long) (unsigned long) alphType->minVal; 
			maxKey = (long) (unsigned long) alphType->maxVal;
		}
	}

	/* Compute the distance between two keys. */
	Size span( Key key1, Key key2 )
	{
		return isSigned ? 
			(unsigned long long)(
				(long long)key2.key - 
				(long long)key1.key + 1) : 
			(unsigned long long)(
				(unsigned long)key2.key) - 
				(unsigned long long)((unsigned long)key1.key) + 1;
	}

	Size alphSize()
		{ return span( minKey, maxKey ); }

	HostType *typeSubsumes( long long maxVal )
	{
		for ( int i = 0; i < hostLang->numHostTypes; i++ ) {
			if ( maxVal <= hostLang->hostTypes[i].maxVal )
				return hostLang->hostTypes + i;
		}
		return 0;
	}

	HostType *typeSubsumes( bool isSigned, long long maxVal )
	{
		for ( int i = 0; i < hostLang->numHostTypes; i++ ) {
			if ( ( ( isSigned && hostLang->hostTypes[i].isSigned ) || !isSigned ) &&
					maxVal <= hostLang->hostTypes[i].maxVal )
				return hostLang->hostTypes + i;
		}
		return 0;
	}
};

extern KeyOps *keyOps;

inline bool operator<( const Key key1, const Key key2 )
{
	return keyOps->isSigned ? key1.key < key2.key : 
		(unsigned long)key1.key < (unsigned long)key2.key;
}

inline bool operator<=( const Key key1, const Key key2 )
{
	return keyOps->isSigned ?  key1.key <= key2.key : 
		(unsigned long)key1.key <= (unsigned long)key2.key;
}

inline bool operator>( const Key key1, const Key key2 )
{
	return keyOps->isSigned ? key1.key > key2.key : 
		(unsigned long)key1.key > (unsigned long)key2.key;
}

inline bool operator>=( const Key key1, const Key key2 )
{
	return keyOps->isSigned ? key1.key >= key2.key : 
		(unsigned long)key1.key >= (unsigned long)key2.key;
}

inline bool operator==( const Key key1, const Key key2 )
{
	return key1.key == key2.key;
}

inline bool operator!=( const Key key1, const Key key2 )
{
	return key1.key != key2.key;
}

/* Decrement. Needed only for ranges. */
inline void Key::decrement()
{
	key = keyOps->isSigned ? key - 1 : ((unsigned long)key)-1;
}

/* Increment. Needed only for ranges. */
inline void Key::increment()
{
	key = keyOps->isSigned ? key+1 : ((unsigned long)key)+1;
}

inline long long Key::getLongLong() const
{
	return keyOps->isSigned ? (long long)key : (long long)(unsigned long)key;
}

inline Size Key::availableSpace() const
{
	if ( keyOps->isSigned ) 
		return (long long)LONG_MAX - (long long)key;
	else
		return (unsigned long long)ULONG_MAX - (unsigned long long)(unsigned long)key;
}
	
inline Key operator+(const Key key1, const Key key2)
{
	/* FIXME: must be made aware of isSigned. */
	return Key( key1.key + key2.key );
}

inline Key operator-(const Key key1, const Key key2)
{
	/* FIXME: must be made aware of isSigned. */
	return Key( key1.key - key2.key );
}

inline long operator&(const Key key1, const Key key2)
{
	/* FIXME: must be made aware of isSigned. */
	return key1.key & key2.key;
}

inline Key operator/(const Key key1, const Key key2)
{
	/* FIXME: must be made aware of isSigned. */
	return key1.key / key2.key;
}

/* Filter on the output stream that keeps track of the number of lines
 * output. */
class output_filter : public std::wfilebuf
{
public:
	output_filter( const wchar_t *fileName ) : fileName(fileName), line(1) { }

	virtual int sync();
	virtual std::streamsize xsputn(const wchar_t* s, std::streamsize n);

	const wchar_t *fileName;
	int line;
};

class cfilebuf : public std::wstreambuf
{
public:
	cfilebuf( wchar_t *fileName, FILE* file ) : fileName(fileName), file(file) { }
	wchar_t *fileName;
	FILE *file;

	int sync()
	{
		fflush( file );
		return 0;
	}

	int overflow( int c )
	{
		if ( c != EOF )
			fputc( c, file );
		return 0;
	}

	std::streamsize xsputn( const wchar_t* s, std::streamsize n )
	{
		std::streamsize written = fwrite( s, 1, n, file );
		return written;
	}
};

class costream : public std::wostream
{
public:
	costream( cfilebuf *b ) : 
		std::wostream(b), b(b) {}
	
	~costream()
		{ delete b; }

	void fclose()
		{ ::fclose( b->file ); }

	cfilebuf *b;
};


const wchar_t *findFileExtension( const wchar_t *stemFile );
const wchar_t *fileNameFromStem( const wchar_t *stemFile, const wchar_t *suffix );

struct Export
{
	Export( const wchar_t *name, Key key )
		: name(name), key(key) {}

	const wchar_t *name;
	Key key;

	Export *prev, *next;
};

typedef DList<Export> ExportList;

struct exit_object { };
extern exit_object endp;
void operator<<( std::wostream &out, exit_object & );

#endif
