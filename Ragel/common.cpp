/*
 *  Copyright 2006-2007 Adrian Thurston <thurston@complang.org>
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

#include "pcheck.h"
#include "common.h"
#include "stdlib.h"
#include <string.h>
#include <assert.h>

HostType hostTypesC[] =
{
	{ L"char",     0,       L"char",    true,   true,  false,  CHAR_MIN,  CHAR_MAX,   sizeof(char) },
	{ L"unsigned", L"char",  L"uchar",   false,  true,  false,  0,         UCHAR_MAX,  sizeof(unsigned char) },
	{ L"short",    0,       L"short",   true,   true,  false,  SHRT_MIN,  SHRT_MAX,   sizeof(short) },
	{ L"unsigned", L"short", L"ushort",  false,  true,  false,  0,         USHRT_MAX,  sizeof(unsigned short) },
	{ L"int",      0,       L"int",     true,   true,  false,  INT_MIN,   INT_MAX,    sizeof(int) },
	{ L"unsigned", L"int",   L"uint",    false,  true,  false,  0,         UINT_MAX,   sizeof(unsigned int) },
	{ L"long",     0,       L"long",    true,   true,  false,  LONG_MIN,  LONG_MAX,   sizeof(long) },
	{ L"unsigned", L"long",  L"ulong",   false,  true,  false,  0,         ULONG_MAX,  sizeof(unsigned long) }
};

#define S8BIT_MIN  -128
#define S8BIT_MAX  127

#define U8BIT_MIN  0
#define U8BIT_MAX  255

#define S16BIT_MIN -32768
#define S16BIT_MAX 32767

#define U16BIT_MIN 0
#define U16BIT_MAX 65535

#define S31BIT_MIN -1073741824L
#define S31BIT_MAX 1073741823L

#define S32BIT_MIN -2147483648LL
#define S32BIT_MAX 2147483647L

#define U32BIT_MIN 0
#define U32BIT_MAX 4294967295UL

#define S64BIT_MIN -9223372036854775807LL
#define S64BIT_MAX 9223372036854775807LL

#define U64BIT_MIN 0
#define U64BIT_MAX 18446744073709551615ULL

HostType hostTypesD[] =
{
	{ L"byte",    0,  L"byte",    true,   true,  false,  CHAR_MIN,  CHAR_MAX,    1 },
	{ L"ubyte",   0,  L"ubyte",   false,  true,  false,  0,         UCHAR_MAX,   1 },
	{ L"char",    0,  L"char",    false,  true,  false,  0,         UCHAR_MAX,   1 },
	{ L"short",   0,  L"short",   true,   true,  false,  SHRT_MIN,  SHRT_MAX,    2 },
	{ L"ushort",  0,  L"ushort",  false,  true,  false,  0,         USHRT_MAX,   2 },
	{ L"wchar_t",   0,  L"wchar_t",   false,  true,  false,  0,         USHRT_MAX,   2 },
	{ L"int",     0,  L"int",     true,   true,  false,  INT_MIN,   INT_MAX,     4 },
	{ L"uint",    0,  L"uint",    false,  true,  false,  0,         UINT_MAX,    4 },
	{ L"dchar",   0,  L"dchar",   false,  true,  false,  0,         UINT_MAX,    4 }
};

HostType hostTypesGo[] = 
{
	{ L"byte",    0,  L"uint8",   false,  true,  false,  U8BIT_MIN,  U8BIT_MAX,   1 },
	{ L"int8",    0,  L"int8",    true,   true,  false,  S8BIT_MIN,  S8BIT_MAX,   1 },
	{ L"uint8",   0,  L"uint8",   false,  true,  false,  U8BIT_MIN,  U8BIT_MAX,   1 },
	{ L"int16",   0,  L"int16",   true,   true,  false,  S16BIT_MIN, S16BIT_MAX,  2 },
	{ L"uint16",  0,  L"uint16",  false,  true,  false,  U16BIT_MIN, U16BIT_MAX,  2 },
	{ L"int32",   0,  L"int32",   true,   true,  false,  S32BIT_MIN, S32BIT_MAX,  4 },
	{ L"uint32",  0,  L"uint32",  false,  true,  false,  U32BIT_MIN, U32BIT_MAX,  4 },
	{ L"int64",   0,  L"int64",   true,   true,  false,  S64BIT_MIN, S64BIT_MAX,  8 },
	{ L"uint64",  0,  L"uint64",  false,  true,  false,  U64BIT_MIN, U64BIT_MAX,  8 },
	{ L"rune",    0,  L"int32",   true,   true,  true,   S32BIT_MIN, S32BIT_MAX,  4 }
};

HostType hostTypesJava[] = 
{
	{ L"byte",    0,  L"byte",   true,   true,  false,  CHAR_MIN,  CHAR_MAX,    1 },
	{ L"short",   0,  L"short",  true,   true,  false,  SHRT_MIN,  SHRT_MAX,    2 },
	{ L"char",    0,  L"char",   false,  true,  false,  0,         USHRT_MAX,   2 },
	{ L"int",     0,  L"int",    true,   true,  false,  INT_MIN,   INT_MAX,     4 },
};

/* What are the appropriate types for ruby? */
HostType hostTypesRuby[] = 
{
	{ L"char",    0,  L"char",   true,   true,  false,  CHAR_MIN,  CHAR_MAX,    1 },
	{ L"int",     0,  L"int",    true,   true,  false,  INT_MIN,   INT_MAX,     4 },
};

HostType hostTypesCSharp[] =
{
	{ L"sbyte",   0,  L"sbyte",   true,   true,  false,  CHAR_MIN,  CHAR_MAX,    1 },
	{ L"byte",    0,  L"byte",    false,  true,  false,  0,         UCHAR_MAX,   1 },
	{ L"short",   0,  L"short",   true,   true,  false,  SHRT_MIN,  SHRT_MAX,    2 },
	{ L"ushort",  0,  L"ushort",  false,  true,  false,  0,         USHRT_MAX,   2 },
	{ L"char",    0,  L"char",    false,  true,  true,   0,         USHRT_MAX,   2 },
	{ L"int",     0,  L"int",     true,   true,  false,  INT_MIN,   INT_MAX,     4 },
	{ L"uint",    0,  L"uint",    false,  true,  false,  0,         UINT_MAX,    4 },
	{ L"long",    0,  L"long",    true,   true,  false,  LONG_MIN,  LONG_MAX,    8 },
	{ L"ulong",   0,  L"ulong",   false,  true,  false,  0,         ULONG_MAX,   8 }
};

HostType hostTypesOCaml[] =
{
//	{ L"char",   0,  L"char",     false,  true,  false, 0,          UCHAR_MAX,  1 },
	{ L"int",    0,  L"int",      true,   true,  false, S31BIT_MIN, S31BIT_MAX, 4 },
};

HostLang hostLangC =      { HostLang::C,      hostTypesC,      8,  hostTypesC+0,       true };
HostLang hostLangD =      { HostLang::D,      hostTypesD,      9,  hostTypesD+2,       true };
HostLang hostLangD2 =     { HostLang::D2,     hostTypesD,      9,  hostTypesD+2,       true };
HostLang hostLangGo =     { HostLang::Go,     hostTypesGo,    10,  hostTypesGo+0,      false };
HostLang hostLangJava =   { HostLang::Java,   hostTypesJava,   4,  hostTypesJava+2,    false };
HostLang hostLangRuby =   { HostLang::Ruby,   hostTypesRuby,   2,  hostTypesRuby+0,    false };
HostLang hostLangCSharp = { HostLang::CSharp, hostTypesCSharp, 9,  hostTypesCSharp+4,  true };
HostLang hostLangOCaml =  { HostLang::OCaml,  hostTypesOCaml,  1,  hostTypesOCaml+0,   false };

HostLang *hostLang = &hostLangC;

HostType *findAlphType( const wchar_t *s1 )
{
	for ( int i = 0; i < hostLang->numHostTypes; i++ ) {
		if ( wcscmp( s1, hostLang->hostTypes[i].data1 ) == 0 && 
				hostLang->hostTypes[i].data2 == 0 )
		{
			return hostLang->hostTypes + i;
		}
	}

	return 0;
}

HostType *findAlphType( const wchar_t *s1, const wchar_t *s2 )
{
	for ( int i = 0; i < hostLang->numHostTypes; i++ ) {
		if ( wcscmp( s1, hostLang->hostTypes[i].data1 ) == 0 && 
				hostLang->hostTypes[i].data2 != 0 && 
				wcscmp( s2, hostLang->hostTypes[i].data2 ) == 0 )
		{
			return hostLang->hostTypes + i;
		}
	}

	return 0;
}

HostType *findAlphTypeInternal( const wchar_t *s1 )
{
	for ( int i = 0; i < hostLang->numHostTypes; i++ ) {
		if ( wcscmp( s1, hostLang->hostTypes[i].internalName ) == 0 )
			return hostLang->hostTypes + i;
	}

	return 0;
}

/* Construct a new parameter checker with for paramSpec. */
ParamCheck::ParamCheck( const wchar_t *paramSpec, int argc, const wchar_t **argv )
:
	state(noparam),
	argOffset(0),
	curArg(0),
	iCurArg(1),
	paramSpec(paramSpec), 
	argc(argc), 
	argv(argv)
{
}

/* Check a single option. Returns the index of the next parameter.  Sets p to
 * the arg character if valid, 0 otherwise.  Sets parg to the parameter arg if
 * there is one, NULL otherwise. */
bool ParamCheck::check()
{
	bool requiresParam;

	if ( iCurArg >= argc ) {            /* Off the end of the arg list. */
		state = noparam;
		return false;
	}

	if ( argOffset != 0 && *argOffset == 0 ) {
		/* We are at the end of an arg wstring. */
		iCurArg += 1;
		if ( iCurArg >= argc ) {
			state = noparam;
			return false;
		}
		argOffset = 0;
	}

	if ( argOffset == 0 ) {
		/* Set the current arg. */
		curArg = argv[iCurArg];

		/* We are at the beginning of an arg wstring. */
		if ( argv[iCurArg] == 0 ||        /* Argv[iCurArg] is null. */
			 argv[iCurArg][0] != L'-' ||   /* Not a param. */
			 argv[iCurArg][1] == 0 ) {    /* Only a dash. */
			parameter = 0;
			paramArg = 0;

			iCurArg += 1;
			state = noparam;
			return true;
		}
		argOffset = argv[iCurArg] + 1;
	}

	/* Get the arg char. */
	wchar_t argChar = *argOffset;
	
	/* Loop over all the parms and look for a match. */
	const wchar_t *pSpec = paramSpec;
	while ( *pSpec != 0 ) {
		wchar_t pSpecChar = *pSpec;

		/* If there is a L':' following the char then
		 * it requires a parm.  If a parm is required
		 * then move ahead two in the parmspec. Otherwise
		 * move ahead one in the parm spec. */
		if ( pSpec[1] == L':' ) {
			requiresParam = true;
			pSpec += 2;
		}
		else {
			requiresParam = false;
			pSpec += 1;
		}

		/* Do we have a match. */
		if ( argChar == pSpecChar ) {
			if ( requiresParam ) {
				if ( argOffset[1] == 0 ) {
					/* The param must follow. */
					if ( iCurArg + 1 == argc ) {
						/* We are the last arg so there
						 * cannot be a parameter to it. */
						parameter = argChar;
						paramArg = 0;
						iCurArg += 1;
						argOffset = 0;
						state = invalid;
						return true;
					}
					else {
						/* the parameter to the arg is the next arg. */
						parameter = pSpecChar;
						paramArg = argv[iCurArg + 1];
						iCurArg += 2;
						argOffset = 0;
						state = match;
						return true;
					}
				}
				else {
					/* The param for the arg is built in. */
					parameter = pSpecChar;
					paramArg = argOffset + 1;
					iCurArg += 1;
					argOffset = 0;
					state = match;
					return true;
				}
			}
			else {
				/* Good, we matched the parm and no
				 * arg is required. */
				parameter = pSpecChar;
				paramArg = 0;
				argOffset += 1;
				state = match;
				return true;
			}
		}
	}

	/* We did not find a match. Bad Argument. */
	parameter = argChar;
	paramArg = 0;
	argOffset += 1;
	state = invalid;
	return true;
}

/* Counts newlines before sending sync. */
int output_filter::sync( )
{
	line += 1;
	return std::wfilebuf::sync();
}

/* Counts newlines before sending data out to file. */
std::streamsize output_filter::xsputn( const wchar_t *s, std::streamsize n )
{
	for ( int i = 0; i < n; i++ ) {
		if ( s[i] == L'\n' )
			line += 1;
	}
	return std::wfilebuf::xsputn( s, n );
}

/* Scans a wstring looking for the file extension. If there is a file
 * extension then pointer returned points to inside the wstring
 * passed in. Otherwise returns null. */
const wchar_t *findFileExtension( const wchar_t *stemFile )
{
	const wchar_t *ppos = stemFile + wcslen(stemFile) - 1;

	/* Scan backwards from the end looking for the first dot.
	 * If we encounter a L'/' before the first dot, then stop the scan. */
	while ( 1 ) {
		/* If we found a dot or got to the beginning of the wstring then
		 * we are done. */
		if ( ppos == stemFile || *ppos == L'.' )
			break;

		/* If we hit a / then there is no extension. Done. */
		if ( *ppos == L'/' ) {
			ppos = stemFile;
			break;
		}
		ppos--;
	} 

	/* If we got to the front of the wstring then bail we 
	 * did not find an extension  */
	if ( ppos == stemFile )
		ppos = 0;

	return ppos;
}

/* Make a file name from a stem. Removes the old filename suffix and
 * replaces it with a new one. Returns a newed up wstring. */
const wchar_t *fileNameFromStem( const wchar_t *stemFile, const wchar_t *suffix )
{
	long len = wcslen( stemFile );
	assert( len > 0 );

	/* Get the extension. */
	const wchar_t *ppos = findFileExtension( stemFile );

	/* If an extension was found, then shorten what we think the len is. */
	if ( ppos != 0 )
		len = ppos - stemFile;

	/* Make the return wstring from the stem and the suffix. */
	wchar_t *retVal = new wchar_t[ len + wcslen( suffix ) + 1 ];
	wcsncpy_s( retVal, len + wcslen(suffix) + 1, stemFile, len );
	wcscpy_s( retVal + len, wcslen(suffix) + 1, suffix );

	return retVal;
}

exit_object endp;

void operator<<( std::wostream &out, exit_object & )
{
    out << std::endl;
    exit(1);
}
