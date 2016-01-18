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
#include "cdgoto.h"
#include "redfsm.h"
#include "bstmap.h"
#include "gendata.h"

/* Emit the goto to take for a given transition. */
std::wostream &GotoCodeGen::TRANS_GOTO( RedTransAp *trans, int level )
{
	out << TABS(level) << L"goto tr" << trans->id << L";";
	return out;
}

std::wostream &GotoCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GotoCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GotoCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, true, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &GotoCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\tcase " << act->actionId << L":\n";
			ACTION( out, act, 0, false, false );
			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

void GotoCodeGen::GOTO_HEADER( RedStateAp *state )
{
	/* Label the state. */
	out << L"case " << state->id << L":\n";
}


void GotoCodeGen::emitSingleSwitch( RedStateAp *state )
{
	/* Load up the singles. */
	int numSingles = state->outSingle.length();
	RedTransEl *data = state->outSingle.data;

	if ( numSingles == 1 ) {
		/* If there is a single single key then write it out as an if. */
		out << L"\tif ( " << GET_WIDE_KEY(state) << L" == " << 
				WIDE_KEY(state, data[0].lowKey) << L" )\n\t\t"; 

		/* Virtual function for writing the target of the transition. */
		TRANS_GOTO(data[0].value, 0) << L"\n";
	}
	else if ( numSingles > 1 ) {
		/* Write out single keys in a switch if there is more than one. */
		out << L"\tswitch( " << GET_WIDE_KEY(state) << L" ) {\n";

		/* Write out the single indicies. */
		for ( int j = 0; j < numSingles; j++ ) {
			out << L"\t\tcase " << WIDE_KEY(state, data[j].lowKey) << L": ";
			TRANS_GOTO(data[j].value, 0) << L"\n";
		}
		
		/* Emits a default case for D code. */
		SWITCH_DEFAULT();

		/* Close off the transition switch. */
		out << L"\t}\n";
	}
}

void GotoCodeGen::emitRangeBSearch( RedStateAp *state, int level, int low, int high )
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
		out << TABS(level) << L"if ( " << GET_WIDE_KEY(state) << L" < " << 
				WIDE_KEY(state, data[mid].lowKey) << L" ) {\n";
		emitRangeBSearch( state, level+1, low, mid-1 );
		out << TABS(level) << L"} else if ( " << GET_WIDE_KEY(state) << L" > " << 
				WIDE_KEY(state, data[mid].highKey) << L" ) {\n";
		emitRangeBSearch( state, level+1, mid+1, high );
		out << TABS(level) << L"} else\n";
		TRANS_GOTO(data[mid].value, level+1) << L"\n";
	}
	else if ( anyLower && !anyHigher ) {
		/* Can go lower than mid but not higher. */
		out << TABS(level) << L"if ( " << GET_WIDE_KEY(state) << L" < " << 
				WIDE_KEY(state, data[mid].lowKey) << L" ) {\n";
		emitRangeBSearch( state, level+1, low, mid-1 );

		/* if the higher is the highest in the alphabet then there is no
		 * sense testing it. */
		if ( limitHigh ) {
			out << TABS(level) << L"} else\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else {
			out << TABS(level) << L"} else if ( " << GET_WIDE_KEY(state) << L" <= " << 
					WIDE_KEY(state, data[mid].highKey) << L" )\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
	}
	else if ( !anyLower && anyHigher ) {
		/* Can go higher than mid but not lower. */
		out << TABS(level) << L"if ( " << GET_WIDE_KEY(state) << L" > " << 
				WIDE_KEY(state, data[mid].highKey) << L" ) {\n";
		emitRangeBSearch( state, level+1, mid+1, high );

		/* If the lower end is the lowest in the alphabet then there is no
		 * sense testing it. */
		if ( limitLow ) {
			out << TABS(level) << L"} else\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else {
			out << TABS(level) << L"} else if ( " << GET_WIDE_KEY(state) << L" >= " << 
					WIDE_KEY(state, data[mid].lowKey) << L" )\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
	}
	else {
		/* Cannot go higher or lower than mid. It's mid or bust. What
		 * tests to do depends on limits of alphabet. */
		if ( !limitLow && !limitHigh ) {
			out << TABS(level) << L"if ( " << WIDE_KEY(state, data[mid].lowKey) << L" <= " << 
					GET_WIDE_KEY(state) << L" && " << GET_WIDE_KEY(state) << L" <= " << 
					WIDE_KEY(state, data[mid].highKey) << L" )\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else if ( limitLow && !limitHigh ) {
			out << TABS(level) << L"if ( " << GET_WIDE_KEY(state) << L" <= " << 
					WIDE_KEY(state, data[mid].highKey) << L" )\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else if ( !limitLow && limitHigh ) {
			out << TABS(level) << L"if ( " << WIDE_KEY(state, data[mid].lowKey) << L" <= " << 
					GET_WIDE_KEY(state) << L" )\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else {
			/* Both high and low are at the limit. No tests to do. */
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
	}
}

void GotoCodeGen::STATE_GOTO_ERROR()
{
	/* Label the state and bail immediately. */
	outLabelUsed = true;
	RedStateAp *state = redFsm->errState;
	out << L"case " << state->id << L":\n";
	out << L"	goto _out;\n";
}

void GotoCodeGen::COND_TRANSLATE( GenStateCond *stateCond, int level )
{
	GenCondSpace *condSpace = stateCond->condSpace;
	out << TABS(level) << L"_widec = " << CAST(WIDE_ALPH_TYPE()) << L"(" <<
			KEY(condSpace->baseKey) << L" + (" << GET_KEY() << 
			L" - " << KEY(keyOps->minKey) << L"));\n";

	for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
		out << TABS(level) << L"if ( ";
		CONDITION( out, *csi );
		Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
		out << L" ) _widec += " << condValOffset << L";\n";
	}
}

void GotoCodeGen::emitCondBSearch( RedStateAp *state, int level, int low, int high )
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
		out << TABS(level) << L"if ( " << GET_KEY() << L" < " << 
				KEY(data[mid]->lowKey) << L" ) {\n";
		emitCondBSearch( state, level+1, low, mid-1 );
		out << TABS(level) << L"} else if ( " << GET_KEY() << L" > " << 
				KEY(data[mid]->highKey) << L" ) {\n";
		emitCondBSearch( state, level+1, mid+1, high );
		out << TABS(level) << L"} else {\n";
		COND_TRANSLATE(data[mid], level+1);
		out << TABS(level) << L"}\n";
	}
	else if ( anyLower && !anyHigher ) {
		/* Can go lower than mid but not higher. */
		out << TABS(level) << L"if ( " << GET_KEY() << L" < " << 
				KEY(data[mid]->lowKey) << L" ) {\n";
		emitCondBSearch( state, level+1, low, mid-1 );

		/* if the higher is the highest in the alphabet then there is no
		 * sense testing it. */
		if ( limitHigh ) {
			out << TABS(level) << L"} else {\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}\n";
		}
		else {
			out << TABS(level) << L"} else if ( " << GET_KEY() << L" <= " << 
					KEY(data[mid]->highKey) << L" ) {\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}\n";
		}
	}
	else if ( !anyLower && anyHigher ) {
		/* Can go higher than mid but not lower. */
		out << TABS(level) << L"if ( " << GET_KEY() << L" > " << 
				KEY(data[mid]->highKey) << L" ) {\n";
		emitCondBSearch( state, level+1, mid+1, high );

		/* If the lower end is the lowest in the alphabet then there is no
		 * sense testing it. */
		if ( limitLow ) {
			out << TABS(level) << L"} else {\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}\n";
		}
		else {
			out << TABS(level) << L"} else if ( " << GET_KEY() << L" >= " << 
					KEY(data[mid]->lowKey) << L" ) {\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}\n";
		}
	}
	else {
		/* Cannot go higher or lower than mid. It's mid or bust. What
		 * tests to do depends on limits of alphabet. */
		if ( !limitLow && !limitHigh ) {
			out << TABS(level) << L"if ( " << KEY(data[mid]->lowKey) << L" <= " << 
					GET_KEY() << L" && " << GET_KEY() << L" <= " << 
					KEY(data[mid]->highKey) << L" ) {\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}\n";
		}
		else if ( limitLow && !limitHigh ) {
			out << TABS(level) << L"if ( " << GET_KEY() << L" <= " << 
					KEY(data[mid]->highKey) << L" ) {\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}\n";
		}
		else if ( !limitLow && limitHigh ) {
			out << TABS(level) << L"if ( " << KEY(data[mid]->lowKey) << L" <= " << 
					GET_KEY() << L" )\n {";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"}\n";
		}
		else {
			/* Both high and low are at the limit. No tests to do. */
			COND_TRANSLATE(data[mid], level);
		}
	}
}

std::wostream &GotoCodeGen::STATE_GOTOS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st == redFsm->errState )
			STATE_GOTO_ERROR();
		else {
			/* Writing code above state gotos. */
			GOTO_HEADER( st );

			if ( st->stateCondVect.length() > 0 ) {
				out << L"	_widec = " << GET_KEY() << L";\n";
				emitCondBSearch( st, 1, 0, st->stateCondVect.length() - 1 );
			}

			/* Try singles. */
			if ( st->outSingle.length() > 0 )
				emitSingleSwitch( st );

			/* Default case is to binary search for the ranges, if that fails then */
			if ( st->outRange.length() > 0 )
				emitRangeBSearch( st, 1, 0, st->outRange.length() - 1 );

			/* Write the default transition. */
			TRANS_GOTO( st->defTrans, 1 ) << L"\n";
		}
	}
	return out;
}

std::wostream &GotoCodeGen::TRANSITIONS()
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
			out << L"goto f" << trans->action->actListId << L";\n";
		}
		else {
			/* No code to execute, just loop around. */
			out << L"goto _again;\n";
		}
	}
	return out;
}

std::wostream &GotoCodeGen::EXEC_FUNCS()
{
	/* Make labels that set acts and jump to execFuncs. Loop func indicies. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			out << L"	f" << redAct->actListId << L": " <<
				L"_acts = " << ARR_OFF(A(), itoa( redAct->location+1 ) ) << L";"
				L" goto execFuncs;\n";
		}
	}

	out <<
		L"\n"
		L"execFuncs:\n"
		L"	_nacts = *_acts++;\n"
		L"	while ( _nacts-- > 0 ) {\n"
		L"		switch ( *_acts++ ) {\n";
		ACTION_SWITCH();
		SWITCH_DEFAULT() <<
		L"		}\n"
		L"	}\n"
		L"	goto _again;\n";
	return out;
}

unsigned int GotoCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	return act;
}

unsigned int GotoCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	return act;
}

unsigned int GotoCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	return act;
}

std::wostream &GotoCodeGen::TO_STATE_ACTIONS()
{
	/* Take one off for the psuedo start state. */
	int numStates = redFsm->stateList.length();
	unsigned int *vals = new unsigned int[numStates];
	memset( vals, 0, sizeof(unsigned int)*numStates );

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		vals[st->id] = TO_STATE_ACTION(st);

	out << L"\t";
	for ( int st = 0; st < redFsm->nextStateId; st++ ) {
		/* Write any eof action. */
		out << vals[st];
		if ( st < numStates-1 ) {
			out << L", ";
			if ( (st+1) % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] vals;
	return out;
}

std::wostream &GotoCodeGen::FROM_STATE_ACTIONS()
{
	/* Take one off for the psuedo start state. */
	int numStates = redFsm->stateList.length();
	unsigned int *vals = new unsigned int[numStates];
	memset( vals, 0, sizeof(unsigned int)*numStates );

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		vals[st->id] = FROM_STATE_ACTION(st);

	out << L"\t";
	for ( int st = 0; st < redFsm->nextStateId; st++ ) {
		/* Write any eof action. */
		out << vals[st];
		if ( st < numStates-1 ) {
			out << L", ";
			if ( (st+1) % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] vals;
	return out;
}

std::wostream &GotoCodeGen::EOF_ACTIONS()
{
	/* Take one off for the psuedo start state. */
	int numStates = redFsm->stateList.length();
	unsigned int *vals = new unsigned int[numStates];
	memset( vals, 0, sizeof(unsigned int)*numStates );

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		vals[st->id] = EOF_ACTION(st);

	out << L"\t";
	for ( int st = 0; st < redFsm->nextStateId; st++ ) {
		/* Write any eof action. */
		out << vals[st];
		if ( st < numStates-1 ) {
			out << L", ";
			if ( (st+1) % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] vals;
	return out;
}

std::wostream &GotoCodeGen::FINISH_CASES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* States that are final and have an out action need a case. */
		if ( st->eofAction != 0 ) {
			/* Write the case label. */
			out << L"\t\tcase " << st->id << L": ";

			/* Write the goto func. */
			out << L"goto f" << st->eofAction->actListId << L";\n";
		}
	}
	
	return out;
}

void GotoCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << gotoDest << L"; " << 
			CTRL_FLOW() << L"goto _again;}";
}

void GotoCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"{" << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
	ret << L"); " << CTRL_FLOW() << L"goto _again;}";
}

void GotoCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void GotoCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void GotoCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void GotoCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
	ret << L");";
}

void GotoCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = " << 
			callDest << L"; " << CTRL_FLOW() << L"goto _again;}";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void GotoCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, targState, inFinish, false );
	ret << L"); " << CTRL_FLOW() << L"goto _again;}";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void GotoCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << STACK() << L"[--" << TOP() << L"];";

	if ( postPopExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, postPopExpr, 0, false, false );
		ret << L"}";
	}

	ret << CTRL_FLOW() << L"goto _again;}";
}

void GotoCodeGen::BREAK( wostream &ret, int targState, bool csForced )
{
	outLabelUsed = true;
	ret << L"{" << P() << L"++; " << CTRL_FLOW() << L"goto _out; }";
}

void GotoCodeGen::writeData()
{
	if ( redFsm->anyActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActArrItem), A() );
		ACTIONS_ARRAY();
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

	STATE_IDS();
}

void GotoCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out << L"	{\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	int _ps = 0;\n";

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() 
			|| redFsm->anyFromStateActions() )
	{
		out << 
			L"	" << PTR_CONST() << ARRAY_TYPE(redFsm->maxActArrItem) << PTR_CONST_END() << POINTER() << L"_acts;\n"
			L"	" << UINT() << L" _nacts;\n";
	}

	if ( redFsm->anyConditions() )
		out << L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";

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
			L"	_acts = " << ARR_OFF( A(), FSA() + L"[" + vCS() + L"]" ) << L";\n"
			L"	_nacts = " << CAST(UINT()) << L" *_acts++;\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( *_acts++ ) {\n";
			FROM_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	out <<
		L"	switch ( " << vCS() << L" ) {\n";
		STATE_GOTOS();
		SWITCH_DEFAULT() <<
		L"	}\n"
		L"\n";
		TRANSITIONS() <<
		L"\n";

	if ( redFsm->anyRegActions() )
		EXEC_FUNCS() << L"\n";

	out << L"_again:\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	_acts = " << ARR_OFF( A(), TSA() + L"[" + vCS() + L"]" ) << L";\n"
			L"	_nacts = " << CAST(UINT()) << L" *_acts++;\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( *_acts++ ) {\n";
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
				L"	switch ( " << vCS() << L" ) {\n";

			for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
				if ( st->eofTrans != 0 )
					out << L"	case " << st->id << L": goto tr" << st->eofTrans->id << L";\n";
			}

			SWITCH_DEFAULT() <<
				L"	}\n";
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	" << PTR_CONST() << ARRAY_TYPE(redFsm->maxActArrItem) << PTR_CONST_END() << 
						POINTER() << L"__acts = " << 
						ARR_OFF( A(), EA() + L"[" + vCS() + L"]" ) << L";\n"
				L"	" << UINT() << L" __nacts = " << CAST(UINT()) << L" *__acts++;\n"
				L"	while ( __nacts-- > 0 ) {\n"
				L"		switch ( *__acts++ ) {\n";
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
