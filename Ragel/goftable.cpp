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
#include "goftable.h"
#include "redfsm.h"
#include "gendata.h"

using std::endl;

/* Determine if we should use indicies or not. */
void GoFTabCodeGen::calcIndexSize()
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
		sizeWithInds += arrayTypeSize(redFsm->maxActListId) * redFsm->transSet.length();

	/* Calculate the cost of not using indicies. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		int totalIndex = st->outSingle.length() + st->outRange.length() +
				(st->defTrans == 0 ? 0 : 1);
		sizeWithoutInds += arrayTypeSize(redFsm->maxState) * totalIndex;
		if ( redFsm->anyActions() )
			sizeWithoutInds += arrayTypeSize(redFsm->maxActListId) * totalIndex;
	}

	/* If using indicies reduces the size, use them. */
	useIndicies = sizeWithInds < sizeWithoutInds;
}

std::wostream &GoFTabCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->actListId+1;
	out << act;
	return out;
}

std::wostream &GoFTabCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->actListId+1;
	out << act;
	return out;
}

std::wostream &GoFTabCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->actListId+1;
	out << act;
	return out;
}


/* Write out the function for a transition. */
std::wostream &GoFTabCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	int action = 0;
	if ( trans->action != 0 )
		action = trans->action->actListId+1;
	out << action;
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &GoFTabCodeGen::TO_STATE_ACTION_SWITCH( int level )
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numToStateRefs > 0 ) {
			/* Write the entry label. */
			out << TABS(level) << L"case " << redAct->actListId+1 << L":" << endl;

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false, false );

			out << endl;
		}
	}

	genLineDirective( out );
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &GoFTabCodeGen::FROM_STATE_ACTION_SWITCH( int level )
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numFromStateRefs > 0 ) {
			/* Write the entry label. */
			out << TABS(level) << L"case " << redAct->actListId+1 << L":" << endl;

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false, false );

			out << endl;
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GoFTabCodeGen::EOF_ACTION_SWITCH( int level )
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numEofRefs > 0 ) {
			/* Write the entry label. */
			out << TABS(level) << L"case " << redAct->actListId+1 << L":" << endl;

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, true, false );

			out << endl;
		}
	}

	genLineDirective( out );
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &GoFTabCodeGen::ACTION_SWITCH( int level )
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			/* Write the entry label. */
			out << TABS(level) << L"case " << redAct->actListId+1 << L":" << endl;

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false, false );

			out << endl;
		}
	}

	genLineDirective( out );
	return out;
}

void GoFTabCodeGen::writeData()
{
	if ( redFsm->anyConditions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondOffset), CO() );
		COND_OFFSETS();
		CLOSE_ARRAY() <<
		endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondLen), CL() );
		COND_LENS();
		CLOSE_ARRAY() <<
		endl;

		OPEN_ARRAY( WIDE_ALPH_TYPE(), CK() );
		COND_KEYS();
		CLOSE_ARRAY() <<
		endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondSpaceId), C() );
		COND_SPACES();
		CLOSE_ARRAY() <<
		endl;
	}

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxKeyOffset), KO() );
	KEY_OFFSETS();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( WIDE_ALPH_TYPE(), K() );
	KEYS();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxSingleLen), SL() );
	SINGLE_LENS();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxRangeLen), RL() );
	RANGE_LENS();
	CLOSE_ARRAY() <<
	endl;

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndexOffset), IO() );
	INDEX_OFFSETS();
	CLOSE_ARRAY() <<
	endl;

	if ( useIndicies ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndex), I() );
		INDICIES();
		CLOSE_ARRAY() <<
		endl;

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
		TRANS_TARGS_WI();
		CLOSE_ARRAY() <<
		endl;

		if ( redFsm->anyActions() ) {
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActListId), TA() );
			TRANS_ACTIONS_WI();
			CLOSE_ARRAY() <<
			endl;
		}
	}
	else {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
		TRANS_TARGS();
		CLOSE_ARRAY() <<
		endl;

		if ( redFsm->anyActions() ) {
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActListId), TA() );
			TRANS_ACTIONS();
			CLOSE_ARRAY() <<
			endl;
		}
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
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActListId), EA() );
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

void GoFTabCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out <<
		L"	{" << endl <<
		L"	var _klen " << INT() << endl;

	if ( redFsm->anyRegCurStateRef() )
		out << L"	var _ps " << INT() << endl;

	out <<
		L"	var _keys " << INT() << endl <<
		L"	var _trans " << INT() << endl;

	if ( redFsm->anyConditions() )
		out << L"	var _widec " << WIDE_ALPH_TYPE() << endl;

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
			L"	switch " << FSA() << L"[" << vCS() << L"] {" << endl;
			FROM_STATE_ACTION_SWITCH(1);
			out <<
			L"	}" << endl <<
			endl;
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
		L"	" << vCS() << L" = " << CAST(INT(), TT() + L"[_trans]") << endl <<
		endl;

	if ( redFsm->anyRegActions() ) {
		out <<
			L"	if " << TA() << L"[_trans] == 0 {" << endl <<
			L"		goto _again" << endl <<
			L"	}" << endl <<
			endl <<
			L"	switch " << TA() << L"[_trans] {" << endl;
			ACTION_SWITCH(1);
			out <<
			L"	}" << endl <<
			endl;
	}

	if ( redFsm->anyRegActions() || redFsm->anyActionGotos() ||
			redFsm->anyActionCalls() || redFsm->anyActionRets() )
		out << L"_again:" << endl;

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	switch " << TSA() << L"[" << vCS() << L"] {" << endl;
			TO_STATE_ACTION_SWITCH(1);
			out <<
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
				L"		switch " << EA() << L"[" << vCS() << L"] {" << endl;
				EOF_ACTION_SWITCH(2);
				out <<
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
