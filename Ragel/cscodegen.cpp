/*
 *  Copyright 2001-2006 Adrian Thurston <thurston@complang.org>
 *            2004 Erich Ocean <eric.ocean@ampede.com>
 *            2005 Alan West <alan@alanz.com>
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

#include "ragel.h"
#include "cscodegen.h"
#include "redfsm.h"
#include "gendata.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <assert.h>

using std::wostream;
using std::wostringstream;
using std::wstring;
using std::wcerr;
using std::endl;

using std::wistream;
using std::wifstream;
using std::wostream;
using std::ios;
using std::cin;
using std::wcout;
using std::wcerr;
using std::endl;

void csharpLineDirective( wostream &out, const wchar_t *fileName, int line )
{
	if (fileName == NULL)
	{
		return;
	}
	if ( noLineDirectives )
		out << L"/* ";
	/* Write the preprocessor line info for to the input file. */
	out << L"#line " << line  << L" \"";
	for ( const wchar_t *pc = fileName; *pc != 0; pc++ ) {
		if ( *pc == L'\\' )
			out << L"\\\\";
		else
			out << *pc;
	}
	out << L'"';

	if ( noLineDirectives )
		out << L" */";

	out << L'\n';
}

void CSharpFsmCodeGen::genLineDirective( wostream &out )
{
	std::wstreambuf *sbuf = out.rdbuf();
	output_filter *filter = static_cast<output_filter*>(sbuf);
	csharpLineDirective( out, filter->fileName, filter->line + 1 );
}


/* Init code gen with in parameters. */
CSharpFsmCodeGen::CSharpFsmCodeGen( wostream &out )
:
	CodeGenData(out)
{
}

unsigned int CSharpFsmCodeGen::arrayTypeSize( unsigned long maxVal )
{
	long long maxValLL = (long long) maxVal;
	HostType *arrayType = keyOps->typeSubsumes( maxValLL );
	assert( arrayType != 0 );
	return arrayType->size;
}

wstring CSharpFsmCodeGen::ARRAY_TYPE( unsigned long maxVal )
{
	return ARRAY_TYPE( maxVal, false );
}

wstring CSharpFsmCodeGen::ARRAY_TYPE( unsigned long maxVal, bool forceSigned )
{
	long long maxValLL = (long long) maxVal;
	HostType *arrayType;
	if (forceSigned)
		arrayType = keyOps->typeSubsumes(true, maxValLL);
	else
		arrayType = keyOps->typeSubsumes( maxValLL );
	assert( arrayType != 0 );

	wstring ret = arrayType->data1;
	if ( arrayType->data2 != 0 ) {
		ret += L" ";
		ret += arrayType->data2;
	}
	return ret;
}

/* Write out the fsm name. */
wstring CSharpFsmCodeGen::FSM_NAME()
{
	return fsmName;
}

/* Emit the offset of the start state as a decimal integer. */
wstring CSharpFsmCodeGen::START_STATE_ID()
{
	wostringstream ret;
	ret << redFsm->startState->id;
	return ret.str();
};

/* Write out the array of actions. */
std::wostream &CSharpFsmCodeGen::ACTIONS_ARRAY()
{
	out << L"\t0, ";
	int totalActions = 1;
	for ( GenActionTableMap::Iter act = redFsm->actionMap; act.lte(); act++ ) {
		/* Write out the length, which will never be the last character. */
		out << act->key.length() << L", ";
		/* Put in a line break every 8 */
		if ( totalActions++ % 8 == 7 )
			out << L"\n\t";

		for ( GenActionTable::Iter item = act->key; item.lte(); item++ ) {
			out << item->value->actionId;
			if ( ! (act.last() && item.last()) )
				out << L", ";

			/* Put in a line break every 8 */
			if ( totalActions++ % 8 == 7 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


wstring CSharpFsmCodeGen::ACCESS()
{
	wostringstream ret;
	if ( accessExpr != 0 )
		INLINE_LIST( ret, accessExpr, 0, false );
	return ret.str();
}


wstring CSharpFsmCodeGen::P()
{ 
	wostringstream ret;
	if ( pExpr == 0 )
		ret << L"p";
	else {
		ret << L"(";
		INLINE_LIST( ret, pExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::PE()
{
	wostringstream ret;
	if ( peExpr == 0 )
		ret << L"pe";
	else {
		ret << L"(";
		INLINE_LIST( ret, peExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::vEOF()
{
	wostringstream ret;
	if ( eofExpr == 0 )
		ret << L"eof";
	else {
		ret << L"(";
		INLINE_LIST( ret, eofExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::vCS()
{
	wostringstream ret;
	if ( csExpr == 0 )
		ret << ACCESS() << L"cs";
	else {
		/* Emit the user supplied method of retrieving the key. */
		ret << L"(";
		INLINE_LIST( ret, csExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::TOP()
{
	wostringstream ret;
	if ( topExpr == 0 )
		ret << ACCESS() + L"top";
	else {
		ret << L"(";
		INLINE_LIST( ret, topExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::STACK()
{
	wostringstream ret;
	if ( stackExpr == 0 )
		ret << ACCESS() + L"stack";
	else {
		ret << L"(";
		INLINE_LIST( ret, stackExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::ACT()
{
	wostringstream ret;
	if ( actExpr == 0 )
		ret << ACCESS() + L"act";
	else {
		ret << L"(";
		INLINE_LIST( ret, actExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::TOKSTART()
{
	wostringstream ret;
	if ( tokstartExpr == 0 )
		ret << ACCESS() + L"ts";
	else {
		ret << L"(";
		INLINE_LIST( ret, tokstartExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::TOKEND()
{
	wostringstream ret;
	if ( tokendExpr == 0 )
		ret << ACCESS() + L"te";
	else {
		ret << L"(";
		INLINE_LIST( ret, tokendExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring CSharpFsmCodeGen::GET_WIDE_KEY()
{
	if ( redFsm->anyConditions() ) 
		return L"_widec";
	else
		return GET_KEY();
}

wstring CSharpFsmCodeGen::GET_WIDE_KEY( RedStateAp *state )
{
	if ( state->stateCondList.length() > 0 )
		return L"_widec";
	else
		return GET_KEY();
}

wstring CSharpFsmCodeGen::GET_KEY()
{
	wostringstream ret;
	if ( getKeyExpr != 0 ) { 
		/* Emit the user supplied method of retrieving the key. */
		ret << L"(";
		INLINE_LIST( ret, getKeyExpr, 0, false );
		ret << L")";
	}
	else {
		/* Expression for retrieving the key, use simple dereference. */
		ret << L"(*" << P() << L")";
	}
	return ret.str();
}

/* Write out level number of tabs. Makes the nested binary search nice
 * looking. */
wstring CSharpFsmCodeGen::TABS( int level )
{
	wstring result;
	while ( level-- > 0 )
		result += L"\t";
	return result;
}

/* Write out a key from the fsm code gen. Depends on wether or not the key is
 * signed. */
wstring CSharpFsmCodeGen::KEY( Key key )
{
	wostringstream ret;
	if ( keyOps->isSigned || !hostLang->explicitUnsigned )
		ret << key.getVal();
	else
		ret << (unsigned long) key.getVal() << L'u';
	return ret.str();
}

wstring CSharpFsmCodeGen::ALPHA_KEY( Key key )
{
	wostringstream ret;
	if (key.getVal() > 0xFFFF) {
		ret << key.getVal();
	} else {
		if ( keyOps->alphType->isChar )
			ret << L"'\\u" << std::hex << std::setw(4) << std::setfill(L'0') << key.getVal() << L"'";
		else
			ret << key.getVal();
	}
	//ret << L"(char) " << key.getVal();
	return ret.str();
}

void CSharpFsmCodeGen::EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish )
{
	/* The parser gives fexec two children. The double brackets are for D
	 * code. If the inline list is a single word it will get interpreted as a
	 * C-style cast by the D compiler. */
	ret << L"{" << P() << L" = ((";
	INLINE_LIST( ret, item->children, targState, inFinish );
	ret << L"))-1;}";
}

void CSharpFsmCodeGen::LM_SWITCH( wostream &ret, GenInlineItem *item, 
		int targState, int inFinish )
{
	ret << 
		L"	switch( " << ACT() << L" ) {\n";

	for ( GenInlineList::Iter lma = *item->children; lma.lte(); lma++ ) {
		/* Write the case label, the action and the case break. */
		if ( lma->lmId < 0 )
			ret << L"	default:\n";
		else
			ret << L"	case " << lma->lmId << L":\n";

		/* Write the block and close it off. */
		ret << L"	{";
		INLINE_LIST( ret, lma->children, targState, inFinish );
		ret << L"}\n";

		ret << L"	break;\n";
	}

	ret << 
		L"	}\n"
		L"\t";
}

void CSharpFsmCodeGen::SET_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = " << item->lmId << L";";
}

void CSharpFsmCodeGen::SET_TOKEND( wostream &ret, GenInlineItem *item )
{
	/* The tokend action sets tokend. */
	ret << TOKEND() << L" = " << P();
	if ( item->offset != 0 ) 
		out << L"+" << item->offset;
	out << L";";
}

void CSharpFsmCodeGen::GET_TOKEND( wostream &ret, GenInlineItem *item )
{
	ret << TOKEND();
}

void CSharpFsmCodeGen::INIT_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << NULL_ITEM() << L";";
}

void CSharpFsmCodeGen::INIT_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = 0;";
}

void CSharpFsmCodeGen::SET_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << P() << L";";
}

void CSharpFsmCodeGen::SUB_ACTION( wostream &ret, GenInlineItem *item, 
		int targState, bool inFinish )
{
	if ( item->children->length() > 0 ) {
		/* Write the block and close it off. */
		ret << L"{";
		INLINE_LIST( ret, item->children, targState, inFinish );
		ret << L"}";
	}
}


/* Write out an inline tree structure. Walks the list and possibly calls out
 * to virtual functions than handle language specific items in the tree. */
void CSharpFsmCodeGen::INLINE_LIST( wostream &ret, GenInlineList *inlineList, 
		int targState, bool inFinish )
{
	for ( GenInlineList::Iter item = *inlineList; item.lte(); item++ ) {
		switch ( item->type ) {
		case GenInlineItem::Text:
			ret << item->data;
			break;
		case GenInlineItem::Goto:
			GOTO( ret, item->targState->id, inFinish );
			break;
		case GenInlineItem::Call:
			CALL( ret, item->targState->id, targState, inFinish );
			break;
		case GenInlineItem::Next:
			NEXT( ret, item->targState->id, inFinish );
			break;
		case GenInlineItem::Ret:
			RET( ret, inFinish );
			break;
		case GenInlineItem::PChar:
			ret << P();
			break;
		case GenInlineItem::Char:
			ret << GET_KEY();
			break;
		case GenInlineItem::Hold:
			ret << P() << L"--;";
			break;
		case GenInlineItem::Exec:
			EXEC( ret, item, targState, inFinish );
			break;
		case GenInlineItem::Curs:
			CURS( ret, inFinish );
			break;
		case GenInlineItem::Targs:
			TARGS( ret, inFinish, targState );
			break;
		case GenInlineItem::Entry:
			ret << item->targState->id;
			break;
		case GenInlineItem::GotoExpr:
			GOTO_EXPR( ret, item, inFinish );
			break;
		case GenInlineItem::CallExpr:
			CALL_EXPR( ret, item, targState, inFinish );
			break;
		case GenInlineItem::NextExpr:
			NEXT_EXPR( ret, item, inFinish );
			break;
		case GenInlineItem::LmSwitch:
			LM_SWITCH( ret, item, targState, inFinish );
			break;
		case GenInlineItem::LmSetActId:
			SET_ACT( ret, item );
			break;
		case GenInlineItem::LmSetTokEnd:
			SET_TOKEND( ret, item );
			break;
		case GenInlineItem::LmGetTokEnd:
			GET_TOKEND( ret, item );
			break;
		case GenInlineItem::LmInitTokStart:
			INIT_TOKSTART( ret, item );
			break;
		case GenInlineItem::LmInitAct:
			INIT_ACT( ret, item );
			break;
		case GenInlineItem::LmSetTokStart:
			SET_TOKSTART( ret, item );
			break;
		case GenInlineItem::SubAction:
			SUB_ACTION( ret, item, targState, inFinish );
			break;
		case GenInlineItem::Break:
			BREAK( ret, targState );
			break;
		}
	}
}
/* Write out paths in line directives. Escapes any special characters. */
wstring CSharpFsmCodeGen::LDIR_PATH( wchar_t *path )
{
	wostringstream ret;
	for ( wchar_t *pc = path; *pc != 0; pc++ ) {
		if ( *pc == L'\\' )
			ret << L"\\\\";
		else
			ret << *pc;
	}
	return ret.str();
}

void CSharpFsmCodeGen::ACTION( wostream &ret, GenAction *action, int targState, bool inFinish )
{
	/* Write the preprocessor line info for going into the source file. */
	csharpLineDirective( ret, action->loc.fileName, action->loc.line );

	/* Write the block and close it off. */
	ret << L"\t{";
	INLINE_LIST( ret, action->inlineList, targState, inFinish );
	ret << L"}\n";
}

void CSharpFsmCodeGen::CONDITION( wostream &ret, GenAction *condition )
{
	ret << L"\n";
	csharpLineDirective( ret, condition->loc.fileName, condition->loc.line );
	INLINE_LIST( ret, condition->inlineList, 0, false );
}

wstring CSharpFsmCodeGen::ERROR_STATE()
{
	wostringstream ret;
	if ( redFsm->errState != 0 )
		ret << redFsm->errState->id;
	else
		ret << L"-1";
	return ret.str();
}

wstring CSharpFsmCodeGen::FIRST_FINAL_STATE()
{
	wostringstream ret;
	if ( redFsm->firstFinState != 0 )
		ret << redFsm->firstFinState->id;
	else
		ret << redFsm->nextStateId;
	return ret.str();
}

void CSharpFsmCodeGen::writeInit()
{
	out << L"	{\n";

	if ( !noCS )
		out << L"\t" << vCS() << L" = " << START() << L";\n";
	
	/* If there are any calls, then the stack top needs initialization. */
	if ( redFsm->anyActionCalls() || redFsm->anyActionRets() )
		out << L"\t" << TOP() << L" = 0;\n";

	if ( hasLongestMatch ) {
		out << 
			L"	" << TOKSTART() << L" = " << NULL_ITEM() << L";\n"
			L"	" << TOKEND() << L" = " << NULL_ITEM() << L";\n"
			L"	" << ACT() << L" = 0;\n";
	}
	out << L"	}\n";
}

wstring CSharpFsmCodeGen::DATA_PREFIX()
{
	if ( !noPrefix )
		return FSM_NAME() + L"_";
	return L"";
}

/* Emit the alphabet data type. */
wstring CSharpFsmCodeGen::ALPH_TYPE()
{
	wstring ret = keyOps->alphType->data1;
	if ( keyOps->alphType->data2 != 0 ) {
		ret += L" ";
		ret += + keyOps->alphType->data2;
	}
	return ret;
}

/* Emit the alphabet data type. */
wstring CSharpFsmCodeGen::WIDE_ALPH_TYPE()
{
	wstring ret;
	if ( redFsm->maxKey <= keyOps->maxKey )
		ret = ALPH_TYPE();
	else {
		long long maxKeyVal = redFsm->maxKey.getLongLong();
		HostType *wideType = keyOps->typeSubsumes( keyOps->isSigned, maxKeyVal );
		assert( wideType != 0 );

		ret = wideType->data1;
		if ( wideType->data2 != 0 ) {
			ret += L" ";
			ret += wideType->data2;
		}
	}
	return ret;
}

void CSharpFsmCodeGen::STATE_IDS()
{
	if ( redFsm->startState != 0 )
		STATIC_VAR( L"int", START() ) << L" = " << START_STATE_ID() << L";\n";

	if ( !noFinal )
		STATIC_VAR( L"int" , FIRST_FINAL() ) << L" = " << FIRST_FINAL_STATE() << L";\n";

	if ( !noError )
		STATIC_VAR( L"int", ERROR() ) << L" = " << ERROR_STATE() << L";\n";

	out << L"\n";

	if ( entryPointNames.length() > 0 ) {
		for ( EntryNameVect::Iter en = entryPointNames; en.lte(); en++ ) {
			STATIC_VAR( L"int", DATA_PREFIX() + L"en_" + *en ) << 
					L" = " << entryPointIds[en.pos()] << L";\n";
		}
		out << L"\n";
	}
}


void CSharpFsmCodeGen::writeStart()
{
	out << START_STATE_ID();
}

void CSharpFsmCodeGen::writeFirstFinal()
{
	out << FIRST_FINAL_STATE();
}

void CSharpFsmCodeGen::writeError()
{
	out << ERROR_STATE();
}

/*
 * C# Specific
 */
wstring CSharpCodeGen::GET_KEY()
{
	wostringstream ret;
	if ( getKeyExpr != 0 ) { 
		/* Emit the user supplied method of retrieving the key. */
		ret << L"(";
		INLINE_LIST( ret, getKeyExpr, 0, false );
		ret << L")";
	}
	else {
		/* Expression for retrieving the key, use simple dereference. */
		if ( dataExpr == 0 )
			ret << L"data";
		else
			INLINE_LIST( ret, dataExpr, 0, false );

		ret << L"[" << P() << L"]";
	}
	return ret.str();
}
wstring CSharpCodeGen::NULL_ITEM()
{
	return L"-1";
}

wstring CSharpCodeGen::POINTER()
{
	// XXX C# has no pointers
	// multiple items seperated by commas can also be pointer types.
	return L" ";
}

wstring CSharpCodeGen::PTR_CONST()
{
	return L"";
}

std::wostream &CSharpCodeGen::OPEN_ARRAY( wstring type, wstring name )
{
	out << L"static readonly " << type << L"[] " << name << L" =  ";
	/*
	if (type == L"char")
		out << L"Encoding.ASCII.Get";
	else */
		out << L"new " << type << L" [] {\n";
	return out;
}

std::wostream &CSharpCodeGen::CLOSE_ARRAY()
{
	return out << L"};\n";
}

std::wostream &CSharpCodeGen::STATIC_VAR( wstring type, wstring name )
{
	out << L"const " << type << L" " << name;
	return out;
}

wstring CSharpCodeGen::ARR_OFF( wstring ptr, wstring offset )
{
	// XXX C# can't do pointer arithmetic
	return L"&" + ptr + L"[" + offset + L"]";
}

wstring CSharpCodeGen::CAST( wstring type )
{
	return L"(" + type + L")";
}

wstring CSharpCodeGen::UINT( )
{
	return L"uint";
}

std::wostream &CSharpCodeGen::SWITCH_DEFAULT()
{
	out << L"		default: break;\n";
	return out;
}

wstring CSharpCodeGen::CTRL_FLOW()
{
	return L"if (true) ";
}

void CSharpCodeGen::writeExports()
{
	if ( exportList.length() > 0 ) {
		for ( ExportList::Iter ex = exportList; ex.lte(); ex++ ) {
			out << L"const " << ALPH_TYPE() << L" " << DATA_PREFIX() << 
					L"ex_" << ex->name << L" = " << KEY(ex->key) << L";\n";
		}
		out << L"\n";
	}
}

/*
 * End C#-specific code.
 */

void CSharpFsmCodeGen::finishRagelDef()
{
	if ( codeStyle == GenGoto || codeStyle == GenFGoto || 
			codeStyle == GenIpGoto || codeStyle == GenSplit )
	{
		/* For directly executable machines there is no required state
		 * ordering. Choose a depth-first ordering to increase the
		 * potential for fall-throughs. */
		redFsm->depthFirstOrdering();
	}
	else {
		/* The frontend will do this for us, but it may be a good idea to
		 * force it if the intermediate file is edited. */
		redFsm->sortByStateId();
	}

	/* Choose default transitions and the single transition. */
	redFsm->chooseDefaultSpan();
		
	/* Maybe do flat expand, otherwise choose single. */
	if ( codeStyle == GenFlat || codeStyle == GenFFlat )
		redFsm->makeFlat();
	else
		redFsm->chooseSingle();

	/* If any errors have occured in the input file then don't write anything. */
	if ( gblErrorCount > 0 )
		return;
	
	if ( codeStyle == GenSplit )
		redFsm->partitionFsm( numSplitPartitions );

	if ( codeStyle == GenIpGoto || codeStyle == GenSplit )
		redFsm->setInTrans();

	/* Anlayze Machine will find the final action reference counts, among
	 * other things. We will use these in reporting the usage
	 * of fsm directives in action code. */
	analyzeMachine();

	/* Determine if we should use indicies. */
	calcIndexSize();
}

wostream &CSharpFsmCodeGen::source_warning( const InputLoc &loc )
{
	wcerr << sourceFileName << L":" << loc.line << L":" << loc.col << L": warning: ";
	return wcerr;
}

wostream &CSharpFsmCodeGen::source_error( const InputLoc &loc )
{
	gblErrorCount += 1;
	assert( sourceFileName != 0 );
	wcerr << sourceFileName << L":" << loc.line << L":" << loc.col << L": ";
	return wcerr;
}

