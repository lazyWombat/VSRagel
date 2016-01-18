/*
 *  Copyright 2006-2007 Adrian Thurston <thurston@complang.org>
 *            2007 Colin Fleming <colin.fleming@caverock.com>
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
#include "javacodegen.h"
#include "redfsm.h"
#include "gendata.h"
#include <iomanip>
#include <sstream>

/* Integer array line length. */
#define IALL 12

/* Static array initialization item count 
 * (should be multiple of IALL). */
#define SAIIC 8184

#define _resume    1
#define _again     2
#define _eof_trans 3
#define _test_eof  4
#define _out       5

using std::setw;
using std::ios;
using std::wostringstream;
using std::wstring;
using std::wcerr;

using std::wistream;
using std::wifstream;
using std::wostream;
using std::ios;
using std::cin;
using std::wcout;
using std::wcerr;
using std::endl;
using std::setiosflags;

void javaLineDirective( wostream &out, const wchar_t *fileName, int line )
{
	/* Write the preprocessor line info for to the input file. */
	out << L"// line " << line  << L" \"";
	for ( const wchar_t *pc = fileName; *pc != 0; pc++ ) {
		if ( *pc == L'\\' )
			out << L"\\\\";
		else
			out << *pc;
	}
	out << L"\"\n";
}

void JavaTabCodeGen::genLineDirective( wostream &out )
{
	std::wstreambuf *sbuf = out.rdbuf();
	output_filter *filter = static_cast<output_filter*>(sbuf);
	javaLineDirective( out, filter->fileName, filter->line + 1 );
}

void JavaTabCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << gotoDest << L"; _goto_targ = " << _again << L"; " << 
			CTRL_FLOW() << L"continue _goto;}";
}

void JavaTabCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"{" << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L"); _goto_targ = " << _again << L"; " << CTRL_FLOW() << L"continue _goto;}";
}

void JavaTabCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = " << 
			callDest << L"; _goto_targ = " << _again << L"; " << CTRL_FLOW() << L"continue _goto;}";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void JavaTabCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, targState, inFinish );
	ret << L"); _goto_targ = " << _again << L"; " << CTRL_FLOW() << L"continue _goto;}";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void JavaTabCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << STACK() << L"[--" << TOP() << L"];";

	if ( postPopExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, postPopExpr, 0, false );
		ret << L"}";
	}

	ret << L"_goto_targ = " << _again << L"; " << CTRL_FLOW() << L"continue _goto;}";
}

void JavaTabCodeGen::BREAK( wostream &ret, int targState )
{
	ret << L"{ " << P() << L" += 1; _goto_targ = " << _out << L"; " << 
			CTRL_FLOW() << L" continue _goto;}";
}

void JavaTabCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void JavaTabCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}

void JavaTabCodeGen::EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish )
{
	/* The parser gives fexec two children. The double brackets are for D
	 * code. If the inline list is a single word it will get interpreted as a
	 * C-style cast by the D compiler. */
	ret << L"{" << P() << L" = ((";
	INLINE_LIST( ret, item->children, targState, inFinish );
	ret << L"))-1;}";
}

/* Write out an inline tree structure. Walks the list and possibly calls out
 * to virtual functions than handle language specific items in the tree. */
void JavaTabCodeGen::INLINE_LIST( wostream &ret, GenInlineList *inlineList, 
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

wstring JavaTabCodeGen::DATA_PREFIX()
{
	if ( !noPrefix )
		return FSM_NAME() + L"_";
	return L"";
}

/* Emit the alphabet data type. */
wstring JavaTabCodeGen::ALPH_TYPE()
{
	wstring ret = keyOps->alphType->data1;
	if ( keyOps->alphType->data2 != 0 ) {
		ret += L" ";
		ret += + keyOps->alphType->data2;
	}
	return ret;
}

/* Emit the alphabet data type. */
wstring JavaTabCodeGen::WIDE_ALPH_TYPE()
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



void JavaTabCodeGen::COND_TRANSLATE()
{
	out << 
		L"	_widec = " << GET_KEY() << L";\n"
		L"	_keys = " << CO() << L"[" << vCS() << L"]*2\n;"
		L"	_klen = " << CL() << L"[" << vCS() << L"];\n"
		L"	if ( _klen > 0 ) {\n"
		L"		int _lower = _keys\n;"
		L"		int _mid;\n"
		L"		int _upper = _keys + (_klen<<1) - 2;\n"
		L"		while (true) {\n"
		L"			if ( _upper < _lower )\n"
		L"				break;\n"
		L"\n"
		L"			_mid = _lower + (((_upper-_lower) >> 1) & ~1);\n"
		L"			if ( " << GET_WIDE_KEY() << L" < " << CK() << L"[_mid] )\n"
		L"				_upper = _mid - 2;\n"
		L"			else if ( " << GET_WIDE_KEY() << L" > " << CK() << L"[_mid+1] )\n"
		L"				_lower = _mid + 2;\n"
		L"			else {\n"
		L"				switch ( " << C() << L"[" << CO() << L"[" << vCS() << L"]"
							L" + ((_mid - _keys)>>1)] ) {\n"
		;

	for ( CondSpaceList::Iter csi = condSpaceList; csi.lte(); csi++ ) {
		GenCondSpace *condSpace = csi;
		out << L"	case " << condSpace->condSpaceId << L": {\n";
		out << TABS(2) << L"_widec = " << KEY(condSpace->baseKey) << 
				L" + (" << GET_KEY() << L" - " << KEY(keyOps->minKey) << L");\n";

		for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
			out << TABS(2) << L"if ( ";
			CONDITION( out, *csi );
			Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
			out << L" ) _widec += " << condValOffset << L";\n";
		}

		out << 
			L"		break;\n"
			L"	}\n";
	}

	out << 
		L"				}\n"
		L"				break;\n"
		L"			}\n"
		L"		}\n"
		L"	}\n"
		L"\n";
}


void JavaTabCodeGen::LOCATE_TRANS()
{
	out <<
		L"	_match: do {\n"
		L"	_keys = " << KO() << L"[" << vCS() << L"]" << L";\n"
		L"	_trans = " << IO() << L"[" << vCS() << L"];\n"
		L"	_klen = " << SL() << L"[" << vCS() << L"];\n"
		L"	if ( _klen > 0 ) {\n"
		L"		int _lower = _keys;\n"
		L"		int _mid;\n"
		L"		int _upper = _keys + _klen - 1;\n"
		L"		while (true) {\n"
		L"			if ( _upper < _lower )\n"
		L"				break;\n"
		L"\n"
		L"			_mid = _lower + ((_upper-_lower) >> 1);\n"
		L"			if ( " << GET_WIDE_KEY() << L" < " << K() << L"[_mid] )\n"
		L"				_upper = _mid - 1;\n"
		L"			else if ( " << GET_WIDE_KEY() << L" > " << K() << L"[_mid] )\n"
		L"				_lower = _mid + 1;\n"
		L"			else {\n"
		L"				_trans += (_mid - _keys);\n"
		L"				break _match;\n"
		L"			}\n"
		L"		}\n"
		L"		_keys += _klen;\n"
		L"		_trans += _klen;\n"
		L"	}\n"
		L"\n"
		L"	_klen = " << RL() << L"[" << vCS() << L"];\n"
		L"	if ( _klen > 0 ) {\n"
		L"		int _lower = _keys;\n"
		L"		int _mid;\n"
		L"		int _upper = _keys + (_klen<<1) - 2;\n"
		L"		while (true) {\n"
		L"			if ( _upper < _lower )\n"
		L"				break;\n"
		L"\n"
		L"			_mid = _lower + (((_upper-_lower) >> 1) & ~1);\n"
		L"			if ( " << GET_WIDE_KEY() << L" < " << K() << L"[_mid] )\n"
		L"				_upper = _mid - 2;\n"
		L"			else if ( " << GET_WIDE_KEY() << L" > " << K() << L"[_mid+1] )\n"
		L"				_lower = _mid + 2;\n"
		L"			else {\n"
		L"				_trans += ((_mid - _keys)>>1);\n"
		L"				break _match;\n"
		L"			}\n"
		L"		}\n"
		L"		_trans += _klen;\n"
		L"	}\n"
		L"	} while (false);\n"
		L"\n";
}

/* Determine if we should use indicies or not. */
void JavaTabCodeGen::calcIndexSize()
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

int JavaTabCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	return act;
}

int JavaTabCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	return act;
}

int JavaTabCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	return act;
}


int JavaTabCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	return act;
}

std::wostream &JavaTabCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &JavaTabCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &JavaTabCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, true );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &JavaTabCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &JavaTabCodeGen::COND_OFFSETS()
{
	int curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		ARRAY_ITEM( INT(curKeyOffset), st.last() );

		/* Move the key offset ahead. */
		curKeyOffset += st->stateCondList.length();
	}
	return out;
}

std::wostream &JavaTabCodeGen::KEY_OFFSETS()
{
	int curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		ARRAY_ITEM( INT(curKeyOffset), st.last() );

		/* Move the key offset ahead. */
		curKeyOffset += st->outSingle.length() + st->outRange.length()*2;
	}
	return out;
}


std::wostream &JavaTabCodeGen::INDEX_OFFSETS()
{
	int curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		ARRAY_ITEM( INT(curIndOffset), st.last() );

		/* Move the index offset ahead. */
		curIndOffset += st->outSingle.length() + st->outRange.length();
		if ( st->defTrans != 0 )
			curIndOffset += 1;
	}
	return out;
}

std::wostream &JavaTabCodeGen::COND_LENS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		ARRAY_ITEM( INT(st->stateCondList.length()), st.last() );
	}
	return out;
}


std::wostream &JavaTabCodeGen::SINGLE_LENS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		ARRAY_ITEM( INT(st->outSingle.length()), st.last() );
	}
	return out;
}

std::wostream &JavaTabCodeGen::RANGE_LENS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit length of range index. */
		ARRAY_ITEM( INT(st->outRange.length()), st.last() );
	}
	return out;
}

std::wostream &JavaTabCodeGen::TO_STATE_ACTIONS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT(TO_STATE_ACTION(st)), st.last() );
	}
	return out;
}

std::wostream &JavaTabCodeGen::FROM_STATE_ACTIONS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT(FROM_STATE_ACTION(st)), st.last() );
	}
	return out;
}

std::wostream &JavaTabCodeGen::EOF_ACTIONS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT(EOF_ACTION(st)), st.last() );
	}
	return out;
}

std::wostream &JavaTabCodeGen::EOF_TRANS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		long trans = 0;
		if ( st->eofTrans != 0 ) {
			assert( st->eofTrans->pos >= 0 );
			trans = st->eofTrans->pos+1;
		}

		/* Write any eof action. */
		ARRAY_ITEM( INT(trans), st.last() );
	}
	return out;
}


std::wostream &JavaTabCodeGen::COND_KEYS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Lower key. */
			ARRAY_ITEM( KEY( sc->lowKey ), false );
			ARRAY_ITEM( KEY( sc->highKey ), false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), true );
	return out;
}

std::wostream &JavaTabCodeGen::COND_SPACES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Cond Space id. */
			ARRAY_ITEM( KEY( sc->condSpace->condSpaceId ), false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), true );
	return out;
}

std::wostream &JavaTabCodeGen::KEYS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			ARRAY_ITEM( KEY( stel->lowKey ), false );
		}

		/* Loop the state's transitions. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			/* Lower key. */
			ARRAY_ITEM( KEY( rtel->lowKey ), false );

			/* Upper key. */
			ARRAY_ITEM( KEY( rtel->highKey ), false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), true );
	return out;
}

std::wostream &JavaTabCodeGen::INDICIES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			ARRAY_ITEM( KEY( stel->value->id ), false );
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			ARRAY_ITEM( KEY( rtel->value->id ), false );
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			ARRAY_ITEM( KEY( st->defTrans->id ), false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), true );
	return out;
}

std::wostream &JavaTabCodeGen::TRANS_TARGS()
{
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			ARRAY_ITEM( KEY( trans->targ->id ), false );
			totalTrans++;
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			ARRAY_ITEM( KEY( trans->targ->id ), false );
			totalTrans++;
		}

		/* The state's default target state. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			ARRAY_ITEM( KEY( trans->targ->id ), false );
			totalTrans++;
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			trans->pos = totalTrans++;
			ARRAY_ITEM( KEY( trans->targ->id ), false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), true );
	return out;
}


std::wostream &JavaTabCodeGen::TRANS_ACTIONS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), false );
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), false );
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), false );
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), true );
	return out;
}

std::wostream &JavaTabCodeGen::TRANS_TARGS_WI()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Save the position. Needed for eofTargs. */
		RedTransAp *trans = transPtrs[t];
		trans->pos = t;

		/* Write out the target state. */
		ARRAY_ITEM( INT(trans->targ->id), ( t >= redFsm->transSet.length()-1 ) );
	}
	delete[] transPtrs;
	return out;
}


std::wostream &JavaTabCodeGen::TRANS_ACTIONS_WI()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Write the function for the transition. */
		RedTransAp *trans = transPtrs[t];
		ARRAY_ITEM( INT(TRANS_ACTION( trans )), ( t >= redFsm->transSet.length()-1 ) );
	}
	delete[] transPtrs;
	return out;
}

void JavaTabCodeGen::writeExports()
{
	if ( exportList.length() > 0 ) {
		for ( ExportList::Iter ex = exportList; ex.lte(); ex++ ) {
			STATIC_VAR( ALPH_TYPE(), DATA_PREFIX() + L"ex_" + ex->name ) 
					<< L" = " << KEY(ex->key) << L";\n";
		}
		out << L"\n";
	}
}

void JavaTabCodeGen::writeStart()
{
	out << START_STATE_ID();
}

void JavaTabCodeGen::writeFirstFinal()
{
	out << FIRST_FINAL_STATE();
}

void JavaTabCodeGen::writeError()
{
	out << ERROR_STATE();
}

void JavaTabCodeGen::writeData()
{
	/* If there are any transtion functions then output the array. If there
	 * are none, donL't bother emitting an empty array that won't be used. */
	if ( redFsm->anyActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActArrItem), A() );
		ACTIONS_ARRAY();
		CLOSE_ARRAY() <<
		L"\n";
	}

	if ( redFsm->anyConditions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondOffset), CO() );
		COND_OFFSETS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondLen), CL() );
		COND_LENS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( WIDE_ALPH_TYPE(), CK() );
		COND_KEYS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondSpaceId), C() );
		COND_SPACES();
		CLOSE_ARRAY() <<
		L"\n";
	}

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxKeyOffset), KO() );
	KEY_OFFSETS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( WIDE_ALPH_TYPE(), K() );
	KEYS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxSingleLen), SL() );
	SINGLE_LENS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxRangeLen), RL() );
	RANGE_LENS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndexOffset), IO() );
	INDEX_OFFSETS();
	CLOSE_ARRAY() <<
	L"\n";

	if ( useIndicies ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndex), I() );
		INDICIES();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
		TRANS_TARGS_WI();
		CLOSE_ARRAY() <<
		L"\n";

		if ( redFsm->anyActions() ) {
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TA() );
			TRANS_ACTIONS_WI();
			CLOSE_ARRAY() <<
			L"\n";
		}
	}
	else {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
		TRANS_TARGS();
		CLOSE_ARRAY() <<
		L"\n";

		if ( redFsm->anyActions() ) {
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TA() );
			TRANS_ACTIONS();
			CLOSE_ARRAY() <<
			L"\n";
		}
	}

	if ( redFsm->anyToStateActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TSA() );
		TO_STATE_ACTIONS();
		CLOSE_ARRAY() <<
		L"\n";
	}

	if ( redFsm->anyFromStateActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), FSA() );
		FROM_STATE_ACTIONS();
		CLOSE_ARRAY() <<
		L"\n";
	}

	if ( redFsm->anyEofActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), EA() );
		EOF_ACTIONS();
		CLOSE_ARRAY() <<
		L"\n";
	}

	if ( redFsm->anyEofTrans() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndexOffset+1), ET() );
		EOF_TRANS();
		CLOSE_ARRAY() <<
		L"\n";
	}

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

void JavaTabCodeGen::writeExec()
{
	out <<
		L"	{\n"
		L"	int _klen";

	if ( redFsm->anyRegCurStateRef() )
		out << L", _ps";

	out << 
		L";\n"
		L"	int _trans = 0;\n";

	if ( redFsm->anyConditions() )
		out << L"	int _widec;\n";

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() || 
			redFsm->anyFromStateActions() )
	{
		out << 
			L"	int _acts;\n"
			L"	int _nacts;\n";
	}

	out <<
		L"	int _keys;\n"
		L"	int _goto_targ = 0;\n"
		L"\n";
	
	out <<
		L"	_goto: while (true) {\n"
		L"	switch ( _goto_targ ) {\n"
		L"	case 0:\n";

	if ( !noEnd ) {
		out << 
			L"	if ( " << P() << L" == " << PE() << L" ) {\n"
			L"		_goto_targ = " << _test_eof << L";\n"
			L"		continue _goto;\n"
			L"	}\n";
	}

	if ( redFsm->errState != 0 ) {
		out << 
			L"	if ( " << vCS() << L" == " << redFsm->errState->id << L" ) {\n"
			L"		_goto_targ = " << _out << L";\n"
			L"		continue _goto;\n"
			L"	}\n";
	}

	out << L"case " << _resume << L":\n"; 

	if ( redFsm->anyFromStateActions() ) {
		out <<
			L"	_acts = " << FSA() << L"[" << vCS() << L"]" << L";\n"
			L"	_nacts = " << CAST(L"int") << L" " << A() << L"[_acts++];\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( " << A() << L"[_acts++] ) {\n";
			FROM_STATE_ACTION_SWITCH() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

	LOCATE_TRANS();

	if ( useIndicies )
		out << L"	_trans = " << I() << L"[_trans];\n";
	
	if ( redFsm->anyEofTrans() )
		out << L"case " << _eof_trans << L":\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = " << vCS() << L";\n";

	out <<
		L"	" << vCS() << L" = " << TT() << L"[_trans];\n"
		L"\n";

	if ( redFsm->anyRegActions() ) {
		out <<
			L"	if ( " << TA() << L"[_trans] != 0 ) {\n"
			L"		_acts = " <<  TA() << L"[_trans]" << L";\n"
			L"		_nacts = " << CAST(L"int") << L" " <<  A() << L"[_acts++];\n"
			L"		while ( _nacts-- > 0 )\n	{\n"
			L"			switch ( " << A() << L"[_acts++] )\n"
			L"			{\n";
			ACTION_SWITCH() <<
			L"			}\n"
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	out << L"case " << _again << L":\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	_acts = " << TSA() << L"[" << vCS() << L"]" << L";\n"
			L"	_nacts = " << CAST(L"int") << L" " << A() << L"[_acts++];\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( " << A() << L"[_acts++] ) {\n";
			TO_STATE_ACTION_SWITCH() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->errState != 0 ) {
		out << 
			L"	if ( " << vCS() << L" == " << redFsm->errState->id << L" ) {\n"
			L"		_goto_targ = " << _out << L";\n"
			L"		continue _goto;\n"
			L"	}\n";
	}

	if ( !noEnd ) {
		out << 
			L"	if ( ++" << P() << L" != " << PE() << L" ) {\n"
			L"		_goto_targ = " << _resume << L";\n"
			L"		continue _goto;\n"
			L"	}\n";
	}
	else {
		out << 
			L"	" << P() << L" += 1;\n"
			L"	_goto_targ = " << _resume << L";\n"
			L"	continue _goto;\n";
	}

	out << L"case " << _test_eof << L":\n"; 

	if ( redFsm->anyEofTrans() || redFsm->anyEofActions() ) {
		out <<
			L"	if ( " << P() << L" == " << vEOF() << L" )\n"
			L"	{\n";

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"	if ( " << ET() << L"[" << vCS() << L"] > 0 ) {\n"
				L"		_trans = " << ET() << L"[" << vCS() << L"] - 1;\n"
				L"		_goto_targ = " << _eof_trans << L";\n"
				L"		continue _goto;\n"
				L"	}\n";
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	int __acts = " << EA() << L"[" << vCS() << L"]" << L";\n"
				L"	int __nacts = " << CAST(L"int") << L" " << A() << L"[__acts++];\n"
				L"	while ( __nacts-- > 0 ) {\n"
				L"		switch ( " << A() << L"[__acts++] ) {\n";
				EOF_ACTION_SWITCH() <<
				L"		}\n"
				L"	}\n";
		}

		out <<
			L"	}\n"
			L"\n";
	}

	out << L"case " << _out << L":\n"; 

	/* The switch and goto loop. */
	out << L"	}\n";
	out << L"	break; }\n";

	/* The execute block. */
	out << L"	}\n";
}

std::wostream &JavaTabCodeGen::OPEN_ARRAY( wstring type, wstring name )
{
	array_type = type;
	array_name = name;
	item_count = 0;
	div_count = 1;

	out <<  L"private static " << type << L"[] init_" << name << L"_0()\n"
		L"{\n\t"
		L"return new " << type << L" [] {\n\t";
	return out;
}

std::wostream &JavaTabCodeGen::ARRAY_ITEM( wstring item, bool last )
{
	item_count++;

	out << setw(5) << setiosflags(ios::right) << item;
	
	if ( !last ) {
		if ( item_count % SAIIC == 0 ) {
			out << L"\n\t};\n};\n"
				L"private static "<< array_type << L"[] init_" << 
				array_name << L"_" << div_count << L"()\n"
				L"{\n\t"
				L"return new " << array_type << L" [] {\n\t";
			div_count++;
		} else if (item_count % IALL == 0) { 
			out << L",\n\t";
		} else {
			out << L",";
		}
	}
	return out;
}

std::wostream &JavaTabCodeGen::CLOSE_ARRAY()
{
	out << L"\n\t};\n}\n\n";

	if (item_count < SAIIC) {
		out << L"private static final " << array_type << L" " << array_name << 
			L"[] = init_" << array_name << L"_0();\n\n";
	} else {
		out << L"private static final " << array_type << L" [] combine_" << array_name
			<< L"() {\n\t"
			<< array_type << L" [] combined = new " << array_type << 
			L" [ " << item_count << L" ];\n\t";
		int block = 0;
		int full_blocks = item_count / SAIIC;
		for (;block < full_blocks; ++block) {
			out << L"System.arraycopy ( init_" << array_name << L"_" << block << 
				L"(), 0, combined, " << SAIIC * block << L", " << SAIIC << L" );\n\t";
		}
		if ( (item_count % SAIIC) > 0 ) {
			out << L"System.arraycopy ( init_" << array_name << L"_" << block << 
				L"(), 0, combined, " << SAIIC * block << L", " << 
				(item_count % SAIIC) << L" );\n\t";
		}
		out << L"return combined;\n}\n";
		out << L"private static final " << array_type << L" [] " << array_name << 
			L" = combine_" << array_name << L"();";
	}
	return out;
}


std::wostream &JavaTabCodeGen::STATIC_VAR( wstring type, wstring name )
{
	out << L"static final " << type << L" " << name;
	return out;
}

wstring JavaTabCodeGen::ARR_OFF( wstring ptr, wstring offset )
{
	return ptr + L" + " + offset;
}

wstring JavaTabCodeGen::CAST( wstring type )
{
	return L"(" + type + L")";
}

wstring JavaTabCodeGen::NULL_ITEM()
{
	/* In java we use integers instead of pointers. */
	return L"-1";
}

wstring JavaTabCodeGen::GET_KEY()
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
		ret << DATA() << L"[" << P() << L"]";
	}
	return ret.str();
}

wstring JavaTabCodeGen::CTRL_FLOW()
{
	return L"if (true) ";
}

unsigned int JavaTabCodeGen::arrayTypeSize( unsigned long maxVal )
{
	long long maxValLL = (long long) maxVal;
	HostType *arrayType = keyOps->typeSubsumes( maxValLL );
	assert( arrayType != 0 );
	return arrayType->size;
}

wstring JavaTabCodeGen::ARRAY_TYPE( unsigned long maxVal )
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
wstring JavaTabCodeGen::FSM_NAME()
{
	return fsmName;
}

/* Emit the offset of the start state as a decimal integer. */
wstring JavaTabCodeGen::START_STATE_ID()
{
	wostringstream ret;
	ret << redFsm->startState->id;
	return ret.str();
};

/* Write out the array of actions. */
std::wostream &JavaTabCodeGen::ACTIONS_ARRAY()
{
	ARRAY_ITEM( INT(0), false );
	for ( GenActionTableMap::Iter act = redFsm->actionMap; act.lte(); act++ ) {
		/* Write out the length, which will never be the last character. */
		ARRAY_ITEM( INT(act->key.length()), false );

		for ( GenActionTable::Iter item = act->key; item.lte(); item++ )
			ARRAY_ITEM( INT(item->value->actionId), (act.last() && item.last()) );
	}
	return out;
}


wstring JavaTabCodeGen::ACCESS()
{
	wostringstream ret;
	if ( accessExpr != 0 )
		INLINE_LIST( ret, accessExpr, 0, false );
	return ret.str();
}

wstring JavaTabCodeGen::P()
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

wstring JavaTabCodeGen::PE()
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

wstring JavaTabCodeGen::vEOF()
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

wstring JavaTabCodeGen::vCS()
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

wstring JavaTabCodeGen::TOP()
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

wstring JavaTabCodeGen::STACK()
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

wstring JavaTabCodeGen::ACT()
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

wstring JavaTabCodeGen::TOKSTART()
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

wstring JavaTabCodeGen::TOKEND()
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

wstring JavaTabCodeGen::DATA()
{
	wostringstream ret;
	if ( dataExpr == 0 )
		ret << ACCESS() + L"data";
	else {
		ret << L"(";
		INLINE_LIST( ret, dataExpr, 0, false );
		ret << L")";
	}
	return ret.str();
}


wstring JavaTabCodeGen::GET_WIDE_KEY()
{
	if ( redFsm->anyConditions() ) 
		return L"_widec";
	else
		return GET_KEY();
}

wstring JavaTabCodeGen::GET_WIDE_KEY( RedStateAp *state )
{
	if ( state->stateCondList.length() > 0 )
		return L"_widec";
	else
		return GET_KEY();
}

/* Write out level number of tabs. Makes the nested binary search nice
 * looking. */
wstring JavaTabCodeGen::TABS( int level )
{
	wstring result;
	while ( level-- > 0 )
		result += L"\t";
	return result;
}

wstring JavaTabCodeGen::KEY( Key key )
{
	wostringstream ret;
	if ( keyOps->isSigned || !hostLang->explicitUnsigned )
		ret << key.getVal();
	else
		ret << (unsigned long) key.getVal();
	return ret.str();
}

wstring JavaTabCodeGen::INT( int i )
{
	wostringstream ret;
	ret << i;
	return ret.str();
}

void JavaTabCodeGen::LM_SWITCH( wostream &ret, GenInlineItem *item, 
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

void JavaTabCodeGen::SET_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = " << item->lmId << L";";
}

void JavaTabCodeGen::SET_TOKEND( wostream &ret, GenInlineItem *item )
{
	/* The tokend action sets tokend. */
	ret << TOKEND() << L" = " << P();
	if ( item->offset != 0 ) 
		out << L"+" << item->offset;
	out << L";";
}

void JavaTabCodeGen::GET_TOKEND( wostream &ret, GenInlineItem *item )
{
	ret << TOKEND();
}

void JavaTabCodeGen::INIT_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << NULL_ITEM() << L";";
}

void JavaTabCodeGen::INIT_ACT( wostream &ret, GenInlineItem *item )
{
	ret << ACT() << L" = 0;";
}

void JavaTabCodeGen::SET_TOKSTART( wostream &ret, GenInlineItem *item )
{
	ret << TOKSTART() << L" = " << P() << L";";
}

void JavaTabCodeGen::SUB_ACTION( wostream &ret, GenInlineItem *item, 
		int targState, bool inFinish )
{
	if ( item->children->length() > 0 ) {
		/* Write the block and close it off. */
		ret << L"{";
		INLINE_LIST( ret, item->children, targState, inFinish );
		ret << L"}";
	}
}

void JavaTabCodeGen::ACTION( wostream &ret, GenAction *action, int targState, bool inFinish )
{
	/* Write the preprocessor line info for going into the source file. */
	javaLineDirective( ret, action->loc.fileName, action->loc.line );

	/* Write the block and close it off. */
	ret << L"\t{";
	INLINE_LIST( ret, action->inlineList, targState, inFinish );
	ret << L"}\n";
}

void JavaTabCodeGen::CONDITION( wostream &ret, GenAction *condition )
{
	ret << L"\n";
	javaLineDirective( ret, condition->loc.fileName, condition->loc.line );
	INLINE_LIST( ret, condition->inlineList, 0, false );
}

wstring JavaTabCodeGen::ERROR_STATE()
{
	wostringstream ret;
	if ( redFsm->errState != 0 )
		ret << redFsm->errState->id;
	else
		ret << L"-1";
	return ret.str();
}

wstring JavaTabCodeGen::FIRST_FINAL_STATE()
{
	wostringstream ret;
	if ( redFsm->firstFinState != 0 )
		ret << redFsm->firstFinState->id;
	else
		ret << redFsm->nextStateId;
	return ret.str();
}

void JavaTabCodeGen::writeInit()
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

void JavaTabCodeGen::finishRagelDef()
{
	/* The frontend will do this for us, but it may be a good idea to force it
	 * if the intermediate file is edited. */
	redFsm->sortByStateId();

	/* Choose default transitions and the single transition. */
	redFsm->chooseDefaultSpan();
		
	/* Maybe do flat expand, otherwise choose single. */
	redFsm->chooseSingle();

	/* If any errors have occured in the input file then don't write anything. */
	if ( gblErrorCount > 0 )
		return;
	
	/* Anlayze Machine will find the final action reference counts, among
	 * other things. We will use these in reporting the usage
	 * of fsm directives in action code. */
	analyzeMachine();

	/* Determine if we should use indicies. */
	calcIndexSize();
}

wostream &JavaTabCodeGen::source_warning( const InputLoc &loc )
{
	wcerr << sourceFileName << L":" << loc.line << L":" << loc.col << L": warning: ";
	return wcerr;
}

wostream &JavaTabCodeGen::source_error( const InputLoc &loc )
{
	gblErrorCount += 1;
	assert( sourceFileName != 0 );
	wcerr << sourceFileName << L":" << loc.line << L":" << loc.col << L": ";
	return wcerr;
}


