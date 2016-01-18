/*
 *  Copyright 2004-2006 Adrian Thurston <thurston@complang.org>
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

#include <sstream>
#include "ragel.h"
#include "mlflat.h"
#include "redfsm.h"
#include "gendata.h"

std::wostream &OCamlFlatCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	out << act;
	return out;
}

std::wostream &OCamlFlatCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	out << act;
	return out;
}

std::wostream &OCamlFlatCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	out << act;
	return out;
}

std::wostream &OCamlFlatCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	out << act;
	return out;
}

std::wostream &OCamlFlatCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, false );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlFlatCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, false );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlFlatCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, true );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &OCamlFlatCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, false );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &OCamlFlatCodeGen::FLAT_INDEX_OFFSET()
{
	out << L"\t";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset;
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}

		/* Move the index offset ahead. */
		if ( st->transList != 0 )
			curIndOffset += keyOps->span( st->lowKey, st->highKey );

		if ( st->defTrans != 0 )
			curIndOffset += 1;
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::KEY_SPANS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		unsigned long long span = 0;
		if ( st->transList != 0 )
			span = keyOps->span( st->lowKey, st->highKey );
		out << span;
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::TO_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		TO_STATE_ACTION(st);
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::FROM_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		FROM_STATE_ACTION(st);
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::EOF_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		EOF_ACTION(st);
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::EOF_TRANS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */

		long trans = 0;
		if ( st->eofTrans != 0 ) {
			assert( st->eofTrans->pos >= 0 );
			trans = st->eofTrans->pos+1;
		}
		out << trans;

		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


std::wostream &OCamlFlatCodeGen::COND_KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just cond low key and cond high key. */
		out << ALPHA_KEY( st->condLowKey ) << ARR_SEP();
		out << ALPHA_KEY( st->condHighKey ) << ARR_SEP();
		if ( ++totalTrans % IALL == 0 )
			out << L"\n\t";
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << /*L"(char) " <<*/ 0 << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::COND_KEY_SPANS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		unsigned long long span = 0;
		if ( st->condList != 0 )
			span = keyOps->span( st->condLowKey, st->condHighKey );
		out << span;
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::CONDS()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->condList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->condLowKey, st->condHighKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				if ( st->condList[pos] != 0 )
					out << st->condList[pos]->condSpaceId + 1 << ARR_SEP();
				else
					out << L"0" << ARR_SEP();
				if ( ++totalTrans % IALL == 0 )
					out << L"\n\t";
			}
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::COND_INDEX_OFFSET()
{
	out << L"\t";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset;
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}

		/* Move the index offset ahead. */
		if ( st->condList != 0 )
			curIndOffset += keyOps->span( st->condLowKey, st->condHighKey );
	}
	out << L"\n";
	return out;
}


std::wostream &OCamlFlatCodeGen::KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just low key and high key. */
		out << ALPHA_KEY( st->lowKey ) << ARR_SEP();
		out << ALPHA_KEY( st->highKey ) << ARR_SEP();
		if ( ++totalTrans % IALL == 0 )
			out << L"\n\t";
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << /*L"(char) " <<*/ 0 << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::INDICIES()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->transList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->lowKey, st->highKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				out << st->transList[pos]->id << ARR_SEP();
				if ( ++totalTrans % IALL == 0 )
					out << L"\n\t";
			}
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 )
			out << st->defTrans->id << ARR_SEP();

		if ( ++totalTrans % IALL == 0 )
			out << L"\n\t";
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &OCamlFlatCodeGen::TRANS_TARGS()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	out << L'\t';
	int totalStates = 0;
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Record the position, need this for eofTrans. */
		RedTransAp *trans = transPtrs[t];
		trans->pos = t;

		/* Write out the target state. */
		out << trans->targ->id;
		if ( t < redFsm->transSet.length()-1 ) {
			out << ARR_SEP();
			if ( ++totalStates % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}


std::wostream &OCamlFlatCodeGen::TRANS_ACTIONS()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	out << L'\t';
	int totalAct = 0;
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Write the function for the transition. */
		RedTransAp *trans = transPtrs[t];
		TRANS_ACTION( trans );
		if ( t < redFsm->transSet.length()-1 ) {
			out << ARR_SEP();
			if ( ++totalAct % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}

void OCamlFlatCodeGen::LOCATE_TRANS()
{
  std::wostringstream temp;
  temp << L"inds + (\n"
		L"		if slen > 0 && " << AT( K(), L"keys" ) << L" <= " << GET_WIDE_KEY() << L" &&\n"
		L"		" << GET_WIDE_KEY() << L" <= " << AT( K(), L"keys+1" ) << L" then\n"
		L"		" << GET_WIDE_KEY() << L" - " << AT(K(), L"keys" ) << L" else slen)";
	out <<
		L"	let keys = " << vCS() << L" lsl 1 in\n"
		L"	let inds = " << AT( IO(), vCS() ) << L" in\n"
		L"\n"
		L"	let slen = " << AT( SP(), vCS() ) << L" in\n"
		L"	state.trans <- " << AT( I(), temp.str() ) << L";\n"
		L"\n";
}

void OCamlFlatCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- " << gotoDest << L"; " << 
			CTRL_FLOW() << L"raise Goto_again end";
}

void OCamlFlatCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L"); " << CTRL_FLOW() << L" raise Goto_again end";
}

void OCamlFlatCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void OCamlFlatCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void OCamlFlatCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" <- " << nextDest << L";";
}

void OCamlFlatCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" <- (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}

void OCamlFlatCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"begin ";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"begin " << AT( STACK(), POST_INCR(TOP()) ) << L" <- " << vCS() << L"; ";
  ret << vCS() << L" <- " << callDest << L"; " << CTRL_FLOW() << L"raise Goto_again end ";

	if ( prePushExpr != 0 )
		ret << L"end";
}

void OCamlFlatCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"begin ";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"begin " << AT(STACK(), POST_INCR(TOP()) ) << L" <- " << vCS() << L"; " << vCS() << L" <- (";
	INLINE_LIST( ret, ilItem->children, targState, inFinish );
	ret << L"); " << CTRL_FLOW() << L"raise Goto_again end ";

	if ( prePushExpr != 0 )
		ret << L"end";
}

void OCamlFlatCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- " << AT(STACK(), PRE_DECR(TOP()) ) << L"; ";

	if ( postPopExpr != 0 ) {
		ret << L"begin ";
		INLINE_LIST( ret, postPopExpr, 0, false );
		ret << L"end ";
	}

	ret << CTRL_FLOW() <<  L"raise Goto_again end";
}

void OCamlFlatCodeGen::BREAK( wostream &ret, int targState )
{
	outLabelUsed = true;
	ret << L"begin " << P() << L" <- " << P() << L" + 1; " << CTRL_FLOW() << L"raise Goto_out end";
}

void OCamlFlatCodeGen::writeData()
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
		OPEN_ARRAY( WIDE_ALPH_TYPE(), CK() );
		COND_KEYS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondSpan), CSP() );
		COND_KEY_SPANS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCond), C() );
		CONDS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondIndexOffset), CO() );
		COND_INDEX_OFFSET();
		CLOSE_ARRAY() <<
		L"\n";
	}

	OPEN_ARRAY( WIDE_ALPH_TYPE(), K() );
	KEYS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxSpan), SP() );
	KEY_SPANS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxFlatIndexOffset), IO() );
	FLAT_INDEX_OFFSET();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndex), I() );
	INDICIES();
	CLOSE_ARRAY() <<
	L"\n";

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

	STATE_IDS();

  out << L"type " << TYPE_STATE() << L" = { mutable trans : int; mutable acts : int; mutable nacts : int; }"
    << TOP_SEP();

  out << L"exception Goto_match" << TOP_SEP();
  out << L"exception Goto_again" << TOP_SEP();
  out << L"exception Goto_eof_trans" << TOP_SEP();
}

void OCamlFlatCodeGen::COND_TRANSLATE()
{
	out << 
		L"	_widec = " << GET_KEY() << L";\n";

	out <<
		L"   _keys = " << vCS() << L"<<1;\n"
		L"   _conds = " << CO() << L"[" << vCS() << L"];\n"
//		L"	_keys = " << ARR_OFF( CK(), L"(" + vCS() + L"<<1)" ) << L";\n"
//		L"	_conds = " << ARR_OFF( C(), CO() + L"[" + vCS() + L"]" ) << L";\n"
		L"\n"
		L"	_slen = " << CSP() << L"[" << vCS() << L"];\n"
		L"	if (_slen > 0 && " << CK() << L"[_keys] <=" 
			<< GET_WIDE_KEY() << L" &&\n"
		L"		" << GET_WIDE_KEY() << L" <= " << CK() << L"[_keys+1])\n"
		L"		_cond = " << C() << L"[_conds+" << GET_WIDE_KEY() << L" - " << 
			CK() << L"[_keys]];\n"
		L"	else\n"
		L"		_cond = 0;"
		L"\n";
	/*  XXX This version of the code doesn't work because Mono is weird.  Works
	 *  fine in Microsoft's csc, even though the bug report filed claimed it
	 *  didn't.
		L"	_slen = " << CSP() << L"[" << vCS() << L"];\n"
		L"	_cond = _slen > 0 && " << CK() << L"[_keys] <=" 
			<< GET_WIDE_KEY() << L" &&\n"
		L"		" << GET_WIDE_KEY() << L" <= " << CK() << L"[_keys+1] ?\n"
		L"		" << C() << L"[_conds+" << GET_WIDE_KEY() << L" - " << CK() 
			<< L"[_keys]] : 0;\n"
		L"\n";
		*/
	out <<
		L"	switch ( _cond ) {\n";
	for ( CondSpaceList::Iter csi = condSpaceList; csi.lte(); csi++ ) {
		GenCondSpace *condSpace = csi;
		out << L"	case " << condSpace->condSpaceId + 1 << L": {\n";
		out << TABS(2) << L"_widec = " << CAST(WIDE_ALPH_TYPE()) << L"(" <<
				KEY(condSpace->baseKey) << L" + (" << GET_KEY() << 
				L" - " << KEY(keyOps->minKey) << L"));\n";

		for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
			out << TABS(2) << L"if ( ";
			CONDITION( out, *csi );
			Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
			out << L" ) _widec += " << condValOffset << L";\n";
		}

		out << L"		}\n";
		out << L"		break;\n";
	}

	SWITCH_DEFAULT();

	out <<
		L"	}\n";
}

void OCamlFlatCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;
	initVarTypes();

	out << 
		L"	begin\n";
//		L"	" << slenType << L" _slen";

//	if ( redFsm->anyRegCurStateRef() )
//		out << L", _ps";

//	out << 
//		L"	" << transType << L" _trans";

//	if ( redFsm->anyConditions() )
//		out << L", _cond";
//	out << L";\n";

//	if ( redFsm->anyToStateActions() || 
//			redFsm->anyRegActions() || redFsm->anyFromStateActions() )
//	{
//		out << 
//			L"	int _acts;\n"
//			L"	int _nacts;\n"; 
//	}

//	out <<
//		L"	" << L"int _keys;\n"
//		L"	" << indsType << L" _inds;\n";
		/*
		L"	" << PTR_CONST() << WIDE_ALPH_TYPE() << POINTER() << L"_keys;\n"
		L"	" << PTR_CONST() << ARRAY_TYPE(redFsm->maxIndex) << POINTER() << L"_inds;\n";*/

	if ( redFsm->anyConditions() ) {
		out << 
			L"	" << condsType << L" _conds;\n"
			L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";
	}

	out << L"\n";

  out <<
    L"	let state = { trans = 0; acts = 0; nacts = 0; } in\n"
    L"	let rec do_start () =\n";

	if ( !noEnd ) {
		testEofUsed = true;
		out << 
			L"	if " << P() << L" = " << PE() << L" then\n"
			L"		do_test_eof ()\n"
      L"\telse\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			L"	if " << vCS() << L" = " << redFsm->errState->id << L" then\n"
			L"		do_out ()\n"
      L"\telse\n";
	}

  out << L"\tdo_resume ()\n";

	out << L"and do_resume () =\n";

	if ( redFsm->anyFromStateActions() ) {
		out <<
			L"	state.acts <- " << AT( FSA(), vCS() ) << L";\n"
			L"	state.nacts <- " << AT( A(), POST_INCR(L"state.acts") ) << L";\n"
			L"	while " << POST_DECR(L"state.nacts") << L" > 0 do\n"
			L"		begin match " << AT( A(), POST_INCR(L"state.acts") ) << L" with\n";
			FROM_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		end\n"
			L"	done;\n"
			L"\n";
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

//  out << L"\tbegin try\n";
	LOCATE_TRANS();
//  out << L"\twith Goto_match -> () end;\n";

  out << L"\tdo_eof_trans ()\n";

//	if ( redFsm->anyEofTrans() )
  out << L"and do_eof_trans () =\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	let ps = " << vCS() << L" in\n";

	out <<
		L"	" << vCS() << L" <- " << AT( TT() ,L"state.trans" ) << L";\n"
		L"\n";

	if ( redFsm->anyRegActions() ) {
		out <<
			L"\tbegin try\n"
      L"	match " << AT( TA(), L"state.trans" ) << L" with\n"
			L"\t| 0 -> raise Goto_again\n"
      L"\t| _ ->\n"
			L"	state.acts <- " << AT( TA(), L"state.trans" ) << L";\n"
			L"	state.nacts <- " << AT( A(), POST_INCR(L"state.acts") ) << L";\n"
			L"	while " << POST_DECR(L"state.nacts") << L" > 0 do\n"
			L"		begin match " << AT( A(), POST_INCR(L"state.acts") ) << L" with\n";
			ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		end;\n"
			L"	done\n"
      L"\twith Goto_again -> () end;\n";
	}
  out << L"\tdo_again ()\n";

//	if ( redFsm->anyRegActions() || redFsm->anyActionGotos() || 
//			redFsm->anyActionCalls() || redFsm->anyActionRets() )
  out << L"\tand do_again () =\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	state.acts <- " << AT( TSA(), vCS() ) << L";\n"
			L"	state.nacts <- " << AT( A(), POST_INCR(L"state.acts") ) << L";\n"
			L"	while " << POST_DECR(L"state.nacts") << L" > 0 do\n"
			L"		begin match " << AT( A(), POST_INCR(L"state.acts") ) << L" with\n";
			TO_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		end\n"
			L"	done;\n"
			L"\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			L"	match " << vCS() << L" with\n"
      L"\t| " << redFsm->errState->id << L" -> do_out ()\n"
      L"\t| _ ->\n";
	}

  out << L"\t" << P() << L" <- " << P() << L" + 1;\n";

	if ( !noEnd ) {
		out << 
			L"	if " << P() << L" <> " << PE() << L" then\n"
			L"		do_resume ()\n"
      L"\telse do_test_eof ()\n";
	}
	else {
		out << 
			L"	do_resume ()\n";
	}

//	if ( testEofUsed )
	out << L"and do_test_eof () =\n";

	if ( redFsm->anyEofTrans() || redFsm->anyEofActions() ) {
		out << 
			L"	if " << P() << L" = " << vEOF() << L" then\n"
			L"	begin try\n";

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"	if " << AT( ET(), vCS() ) << L" > 0 then\n"
				L"	begin\n"
        L"   state.trans <- " << CAST(transType) << L"(" << AT( ET(), vCS() ) << L" - 1);\n"
				L"		raise Goto_eof_trans;\n"
				L"	end;\n";
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	let __acts = ref " << AT( EA(), vCS() ) << L" in\n"
				L"	let __nacts = ref " << AT( A(), L"!__acts" ) << L" in\n"
        L" incr __acts;\n"
				L"	while !__nacts > 0 do\n"
        L"   decr __nacts;\n"
				L"		begin match " << AT( A(), POST_INCR(L"__acts.contents") ) << L" with\n";
				EOF_ACTION_SWITCH();
				SWITCH_DEFAULT() <<
				L"		end;\n"
				L"	done\n";
		}

		out << 
			L"	with Goto_again -> do_again ()\n"
			L"	| Goto_eof_trans -> do_eof_trans () end\n"
			L"\n";
	}
  else
  {
    out << L"\t()\n";
  }

	if ( outLabelUsed )
		out << L"	and do_out () = ()\n";

  out << L"\tin do_start ()\n";
	out << L"	end;\n";
}

void OCamlFlatCodeGen::initVarTypes()
{
	slenType = ARRAY_TYPE(MAX(redFsm->maxSpan, redFsm->maxCondSpan));
	transType = ARRAY_TYPE(redFsm->maxIndex+1);
	indsType = ARRAY_TYPE(redFsm->maxFlatIndexOffset);
	condsType = ARRAY_TYPE(redFsm->maxCondIndexOffset);
}
