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

#include <sstream>
#include "ragel.h"
#include "gotable.h"
#include "redfsm.h"
#include "gendata.h"

using std::endl;

/* Determine if we should use indicies or not. */
void GoTabCodeGen::calcIndexSize()
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

std::wostream &GoTabCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	out << act;
	return out;
}

std::wostream &GoTabCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	out << act;
	return out;
}

std::wostream &GoTabCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	out << act;
	return out;
}


std::wostream &GoTabCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	out << act;
	return out;
}

std::wostream &GoTabCodeGen::TO_STATE_ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, false, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GoTabCodeGen::FROM_STATE_ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, false, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GoTabCodeGen::EOF_ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, true, false );
		}
	}

	genLineDirective(out);
	return out;
}


std::wostream &GoTabCodeGen::ACTION_SWITCH( int level )
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << TABS(level) << L"case " << act->actionId << L":" << endl;
			ACTION( out, act, 0, false, false );
		}
	}

	genLineDirective(out);
	return out;
}

std::wostream &GoTabCodeGen::COND_OFFSETS()
{
	out << L"	";
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		out << curKeyOffset;
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}

		/* Move the key offset ahead. */
		curKeyOffset += st->stateCondList.length();
	}
	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::KEY_OFFSETS()
{
	out << L"	";
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		out << curKeyOffset;
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}

		/* Move the key offset ahead. */
		curKeyOffset += st->outSingle.length() + st->outRange.length()*2;
	}
	out << endl;
	return out;
}


std::wostream &GoTabCodeGen::INDEX_OFFSETS()
{
	out << L"	";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset;
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}

		/* Move the index offset ahead. */
		curIndOffset += st->outSingle.length() + st->outRange.length();
		if ( st->defTrans != 0 )
			curIndOffset += 1;
	}
	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::COND_LENS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		out << st->stateCondList.length();
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}


std::wostream &GoTabCodeGen::SINGLE_LENS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		out << st->outSingle.length();
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::RANGE_LENS()
{
	out << L"	";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit length of range index. */
		out << st->outRange.length();
		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::TO_STATE_ACTIONS()
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

std::wostream &GoTabCodeGen::FROM_STATE_ACTIONS()
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

std::wostream &GoTabCodeGen::EOF_ACTIONS()
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

std::wostream &GoTabCodeGen::EOF_TRANS()
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
		out << trans;

		out << L", ";
		if ( !st.last() ) {
			if ( ++totalStateNum % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	return out;
}


std::wostream &GoTabCodeGen::COND_KEYS()
{
	out << L"	";
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Lower key. */
			out << KEY( sc->lowKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";

			/* Upper key. */
			out << KEY( sc->highKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::COND_SPACES()
{
	out << L"	";
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Cond Space id. */
			out << sc->condSpace->condSpaceId << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::KEYS()
{
	out << L"	";
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			out << KEY( stel->lowKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}

		/* Loop the state's transitions. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			/* Lower key. */
			out << KEY( rtel->lowKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";

			/* Upper key. */
			out << KEY( rtel->highKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::INDICIES()
{
	out << L"	";
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			out << stel->value->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			out << rtel->value->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			out << st->defTrans->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::TRANS_TARGS()
{
	out << L"	";
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}

		/* The state's default target state. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}

	/* Add any eof transitions that have not yet been written out above. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			trans->pos = totalTrans;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}


	out << endl;
	return out;
}


std::wostream &GoTabCodeGen::TRANS_ACTIONS()
{
	out << L"	";
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}

	/* Add any eof transitions that have not yet been written out above. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << endl << L"	";
		}
	}

	out << endl;
	return out;
}

std::wostream &GoTabCodeGen::TRANS_TARGS_WI()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	out << L"	";
	int totalStates = 0;
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Record the position, need this for eofTrans. */
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


std::wostream &GoTabCodeGen::TRANS_ACTIONS_WI()
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

void GoTabCodeGen::writeData()
{
	/* If there are any transtion functions then output the array. If there
	 * are none, donL't bother emitting an empty array that won't be used. */
	if ( redFsm->anyActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActArrItem), A() );
		ACTIONS_ARRAY();
		CLOSE_ARRAY() << endl;
	}

	if ( redFsm->anyConditions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondOffset), CO() );
		COND_OFFSETS();
		CLOSE_ARRAY() << endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondLen), CL() );
		COND_LENS();
		CLOSE_ARRAY() << endl;

		OPEN_ARRAY( WIDE_ALPH_TYPE(), CK() );
		COND_KEYS();
		CLOSE_ARRAY() << endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondSpaceId), C() );
		COND_SPACES();
		CLOSE_ARRAY() << endl;
	}

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxKeyOffset), KO() );
	KEY_OFFSETS();
	CLOSE_ARRAY() << endl;

	OPEN_ARRAY( WIDE_ALPH_TYPE(), K() );
	KEYS();
	CLOSE_ARRAY() << endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxSingleLen), SL() );
	SINGLE_LENS();
	CLOSE_ARRAY() << endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxRangeLen), RL() );
	RANGE_LENS();
	CLOSE_ARRAY() << endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndexOffset), IO() );
	INDEX_OFFSETS();
	CLOSE_ARRAY() << endl;

	if ( useIndicies ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndex), I() );
		INDICIES();
		CLOSE_ARRAY() << endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
		TRANS_TARGS_WI();
		CLOSE_ARRAY() << endl;

		if ( redFsm->anyActions() ) {
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TA() );
			TRANS_ACTIONS_WI();
			CLOSE_ARRAY() << endl;
		}
	}
	else {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
		TRANS_TARGS();
		CLOSE_ARRAY() << endl;

		if ( redFsm->anyActions() ) {
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TA() );
			TRANS_ACTIONS();
			CLOSE_ARRAY() << endl;
		}
	}

	if ( redFsm->anyToStateActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TSA() );
		TO_STATE_ACTIONS();
		CLOSE_ARRAY() << endl;
	}

	if ( redFsm->anyFromStateActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), FSA() );
		FROM_STATE_ACTIONS();
		CLOSE_ARRAY() << endl;
	}

	if ( redFsm->anyEofActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), EA() );
		EOF_ACTIONS();
		CLOSE_ARRAY() << endl;
	}

	if ( redFsm->anyEofTrans() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndexOffset+1), ET() );
		EOF_TRANS();
		CLOSE_ARRAY() << endl;
	}

	STATE_IDS();
}

void GoTabCodeGen::LOCATE_TRANS()
{
	out <<
		L"	_keys = " << CAST(INT(), KO() + L"[" + vCS() + L"]") << endl <<
		L"	_trans = " << CAST(INT(), IO() + L"[" + vCS() + L"]") << endl <<
		endl <<
		L"	_klen = " << CAST(INT(), SL() + L"[" + vCS() + L"]") << endl <<
		L"	if _klen > 0 {" << endl <<
		L"		_lower := " << CAST(INT(), L"_keys") << endl <<
		L"		var _mid " << INT() << endl <<
		L"		_upper := " << CAST(INT(), L"_keys + _klen - 1") << endl <<
		L"		for {" << endl <<
		L"			if _upper < _lower {" << endl <<
		L"				break" << endl <<
		L"			}" << endl <<
		endl <<
		L"			_mid = _lower + ((_upper - _lower) >> 1)" << endl <<
		L"			switch {" << endl <<
		L"			case " << GET_WIDE_KEY() << L" < " << K() << L"[_mid]" << L":" << endl <<
		L"				_upper = _mid - 1" << endl <<
		L"			case " << GET_WIDE_KEY() << L" > " << K() << L"[_mid]" << L":" << endl <<
		L"				_lower = _mid + 1" << endl <<
		L"			default:" << endl <<
		L"				_trans += " << CAST(INT(), L"_mid - " + CAST(INT(), L"_keys")) << endl <<
		L"				goto _match" << endl <<
		L"			}" << endl <<
		L"		}" << endl <<
		L"		_keys += _klen" << endl <<
		L"		_trans += _klen" << endl <<
		L"	}" << endl <<
		endl <<
		L"	_klen = " << CAST(INT(), RL() + L"[" + vCS() + L"]") << endl <<
		L"	if _klen > 0 {" << endl <<
		L"		_lower := " << CAST(INT(), L"_keys") << endl <<
		L"		var _mid " << INT() << endl <<
		L"		_upper := " << CAST(INT(), L"_keys + (_klen << 1) - 2") << endl <<
		L"		for {" << endl <<
		L"			if _upper < _lower {" << endl <<
		L"				break" << endl <<
		L"			}" << endl <<
		endl <<
		L"			_mid = _lower + (((_upper - _lower) >> 1) & ^1)" << endl <<
		L"			switch {" << endl <<
		L"			case " << GET_WIDE_KEY() << L" < " << K() << L"[_mid]" << L":" << endl <<
		L"				_upper = _mid - 2" << endl <<
		L"			case " << GET_WIDE_KEY() << L" > " << K() << L"[_mid + 1]" << L":" << endl <<
		L"				_lower = _mid + 2" << endl <<
		L"			default:" << endl <<
		L"				_trans += " << CAST(INT(), L"(_mid - " + CAST(INT(), L"_keys") + L") >> 1") << endl <<
		L"				goto _match" << endl <<
		L"			}" << endl <<
		L"		}" << endl <<
		L"		_trans += _klen" << endl <<
		L"	}" << endl <<
		endl;
}

void GoTabCodeGen::COND_TRANSLATE()
{
	out <<
		L"	_widec = " << CAST(WIDE_ALPH_TYPE(), GET_KEY()) << endl <<
		L"	_klen = " << CAST(INT(), CL() + L"[" + vCS() + L"]") << endl <<
		L"	_keys = " << CAST(INT(), CO() + L"[" + vCS() + L"] * 2") << endl <<
		L"	if _klen > 0 {" << endl <<
		L"		_lower := " << CAST(INT(), L"_keys") << endl <<
		L"		var _mid " << INT() << endl <<
		L"		_upper := " << CAST(INT(), L"_keys + (_klen << 1) - 2") << endl <<
		L"	COND_LOOP:" << endl <<
		L"		for {" << endl <<
		L"			if _upper < _lower {" << endl <<
		L"				break" << endl <<
		L"			}" << endl <<
		endl <<
		L"			_mid = _lower + (((_upper - _lower) >> 1) & ^1)" << endl <<
		L"			switch {" << endl <<
		L"			case " << GET_WIDE_KEY() << L" < " << CAST(WIDE_ALPH_TYPE(), CK() + L"[_mid]") << L":" << endl <<
		L"				_upper = _mid - 2" << endl <<
		L"			case " << GET_WIDE_KEY() << L" > " << CAST(WIDE_ALPH_TYPE(), CK() + L"[_mid + 1]") << L":" << endl <<
		L"				_lower = _mid + 2" << endl <<
		L"			default:" << endl <<
		L"				switch " << C() << L"[" << CAST(INT(), CO() + L"[" + vCS() + L"]") <<
							L" + ((_mid - _keys)>>1)] {" << endl;

	for ( CondSpaceList::Iter csi = condSpaceList; csi.lte(); csi++ ) {
		GenCondSpace *condSpace = csi;
		out << TABS(4) << L"case " << condSpace->condSpaceId << L":" << endl;
		out << TABS(5) << L"_widec = " << KEY(condSpace->baseKey) << L" + (" << CAST(WIDE_ALPH_TYPE(), GET_KEY()) <<
					L" - " << KEY(keyOps->minKey) << L")" << endl;

		for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
			out << TABS(5) << L"if ";
			CONDITION( out, *csi );
			Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
			out << L" {" << endl << TABS(6) << L"_widec += " << condValOffset << endl << TABS(5) << L"}" << endl;
		}
	}

	out <<
		L"				}" << endl <<
		L"				break COND_LOOP" << endl <<
		L"			}" << endl <<
		L"		}" << endl <<
		L"	}" << endl <<
		endl;
}

void GoTabCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out <<
		L"	{" << endl <<
		L"	var _klen " << INT() << endl;

	if ( redFsm->anyRegCurStateRef() )
		out << L"	var _ps " << INT() << endl;

	out <<
		L"	var _trans " << INT() << endl;

	if ( redFsm->anyConditions() )
		out << L"	var _widec " << WIDE_ALPH_TYPE() << endl;

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions()
			|| redFsm->anyFromStateActions() )
	{
		out <<
			L"	var _acts " << INT() << endl <<
			L"	var _nacts " << UINT() << endl;
	}

	out <<
		L"	var _keys " << INT() << endl;

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
			L"		 _acts++" << endl <<
			L"		switch " << A() << L"[_acts - 1]" << L" {" << endl;
			FROM_STATE_ACTION_SWITCH(2);
			out <<
			L"		}" << endl <<
			L"	}" << endl << endl;
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

	LOCATE_TRANS();

	out << L"_match:" << endl;

	if ( useIndicies )
		out << L"	_trans = " << CAST(INT(), I() + L"[_trans]") << endl;

	if ( redFsm->anyEofTrans() )
		out << L"_eof_trans:" << endl;

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = " << vCS() << endl;

	out <<
		L"	" << vCS() << L" = " << CAST(INT(), TT() + L"[_trans]") << endl << endl;

	if ( redFsm->anyRegActions() ) {
		out <<
			L"	if " << TA() << L"[_trans] == 0 {" <<  endl <<
			L"		goto _again" << endl <<
			L"	}" << endl <<
			endl <<
			L"	_acts = " << CAST(INT(), TA() + L"[_trans]") << endl <<
			L"	_nacts = " << CAST(UINT(), A() + L"[_acts]") << L"; _acts++" << endl <<
			L"	for ; _nacts > 0; _nacts-- {" << endl <<
			L"		_acts++" << endl <<
			L"		switch " << A() << L"[_acts-1]" << L" {" << endl;
			ACTION_SWITCH(2);
			out <<
			L"		}" << endl <<
			L"	}" << endl << endl;
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
			L"		switch " << A() << L"[_acts-1] {" << endl;
			TO_STATE_ACTION_SWITCH(2);
			out <<
			L"		}" << endl <<
			L"	}" << endl << endl;
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
			L"	" << P() << L"++" << endl <<
			L"	if " << P() << L" != " << PE() << L" {" << endl <<
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
				L"		__acts := " << EA() << L"[" << vCS() << L"]" << endl <<
				L"		__nacts := " << CAST(UINT(), A() + L"[__acts]") << L"; __acts++" << endl <<
				L"		for ; __nacts > 0; __nacts-- {" << endl <<
				L"			__acts++" << endl <<
				L"			switch " << A() << L"[__acts-1] {" << endl;
				EOF_ACTION_SWITCH(3);
				out <<
				L"			}" << endl <<
				L"		}" << endl;
		}

		out <<
			L"	}" << endl << endl;
	}

	if ( outLabelUsed )
		out << L"	_out: {}" << endl;

	out << L"	}" << endl;
}
