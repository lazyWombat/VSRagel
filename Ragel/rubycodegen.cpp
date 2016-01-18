/*
 *  2007 Victor Hugo Borja <vic@rubyforge.org>
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

#include <iomanip>
#include <sstream>
#include "redfsm.h"
#include "gendata.h"
#include "ragel.h"
#include "rubycodegen.h"
#include "pcheck.h"
#include "vector.h"
#include "version.h"
#include "common.h"

#include "ragel.h"
#include "rubytable.h"
#include "rubyftable.h"
#include "rubyflat.h"
#include "rubyfflat.h"
#include "rbxgoto.h"

using std::wostream;
using std::wostringstream;
using std::wstring;
using std::endl;
using std::wistream;
using std::wifstream;
using std::wostream;
using std::ios;
using std::cin;
using std::wcout;
using std::endl;

/* Target ruby impl */

/* Target language and output style. */
extern CodeStyle codeStyle;

extern int numSplitPartitions;
extern bool noLineDirectives;

/*
 * Callbacks invoked by the XML data parser.
 */


void rubyLineDirective( wostream &out, const wchar_t *fileName, int line )
{
	if ( noLineDirectives )
		return;

	/* Write a comment containing line info. */
	out << L"# line " << line  << L" \"";
	for ( const wchar_t *pc = fileName; *pc != 0; pc++ ) {
		if ( *pc == L'\\' )
			out << L"\\\\";
		else
			out << *pc;
	}
	out << L"\"\n";
}

void RubyCodeGen::genLineDirective( wostream &out )
{
	std::wstreambuf *sbuf = out.rdbuf();
	output_filter *filter = static_cast<output_filter*>(sbuf);
	rubyLineDirective( out, filter->fileName, filter->line + 1 );
}

wstring RubyCodeGen::DATA_PREFIX()
{
	if ( !noPrefix )
		return FSM_NAME() + L"_";
	return L"";
}

std::wostream &RubyCodeGen::STATIC_VAR( wstring type, wstring name )
{
	out << 
		L"class << self\n"
		L"	attr_accessor :" << name << L"\n"
		L"end\n"
		L"self." << name;
	return out;
}


std::wostream &RubyCodeGen::OPEN_ARRAY( wstring type, wstring name )
{
	out << 
		L"class << self\n"
		L"	attr_accessor :" << name << L"\n"
		L"	private :" << name << L", :" << name << L"=\n"
		L"end\n"
		L"self." << name << L" = [\n";
	return out;
}

std::wostream &RubyCodeGen::CLOSE_ARRAY()
{
	out << L"]\n";
	return out;
}


wstring RubyCodeGen::ARR_OFF( wstring ptr, wstring offset )
{
	return ptr + L"[" + offset + L"]";
}

wstring RubyCodeGen::NULL_ITEM()
{
	return L"nil";
}


wstring RubyCodeGen::P()
{ 
	wostringstream ret;
	if ( pExpr == 0 )
		ret << L"p";
	else {
		//ret << L"(";
		INLINE_LIST( ret, pExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::PE()
{
	wostringstream ret;
	if ( peExpr == 0 )
		ret << L"pe";
	else {
		//ret << L"(";
		INLINE_LIST( ret, peExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::vEOF()
{
	wostringstream ret;
	if ( eofExpr == 0 )
		ret << L"eof";
	else {
		//ret << L"(";
		INLINE_LIST( ret, eofExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::vCS()
{
	wostringstream ret;
	if ( csExpr == 0 )
		ret << ACCESS() << L"cs";
	else {
		//ret << L"(";
		INLINE_LIST( ret, csExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::TOP()
{
	wostringstream ret;
	if ( topExpr == 0 )
		ret << ACCESS() + L"top";
	else {
		//ret << L"(";
		INLINE_LIST( ret, topExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::STACK()
{
	wostringstream ret;
	if ( stackExpr == 0 )
		ret << ACCESS() + L"stack";
	else {
		//ret << L"(";
		INLINE_LIST( ret, stackExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::ACT()
{
	wostringstream ret;
	if ( actExpr == 0 )
		ret << ACCESS() + L"act";
	else {
		//ret << L"(";
		INLINE_LIST( ret, actExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::TOKSTART()
{
	wostringstream ret;
	if ( tokstartExpr == 0 )
		ret << ACCESS() + L"ts";
	else {
		//ret << L"(";
		INLINE_LIST( ret, tokstartExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::TOKEND()
{
	wostringstream ret;
	if ( tokendExpr == 0 )
		ret << ACCESS() + L"te";
	else {
		//ret << L"(";
		INLINE_LIST( ret, tokendExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

wstring RubyCodeGen::DATA()
{
	wostringstream ret;
	if ( dataExpr == 0 )
		ret << ACCESS() + L"data";
	else {
		//ret << L"(";
		INLINE_LIST( ret, dataExpr, 0, false );
		//ret << L")";
	}
	return ret.str();
}

/* Write out the fsm name. */
wstring RubyCodeGen::FSM_NAME()
{
	return fsmName;
}


void RubyCodeGen::ACTION( wostream &ret, GenAction *action, int targState, bool inFinish )
{
	/* Write the preprocessor line info for going into the source file. */
	rubyLineDirective( ret, action->loc.fileName, action->loc.line );

	/* Write the block and close it off. */
	ret << L"		begin\n";
	INLINE_LIST( ret, action->inlineList, targState, inFinish );
	ret << L"		end\n";
}



wstring RubyCodeGen::GET_WIDE_KEY()
{
	if ( redFsm->anyConditions() ) 
		return L"_widec";
	else
		return GET_KEY();
}

wstring RubyCodeGen::GET_WIDE_KEY( RedStateAp *state )
{
	if ( state->stateCondList.length() > 0 )
		return L"_widec";
	else
		return GET_KEY();
}

wstring RubyCodeGen::GET_KEY()
{
	wostringstream ret;
	if ( getKeyExpr != 0 ) { 
		/* Emit the user supplied method of retrieving the key. */
		ret << L"(";
		INLINE_LIST( ret, getKeyExpr, 0, false );
		ret << L")";
	}
	else {
		/* Expression for retrieving the key, use dereference and read ordinal,
		 * for compatibility with Ruby 1.9. */
		ret << DATA() << L"[" << P() << L"].ord";
	}
	return ret.str();
}

wstring RubyCodeGen::KEY( Key key )
{
	wostringstream ret;
	if ( keyOps->isSigned || !hostLang->explicitUnsigned )
		ret << key.getVal();
	else
		ret << (unsigned long) key.getVal();
	return ret.str();
}


/* Write out level number of tabs. Makes the nested binary search nice
 * looking. */
wstring RubyCodeGen::TABS( int level )
{
	wstring result;
	while ( level-- > 0 )
		result += L"\t";
	return result;
}

wstring RubyCodeGen::INT( int i )
{
	wostringstream ret;
	ret << i;
	return ret.str();
}

void RubyCodeGen::CONDITION( wostream &ret, GenAction *condition )
{
	ret << L"\n";
	rubyLineDirective( ret, condition->loc.fileName, condition->loc.line );
	INLINE_LIST( ret, condition->inlineList, 0, false );
}

/* Emit the alphabet data type. */
wstring RubyCodeGen::ALPH_TYPE()
{
	wstring ret = keyOps->alphType->data1;
	if ( keyOps->alphType->data2 != 0 ) {
		ret += L" ";
		ret += + keyOps->alphType->data2;
	}
	return ret;
}

/* Emit the alphabet data type. */
wstring RubyCodeGen::WIDE_ALPH_TYPE()
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


wstring RubyCodeGen::ARRAY_TYPE( unsigned long maxVal )
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

/* Write out the array of actions. */
std::wostream &RubyCodeGen::ACTIONS_ARRAY()
{
	START_ARRAY_LINE();
	int totalActions = 0;
	ARRAY_ITEM( INT(0), ++totalActions, false );
	for ( GenActionTableMap::Iter act = redFsm->actionMap; act.lte(); act++ ) {
		/* Write out the length, which will never be the last character. */
		ARRAY_ITEM( INT(act->key.length()), ++totalActions, false );

		for ( GenActionTable::Iter item = act->key; item.lte(); item++ ) {
			ARRAY_ITEM( INT(item->value->actionId), ++totalActions, (act.last() && item.last()) );
		}
	}
	END_ARRAY_LINE();
	return out;
}

void RubyCodeGen::STATE_IDS()
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

std::wostream &RubyCodeGen::START_ARRAY_LINE()
{
	out << L"\t";
	return out;
}

std::wostream &RubyCodeGen::ARRAY_ITEM( wstring item, int count, bool last )
{
	out << item;
	if ( !last )
	{
		out << L", ";
		if ( count % IALL == 0 )
		{
			END_ARRAY_LINE();
			START_ARRAY_LINE();
		}
	}
	return out;
}

std::wostream &RubyCodeGen::END_ARRAY_LINE()
{
	out << L"\n";
	return out;
}

/* Emit the offset of the start state as a decimal integer. */
wstring RubyCodeGen::START_STATE_ID()
{
	wostringstream ret;
	ret << redFsm->startState->id;
	return ret.str();
};

wstring RubyCodeGen::ERROR_STATE()
{
	wostringstream ret;
	if ( redFsm->errState != 0 )
		ret << redFsm->errState->id;
	else
		ret << L"-1";
	return ret.str();
}

wstring RubyCodeGen::FIRST_FINAL_STATE()
{
	wostringstream ret;
	if ( redFsm->firstFinState != 0 )
		ret << redFsm->firstFinState->id;
	else
		ret << redFsm->nextStateId;
	return ret.str();
}

wstring RubyCodeGen::ACCESS()
{
	wostringstream ret;
	if ( accessExpr != 0 )
		INLINE_LIST( ret, accessExpr, 0, false );
	return ret.str();
}

/* Write out an inline tree structure. Walks the list and possibly calls out
 * to virtual functions than handle language specific items in the tree. */
void RubyCodeGen::INLINE_LIST( wostream &ret, GenInlineList *inlineList, 
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
			ret << P() << L" = " << P() << L" - 1;";
			break;
		case GenInlineItem::Exec:
			EXEC( ret, item, targState, inFinish );
			break;
		case GenInlineItem::Curs:
			ret << L"(_ps)";
			break;
		case GenInlineItem::Targs:
			ret << L"(" << vCS() << L")";
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


void RubyCodeGen::EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish )
{
	/* The parser gives fexec two children. The double brackets are for D
	 * code. If the inline list is a single word it will get interpreted as a
	 * C-style cast by the D compiler. */
	ret << L" begin " << P() << L" = ((";
	INLINE_LIST( ret, item->children, targState, inFinish );
	ret << L"))-1; end\n";
}

void RubyCodeGen::LM_SWITCH( wostream &ret, GenInlineItem *item, 
		int targState, int inFinish )
{
	ret << 
		L"	case " << ACT() << L"\n";

	for ( GenInlineList::Iter lma = *item->children; lma.lte(); lma++ ) {
		/* Write the case label, the action and the case break. */
		if ( lma->lmId < 0 )
			ret << L"	else\n";
		else
			ret << L"	when " << lma->lmId << L" then\n";


		/* Write the block and close it off. */
		ret << L"	begin";
		INLINE_LIST( ret, lma->children, targState, inFinish );
		ret << L"end\n";
	}

	ret << L"end \n\t";
}

void RubyCodeGen::SET_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = " << item->lmId << L";";
}

void RubyCodeGen::INIT_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << NULL_ITEM() << L";";
}

void RubyCodeGen::INIT_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = 0\n";
}

void RubyCodeGen::SET_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << P() << L"\n";
}

void RubyCodeGen::SET_TOKEND( wostream &ret, GenInlineItem *item )
{
	/* The tokend action sets tokend. */
	ret << TOKEND() << L" = " << P();
	if ( item->offset != 0 ) 
		out << L"+" << item->offset;
	out << L"\n";
}

void RubyCodeGen::GET_TOKEND( wostream &ret, GenInlineItem *item )
{
	ret << TOKEND();
}

void RubyCodeGen::SUB_ACTION( wostream &ret, GenInlineItem *item, 
		int targState, bool inFinish )
{
	if ( item->children->length() > 0 ) {
		/* Write the block and close it off. */
		ret << L" begin ";
		INLINE_LIST( ret, item->children, targState, inFinish );
		ret << L" end\n";
	}
}

int RubyCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	return act;
}

wostream &RubyCodeGen::source_warning( const InputLoc &loc )
{
	err() << sourceFileName << L":" << loc.line << L":" << loc.col << L": warning: ";
	return err();
}

wostream &RubyCodeGen::source_error( const InputLoc &loc )
{
	gblErrorCount += 1;
	assert( sourceFileName != 0 );
	err() << sourceFileName << L":" << loc.line << L":" << loc.col << L": ";
	return err();
}

void RubyCodeGen::finishRagelDef()
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


/* Determine if we should use indicies or not. */
void RubyCodeGen::calcIndexSize()
{
	int sizeWithInds = 0, sizeWithoutInds = 0;

	/* Calculate cost of using with indicies. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		int totalIndex = st->outSingle.length() + st->outRange.length() + 
				(st->defTrans == 0 ? 0 : 1);
		sizeWithInds += arrayTypeSize(redFsm->maxIndex) * totalIndex;
	}
	sizeWithInds += arrayTypeSize(redFsm->maxState) * redFsm->transSet.length();
	if ( redFsm->anyActions() )
		sizeWithInds += arrayTypeSize(redFsm->maxActionLoc) * redFsm->transSet.length();

	/* Calculate the cost of not using indicies. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		int totalIndex = st->outSingle.length() + st->outRange.length() + 
				(st->defTrans == 0 ? 0 : 1);
		sizeWithoutInds += arrayTypeSize(redFsm->maxState) * totalIndex;
		if ( redFsm->anyActions() )
			sizeWithoutInds += arrayTypeSize(redFsm->maxActionLoc) * totalIndex;
	}

	/* If using indicies reduces the size, use them. */
	useIndicies = sizeWithInds < sizeWithoutInds;
}

unsigned int RubyCodeGen::arrayTypeSize( unsigned long maxVal )
{
	long long maxValLL = (long long) maxVal;
	HostType *arrayType = keyOps->typeSubsumes( maxValLL );
	assert( arrayType != 0 );
	return arrayType->size;
}


void RubyCodeGen::writeInit()
{
	out << L"begin\n";
	
	out << L"	" << P() << L" ||= 0\n";

	if ( !noEnd ) 
		out << L"	" << PE() << L" ||= " << DATA() << L".length\n";

	if ( !noCS )
		out << L"	" << vCS() << L" = " << START() << L"\n";

	/* If there are any calls, then the stack top needs initialization. */
	if ( redFsm->anyActionCalls() || redFsm->anyActionRets() )
		out << L"	" << TOP() << L" = 0\n";

	if ( hasLongestMatch ) {
		out <<
			L"	" << TOKSTART() << L" = " << NULL_ITEM() << L"\n"
			L"	" << TOKEND() << L" = " << NULL_ITEM() << L"\n"
			L"	" << ACT() << L" = 0\n";
	}

	out << L"end\n";
}

void RubyCodeGen::writeExports()
{
	if ( exportList.length() > 0 ) {
		for ( ExportList::Iter ex = exportList; ex.lte(); ex++ ) {
			STATIC_VAR( ALPH_TYPE(), DATA_PREFIX() + L"ex_" + ex->name ) 
					<< L" = " << KEY(ex->key) << L"\n";
		}
		out << L"\n";
	}
}

void RubyCodeGen::writeStart()
{
	out << START_STATE_ID();
}

void RubyCodeGen::writeFirstFinal()
{
	out << FIRST_FINAL_STATE();
}

void RubyCodeGen::writeError()
{
	out << ERROR_STATE();
}


/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */
