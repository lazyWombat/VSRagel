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

#include "ragel.h"
#include "csflat.h"
#include "redfsm.h"
#include "gendata.h"

std::wostream &CSharpFlatCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	out << act;
	return out;
}

std::wostream &CSharpFlatCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	out << act;
	return out;
}

std::wostream &CSharpFlatCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	out << act;
	return out;
}

std::wostream &CSharpFlatCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	out << act;
	return out;
}

std::wostream &CSharpFlatCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &CSharpFlatCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &CSharpFlatCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, true );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &CSharpFlatCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &CSharpFlatCodeGen::FLAT_INDEX_OFFSET()
{
	out << L"\t";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset;
		if ( !st.last() ) {
			out << L", ";
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

std::wostream &CSharpFlatCodeGen::KEY_SPANS()
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
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &CSharpFlatCodeGen::TO_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		TO_STATE_ACTION(st);
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &CSharpFlatCodeGen::FROM_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		FROM_STATE_ACTION(st);
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &CSharpFlatCodeGen::EOF_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		EOF_ACTION(st);
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &CSharpFlatCodeGen::EOF_TRANS()
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
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


std::wostream &CSharpFlatCodeGen::COND_KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just cond low key and cond high key. */
		out << ALPHA_KEY( st->condLowKey ) << L", ";
		out << ALPHA_KEY( st->condHighKey ) << L", ";
		if ( ++totalTrans % IALL == 0 )
			out << L"\n\t";
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	if ( keyOps->alphType->isChar )
		out << L"(char) " << 0 << L"\n";
	else
		out << 0 << L"\n";

	return out;
}

std::wostream &CSharpFlatCodeGen::COND_KEY_SPANS()
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
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &CSharpFlatCodeGen::CONDS()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->condList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->condLowKey, st->condHighKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				if ( st->condList[pos] != 0 )
					out << st->condList[pos]->condSpaceId + 1 << L", ";
				else
					out << L"0, ";
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

std::wostream &CSharpFlatCodeGen::COND_INDEX_OFFSET()
{
	out << L"\t";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset;
		if ( !st.last() ) {
			out << L", ";
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


std::wostream &CSharpFlatCodeGen::KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just low key and high key. */
		out << ALPHA_KEY( st->lowKey ) << L", ";
		out << ALPHA_KEY( st->highKey ) << L", ";
		if ( ++totalTrans % IALL == 0 )
			out << L"\n\t";
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	if ( keyOps->alphType->isChar )
		out << L"(char) " << 0 << L"\n";
	else
		out << 0 << L"\n";

	return out;
}

std::wostream &CSharpFlatCodeGen::INDICIES()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->transList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->lowKey, st->highKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				out << st->transList[pos]->id << L", ";
				if ( ++totalTrans % IALL == 0 )
					out << L"\n\t";
			}
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 )
			out << st->defTrans->id << L", ";

		if ( ++totalTrans % IALL == 0 )
			out << L"\n\t";
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &CSharpFlatCodeGen::TRANS_TARGS()
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
			out << L", ";
			if ( ++totalStates % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}


std::wostream &CSharpFlatCodeGen::TRANS_ACTIONS()
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
			out << L", ";
			if ( ++totalAct % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}

void CSharpFlatCodeGen::LOCATE_TRANS()
{
	out <<
		L"	_keys = " << vCS() << L"<<1;\n"
		L"	_inds = " << IO() << L"[" << vCS() << L"];\n"
		L"\n"
		L"	_slen = " << SP() << L"[" << vCS() << L"];\n"
		L"	_trans = " << I() << L"[_inds + (\n"
		L"		_slen > 0 && " << K() << L"[_keys] <=" << GET_WIDE_KEY() << L" &&\n"
		L"		" << GET_WIDE_KEY() << L" <= " << K() <<L"[_keys+1] ?\n"
		L"		" << GET_WIDE_KEY() << L" - " << K() << L"[_keys] : _slen ) ];\n"
		L"\n";
}

void CSharpFlatCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << gotoDest << L"; " << 
			CTRL_FLOW() << L"goto _again;}";
}

void CSharpFlatCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"{" << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L"); " << CTRL_FLOW() << L"goto _again;}";
}

void CSharpFlatCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void CSharpFlatCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void CSharpFlatCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void CSharpFlatCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}

void CSharpFlatCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = " << 
			callDest << L"; " << CTRL_FLOW() << L"goto _again;}";

	if ( prePushExpr != 0 )
		ret << L"}";
}


void CSharpFlatCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, targState, inFinish );
	ret << L"); " << CTRL_FLOW() << L"goto _again;}";

	if ( prePushExpr != 0 )
		ret << L"}";
}


void CSharpFlatCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << STACK() << L"[--" << TOP() << L"];";

	if ( postPopExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, postPopExpr, 0, false );
		ret << L"}";
	}

	ret << CTRL_FLOW() << L"goto _again;}";
}

void CSharpFlatCodeGen::BREAK( wostream &ret, int targState )
{
	outLabelUsed = true;
	ret << L"{" << P() << L"++; " << CTRL_FLOW() << L"goto _out; }";
}

void CSharpFlatCodeGen::writeData()
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
}

void CSharpFlatCodeGen::COND_TRANSLATE()
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

void CSharpFlatCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;
	initVarTypes();

	out << 
		L"	{\n"
		L"	" << slenType << L" _slen";

	if ( redFsm->anyRegCurStateRef() )
		out << L", _ps";

	out << 
		L";\n"
		L"	" << transType << L" _trans";

	if ( redFsm->anyConditions() )
		out << L", _cond";
	out << L";\n";

	if ( redFsm->anyToStateActions() || 
			redFsm->anyRegActions() || redFsm->anyFromStateActions() )
	{
		out << 
			L"	int _acts;\n"
			L"	int _nacts;\n"; 
	}

	out <<
		L"	" << L"int _keys;\n"
		L"	" << indsType << L" _inds;\n";
		/*
		L"	" << PTR_CONST() << WIDE_ALPH_TYPE() << POINTER() << L"_keys;\n"
		L"	" << PTR_CONST() << ARRAY_TYPE(redFsm->maxIndex) << POINTER() << L"_inds;\n";*/

	if ( redFsm->anyConditions() ) {
		out << 
			L"	" << condsType << L" _conds;\n"
			L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";
	}

	out << L"\n";

	if ( !noEnd ) {
		testEofUsed = true;
		out << 
			L"	if ( " << P() << L" == " << PE() << L" )\n"
			L"		goto _test_eof;\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			L"	if ( " << vCS() << L" == " << redFsm->errState->id << L" )\n"
			L"		goto _out;\n";
	}

	out << L"_resume:\n";

	if ( redFsm->anyFromStateActions() ) {
		out <<
			L"	_acts = " << FSA() << L"[" << vCS() << L"];\n"
			L"	_nacts = " << A() << L"[_acts++];\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( " << A() << L"[_acts++] ) {\n";
			FROM_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

	LOCATE_TRANS();

	if ( redFsm->anyEofTrans() )
		out << L"_eof_trans:\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = " << vCS() << L";\n";

	out <<
		L"	" << vCS() << L" = " << TT() << L"[_trans];\n"
		L"\n";

	if ( redFsm->anyRegActions() ) {
		out <<
			L"	if ( " << TA() << L"[_trans] == 0 )\n"
			L"		goto _again;\n"
			L"\n"
			L"	_acts = " << TA() << L"[_trans];\n"
			L"	_nacts = " << A() << L"[_acts++];\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( " << A() << L"[_acts++] )\n		{\n";
			ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->anyRegActions() || redFsm->anyActionGotos() || 
			redFsm->anyActionCalls() || redFsm->anyActionRets() )
		out << L"_again:\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	_acts = " << TSA() << L"[" << vCS() << L"];\n"
			L"	_nacts = " << A() << L"[_acts++];\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( " << A() << L"[_acts++] ) {\n";
			TO_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			L"	if ( " << vCS() << L" == " << redFsm->errState->id << L" )\n"
			L"		goto _out;\n";
	}

	if ( !noEnd ) {
		out << 
			L"	if ( ++" << P() << L" != " << PE() << L" )\n"
			L"		goto _resume;\n";
	}
	else {
		out << 
			L"	" << P() << L" += 1;\n"
			L"	goto _resume;\n";
	}

	if ( testEofUsed )
		out << L"	_test_eof: {}\n";

	if ( redFsm->anyEofTrans() || redFsm->anyEofActions() ) {
		out << 
			L"	if ( " << P() << L" == " << vEOF() << L" )\n"
			L"	{\n";

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"	if ( " << ET() << L"[" << vCS() << L"] > 0 ) {\n"
				L"		_trans = " << CAST(transType) << L" (" << ET() << 
					L"[" << vCS() << L"] - 1);\n"
				L"		goto _eof_trans;\n"
				L"	}\n";
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	" << PTR_CONST() << ARRAY_TYPE(redFsm->maxActArrItem) << 
						POINTER() << L"__acts = " << 
						EA() << L"[" << vCS() << L"];\n"
				L"	" << UINT() << L" __nacts = " << CAST(UINT()) << L" " <<
							A() << L"[__acts++];\n"
				L"	while ( __nacts-- > 0 ) {\n"
				L"		switch ( " << A() << L"[__acts++] ) {\n";
				EOF_ACTION_SWITCH();
				SWITCH_DEFAULT() <<
				L"		}\n"
				L"	}\n";
		}

		out <<
			L"	}\n"
			L"\n";
	}

	if ( outLabelUsed )
		out << L"	_out: {}\n";

	out << L"	}\n";
}

void CSharpFlatCodeGen::initVarTypes()
{
	slenType = ARRAY_TYPE(MAX(redFsm->maxSpan, redFsm->maxCondSpan));
	transType = ARRAY_TYPE(redFsm->maxIndex+1);
	indsType = ARRAY_TYPE(redFsm->maxFlatIndexOffset);
	condsType = ARRAY_TYPE(redFsm->maxCondIndexOffset);
}
