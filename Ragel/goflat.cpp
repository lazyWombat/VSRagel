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
#include "goflat.h"
#include "redfsm.h"
#include "gendata.h"

using std::endl;

std::wostream &GoFlatCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	out << act;
	return out;
}

std::wostream &GoFlatCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	out << act;
	return out;
}

std::wostream &GoFlatCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	out << act;
	return out;
}

std::wostream &GoFlatCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	out << act;
	return out;
}

std::wostream &GoFlatCodeGen::TO_STATE_ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, false, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GoFlatCodeGen::FROM_STATE_ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, false, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GoFlatCodeGen::EOF_ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, true, false );
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &GoFlatCodeGen::ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, false, false );
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &GoFlatCodeGen::FLAT_INDEX_OFFSET()
{
	out << L"	";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}

		/* Move the index offset ahead. */
		if ( st->transList != 0 )
			curIndOffset += keyOps->span( st->lowKey, st->highKey );

		if ( st->defTrans != 0 )
			curIndOffset += 1;
	}
	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::KEY_SPANS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		unsigned long long span = 0;
		if ( st->transList != 0 )
			span = keyOps->span( st->lowKey, st->highKey );
		out << span << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::TO_STATE_ACTIONS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		TO_STATE_ACTION(st);
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::FROM_STATE_ACTIONS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		FROM_STATE_ACTION(st);
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::EOF_ACTIONS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		EOF_ACTION(st);
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::EOF_TRANS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */

		long trans = 0;
		if ( st->eofTrans != 0 ) {
			assert( st->eofTrans->pos >= 0 );
			trans = st->eofTrans->pos+1;
		}
		out << trans << L", ";

		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}


std::wostream &GoFlatCodeGen::COND_KEYS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just cond low key and cond high key. */
		out << KEY( st->condLowKey ) << L", ";
		out << KEY( st->condHighKey ) << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::COND_KEY_SPANS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		unsigned long long span = 0;
		if ( st->condList != 0 )
			span = keyOps->span( st->condLowKey, st->condHighKey );
		out << span << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::CONDS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->condList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->condLowKey, st->condHighKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				if ( st->condList[pos] != 0 )
					out << st->condList[pos]->condSpaceId + 1 << L", ";
				else
					out << L"0, ";
				if ( !st.last() ) {
					if ( ++totalStateNum % IALL == 0 )
						out << endl << L"	";
				}
			}
		}
	}

	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::COND_INDEX_OFFSET()
{
	out << L"	";
	int totalStateNum = 0;
	int curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}

		/* Move the index offset ahead. */
		if ( st->condList != 0 )
			curIndOffset += keyOps->span( st->condLowKey, st->condHighKey );
	}
	out << endl;
	return out;
}


std::wostream &GoFlatCodeGen::KEYS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just low key and high key. */
		out << KEY( st->lowKey ) << L", ";
		out << KEY( st->highKey ) << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::INDICIES()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->transList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->lowKey, st->highKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				out << st->transList[pos]->id << L", ";
				if ( ++totalStateNum % IALL == 0 )
					out << endl << L"	";
			}
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			out << st->defTrans->id << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoFlatCodeGen::TRANS_TARGS()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	out << L"	";
	int totalStates = 0;
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Save the position. Needed for eofTargs. */
		RedTransAp *trans = transPtrs[t];
		trans->pos = t;

		/* Write out the target state. */
		out << trans->targ->id << L", ";
		if ( t < redFsm->transSet.length()-1 ) {
			if ( ++totalStates % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	delete[] transPtrs;
	return out;
}


std::wostream &GoFlatCodeGen::TRANS_ACTIONS()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	out << L"	";
	int totalAct = 0;
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Write the function for the transition. */
		RedTransAp *trans = transPtrs[t];
		TRANS_ACTION( trans );
		out << L", ";
		if ( t < redFsm->transSet.length()-1 ) {
			if ( ++totalAct % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	delete[] transPtrs;
	return out;
}

void GoFlatCodeGen::LOCATE_TRANS()
{
	out <<
		L"	_keys = " << CAST(INT(), vCS() + L" << 1") << endl <<
		L"	_inds = " << CAST(INT(), IO() + L"[" + vCS() + L"]") << endl <<
		endl <<
		L"	_slen = " << CAST(INT(), SP() + L"[" + vCS() + L"]") << endl <<
		L"	if _slen > 0 && " << K() << L"[_keys] <= " << GET_WIDE_KEY() << L" && " <<
			GET_WIDE_KEY() << L" <= " << K() << L"[_keys + 1]" << L" {" << endl <<
		L"		_trans = " << CAST(INT(), I() + L"[_inds + " + CAST(INT(), GET_WIDE_KEY() + L" - " + K() + L"[_keys]") + L"]") << endl <<
		L"	} else {" << endl <<
		L"		_trans = " << CAST(INT(), I() + L"[_inds + _slen]") << endl <<
		L"	}" << endl <<
		endl;
}

void GoFlatCodeGen::writeData()
{
	/* If there are any transtion functions then output the array. If there
	 * are none, donL't bother emitting an empty array that won't be used. */
	if ( redFsm->anyActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActArrItem), A() );
		ACTIONS_ARRAY();
		CLOSE_ARRAY() <<
		endl;
	}

	if ( redFsm->anyConditions() ) {
		OPEN_ARRAY( WIDE_ALPH_TYPE(), CK() );
		COND_KEYS();
		CLOSE_ARRAY() <<
		endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondSpan), CSP() );
		COND_KEY_SPANS();
		CLOSE_ARRAY() <<
		endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCond), C() );
		CONDS();
		CLOSE_ARRAY() <<
		endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondIndexOffset), CO() );
		COND_INDEX_OFFSET();
		CLOSE_ARRAY() <<
		endl;
	}

	OPEN_ARRAY( WIDE_ALPH_TYPE(), K() );
	KEYS();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxSpan), SP() );
	KEY_SPANS();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxFlatIndexOffset), IO() );
	FLAT_INDEX_OFFSET();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndex), I() );
	INDICIES();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
	TRANS_TARGS();
	CLOSE_ARRAY() <<
	endl;

	if ( redFsm->anyActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TA() );
		TRANS_ACTIONS();
		CLOSE_ARRAY() <<
		endl;
	}

	if ( redFsm->anyToStateActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TSA() );
		TO_STATE_ACTIONS();
		CLOSE_ARRAY() <<
		endl;
	}

	if ( redFsm->anyFromStateActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), FSA() );
		FROM_STATE_ACTIONS();
		CLOSE_ARRAY() <<
		endl;
	}

	if ( redFsm->anyEofActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), EA() );
		EOF_ACTIONS();
		CLOSE_ARRAY() <<
		endl;
	}

	if ( redFsm->anyEofTrans() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndexOffset+1), ET() );
		EOF_TRANS();
		CLOSE_ARRAY() <<
		endl;
	}

	STATE_IDS();
}

void GoFlatCodeGen::COND_TRANSLATE()
{
	out <<
		L"	_widec = " << CAST(WIDE_ALPH_TYPE(), GET_KEY()) << endl;

	out <<
		L"	_keys = " << CAST(INT(), vCS() + L" << 1") << endl <<
		L"	_conds = " << CAST(INT(), CO() + L"[" + vCS() + L"]") << endl <<
		endl <<
		L"	_slen = " << CAST(INT(), CSP() + L"[" + vCS() + L"]") << endl <<
		L"	if _slen > 0 && " << CK() << L"[_keys]" << L" <= " << GET_WIDE_KEY() << L" && " <<
				GET_WIDE_KEY() << L" <= " << CK() << L"[_keys + 1] {" << endl <<
		L"		_cond = " << CAST(INT(), C() + L"[_conds + " + CAST(INT(), GET_WIDE_KEY() + L" - " + CK() + L"[_keys]") + L"]") << endl <<
		L"	} else {" << endl <<
		L"		_cond = 0" << endl <<
		L"	}" << endl <<
		endl;

	out <<
		L"	switch _cond {" << endl;
	for ( CondSpaceList::Iter csi = condSpaceList; csi.lte(); csi++ ) {
		GenCondSpace *condSpace = csi;
		out << L"	case " << condSpace->condSpaceId + 1 << L":" << endl;
		out << TABS(2) << L"_widec = " <<
				KEY(condSpace->baseKey) << L" + (" << CAST(WIDE_ALPH_TYPE(), GET_KEY()) <<
				L" - " << KEY(keyOps->minKey) << L")" << endl;

		for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
			out << TABS(2) << L"if ";
			CONDITION( out, *csi );
			Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
			out << L" {" << endl <<
				L"			_widec += " << condValOffset << endl <<
				L"		}" << endl;
		}
	}

	out <<
		L"	}" << endl;
}

void GoFlatCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out <<
		L"	{" << endl <<
		L"	var _slen " << INT() << endl;

	if ( redFsm->anyRegCurStateRef() )
		out << L"	var _ps " << INT() << endl;

	out <<
		L"	var _trans " << INT() << endl;

	if ( redFsm->anyConditions() )
		out << L"	var _cond " << INT() << endl;

	if ( redFsm->anyToStateActions() ||
			redFsm->anyRegActions() || redFsm->anyFromStateActions() )
	{
		out <<
			L"	var _acts " << INT() << endl <<
			L"	var _nacts " << UINT() << endl;
	}

	out <<
		L"	var _keys " << INT() << endl <<
		L"	var _inds " << INT() << endl;

	if ( redFsm->anyConditions() ) {
		out <<
			L"	var _conds " << INT() << endl <<
			L"	var _widec " << WIDE_ALPH_TYPE() << endl;
	}

	out << endl;

	if ( !noEnd ) {
		testEofUsed = true;
		out <<
			L"	if " << P() << L" == " << PE() << L" {" << endl <<
			L"		goto _test_eof" << endl <<
			L"	}" << endl;
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out <<
			L"	if " << vCS() << L" == " << redFsm->errState->id << L" {" << endl <<
			L"		goto _out" << endl <<
			L"	}" << endl;
	}

	out << L"_resume:" << endl;

	if ( redFsm->anyFromStateActions() ) {
		out <<
			L"	_acts = " << CAST(INT(), FSA() + L"[" + vCS() + L"]") << endl <<
			L"	_nacts = " << CAST(UINT(), A() + L"[_acts]") << L"; _acts++" << endl <<
			L"	for ; _nacts > 0; _nacts-- {" << endl <<
			L"		_acts++" << endl <<
			L"		switch " << A() << L"[_acts - 1]" << L" {" << endl;
			FROM_STATE_ACTION_SWITCH(2);
			out <<
			L"		}" << endl <<
			L"	}" << endl <<
			endl;
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

	LOCATE_TRANS();

	if ( redFsm->anyEofTrans() )
		out << L"_eof_trans:" << endl;

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = " << vCS() << endl;

	out <<
		L"	" << vCS() << L" = " << CAST(INT(), TT() + L"[_trans]") << endl <<
		endl;

	if ( redFsm->anyRegActions() ) {
		out <<
			L"	if " << TA() << L"[_trans] == 0 {" << endl <<
			L"		goto _again" << endl <<
			L"	}" << endl <<
			endl <<
			L"	_acts = " << CAST(INT(), TA() + L"[_trans]") << endl <<
			L"	_nacts = " << CAST(UINT(), A() + L"[_acts]") << L"; _acts++" << endl <<
			L"	for ; _nacts > 0; _nacts-- {" << endl <<
			L"		_acts++" << endl <<
			L"		switch " << A() << L"[_acts - 1]" << L" {" << endl;
			ACTION_SWITCH(2);
			out <<
			L"		}" << endl <<
			L"	}" << endl <<
			endl;
	}

	if ( redFsm->anyRegActions() || redFsm->anyActionGotos() ||
			redFsm->anyActionCalls() || redFsm->anyActionRets() )
		out << L"_again:" << endl;

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	_acts = " << CAST(INT(), TSA() + L"[" + vCS() + L"]") << endl <<
			L"	_nacts = " << CAST(UINT(), A() + L"[_acts]") << L"; _acts++" << endl <<
			L"	for ; _nacts > 0; _nacts-- {" << endl <<
			L"		_acts++" << endl <<
			L"		switch " << A() << L"[_acts - 1]" << L" {" << endl;
			TO_STATE_ACTION_SWITCH(2);
			out <<
			L"		}" << endl <<
			L"	}" << endl <<
			endl;
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out <<
			L"	if " << vCS() << L" == " << redFsm->errState->id << L" {" << endl <<
			L"		goto _out" << endl <<
			L"	}" << endl;
	}

	if ( !noEnd ) {
		out <<
			L"	if " << P() << L"++; " << P() << L" != " << PE() << L" {" << endl <<
			L"		goto _resume" << endl <<
			L"	}" << endl;
	}
	else {
		out <<
			L"	" << P() << L"++" << endl <<
			L"	goto _resume" << endl;
	}

	if ( testEofUsed )
		out << L"	_test_eof: {}" << endl;

	if ( redFsm->anyEofTrans() || redFsm->anyEofActions() ) {
		out <<
			L"	if " << P() << L" == " << vEOF() << L" {" << endl;

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"		if " << ET() << L"[" << vCS() << L"] > 0 {" << endl <<
				L"			_trans = " << CAST(INT(), ET() + L"[" + vCS() + L"] - 1") << endl <<
				L"			goto _eof_trans" << endl <<
				L"		}" << endl;
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"		__acts := " << CAST(INT(), EA() + L"[" + vCS() + L"]") << endl <<
				L"		__nacts := " << CAST(UINT(), A() + L"[__acts]") << L"; __acts++" << endl <<
				L"		for ; __nacts > 0; __nacts-- {" << endl <<
				L"			__acts++" << endl <<
				L"			switch " << A() << L"[__acts - 1]" << L" {" << endl;
				EOF_ACTION_SWITCH(3);
				out <<
				L"			}" << endl <<
				L"		}" << endl;
		}

		out <<
			L"	}" << endl <<
			endl;
	}

	if ( outLabelUsed )
		out << L"	_out: {}" << endl;

	out << L"	}" << endl;
}
