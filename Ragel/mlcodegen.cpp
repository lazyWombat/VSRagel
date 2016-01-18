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
#include "mlcodegen.h"
#include "redfsm.h"
#include "gendata.h"
#include <sstream>
#include <iomanip>
#include <string>
#include <assert.h>

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

void ocamlLineDirective( wostream &out, const wchar_t *fileName, int line )
{
	if ( noLineDirectives )
		return;

	/* Write the line info for to the input file. */
	out << L"# " << line << L" \"";
	for ( const wchar_t *pc = fileName; *pc != 0; pc++ ) {
		if ( *pc == L'\\' || *pc == L'"' )
			out << L"\\";
  	out << *pc;
	}
	out << L"\"\n";
}

void OCamlCodeGen::genLineDirective( wostream &out )
{
	std::wstreambuf *sbuf = out.rdbuf();
	output_filter *filter = static_cast<output_filter*>(sbuf);
	ocamlLineDirective( out, filter->fileName, filter->line + 1 );
}


/* Init code gen with in parameters. */
OCamlCodeGen::OCamlCodeGen( wostream &out )
:
	CodeGenData(out)
{
}

unsigned int OCamlCodeGen::arrayTypeSize( unsigned long maxVal )
{
	long long maxValLL = (long long) maxVal;
	HostType *arrayType = keyOps->typeSubsumes( maxValLL );
	assert( arrayType != 0 );
	return arrayType->size;
}

wstring OCamlCodeGen::ARRAY_TYPE( unsigned long maxVal )
{
	return ARRAY_TYPE( maxVal, false );
}

wstring OCamlCodeGen::ARRAY_TYPE( unsigned long maxVal, bool forceSigned )
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
wstring OCamlCodeGen::FSM_NAME()
{
	return fsmName;
}

/* Emit the offset of the start state as a decimal integer. */
wstring OCamlCodeGen::START_STATE_ID()
{
	wostringstream ret;
	ret << redFsm->startState->id;
	return ret.str();
};

/* Write out the array of actions. */
std::wostream &OCamlCodeGen::ACTIONS_ARRAY()
{
	out << L"\t0; ";
	int totalActions = 1;
	for ( GenActionTableMap::Iter act = redFsm->actionMap; act.lte(); act++ ) {
		/* Write out the length, which will never be the last character. */
		out << act->key.length() << ARR_SEP();
		/* Put in a line break every 8 */
		if ( totalActions++ % 8 == 7 )
			out << L"\n\t";

		for ( GenActionTable::Iter item = act->key; item.lte(); item++ ) {
			out << item->value->actionId;
			if ( ! (act.last() && item.last()) )
				out << ARR_SEP();

			/* Put in a line break every 8 */
			if ( totalActions++ % 8 == 7 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


/*
wstring OCamlCodeGen::ACCESS()
{
	wostringstream ret;
	if ( accessExpr != 0 )
		INLINE_LIST( ret, accessExpr, 0, false );
	return ret.str();
}
*/

wstring OCamlCodeGen::make_access(wchar_t const* name, GenInlineList* x, bool prefix = true)
{ 
	wostringstream ret;
	if ( x == 0 )
  {
    if (prefix && accessExpr != 0)
    {
      INLINE_LIST( ret, accessExpr, 0, false);
      ret << name;
    }
    else
      ret << name << L".contents"; // ref cell
  }
	else {
		ret << L"(";
		INLINE_LIST( ret, x, 0, false );
		ret << L")";
	}
	return ret.str();
}

wstring OCamlCodeGen::P() { return make_access(L"p", pExpr, false); }
wstring OCamlCodeGen::PE() { return make_access(L"pe", peExpr, false); }
wstring OCamlCodeGen::vEOF() { return make_access(L"eof", eofExpr, false); }
wstring OCamlCodeGen::vCS() { return make_access(L"cs", csExpr); }
wstring OCamlCodeGen::TOP() { return make_access(L"top", topExpr); }
wstring OCamlCodeGen::STACK() { return make_access(L"stack", stackExpr); }
wstring OCamlCodeGen::ACT() { return make_access(L"act", actExpr); }
wstring OCamlCodeGen::TOKSTART() { return make_access(L"ts", tokstartExpr); }
wstring OCamlCodeGen::TOKEND() { return make_access(L"te", tokendExpr); }

wstring OCamlCodeGen::GET_WIDE_KEY()
{
	if ( redFsm->anyConditions() ) 
		return L"_widec";
	else
    { wostringstream ret; ret << L"Char.code " << GET_KEY(); return ret.str(); }
}

wstring OCamlCodeGen::GET_WIDE_KEY( RedStateAp *state )
{
	if ( state->stateCondList.length() > 0 )
		return L"_widec";
	else
    { wostringstream ret; ret << L"Char.code " << GET_KEY(); return ret.str(); }
}

/* Write out level number of tabs. Makes the nested binary search nice
 * looking. */
wstring OCamlCodeGen::TABS( int level )
{
	wstring result;
	while ( level-- > 0 )
		result += L"\t";
	return result;
}

/* Write out a key from the fsm code gen. Depends on wether or not the key is
 * signed. */
wstring OCamlCodeGen::KEY( Key key )
{
	wostringstream ret;
	if ( keyOps->isSigned || !hostLang->explicitUnsigned )
		ret << key.getVal();
	else
		ret << (unsigned long) key.getVal() << L'u';
	return ret.str();
}

wstring OCamlCodeGen::ALPHA_KEY( Key key )
{
	wostringstream ret;
  ret << key.getVal();
  /*
	if (key.getVal() > 0xFFFF) {
		ret << key.getVal();
	} else {
		ret << L"'\\u" << std::hex << std::setw(4) << std::setfill(L'0') << 
			key.getVal() << L"'";
	}
  */
	//ret << L"(char) " << key.getVal();
	return ret.str();
}

void OCamlCodeGen::EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish )
{
// The parser gives fexec two children.
	ret << L"begin " << P() << L" <- ";
	INLINE_LIST( ret, item->children, targState, inFinish );
	ret << L" - 1 end; ";
}

void OCamlCodeGen::LM_SWITCH( wostream &ret, GenInlineItem *item, 
		int targState, int inFinish )
{
	bool catch_all = false;
	ret << 
		L"	begin match " << ACT() << L" with\n";

	for ( GenInlineList::Iter lma = *item->children; lma.lte(); lma++ ) {
		/* Write the case label, the action and the case break. */
		if ( lma->lmId < 0 )
		{
			catch_all = true;
			ret << L"	| _ ->\n";
		}
		else
			ret << L"	| " << lma->lmId << L" ->\n";

		/* Write the block and close it off. */
		ret << L"	begin ";
		INLINE_LIST( ret, lma->children, targState, inFinish );
		ret << L" end\n";
	}

	if (!catch_all)
		ret << L"  | _ -> assert false\n";

	ret << 
		L"	end;\n"
		L"\t";
}

void OCamlCodeGen::SET_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" <- " << item->lmId << L"; ";
}

void OCamlCodeGen::SET_TOKEND( wostream &ret, GenInlineItem *item )
{
	/* The tokend action sets tokend. */
	ret << TOKEND() << L" <- " << P();
	if ( item->offset != 0 ) 
		out << L"+" << item->offset;
	out << L"; ";
}

void OCamlCodeGen::GET_TOKEND( wostream &ret, GenInlineItem *item )
{
	ret << TOKEND();
}

void OCamlCodeGen::INIT_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" <- " << NULL_ITEM() << L"; ";
}

void OCamlCodeGen::INIT_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" <- 0;";
}

void OCamlCodeGen::SET_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" <- " << P() << L"; ";
}

void OCamlCodeGen::SUB_ACTION( wostream &ret, GenInlineItem *item, 
		int targState, bool inFinish )
{
	if ( item->children->length() > 0 ) {
		/* Write the block and close it off. */
		ret << L"begin ";
		INLINE_LIST( ret, item->children, targState, inFinish );
		ret << L" end";
	}
}


/* Write out an inline tree structure. Walks the list and possibly calls out
 * to virtual functions than handle language specific items in the tree. */
void OCamlCodeGen::INLINE_LIST( wostream &ret, GenInlineList *inlineList, 
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
			ret << P() << L" <- " << P() << L" - 1; ";
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
wstring OCamlCodeGen::LDIR_PATH( wchar_t *path )
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

void OCamlCodeGen::ACTION( wostream &ret, GenAction *action, int targState, bool inFinish )
{
	/* Write the preprocessor line info for going into the source file. */
	ocamlLineDirective( ret, action->loc.fileName, action->loc.line );

	/* Write the block and close it off. */
	ret << L"\t\tbegin ";
	INLINE_LIST( ret, action->inlineList, targState, inFinish );
	ret << L" end;\n";
}

void OCamlCodeGen::CONDITION( wostream &ret, GenAction *condition )
{
	ret << L"\n";
	ocamlLineDirective( ret, condition->loc.fileName, condition->loc.line );
	INLINE_LIST( ret, condition->inlineList, 0, false );
}

wstring OCamlCodeGen::ERROR_STATE()
{
	wostringstream ret;
	if ( redFsm->errState != 0 )
		ret << redFsm->errState->id;
	else
		ret << L"-1";
	return ret.str();
}

wstring OCamlCodeGen::FIRST_FINAL_STATE()
{
	wostringstream ret;
	if ( redFsm->firstFinState != 0 )
		ret << redFsm->firstFinState->id;
	else
		ret << redFsm->nextStateId;
	return ret.str();
}

void OCamlCodeGen::writeInit()
{
	out << L"	begin\n";

	if ( !noCS )
		out << L"\t" << vCS() << L" <- " << START() << L";\n";
	
	/* If there are any calls, then the stack top needs initialization. */
	if ( redFsm->anyActionCalls() || redFsm->anyActionRets() )
		out << L"\t" << TOP() << L" <- 0;\n";

	if ( hasLongestMatch ) {
		out << 
			L"	" << TOKSTART() << L" <- " << NULL_ITEM() << L";\n"
			L"	" << TOKEND() << L" <- " << NULL_ITEM() << L";\n"
			L"	" << ACT() << L" <- 0;\n";
	}
	out << L"	end;\n";
}

wstring OCamlCodeGen::PRE_INCR(wstring val)
{
  wostringstream ret;
  ret << L"(" << val << L" <- " << val << L" + 1; " << val << L")";
  return ret.str();
}

wstring OCamlCodeGen::POST_INCR(wstring val)
{
  wostringstream ret;
  ret << L"(let temp = " << val << L" in " << val << L" <- " << val << L" + 1; temp)";
  return ret.str();
}

wstring OCamlCodeGen::PRE_DECR(wstring val)
{
  wostringstream ret;
  ret << L"(" << val << L" <- " << val << L" - 1; " << val << L")";
  return ret.str();
}

wstring OCamlCodeGen::POST_DECR(wstring val)
{
  wostringstream ret;
  ret << L"(let temp = " << val << L" in " << val << L" <- " << val << L" - 1; temp)";
  return ret.str();
}

wstring OCamlCodeGen::DATA_PREFIX()
{
  if ( data_prefix.empty() ) // init
  {
    data_prefix = wstring(fsmName) + L"_";
    if (data_prefix.size() > 0)
      data_prefix[0] = ::tolower(data_prefix[0]); // uncapitalize
  }
	if ( !noPrefix )
		return data_prefix;
	return L"";
}

/* Emit the alphabet data type. */
wstring OCamlCodeGen::ALPH_TYPE()
{
	wstring ret = keyOps->alphType->data1;
	if ( keyOps->alphType->data2 != 0 ) {
		ret += L" ";
		ret += + keyOps->alphType->data2;
	}
	return ret;
}

/* Emit the alphabet data type. */
wstring OCamlCodeGen::WIDE_ALPH_TYPE()
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

void OCamlCodeGen::STATE_IDS()
{
	if ( redFsm->startState != 0 )
		STATIC_VAR( L"int", START() ) << L" = " << START_STATE_ID() << TOP_SEP ();

	if ( !noFinal )
		STATIC_VAR( L"int" , FIRST_FINAL() ) << L" = " << FIRST_FINAL_STATE() << TOP_SEP();

	if ( !noError )
		STATIC_VAR( L"int", ERROR() ) << L" = " << ERROR_STATE() << TOP_SEP();

	out << L"\n";

	if ( entryPointNames.length() > 0 ) {
		for ( EntryNameVect::Iter en = entryPointNames; en.lte(); en++ ) {
			STATIC_VAR( L"int", DATA_PREFIX() + L"en_" + *en ) << 
					L" = " << entryPointIds[en.pos()] << TOP_SEP();
		}
		out << L"\n";
	}
}


void OCamlCodeGen::writeStart()
{
	out << START_STATE_ID();
}

void OCamlCodeGen::writeFirstFinal()
{
	out << FIRST_FINAL_STATE();
}

void OCamlCodeGen::writeError()
{
	out << ERROR_STATE();
}

wstring OCamlCodeGen::GET_KEY()
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
		ret << L"data.[" << P() << L"]";
	}
	return ret.str();
}
wstring OCamlCodeGen::NULL_ITEM()
{
	return L"-1";
}

wstring OCamlCodeGen::POINTER()
{
	// XXX C# has no pointers
	// multiple items seperated by commas can also be pointer types.
	return L" ";
}

wstring OCamlCodeGen::PTR_CONST()
{
	return L"";
}

std::wostream &OCamlCodeGen::OPEN_ARRAY( wstring type, wstring name )
{
	out << L"let " << name << L" : " << type << L" array = [|" << endl;
	return out;
}

std::wostream &OCamlCodeGen::CLOSE_ARRAY()
{
	return out << L"|]" << TOP_SEP();
}

wstring OCamlCodeGen::TOP_SEP()
{
  return L"\n"; // original syntax
}

wstring OCamlCodeGen::ARR_SEP()
{
  return L"; ";
}

wstring OCamlCodeGen::AT(const wstring& array, const wstring& index)
{
  wostringstream ret;
  ret << array << L".(" << index << L")";
  return ret.str();
}

std::wostream &OCamlCodeGen::STATIC_VAR( wstring type, wstring name )
{
	out << L"let " << name << L" : " << type;
	return out;
}

wstring OCamlCodeGen::ARR_OFF( wstring ptr, wstring offset )
{
	// XXX C# can't do pointer arithmetic
	return L"&" + ptr + L"[" + offset + L"]";
}

wstring OCamlCodeGen::CAST( wstring type )
{
  return L"";
//	return L"(" + type + L")";
}

wstring OCamlCodeGen::UINT( )
{
	return L"uint";
}

std::wostream &OCamlCodeGen::SWITCH_DEFAULT()
{
	out << L"		| _ -> ()\n";
	return out;
}

wstring OCamlCodeGen::CTRL_FLOW()
{
	return L"if true then ";
}

void OCamlCodeGen::finishRagelDef()
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

wostream &OCamlCodeGen::source_warning( const InputLoc &loc )
{
	err() << sourceFileName << L":" << loc.line << L":" << loc.col << L": warning: ";
	return err();
}

wostream &OCamlCodeGen::source_error( const InputLoc &loc )
{
	gblErrorCount += 1;
	assert( sourceFileName != 0 );
	err() << sourceFileName << L":" << loc.line << L":" << loc.col << L": ";
	return err();
}

