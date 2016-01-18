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
#include "gogoto.h"
#include "redfsm.h"
#include "bstmap.h"
#include "gendata.h"

using std::endl;

/* Emit the goto to take for a given transition. */
std::wostream &GoGotoCodeGen::TRANS_GOTO( RedTransAp *trans, int level )
{
	out << TABS(level) << L"goto tr" << trans->id << L";";
	return out;
}

int GoGotoCodeGen::TRANS_NR( RedTransAp *trans )
{
	return trans->id;
}

std::wostream &GoGotoCodeGen::TO_STATE_ACTION_SWITCH( int level )
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

std::wostream &GoGotoCodeGen::FROM_STATE_ACTION_SWITCH( int level )
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

std::wostream &GoGotoCodeGen::EOF_ACTION_SWITCH( int level )
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

	genLineDirective( out );
	return out;
}

std::wostream &GoGotoCodeGen::ACTION_SWITCH( int level )
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

	genLineDirective( out );
	return out;
}

void GoGotoCodeGen::GOTO_HEADER( RedStateAp *state, int level )
{
	/* Label the state. */
	out << TABS(level) << L"case " << state->id << L":" << endl;
}

void GoGotoCodeGen::emitSingleSwitch( RedStateAp *state, int level )
{
	/* Load up the singles. */
	int numSingles = state->outSingle.length();
	RedTransEl *data = state->outSingle.data;

	if ( numSingles == 1 ) {
		/* If there is a single single key then write it out as an if. */
		out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" == " <<
				WIDE_KEY(state, data[0].lowKey) << L" {" << endl;

		/* Virtual function for writing the target of the transition. */
		TRANS_GOTO(data[0].value, level + 1) << endl;
		out << TABS(level) << L"}" << endl;
	}
	else if ( numSingles > 1 ) {
		/* Write out single keys in a switch if there is more than one. */
		out << TABS(level) << L"switch " << GET_WIDE_KEY(state) << L" {" << endl;

		/* Write out the single indicies. */
		for ( int j = 0; j < numSingles; j++ ) {
			out << TABS(level) << L"case " << WIDE_KEY(state, data[j].lowKey) << L":" << endl;
			TRANS_GOTO(data[j].value, level + 1) << endl;
		}

		/* Close off the transition switch. */
		out << TABS(level) << L"}" << endl;
	}
}

void GoGotoCodeGen::emitRangeBSearch( RedStateAp *state, int level, int low, int high )
{
	/* Get the mid position, staying on the lower end of the range. */
	int mid = (low + high) >> 1;
	RedTransEl *data = state->outRange.data;

	/* Determine if we need to look higher or lower. */
	bool anyLower = mid > low;
	bool anyHigher = mid < high;

	/* Determine if the keys at mid are the limits of the alphabet. */
	bool limitLow = data[mid].lowKey == keyOps->minKey;
	bool limitHigh = data[mid].highKey == keyOps->maxKey;

	if ( anyLower && anyHigher ) {
		/* Can go lower and higher than mid. */
		out << TABS(level) << L"switch {" << endl;
		out << TABS(level) << L"case " << GET_WIDE_KEY(state) << L" < " <<
				WIDE_KEY(state, data[mid].lowKey) << L":" << endl;
		emitRangeBSearch( state, level+1, low, mid-1 );
		out << TABS(level) << L"case " << GET_WIDE_KEY(state) << L" > " <<
				WIDE_KEY(state, data[mid].highKey) << L":" << endl;
		emitRangeBSearch( state, level+1, mid+1, high );
		out << TABS(level) << L"default:" << endl;
		TRANS_GOTO(data[mid].value, level+1) << endl;
		out << TABS(level) << L"}" << endl;
	}
	else if ( anyLower && !anyHigher ) {
		/* Can go lower than mid but not higher. */
		out << TABS(level) << L"switch {" << endl;
		out << TABS(level) << L"case " << GET_WIDE_KEY(state) << L" < " <<
				WIDE_KEY(state, data[mid].lowKey) << L":" << endl;
		emitRangeBSearch( state, level+1, low, mid-1 );

		/* if the higher is the highest in the alphabet then there is no
		 * sense testing it. */
		if ( limitHigh ) {
			out << TABS(level) << L"default:" << endl;
			TRANS_GOTO(data[mid].value, level+1) << endl;
		}
		else {
			out << TABS(level) << L"case " << GET_WIDE_KEY(state) << L" <= " <<
					WIDE_KEY(state, data[mid].highKey) << L":" << endl;
			TRANS_GOTO(data[mid].value, level+1) << endl;
		}
		out << TABS(level) << L"}" << endl;
	}
	else if ( !anyLower && anyHigher ) {
		/* Can go higher than mid but not lower. */
		out << TABS(level) << L"switch {" << endl;
		out << TABS(level) << L"case " << GET_WIDE_KEY(state) << L" > " <<
				WIDE_KEY(state, data[mid].highKey) << L":" << endl;
		emitRangeBSearch( state, level+1, mid+1, high );

		/* If the lower end is the lowest in the alphabet then there is no
		 * sense testing it. */
		if ( limitLow ) {
			out << TABS(level) << L"default:" << endl;
			TRANS_GOTO(data[mid].value, level+1) << endl;
		}
		else {
			out << TABS(level) << L"case " << GET_WIDE_KEY(state) << L" >= " <<
					WIDE_KEY(state, data[mid].lowKey) << L":" << endl;
			TRANS_GOTO(data[mid].value, level+1) << endl;
		}
		out << TABS(level) << L"}" << endl;
	}
	else {
		/* Cannot go higher or lower than mid. It's mid or bust. What
		 * tests to do depends on limits of alphabet. */
		if ( !limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << WIDE_KEY(state, data[mid].lowKey) << L" <= " <<
					GET_WIDE_KEY(state) << L" && " << GET_WIDE_KEY(state) << L" <= " <<
					WIDE_KEY(state, data[mid].highKey) << L" {" << endl;
			TRANS_GOTO(data[mid].value, level+1) << endl;
			out << TABS(level) << L"}" << endl;
		}
		else if ( limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" <= " <<
					WIDE_KEY(state, data[mid].highKey) << L" {" << endl;
			TRANS_GOTO(data[mid].value, level+1) << endl;
			out << TABS(level) << L"}" << endl;
		}
		else if ( !limitLow && limitHigh ) {
			out << TABS(level) << L"if " << WIDE_KEY(state, data[mid].lowKey) << L" <= " <<
					GET_WIDE_KEY(state) << L" {" << endl;
			TRANS_GOTO(data[mid].value, level+1) << endl;
			out << TABS(level) << L"}" << endl;
		}
		else {
			/* Both high and low are at the limit. No tests to do. */
			TRANS_GOTO(data[mid].value, level) << endl;
		}
	}
}

void GoGotoCodeGen::STATE_GOTO_ERROR( int level )
{
	/* Label the state and bail immediately. */
	outLabelUsed = true;
	RedStateAp *state = redFsm->errState;
	out << TABS(level) << L"case " << state->id << L":" << endl;
	out << TABS(level + 1) << L"goto _out" << endl;
}

void GoGotoCodeGen::COND_TRANSLATE( GenStateCond *stateCond, int level )
{
	GenCondSpace *condSpace = stateCond->condSpace;
	out << TABS(level) << L"_widec = " << 
			KEY(condSpace->baseKey) << L" + (" << CAST(WIDE_ALPH_TYPE(), GET_KEY()) <<
			L" - " << KEY(keyOps->minKey) << L")" << endl;

	for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
		out << TABS(level) << L"if ";
		CONDITION( out, *csi );
		Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
		out << L" {" << endl;
		out << TABS(level + 1) << L"_widec += " << condValOffset << endl;
		out << TABS(level) << L"}" << endl;
	}
}

void GoGotoCodeGen::emitCondBSearch( RedStateAp *state, int level, int low, int high )
{
	/* Get the mid position, staying on the lower end of the range. */
	int mid = (low + high) >> 1;
	GenStateCond **data = state->stateCondVect.data;

	/* Determine if we need to look higher or lower. */
	bool anyLower = mid > low;
	bool anyHigher = mid < high;

	/* Determine if the keys at mid are the limits of the alphabet. */
	bool limitLow = data[mid]->lowKey == keyOps->minKey;
	bool limitHigh = data[mid]->highKey == keyOps->maxKey;

	if ( anyLower && anyHigher ) {
		/* Can go lower and higher than mid. */
		out << TABS(level) << L"switch {" << endl;
		out << TABS(level) << L"case " << GET_KEY() << L" < " <<
				KEY(data[mid]->lowKey) << L":" << endl;
		emitCondBSearch( state, level+1, low, mid-1 );
		out << TABS(level) << L"case " << GET_KEY() << L" > " <<
				KEY(data[mid]->highKey) << L":" << endl;
		emitCondBSearch( state, level+1, mid+1, high );
		out << TABS(level) << L"default:" << endl;
		COND_TRANSLATE(data[mid], level+1);
		out << TABS(level) << L"}" << endl;
	}
	else if ( anyLower && !anyHigher ) {
		/* Can go lower than mid but not higher. */
		out << TABS(level) << L"switch {" << endl;
		out << TABS(level) << L"case " << GET_KEY() << L" < " <<
				KEY(data[mid]->lowKey) << L":" << endl;
		emitCondBSearch( state, level+1, low, mid-1 );

		/* if the higher is the highest in the alphabet then there is no
		 * sense testing it. */
		if ( limitHigh ) {
			out << TABS(level) << L"default:" << endl;
			COND_TRANSLATE(data[mid], level+1);
		}
		else {
			out << TABS(level) << L"case " << GET_KEY() << L" <= " <<
					KEY(data[mid]->highKey) << L":" << endl;
			COND_TRANSLATE(data[mid], level+1);
		}
		out << TABS(level) << L"}" << endl;
	}
	else if ( !anyLower && anyHigher ) {
		/* Can go higher than mid but not lower. */
		out << TABS(level) << L"switch {" << endl;
		out << TABS(level) << L"case " << GET_KEY() << L" > " <<
				KEY(data[mid]->highKey) << L":" << endl;
		emitCondBSearch( state, level+1, mid+1, high );

		/* If the lower end is the lowest in the alphabet then there is no
		 * sense testing it. */
		if ( limitLow ) {
			out << TABS(level) << L"default:" << endl;
			COND_TRANSLATE(data[mid], level+1);
		}
		else {
			out << TABS(level) << L"case " << GET_KEY() << L" >= " <<
					KEY(data[mid]->lowKey) << L":" << endl;
			COND_TRANSLATE(data[mid], level+1);
		}
		out << TABS(level) << L"}" << endl;
	}
	else {
		/* Cannot go higher or lower than mid. It's mid or bust. What
		 * tests to do depends on limits of alphabet. */
		if ( !limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid]->lowKey) << L" <= " <<
					GET_KEY() << L" && " << GET_KEY() << L" <= " <<
					KEY(data[mid]->highKey) << L" {" << endl;
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}" << endl;
		}
		else if ( limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << GET_KEY() << L" <= " <<
					KEY(data[mid]->highKey) << L" {" << endl;
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}" << endl;
		}
		else if ( !limitLow && limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid]->lowKey) << L" <= " <<
					GET_KEY() << L" {" << endl;
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}" << endl;
		}
		else {
			/* Both high and low are at the limit. No tests to do. */
			COND_TRANSLATE(data[mid], level);
		}
	}
}

std::wostream &GoGotoCodeGen::STATE_GOTOS( int level )
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st == redFsm->errState )
			STATE_GOTO_ERROR(level);
		else {
			/* Writing code above state gotos. */
			GOTO_HEADER( st, level );

			if ( st->stateCondVect.length() > 0 ) {
				out << TABS(level + 1) << L"_widec = " << CAST(WIDE_ALPH_TYPE(), GET_KEY()) << endl;
				emitCondBSearch( st, level + 1, 0, st->stateCondVect.length() - 1 );
			}

			/* Try singles. */
			if ( st->outSingle.length() > 0 )
				emitSingleSwitch( st, level + 1 );

			/* Default case is to binary search for the ranges, if that fails then */
			if ( st->outRange.length() > 0 )
				emitRangeBSearch( st, level + 1, 0, st->outRange.length() - 1 );

			/* Write the default transition. */
			TRANS_GOTO( st->defTrans, level + 1 ) << endl;
		}
	}
	return out;
}

std::wostream &GoGotoCodeGen::TRANSITIONS()
{
	/* Emit any transitions that have functions and that go to
	 * this state. */
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ ) {
		/* Write the label for the transition so it can be jumped to. */
		out << L"	tr" << trans->id << L": ";

		/* Destination state. */
		if ( trans->action != 0 && trans->action->anyCurStateRef() )
			out << L"_ps = " << vCS() << L";";
		out << vCS() << L" = " << trans->targ->id << L"; ";

		if ( trans->action != 0 ) {
			/* Write out the transition func. */
			out << L"goto f" << trans->action->actListId << endl;
		}
		else {
			/* No code to execute, just loop around. */
			out << L"goto _again" << endl;
		}
	}
	return out;
}

std::wostream &GoGotoCodeGen::EXEC_FUNCS()
{
	/* Make labels that set acts and jump to execFuncs. Loop func indicies. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			out << L"	f" << redAct->actListId << L": " <<
				L"_acts = " << (redAct->location + 1) << L";"
				L" goto execFuncs" << endl;
		}
	}

	out <<
		endl <<
		L"execFuncs:" << endl <<
		L"	_nacts = " << CAST(UINT(), A() + L"[_acts]") << L"; _acts++" << endl <<
		L"	for ; _nacts > 0; _nacts-- {" << endl <<
		L"		_acts++" << endl <<
		L"		switch " << A() << L"[_acts - 1]" << L" {" << endl;
		ACTION_SWITCH(2);
		out <<
		L"		}" << endl <<
		L"	}" << endl <<
		L"	goto _again" << endl;
	return out;
}

unsigned int GoGotoCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	return act;
}

unsigned int GoGotoCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	return act;
}

unsigned int GoGotoCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	return act;
}

std::wostream &GoGotoCodeGen::TO_STATE_ACTIONS()
{
	/* Take one off for the psuedo start state. */
	int numStates = redFsm->stateList.length();
	unsigned int *vals = new unsigned int[numStates];
	memset( vals, 0, sizeof(unsigned int)*numStates );

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		vals[st->id] = TO_STATE_ACTION(st);

	out << L"	";
	for ( int st = 0; st < redFsm->nextStateId; st++ ) {
		/* Write any eof action. */
		out << vals[st] << L", ";
		if ( st < numStates-1 ) {
			if ( (st+1) % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	delete[] vals;
	return out;
}

std::wostream &GoGotoCodeGen::FROM_STATE_ACTIONS()
{
	/* Take one off for the psuedo start state. */
	int numStates = redFsm->stateList.length();
	unsigned int *vals = new unsigned int[numStates];
	memset( vals, 0, sizeof(unsigned int)*numStates );

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		vals[st->id] = FROM_STATE_ACTION(st);

	out << L"	";
	for ( int st = 0; st < redFsm->nextStateId; st++ ) {
		/* Write any eof action. */
		out << vals[st] << L", ";
		if ( st < numStates-1 ) {
			if ( (st+1) % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	delete[] vals;
	return out;
}

std::wostream &GoGotoCodeGen::EOF_ACTIONS()
{
	/* Take one off for the psuedo start state. */
	int numStates = redFsm->stateList.length();
	unsigned int *vals = new unsigned int[numStates];
	memset( vals, 0, sizeof(unsigned int)*numStates );

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		vals[st->id] = EOF_ACTION(st);

	out << L"	";
	for ( int st = 0; st < redFsm->nextStateId; st++ ) {
		/* Write any eof action. */
		out << vals[st] << L", ";
		if ( st < numStates-1 ) {
			if ( (st+1) % IALL == 0 )
				out << endl << L"	";
		}
	}
	out << endl;
	delete[] vals;
	return out;
}

std::wostream &GoGotoCodeGen::FINISH_CASES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* States that are final and have an out action need a case. */
		if ( st->eofAction != 0 ) {
			/* Write the case label. */
			out << TABS(2) << L"case " << st->id << L":" << endl;

			/* Write the goto func. */
			out << TABS(3) << L"goto f" << st->eofAction->actListId << endl;
		}
	}

	return out;
}

void GoGotoCodeGen::writeData()
{
	if ( redFsm->anyActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActArrItem), A() );
		ACTIONS_ARRAY();
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

	STATE_IDS();
}

void GoGotoCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out << L"	{" << endl;

	if ( redFsm->anyRegCurStateRef() )
		out << L"	var _ps " << INT() << L" = 0" << endl;

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions()
			|| redFsm->anyFromStateActions() )
	{
		out <<
			L"	var _acts " << INT() << endl <<
			L"	var _nacts " << UINT() << endl;
	}

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

	out <<
		L"	switch " << vCS() << L" {" << endl;
		STATE_GOTOS(1);
		out <<
		L"	}" << endl <<
		endl;
		TRANSITIONS() <<
		endl;

	if ( redFsm->anyRegActions() )
		EXEC_FUNCS() << endl;

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
				L"		switch " << vCS() << L" {" << endl;

			for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
				if ( st->eofTrans != 0 )
					out <<
						L"		case " << st->id << L":" << endl <<
						L"			goto tr" << st->eofTrans->id << endl;
			}

			out <<
				L"	}" << endl;
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
