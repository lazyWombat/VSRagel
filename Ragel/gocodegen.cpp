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

#include "gocodegen.h"
#include "ragel.h"
#include "redfsm.h"
#include "gendata.h"
#include <sstream>
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

/*
 * Go Specific
 */

void goLineDirective( wostream &out, const wchar_t *fileName, int line )
{
	out << L"//line " << fileName << L":" << line << endl;
}

void GoCodeGen::genLineDirective( wostream &out )
{
	std::wstreambuf *sbuf = out.rdbuf();
	output_filter *filter = static_cast<output_filter*>(sbuf);
	goLineDirective( out, filter->fileName, filter->line + 1 );
}

unsigned int GoCodeGen::arrayTypeSize( unsigned long maxVal )
{
	long long maxValLL = (long long) maxVal;
	HostType *arrayType = keyOps->typeSubsumes( maxValLL );
	assert( arrayType != 0 );
	return arrayType->size;
}

wstring GoCodeGen::ARRAY_TYPE( unsigned long maxVal )
{
	long long maxValLL = (long long) maxVal;
	HostType *arrayType = keyOps->typeSubsumes( maxValLL );
	assert( arrayType != 0 );

	wstring ret = arrayType->data1;
	if ( arrayType->data2 != 0 ) {
		ret += L" ";
		ret += arrayType->data2;
	}
	return ret;
}


/* Write out the fsm name. */
wstring GoCodeGen::FSM_NAME()
{
	return fsmName;
}

/* Emit the offset of the start state as a decimal integer. */
wstring GoCodeGen::START_STATE_ID()
{
	wostringstream ret;
	ret << redFsm->startState->id;
	return ret.str();
};

/* Write out the array of actions. */
std::wostream &GoCodeGen::ACTIONS_ARRAY()
{
	out << L"	0, ";
	int totalActions = 1;
	for ( GenActionTableMap::Iter act = redFsm->actionMap; act.lte(); act++ ) {
		/* Write out the length, which will never be the last character. */
		out << act->key.length() << L", ";
		if ( totalActions++ % IALL == 0 )
			out << endl << L"	";

		for ( GenActionTable::Iter item = act->key; item.lte(); item++ ) {
			out << item->value->actionId << L", ";
			if ( ! (act.last() && item.last()) ) {
				if ( totalActions++ % IALL == 0 )
					out << endl << L"	";
			}
		}
	}
	out << endl;
	return out;
}


wstring GoCodeGen::ACCESS()
{
	wostringstream ret;
	if ( accessExpr != 0 )
		INLINE_LIST( ret, accessExpr, 0, false, false );
	return ret.str();
}


wstring GoCodeGen::P()
{
	wostringstream ret;
	if ( pExpr == 0 )
		ret << L"p";
	else {
		ret << L"(";
		INLINE_LIST( ret, pExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::PE()
{
	wostringstream ret;
	if ( peExpr == 0 )
		ret << L"pe";
	else {
		ret << L"(";
		INLINE_LIST( ret, peExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::vEOF()
{
	wostringstream ret;
	if ( eofExpr == 0 )
		ret << L"eof";
	else {
		ret << L"(";
		INLINE_LIST( ret, eofExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::vCS()
{
	wostringstream ret;
	if ( csExpr == 0 )
		ret << ACCESS() << L"cs";
	else {
		/* Emit the user supplied method of retrieving the key. */
		ret << L"(";
		INLINE_LIST( ret, csExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::TOP()
{
	wostringstream ret;
	if ( topExpr == 0 )
		ret << ACCESS() + L"top";
	else {
		ret << L"(";
		INLINE_LIST( ret, topExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::STACK()
{
	wostringstream ret;
	if ( stackExpr == 0 )
		ret << ACCESS() + L"stack";
	else {
		ret << L"(";
		INLINE_LIST( ret, stackExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::ACT()
{
	wostringstream ret;
	if ( actExpr == 0 )
		ret << ACCESS() + L"act";
	else {
		ret << L"(";
		INLINE_LIST( ret, actExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::TOKSTART()
{
	wostringstream ret;
	if ( tokstartExpr == 0 )
		ret << ACCESS() + L"ts";
	else {
		ret << L"(";
		INLINE_LIST( ret, tokstartExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::TOKEND()
{
	wostringstream ret;
	if ( tokendExpr == 0 )
		ret << ACCESS() + L"te";
	else {
		ret << L"(";
		INLINE_LIST( ret, tokendExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::GET_WIDE_KEY()
{
	if ( redFsm->anyConditions() )
		return L"_widec";
	else
		return GET_KEY();
}

wstring GoCodeGen::GET_WIDE_KEY( RedStateAp *state )
{
	if ( state->stateCondList.length() > 0 )
		return L"_widec";
	else
		return GET_KEY();
}

wstring GoCodeGen::GET_KEY()
{
	wostringstream ret;
	if ( getKeyExpr != 0 ) {
		/* Emit the user supplied method of retrieving the key. */
		ret << L"(";
		INLINE_LIST( ret, getKeyExpr, 0, false, false );
		ret << L")";
	}
	else {
		/* Expression for retrieving the key, use simple dereference. */
		ret << DATA() << L"[" << P() << L"]";
	}
	return ret.str();
}

/* Write out level number of tabs. Makes the nested binary search nice
 * looking. */
wstring GoCodeGen::TABS( int level )
{
	wstring result;
	while ( level-- > 0 )
		result += L"\t";
	return result;
}

/* Write out a key from the fsm code gen. Depends on wether or not the key is
 * signed. */
wstring GoCodeGen::KEY( Key key )
{
	wostringstream ret;
	if ( keyOps->isSigned || !hostLang->explicitUnsigned )
		ret << key.getVal();
	else
		ret << (unsigned long) key.getVal() << L'u';
	return ret.str();
}

bool GoCodeGen::isAlphTypeSigned()
{
	return keyOps->isSigned;
}

bool GoCodeGen::isWideAlphTypeSigned()
{
	wstring ret;
	if ( redFsm->maxKey <= keyOps->maxKey )
		return isAlphTypeSigned();
	else {
		long long maxKeyVal = redFsm->maxKey.getLongLong();
		HostType *wideType = keyOps->typeSubsumes( keyOps->isSigned, maxKeyVal );
		return wideType->isSigned;
	}
}

wstring GoCodeGen::WIDE_KEY( RedStateAp *state, Key key )
{
	if ( state->stateCondList.length() > 0 ) {
		wostringstream ret;
		if ( isWideAlphTypeSigned() )
			ret << key.getVal();
		else
			ret << (unsigned long) key.getVal() << L'u';
		return ret.str();
	}
	else {
		return KEY( key );
	}
}



void GoCodeGen::EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish )
{
	/* The parser gives fexec two children. The double brackets are for D
	 * code. If the inline list is a single word it will get interpreted as a
	 * C-style cast by the D compiler. */
	ret << P() << L" = (";
	INLINE_LIST( ret, item->children, targState, inFinish, false );
	ret << L") - 1" << endl;
}

void GoCodeGen::LM_SWITCH( wostream &ret, GenInlineItem *item,
		int targState, int inFinish, bool csForced )
{
	ret <<
		L"	switch " << ACT() << L" {" << endl;

	for ( GenInlineList::Iter lma = *item->children; lma.lte(); lma++ ) {
		/* Write the case label, the action and the case break. */
		if ( lma->lmId < 0 ) {
			ret << L"	default:" << endl;
		}
		else
			ret << L"	case " << lma->lmId << L":" << endl;

		/* Write the block and close it off. */
		ret << L"	{";
		INLINE_LIST( ret, lma->children, targState, inFinish, csForced );
		ret << L"}" << endl;
	}

	ret <<
		L"	}" << endl <<
		L"	";
}

void GoCodeGen::SET_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = " << item->lmId << L";";
}

void GoCodeGen::SET_TOKEND( wostream &ret, GenInlineItem *item )
{
	/* The tokend action sets tokend. */
	ret << TOKEND() << L" = " << P();
	if ( item->offset != 0 )
		out << L"+" << item->offset;
	out << endl;
}

void GoCodeGen::GET_TOKEND( wostream &ret, GenInlineItem *item )
{
	ret << TOKEND();
}

void GoCodeGen::INIT_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << NULL_ITEM() << endl;
}

void GoCodeGen::INIT_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = 0" << endl;
}

void GoCodeGen::SET_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << P() << endl;
}

void GoCodeGen::SUB_ACTION( wostream &ret, GenInlineItem *item,
		int targState, bool inFinish, bool csForced )
{
	if ( item->children->length() > 0 ) {
		/* Write the block and close it off. */
		ret << L"{";
		INLINE_LIST( ret, item->children, targState, inFinish, csForced );
		ret << L"}";
	}
}


/* Write out an inline tree structure. Walks the list and possibly calls out
 * to virtual functions than handle language specific items in the tree. */
void GoCodeGen::INLINE_LIST( wostream &ret, GenInlineList *inlineList,
		int targState, bool inFinish, bool csForced )
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
			ret << P() << L"--" << endl;
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
			LM_SWITCH( ret, item, targState, inFinish, csForced );
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
			SUB_ACTION( ret, item, targState, inFinish, csForced );
			break;
		case GenInlineItem::Break:
			BREAK( ret, targState, csForced );
			break;
		}
	}
}
/* Write out paths in line directives. Escapes any special characters. */
wstring GoCodeGen::LDIR_PATH( wchar_t *path )
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

void GoCodeGen::ACTION( wostream &ret, GenAction *action, int targState,
		bool inFinish, bool csForced )
{
	/* Write the preprocessor line info for going into the source file. */
	goLineDirective( ret, action->loc.fileName, action->loc.line );

	/* Write the block and close it off. */
	INLINE_LIST( ret, action->inlineList, targState, inFinish, csForced );
	ret << endl;
}

void GoCodeGen::CONDITION( wostream &ret, GenAction *condition )
{
	INLINE_LIST( ret, condition->inlineList, 0, false, false );
}

wstring GoCodeGen::ERROR_STATE()
{
	wostringstream ret;
	if ( redFsm->errState != 0 )
		ret << redFsm->errState->id;
	else
		ret << L"-1";
	return ret.str();
}

wstring GoCodeGen::FIRST_FINAL_STATE()
{
	wostringstream ret;
	if ( redFsm->firstFinState != 0 )
		ret << redFsm->firstFinState->id;
	else
		ret << redFsm->nextStateId;
	return ret.str();
}

void GoCodeGen::writeInit()
{
	out << L"	{" << endl;

	if ( !noCS )
		out << L"	" << vCS() << L" = " << START() << endl;

	/* If there are any calls, then the stack top needs initialization. */
	if ( redFsm->anyActionCalls() || redFsm->anyActionRets() )
		out << L"	" << TOP() << L" = 0" << endl;

	if ( hasLongestMatch ) {
		out <<
			L"	" << TOKSTART() << L" = " << NULL_ITEM() << endl <<
			L"	" << TOKEND() << L" = " << NULL_ITEM() << endl <<
			L"	" << ACT() << L" = 0" << endl;
	}
	out << L"	}" << endl;
}

wstring GoCodeGen::DATA()
{
	wostringstream ret;
	if ( dataExpr == 0 )
		ret << ACCESS() + L"data";
	else {
		ret << L"(";
		INLINE_LIST( ret, dataExpr, 0, false, false );
		ret << L")";
	}
	return ret.str();
}

wstring GoCodeGen::DATA_PREFIX()
{
	if ( !noPrefix )
		return FSM_NAME() + L"_";
	return L"";
}

/* Emit the alphabet data type. */
wstring GoCodeGen::ALPH_TYPE()
{
	wstring ret = keyOps->alphType->data1;
	if ( keyOps->alphType->data2 != 0 ) {
		ret += L" ";
		ret += + keyOps->alphType->data2;
	}
	return ret;
}

/* Emit the alphabet data type. */
wstring GoCodeGen::WIDE_ALPH_TYPE()
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

void GoCodeGen::STATE_IDS()
{
	if ( redFsm->startState != 0 )
		CONST( L"int", START() ) << L" = " << START_STATE_ID() << endl;

	if ( !noFinal )
		CONST( L"int" , FIRST_FINAL() ) << L" = " << FIRST_FINAL_STATE() << endl;

	if ( !noError )
		CONST( L"int", ERROR() ) << L" = " << ERROR_STATE() << endl;

	out << endl;

	if ( entryPointNames.length() > 0 ) {
		for ( EntryNameVect::Iter en = entryPointNames; en.lte(); en++ ) {
			CONST( L"int", DATA_PREFIX() + L"en_" + *en ) <<
					L" = " << entryPointIds[en.pos()] << endl;
		}
		out << endl;
	}
}

void GoCodeGen::writeStart()
{
	out << START_STATE_ID();
}

void GoCodeGen::writeFirstFinal()
{
	out << FIRST_FINAL_STATE();
}

void GoCodeGen::writeError()
{
	out << ERROR_STATE();
}

void GoCodeGen::finishRagelDef()
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

wostream &GoCodeGen::source_warning( const InputLoc &loc )
{
	wcerr << sourceFileName << L":" << loc.line << L":" << loc.col << L": warning: ";
	return wcerr;
}

wostream &GoCodeGen::source_error( const InputLoc &loc )
{
	gblErrorCount += 1;
	assert( sourceFileName != 0 );
	wcerr << sourceFileName << L":" << loc.line << L":" << loc.col << L": ";
	return wcerr;
}


/*
 * Go implementation.
 *
 */

std::wostream &GoCodeGen::OPEN_ARRAY( wstring type, wstring name )
{
	out << L"var " << name << L" []" << type << L" = []" << type << L"{" << endl;
	return out;
}

std::wostream &GoCodeGen::CLOSE_ARRAY()
{
	return out << L"}" << endl;
}

std::wostream &GoCodeGen::STATIC_VAR( wstring type, wstring name )
{
	out << L"var " << name << L" " << type;
	return out;
}

std::wostream &GoCodeGen::CONST( wstring type, wstring name )
{
	out << L"const " << name << L" " << type;
	return out;
}

wstring GoCodeGen::UINT( )
{
	return L"uint";
}

wstring GoCodeGen::INT()
{
	return L"int";
}

wstring GoCodeGen::CAST( wstring type, wstring expr )
{
	return type + L"(" + expr + L")";
}

wstring GoCodeGen::NULL_ITEM()
{
	return L"0";
}

void GoCodeGen::writeExports()
{
	if ( exportList.length() > 0 ) {
		for ( ExportList::Iter ex = exportList; ex.lte(); ex++ ) {
			out << L"const " << DATA_PREFIX() << L"ex_" << ex->name << L" = " <<
					KEY(ex->key) << endl;
		}
		out << endl;
	}
}
