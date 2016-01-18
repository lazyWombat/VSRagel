/*
 *  Copyright 2001-2007 Adrian Thurston <thurston@complang.org>
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <time.h>
#include <io.h>
#include <process.h>

#if _MSC_VER
#define S_IRUSR _S_IREAD
#define S_IWUSR _S_IWRITE
#endif
#endif

/* Parsing. */
#include "ragel.h"
#include "rlscan.h"

/* Parameters and output. */
#include "pcheck.h"
#include "vector.h"
#include "version.h"
#include "common.h"
#include "inputdata.h"
#include <locale>
#include <codecvt>

using std::wistream;
using std::wostream;
using std::wifstream;
using std::ofstream;
using std::wcin;
using std::wcout;
using std::wcerr;
using std::endl;
using std::ios;
using std::streamsize;
using std::locale;
using std::codecvt_utf8;

/* Controls minimization. */
MinimizeLevel minimizeLevel = MinimizePartition2;
MinimizeOpt minimizeOpt = MinimizeMostOps;

/* Graphviz dot file generation. */
const wchar_t *machineSpec = 0, *machineName = 0;
bool machineSpecFound = false;
bool wantDupsRemoved = true;

bool printStatistics = false;
bool generateXML = false;
bool generateDot = false;
bool useStandardOutput = false;
bool useStandardInput = false;

/* Target language and output style. */
CodeStyle codeStyle = GenTables;

int numSplitPartitions = 0;
bool noLineDirectives = false;

bool displayPrintables = false;

/* Target ruby impl */
RubyImplEnum rubyImpl = MRI;

/* Print a summary of the options. */
void usage()
{
	wcout <<
L"usage: ragel [options] file\n"
L"general:\n"
L"   -h, -H, -?, --help   Print this usage and exit\n"
L"   -v, --version        Print version information and exit\n"
L"   -o <file>            Write output to <file>\n"
L"   -s                   Print some statistics on stderr\n"
L"   -d                   Do not remove duplicates from action lists\n"
L"   -I <dir>             Add <dir> to the list of directories to search\n"
L"                        for included an imported files\n"
L"error reporting format:\n"
L"   --error-format=gnu   file:line:column: message (default)\n"
L"   --error-format=msvc  file(line,column): message\n"
L"fsm minimization:\n"
L"   -n                   Do not perform minimization\n"
L"   -m                   Minimize at the end of the compilation\n"
L"   -l                   Minimize after most operations (default)\n"
L"   -e                   Minimize after every operation\n"
L"visualization:\n"
L"   -x                   Run the frontend only: emit XML intermediate format\n"
L"   -V                   Generate a dot file for Graphviz\n"
L"   -p                   Display printable characters on labels\n"
L"   -S <spec>            FSM specification to output (for graphviz output)\n"
L"   -M <machine>         Machine definition/instantiation to output (for graphviz output)\n"
L"host language:\n"
L"   -C                   The host language is C, C++, Obj-C or Obj-C++ (default)\n"
L"   -D                   The host language is D\n"
L"   -Z                   The host language is Go\n"
L"   -J                   The host language is Java\n"
L"   -R                   The host language is Ruby\n"
L"   -A                   The host language is C#\n"
L"   -O                   The host language is OCaml\n"
L"line directives: (C/D/Ruby/C#/OCaml)\n"
L"   -L                   Inhibit writing of #line directives\n"
L"code style: (C/D/Java/Ruby/C#/OCaml)\n"
L"   -T0                  Table driven FSM (default)\n"
L"code style: (C/D/Ruby/C#/OCaml)\n"
L"   -T1                  Faster table driven FSM\n"
L"   -F0                  Flat table driven FSM\n"
L"   -F1                  Faster flat table-driven FSM\n"
L"code style: (C/D/C#/OCaml)\n"
L"   -G0                  Goto-driven FSM\n"
L"   -G1                  Faster goto-driven FSM\n"
L"code style: (C/D)\n"
L"   -G2                  Really fast goto-driven FSM\n"
L"   -P<N>                N-Way Split really fast goto-driven FSM\n"
	;	

	exit(0);
}

/* Print version information and exit. */
void version()
{
	wcout << L"Ragel State Machine Compiler version " VERSION << L" " PUBDATE << endl <<
			L"Copyright (c) 2001-2009 by Adrian Thurston" << endl;
	exit(0);
}

/* Error reporting format. */
ErrorFormat errorFormat = ErrorFormatGNU;

InputLoc makeInputLoc( const wchar_t *fileName, int line, int col)
{
	InputLoc loc = { fileName, line, col };
	return loc;
}

wostream &operator<<( wostream &out, const InputLoc &loc )
{
	assert( loc.fileName != 0 );
	switch ( errorFormat ) {
	case ErrorFormatMSVC:
		out << loc.fileName << L"(" << loc.line;
		if ( loc.col )
			out << L"," << loc.col;
		out << L")";
		break;

	default:
		out << loc.fileName << L":" << loc.line;
		if ( loc.col )
			out << L":" << loc.col;
		break;
	}
	return out;
}

/* Total error count. */
int gblErrorCount = 0;

/* Print the opening to a warning in the input, then return the error wostream. */
wostream &warning( const InputLoc &loc )
{
	wcerr << loc << L": warning: ";
	return wcerr;
}

/* Print the opening to a program error, then return the error stream. */
wostream &error()
{
	gblErrorCount += 1;
	wcerr << PROGNAME L": ";
	return wcerr;
}

wostream &error( const InputLoc &loc )
{
	gblErrorCount += 1;
	wcerr << loc << L": ";
	return wcerr;
}

void escapeLineDirectivePath( std::wostream &out, wchar_t *path )
{
	for ( wchar_t *pc = path; *pc != 0; pc++ ) {
		if ( *pc == L'\\' )
			out << L"\\\\";
		else
			out << *pc;
	}
}

void processArgs( int argc, const wchar_t **argv, InputData &id )
{
	ParamCheck pc(L"xo:dnmleabjkS:M:I:CDEJZRAOvHh?-:sT:F:G:P:LpVci", argc, argv);

	/* FIXME: Need to check code styles VS langauge. */

	while ( pc.check() ) {
		switch ( pc.state ) {
		case ParamCheck::match:
			switch ( pc.parameter ) {
			case L'V':
				generateDot = true;
				break;
			case L'c':
				useStandardOutput = true;
				break;
			case L'i':
				useStandardInput = true;
				break;
			case L'x':
				generateXML = true;
				break;

			/* Output. */
			case L'o':
				if ( *pc.paramArg == 0 )
					error() << L"a zero length output file name was given" << endl;
				else if ( id.outputFileName != 0 )
					error() << L"more than one output file name was given" << endl;
				else {
					/* Ok, remember the output file name. */
					id.outputFileName = pc.paramArg;
				}
				break;

			/* Flag for turning off duplicate action removal. */
			case L'd':
				wantDupsRemoved = false;
				break;

			/* Minimization, mostly hidden options. */
			case L'n':
				minimizeOpt = MinimizeNone;
				break;
			case L'm':
				minimizeOpt = MinimizeEnd;
				break;
			case L'l':
				minimizeOpt = MinimizeMostOps;
				break;
			case L'e':
				minimizeOpt = MinimizeEveryOp;
				break;
			case L'a':
				minimizeLevel = MinimizeApprox;
				break;
			case L'b':
				minimizeLevel = MinimizeStable;
				break;
			case L'j':
				minimizeLevel = MinimizePartition1;
				break;
			case L'k':
				minimizeLevel = MinimizePartition2;
				break;

			/* Machine spec. */
			case L'S':
				if ( *pc.paramArg == 0 )
					error() << L"please specify an argument to -S" << endl;
				else if ( machineSpec != 0 )
					error() << L"more than one -S argument was given" << endl;
				else {
					/* Ok, remember the path to the machine to generate. */
					machineSpec = pc.paramArg;
				}
				break;

			/* Machine path. */
			case L'M':
				if ( *pc.paramArg == 0 )
					error() << L"please specify an argument to -M" << endl;
				else if ( machineName != 0 )
					error() << L"more than one -M argument was given" << endl;
				else {
					/* Ok, remember the machine name to generate. */
					machineName = pc.paramArg;
				}
				break;

			case L'I':
				if ( *pc.paramArg == 0 )
					error() << L"please specify an argument to -I" << endl;
				else {
					id.includePaths.append( pc.paramArg );
				}
				break;

			/* Host language types. */
			case L'C':
				hostLang = &hostLangC;
				break;
			case L'D':
				hostLang = &hostLangD;
				break;
			case L'E':
				hostLang = &hostLangD2;
				break;
			case L'Z':
				hostLang = &hostLangGo;
				break;
			case L'J':
				hostLang = &hostLangJava;
				break;
			case L'R':
				hostLang = &hostLangRuby;
				break;
			case L'A':
				hostLang = &hostLangCSharp;
				break;
			case L'O':
				hostLang = &hostLangOCaml;
				break;

			/* Version and help. */
			case L'v':
				version();
				break;
			case L'H': case L'h': case L'?':
				usage();
				break;
			case L's':
				printStatistics = true;
				break;
			case L'-': {
				wchar_t *arg = _wcsdup( pc.paramArg );
				wchar_t *eq = wcschr( arg, L'=' );

				if ( eq != 0 )
					*eq++ = 0;

				if ( wcscmp( arg, L"help" ) == 0 )
					usage();
				else if ( wcscmp( arg, L"version" ) == 0 )
					version();
				else if ( wcscmp( arg, L"error-format" ) == 0 ) {
					if ( eq == 0 )
						error() << L"expecting '=value' for error-format" << endl;
					else if ( wcscmp( eq, L"gnu" ) == 0 )
						errorFormat = ErrorFormatGNU;
					else if ( wcscmp( eq, L"msvc" ) == 0 )
						errorFormat = ErrorFormatMSVC;
					else
						error() << L"invalid value for error-format" << endl;
				}
				else if ( wcscmp( arg, L"rbx" ) == 0 )
					rubyImpl = Rubinius;
				else {
					error() << L"--" << pc.paramArg << 
							L" is an invalid argument" << endl;
				}
				free( arg );
				break;
			}

			/* Passthrough args. */
			case L'T': 
				if ( pc.paramArg[0] == L'0' )
					codeStyle = GenTables;
				else if ( pc.paramArg[0] == L'1' )
					codeStyle = GenFTables;
				else {
					error() << L"-T" << pc.paramArg[0] << 
							L" is an invalid argument" << endl;
					exit(1);
				}
				break;
			case L'F': 
				if ( pc.paramArg[0] == L'0' )
					codeStyle = GenFlat;
				else if ( pc.paramArg[0] == L'1' )
					codeStyle = GenFFlat;
				else {
					error() << L"-F" << pc.paramArg[0] << 
							L" is an invalid argument" << endl;
					exit(1);
				}
				break;
			case L'G': 
				if ( pc.paramArg[0] == L'0' )
					codeStyle = GenGoto;
				else if ( pc.paramArg[0] == L'1' )
					codeStyle = GenFGoto;
				else if ( pc.paramArg[0] == L'2' )
					codeStyle = GenIpGoto;
				else {
					error() << L"-G" << pc.paramArg[0] << 
							L" is an invalid argument" << endl;
					exit(1);
				}
				break;
			case L'P':
				codeStyle = GenSplit;
				numSplitPartitions = _wtoi( pc.paramArg );
				break;

			case L'p':
				displayPrintables = true;
				break;

			case L'L':
				noLineDirectives = true;
				break;
			}
			break;

		case ParamCheck::invalid:
			error() << L"-" << pc.parameter << L" is an invalid argument" << endl;
			break;

		case ParamCheck::noparam:
			/* It is interpreted as an input file. */
			if ( *pc.curArg == 0 )
				error() << L"a zero length input file name was given" << endl;
			else if ( id.inputFileName != 0 )
				error() << L"more than one input file name was given" << endl;
			else {
				/* OK, Remember the filename. */
				id.inputFileName = pc.curArg;
			}
			break;
		}
	}
}

void process( InputData &id )
{
	/* Open the input file for reading. */
	assert( id.inputFileName != 0 );

	wistream * input;
	wifstream *inFile = NULL;

	if (useStandardInput)
	{
		input = &wcin;
	}
	else
	{
		inFile = new wifstream(id.inputFileName);
		if (!inFile->is_open())
			error() << L"could not open " << id.inputFileName << L" for reading" << endp;

		inFile->imbue(locale(inFile->getloc(), new codecvt_utf8<wchar_t, 0x10ffff, std::consume_header>));

		input = inFile;
	}
	/* Used for just a few things. */
	std::wostringstream hostData;

	/* Make the first input item. */
	InputItem *firstInputItem = new InputItem;
	firstInputItem->type = InputItem::HostData;
	firstInputItem->loc.fileName = id.inputFileName;
	firstInputItem->loc.line = 1;
	firstInputItem->loc.col = 1;
	id.inputItems.append( firstInputItem );

	Scanner scanner( id, id.inputFileName, *input, 0, 0, 0, false );
	scanner.do_scan();

	/* Finished, final check for errors.. */
	if ( gblErrorCount > 0 )
		exit(1);

	/* Now send EOF to all parsers. */
	id.terminateAllParsers();

	/* Bail on above error. */
	if ( gblErrorCount > 0 )
		exit(1);

	/* Locate the backend program */
	/* Compiles machines. */
	id.prepareMachineGen();

	if ( gblErrorCount > 0 )
		exit(1);

	id.makeOutputStream();

	/* Generates the reduced machine, which we use to write output. */
	if ( !generateXML ) {
		id.generateReduced();

		if ( gblErrorCount > 0 )
			exit(1);
	}

	id.verifyWritesHaveData();
	if ( gblErrorCount > 0 )
		exit(1);

	/*
	 * From this point on we should not be reporting any errors.
	 */

	id.openOutput();
	id.writeOutput();

	/* Close the input and the intermediate file. */
	if (inFile != NULL)
	{
		delete inFile;
	}

	/* If writing to a file, delete the wostream, causing it to flush.
	 * Standard out is flushed automatically. */
	if ( id.outputFileName != 0 ) {
		delete id.outStream;
		delete id.outFilter;
	}

	assert( gblErrorCount == 0 );
}

wchar_t *makeIntermedTemplate( const wchar_t *baseFileName )
{
	wchar_t *result = 0;
	const wchar_t *templ = L"ragel-XXXXXX.xml";
	const wchar_t *lastSlash = wcsrchr( baseFileName, L'/' );
	if ( lastSlash == 0 ) {
		result = new wchar_t[wcslen(templ)+1];
		wcscpy_s( result, wcslen(templ) + 1, templ );
	}
	else {
		int baseLen = lastSlash - baseFileName + 1;
		result = new wchar_t[baseLen + wcslen(templ) + 1];
		memcpy( result, baseFileName, baseLen );
		wcscpy_s( result+baseLen, wcslen(templ) + 1, templ );
	}
	return result;
};

/* Main, process args and call yyparse to start scanning input. */
int wmain( int argc, const wchar_t **argv )
{
	InputData id;
	
	_setmode(_fileno(stdin), _O_U8TEXT);
	_setmode(_fileno(stdout), _O_U8TEXT);

	processArgs( argc, argv, id );

	/* Require an input file. If we use standard in then we won't have a file
	 * name on which to base the output. */
	if ( id.inputFileName == 0 )
		error() << L"no input file given" << endl;

	/* Bail on argument processing errors. */
	if ( gblErrorCount > 0 )
		exit(1);

	/* Make sure we are not writing to the same file as the input file. */
	if ( id.inputFileName != 0 && id.outputFileName != 0 && 
			wcscmp( id.inputFileName, id.outputFileName  ) == 0 )
	{
		error() << L"output file \"" << id.outputFileName  << 
				L"\" is the same as the input file" << endp;
	}

	process( id );

	return 0;
}
