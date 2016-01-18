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
#include "mlgoto.h"
#include "redfsm.h"
#include "bstmap.h"
#include "gendata.h"

/* Emit the goto to take for a given transition. */
std::wostream &OCamlGotoCodeGen::TRANS_GOTO( RedTransAp *trans, int level )
{
	out << TABS(level) << L"tr" << trans->id << L" ()";
	return out;
}

std::wostream &OCamlGotoCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, false );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlGotoCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, false );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlGotoCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, true );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlGotoCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, false );
			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

void OCamlGotoCodeGen::GOTO_HEADER( RedStateAp *state )
{
	/* Label the state. */
	out << L"| " << state->id << L" ->\n";
}


void OCamlGotoCodeGen::emitSingleSwitch( RedStateAp *state )
{
	/* Load up the singles. */
	int numSingles = state->outSingle.length();
	RedTransEl *data = state->outSingle.data;

	if ( numSingles == 1 ) {
		/* If there is a single single key then write it out as an if. */
		out << L"\tif " << GET_WIDE_KEY(state) << L" = " << 
				KEY(data[0].lowKey) << L" then\n\t\t"; 

		/* Virtual function for writing the target of the transition. */
		TRANS_GOTO(data[0].value, 0) << L" else\n";
	}
	else if ( numSingles > 1 ) {
		/* Write out single keys in a switch if there is more than one. */
		out << L"\tmatch " << GET_WIDE_KEY(state) << L" with\n";

		/* Write out the single indicies. */
		for ( int j = 0; j < numSingles; j++ ) {
			out << L"\t\t| " << ALPHA_KEY(data[j].lowKey) << L" -> ";
			TRANS_GOTO(data[j].value, 0) << L"\n";
		}

		out << L"\t\t| _ ->\n";
	}
}

void OCamlGotoCodeGen::emitRangeBSearch( RedStateAp *state, int level, int low, int high, RedTransAp* def)
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
		out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" < " << 
				KEY(data[mid].lowKey) << L" then begin\n";
		emitRangeBSearch( state, level+1, low, mid-1, def );
		out << TABS(level) << L" end else if " << GET_WIDE_KEY(state) << L" > " << 
				KEY(data[mid].highKey) << L" then begin\n";
		emitRangeBSearch( state, level+1, mid+1, high, def );
		out << TABS(level) << L" end else\n";
		TRANS_GOTO(data[mid].value, level+1) << L"\n";
	}
	else if ( anyLower && !anyHigher ) {
		/* Can go lower than mid but not higher. */
		out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" < " << 
				KEY(data[mid].lowKey) << L" then begin\n";
		emitRangeBSearch( state, level+1, low, mid-1, def );

		/* if the higher is the highest in the alphabet then there is no
		 * sense testing it. */
		if ( limitHigh ) {
			out << TABS(level) << L" end else\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else {
			out << TABS(level) << L" end else if " << GET_WIDE_KEY(state) << L" <= " << 
					KEY(data[mid].highKey) << L" then\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n" << TABS(level) << L"else\n";
      TRANS_GOTO(def, level+1) << L"\n";
		}
	}
	else if ( !anyLower && anyHigher ) {
		/* Can go higher than mid but not lower. */
		out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" > " << 
				KEY(data[mid].highKey) << L" then begin\n";
		emitRangeBSearch( state, level+1, mid+1, high, def );

		/* If the lower end is the lowest in the alphabet then there is no
		 * sense testing it. */
		if ( limitLow ) {
			out << TABS(level) << L" end else\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else {
			out << TABS(level) << L" end else if " << GET_WIDE_KEY(state) << L" >= " << 
					KEY(data[mid].lowKey) << L" then\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n" << TABS(level) << L"else\n";
      TRANS_GOTO(def, level+1) << L"\n";
		}
	}
	else {
		/* Cannot go higher or lower than mid. It's mid or bust. What
		 * tests to do depends on limits of alphabet. */
		if ( !limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid].lowKey) << L" <= " << 
					GET_WIDE_KEY(state) << L" && " << GET_WIDE_KEY(state) << L" <= " << 
					KEY(data[mid].highKey) << L" then\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n" << TABS(level) << L"else\n";
      TRANS_GOTO(def, level+1) << L"\n";
		}
		else if ( limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" <= " << 
					KEY(data[mid].highKey) << L" then\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n" << TABS(level) << L"else\n";
      TRANS_GOTO(def, level+1) << L"\n";
		}
		else if ( !limitLow && limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid].lowKey) << L" <= " << 
					GET_WIDE_KEY(state) << L" then\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n" << TABS(level) << L"else\n";
      TRANS_GOTO(def, level+1) << L"\n";
		}
		else {
			/* Both high and low are at the limit. No tests to do. */
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
	}
}

void OCamlGotoCodeGen::STATE_GOTO_ERROR()
{
	/* Label the state and bail immediately. */
	outLabelUsed = true;
	RedStateAp *state = redFsm->errState;
	out << L"| " << state->id << L" ->\n";
	out << L"	do_out ()\n";
}

void OCamlGotoCodeGen::COND_TRANSLATE( GenStateCond *stateCond, int level )
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

void OCamlGotoCodeGen::emitCondBSearch( RedStateAp *state, int level, int low, int high )
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

std::wostream &OCamlGotoCodeGen::STATE_GOTOS()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st == redFsm->errState )
			STATE_GOTO_ERROR();
		else {
			/* Writing code above state gotos. */
			GOTO_HEADER( st );
      out << L"\tbegin\n";

			if ( st->stateCondVect.length() > 0 ) {
				out << L"	_widec = " << GET_KEY() << L";\n";
				emitCondBSearch( st, 1, 0, st->stateCondVect.length() - 1 );
			}

			/* Try singles. */
			if ( st->outSingle.length() > 0 )
				emitSingleSwitch( st );

			/* Default case is to binary search for the ranges, if that fails then */
			if ( st->outRange.length() > 0 )
				emitRangeBSearch( st, 1, 0, st->outRange.length() - 1, st->defTrans );
      else
  			/* Write the default transition. */
  			TRANS_GOTO( st->defTrans, 1 ) << L"\n";

      out << L"\tend\n";
		}
	}
	return out;
}

std::wostream &OCamlGotoCodeGen::TRANSITIONS()
{
	/* Emit any transitions that have functions and that go to 
	 * this state. */
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ ) {
		/* Write the label for the transition so it can be jumped to. */
		out << L"	and tr" << trans->id << L" () = ";

		/* Destination state. */
		if ( trans->action != 0 && trans->action->anyCurStateRef() )
			out << L"_ps = " << vCS() << L";";
		out << vCS() << L" <- " << trans->targ->id << L"; ";

		if ( trans->action != 0 ) {
			/* Write out the transition func. */
			out << L"f" << trans->action->actListId << L" ()\n";
		}
		else {
			/* No code to execute, just loop around. */
			out << L"do_again ()\n";
		}
	}
	return out;
}

std::wostream &OCamlGotoCodeGen::EXEC_FUNCS()
{
	/* Make labels that set acts and jump to execFuncs. Loop func indicies. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			out << L"	and f" << redAct->actListId << L" () = " <<
				L"state.acts <- " << itoa( redAct->location+1 ) << L"; "
				L"execFuncs ()\n";
		}
	}

	out <<
		L"\n"
		L"and execFuncs () =\n"
		L"	state.nacts <- " << AT( A(), POST_INCR( L"state.acts") ) << L";\n"
		L"	begin try while " << POST_DECR(L"state.nacts") << L" > 0 do\n"
		L"		match " << AT( A(), POST_INCR(L"state.acts") ) << L" with\n";
		ACTION_SWITCH();
		SWITCH_DEFAULT() <<
		L"	done with Goto_again -> () end;\n"
		L"	do_again ()\n";
	return out;
}

unsigned int OCamlGotoCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	return act;
}

unsigned int OCamlGotoCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	return act;
}

unsigned int OCamlGotoCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	return act;
}

std::wostream &OCamlGotoCodeGen::TO_STATE_ACTIONS()
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
			out << ARR_SEP();
			if ( (st+1) % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] vals;
	return out;
}

std::wostream &OCamlGotoCodeGen::FROM_STATE_ACTIONS()
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
			out << ARR_SEP();
			if ( (st+1) % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] vals;
	return out;
}

std::wostream &OCamlGotoCodeGen::EOF_ACTIONS()
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
			out << ARR_SEP();
			if ( (st+1) % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] vals;
	return out;
}

std::wostream &OCamlGotoCodeGen::FINISH_CASES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* States that are final and have an out action need a case. */
		if ( st->eofAction != 0 ) {
			/* Write the case label. */
			out << L"\t\t| " << st->id << L" -> ";

			/* Write the goto func. */
			out << L"f" << st->eofAction->actListId << L" ()\n";
		}
	}
	
	return out;
}

void OCamlGotoCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- " << gotoDest << L"; " << 
			CTRL_FLOW() << L"raise Goto_again end";
}

void OCamlGotoCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L"); " << CTRL_FLOW() << L"raise Goto_again end";
}

void OCamlGotoCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void OCamlGotoCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void OCamlGotoCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" <- " << nextDest << L";";
}

void OCamlGotoCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" <- (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}

void OCamlGotoCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
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

void OCamlGotoCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
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

void OCamlGotoCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- " << AT(STACK(), PRE_DECR(TOP()) ) << L"; ";

	if ( postPopExpr != 0 ) {
		ret << L"begin ";
		INLINE_LIST( ret, postPopExpr, 0, false );
		ret << L"end ";
	}

	ret << CTRL_FLOW() <<  L"raise Goto_again end";
}

void OCamlGotoCodeGen::BREAK( wostream &ret, int targState )
{
	outLabelUsed = true;
	ret << L"begin " << P() << L" <- " << P() << L" + 1; " << CTRL_FLOW() << L"raise Goto_out end";
}

void OCamlGotoCodeGen::writeData()
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

  out << L"type " << TYPE_STATE() << L" = { mutable acts : " << ARRAY_TYPE(redFsm->maxActionLoc) <<
         L" ; mutable nacts : " << ARRAY_TYPE(redFsm->maxActArrItem) << L"; }"
    << TOP_SEP();

  out << L"exception Goto_again" << TOP_SEP();
}

void OCamlGotoCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out << L"	begin\n";

//	if ( redFsm->anyRegCurStateRef() )
//		out << L"	int _ps = 0;\n";

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() 
			|| redFsm->anyFromStateActions() )
	{
		out << L"	let state = { acts = 0; nacts = 0; } in\n";
	}

//	if ( redFsm->anyConditions() )
//		out << L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";

	out << L"\n";
  out << L"	let rec do_start () =\n";

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

	out <<
		L"	begin match " << vCS() << L" with\n";
		STATE_GOTOS();
		SWITCH_DEFAULT() <<
		L"	end\n"
		L"\n";
		TRANSITIONS() <<
		L"\n";

	if ( redFsm->anyRegActions() )
		EXEC_FUNCS() << L"\n";

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
			L"	begin\n";

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"	match " << vCS() << L" with\n";

			for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
				if ( st->eofTrans != 0 )
					out << L"	| " << st->id << L" -> tr" << st->eofTrans->id << L" ()\n";
			}

			out << L"\t| _ -> ();\n";
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	let __acts = ref " << AT( EA(), vCS() ) << L" in\n"
				L"	let __nacts = ref " << AT( A(), L"!__acts" ) << L" in\n"
        L" incr __acts;\n"
				L"	begin try while !__nacts > 0 do\n"
        L"   decr __nacts;\n"
				L"		begin match " << AT( A(), POST_INCR(L"__acts.contents") ) << L" with\n";
				EOF_ACTION_SWITCH();
				SWITCH_DEFAULT() <<
				L"		end;\n"
				L"	done with Goto_again -> do_again () end;\n";
		}

		out <<
			L"	end\n"
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
