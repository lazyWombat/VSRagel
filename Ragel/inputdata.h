/*
 *  Copyright 2008 Adrian Thurston <thurston@complang.org>
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

#ifndef _INPUT_DATA
#define _INPUT_DATA

#include "gendata.h"
#include <iostream>
#include <sstream>

struct Parser;
struct ParseData;

struct InputItem
{
	enum Type {
		HostData,
		Write,
	};

	Type type;
	std::wostringstream data;
	std::wstring name;
	ParseData *pd;
	Vector<wchar_t *> writeArgs;

	InputLoc loc;

	InputItem *prev, *next;
};

struct Parser;

typedef AvlMap<const wchar_t*, Parser*, CmpStr> ParserDict;
typedef AvlMapEl<const wchar_t*, Parser*> ParserDictEl;
typedef DList<Parser> ParserList;
typedef DList<InputItem> InputItemList;
typedef Vector<const wchar_t *> ArgsVector;

struct InputData
{
	InputData() : 
		inputFileName(0),
		outputFileName(0),
		inStream(0),
		outStream(0),
		outFilter(0),
		dotGenParser(0)
	{}

	/* The name of the root section, this does not change during an include. */
	const wchar_t *inputFileName;
	const wchar_t *outputFileName;

	/* Io globals. */
	std::wistream *inStream;
	std::wostream *outStream;
	output_filter *outFilter;

	Parser *dotGenParser;

	ParserDict parserDict;
	ParserList parserList;
	InputItemList inputItems;

	ArgsVector includePaths;

	void verifyWritesHaveData();

	void writeOutput();
	void makeOutputStream();
	void openOutput();
	void generateReduced();
	void prepareMachineGen();
	void terminateAllParsers();

	void cdDefaultFileName( const wchar_t *inputFile );
	void goDefaultFileName( const wchar_t *inputFile );
	void javaDefaultFileName( const wchar_t *inputFile );
	void rubyDefaultFileName( const wchar_t *inputFile );
	void csharpDefaultFileName( const wchar_t *inputFile );
	void ocamlDefaultFileName( const wchar_t *inputFile );

	void writeLanguage( std::wostream &out );
	void writeXML( std::wostream &out );
};

#endif
