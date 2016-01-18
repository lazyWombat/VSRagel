/*
 *  Copyright 2007 Victor Hugo Borja <vic@rubyforge.org>
 *            2006-2007 Adrian Thurston <thurston@complang.org>
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

#include <stdio.h>
#include <string>

#include "rbxgoto.h"
#include "ragel.h"
#include "redfsm.h"
#include "bstmap.h"
#include "gendata.h"

using std::wostream;
using std::wstring;

inline wstring label(wstring a, int i)
{
	return a + itoa(i);
}

wostream &RbxGotoCodeGen::rbxLabel(wostream &out, wstring label)
{
	out << L"Rubinius.asm { @labels[:_" << FSM_NAME() << L"_" << label << L"].set! }\n";
	return out;
}

wostream &RbxGotoCodeGen::rbxGoto(wostream &out, wstring label)
{
	out << L"Rubinius.asm { goto @labels[:_" << FSM_NAME() << L"_" << label << L"] }\n";
	return out;
}

/* Emit the goto to take for a given transition. */
std::wostream &RbxGotoCodeGen::TRANS_GOTO( RedTransAp *trans, int level )
{
	out << TABS(level);
	return rbxGoto(out, label(L"tr",trans->id));
}

std::wostream &RbxGotoCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RbxGotoCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RbxGotoCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, true );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RbxGotoCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

void RbxGotoCodeGen::GOTO_HEADER( RedStateAp *state )
{
	/* Label the state. */
	out << L"when " << state->id << L" then\n";
}


void RbxGotoCodeGen::emitSingleSwitch( RedStateAp *state )
{
	/* Load up the singles. */
	int numSingles = state->outSingle.length();
	RedTransEl *data = state->outSingle.data;

	if ( numSingles == 1 ) {
		/* If there is a single single key then write it out as an if. */
		out << L"\tif " << GET_WIDE_KEY(state) << L" == " << 
			KEY(data[0].lowKey) << L" \n\t\t"; 

		/* Virtual function for writing the target of the transition. */
		TRANS_GOTO(data[0].value, 0) << L"\n";

		out << L"end\n";
	}
	else if ( numSingles > 1 ) {
		/* Write out single keys in a switch if there is more than one. */
		out << L"\tcase  " << GET_WIDE_KEY(state) << L"\n";

		/* Write out the single indicies. */
		for ( int j = 0; j < numSingles; j++ ) {
			out << L"\t\twhen " << KEY(data[j].lowKey) << L" then\n";
			TRANS_GOTO(data[j].value, 0) << L"\n";
		}
		
		/* Close off the transition switch. */
		out << L"\tend\n";
	}
}

void RbxGotoCodeGen::emitRangeBSearch( RedStateAp *state, int level, int low, int high )
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
			KEY(data[mid].lowKey) << L" \n";
		emitRangeBSearch( state, level+1, low, mid-1 );
		out << TABS(level) << L"elsif " << GET_WIDE_KEY(state) << L" > " << 
			KEY(data[mid].highKey) << L" \n";
		emitRangeBSearch( state, level+1, mid+1, high );
		out << TABS(level) << L"else\n";
		TRANS_GOTO(data[mid].value, level+1) << L"\n";
		out << TABS(level) << L"end\n";
	}
	else if ( anyLower && !anyHigher ) {
		/* Can go lower than mid but not higher. */
		out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" < " << 
			KEY(data[mid].lowKey) << L" then\n";
		emitRangeBSearch( state, level+1, low, mid-1 );

		/* if the higher is the highest in the alphabet then there is no
		 * sense testing it. */
		if ( limitHigh ) {
			out << TABS(level) << L"else\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else {
			out << TABS(level) << L"elsif" << GET_WIDE_KEY(state) << L" <= " << 
				KEY(data[mid].highKey) << L" )\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		out << TABS(level) << L"end\n";
	}
	else if ( !anyLower && anyHigher ) {
		/* Can go higher than mid but not lower. */
		out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" > " << 
			KEY(data[mid].highKey) << L" \n";
		emitRangeBSearch( state, level+1, mid+1, high );

		/* If the lower end is the lowest in the alphabet then there is no
		 * sense testing it. */
		if ( limitLow ) {
			out << TABS(level) << L"else\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		else {
			out << TABS(level) << L"elsif " << GET_WIDE_KEY(state) << L" >= " << 
				KEY(data[mid].lowKey) << L" then\n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
		out << TABS(level) << L"end\n";
	}
	else {
		/* Cannot go higher or lower than mid. It's mid or bust. What
		 * tests to do depends on limits of alphabet. */
		if ( !limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid].lowKey) << L" <= " << 
				GET_WIDE_KEY(state) << L" && " << GET_WIDE_KEY(state) << L" <= " << 
				KEY(data[mid].highKey) << L" \n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
			out << TABS(level) << L"end\n";
		}
		else if ( limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << GET_WIDE_KEY(state) << L" <= " << 
				KEY(data[mid].highKey) << L" \n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
			out << TABS(level) << L"end\n";
		}
		else if ( !limitLow && limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid].lowKey) << L" <= " << 
				GET_WIDE_KEY(state) << L" \n";
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
			out << TABS(level) << L"end\n";
		}
		else {
			/* Both high and low are at the limit. No tests to do. */
			TRANS_GOTO(data[mid].value, level+1) << L"\n";
		}
	}
}

void RbxGotoCodeGen::STATE_GOTO_ERROR()
{
	/* Label the state and bail immediately. */
	outLabelUsed = true;
	RedStateAp *state = redFsm->errState;
	out << L"when " << state->id << L" then\n";
	rbxGoto(out << L"	", L"_out") << L"\n";
}

void RbxGotoCodeGen::COND_TRANSLATE( GenStateCond *stateCond, int level )
{
	GenCondSpace *condSpace = stateCond->condSpace;
	out << TABS(level) << L"_widec = " <<
		KEY(condSpace->baseKey) << L" + (" << GET_KEY() << 
		L" - " << KEY(keyOps->minKey) << L");\n";

	for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
		out << TABS(level) << L"if ";
		CONDITION( out, *csi );
		Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
		out << L"\n _widec += " << condValOffset << L";\n end";
	}
}

void RbxGotoCodeGen::emitCondBSearch( RedStateAp *state, int level, int low, int high )
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
		out << TABS(level) << L"if " << GET_KEY() << L" < " << 
			KEY(data[mid]->lowKey) << L" \n";
		emitCondBSearch( state, level+1, low, mid-1 );
		out << TABS(level) << L"elsif " << GET_KEY() << L" > " << 
			KEY(data[mid]->highKey) << L" \n";
		emitCondBSearch( state, level+1, mid+1, high );
		out << TABS(level) << L"else\n";
		COND_TRANSLATE(data[mid], level+1);
		out << TABS(level) << L"end\n";
	}
	else if ( anyLower && !anyHigher ) {
		/* Can go lower than mid but not higher. */
		out << TABS(level) << L"if " << GET_KEY() << L" < " << 
			KEY(data[mid]->lowKey) << L" \n";
		emitCondBSearch( state, level+1, low, mid-1 );

		/* if the higher is the highest in the alphabet then there is no
		 * sense testing it. */
		if ( limitHigh ) {
			out << TABS(level) << L"else\n";
			COND_TRANSLATE(data[mid], level+1);
		}
		else {
			out << TABS(level) << L"elsif " << GET_KEY() << L" <= " << 
				KEY(data[mid]->highKey) << L" then\n";
			COND_TRANSLATE(data[mid], level+1);
		}
		out << TABS(level) << L"end\n";

	}
	else if ( !anyLower && anyHigher ) {
		/* Can go higher than mid but not lower. */
		out << TABS(level) << L"if " << GET_KEY() << L" > " << 
			KEY(data[mid]->highKey) << L" \n";
		emitCondBSearch( state, level+1, mid+1, high );

		/* If the lower end is the lowest in the alphabet then there is no
		 * sense testing it. */
		if ( limitLow ) {
			out << TABS(level) << L"else\n";
			COND_TRANSLATE(data[mid], level+1);
		}
		else {
			out << TABS(level) << L"elsif " << GET_KEY() << L" >= " << 
				KEY(data[mid]->lowKey) << L" then\n";
			COND_TRANSLATE(data[mid], level+1);
		}
		out << TABS(level) << L"end\n";
	}
	else {
		/* Cannot go higher or lower than mid. It's mid or bust. What
		 * tests to do depends on limits of alphabet. */
		if ( !limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid]->lowKey) << L" <= " << 
				GET_KEY() << L" && " << GET_KEY() << L" <= " << 
				KEY(data[mid]->highKey) << L" then\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"end\n";
		}
		else if ( limitLow && !limitHigh ) {
			out << TABS(level) << L"if " << GET_KEY() << L" <= " << 
				KEY(data[mid]->highKey) << L" then\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"end\n";
		}
		else if ( !limitLow && limitHigh ) {
			out << TABS(level) << L"if " << KEY(data[mid]->lowKey) << L" <= " << 
				GET_KEY() << L" then\n";
			COND_TRANSLATE(data[mid], level+1);
			out << TABS(level) << L"end\n";
		}
		else {
			/* Both high and low are at the limit. No tests to do. */
			COND_TRANSLATE(data[mid], level);
		}
	}
}

std::wostream &RbxGotoCodeGen::STATE_GOTOS()
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

std::wostream &RbxGotoCodeGen::TRANSITIONS()
{
	/* Emit any transitions that have functions and that go to 
	 * this state. */
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ ) {
		/* Write the label for the transition so it can be jumped to. */
		rbxLabel(out << L"	", label(L"tr", trans->id)) << L"\n";

		/* Destination state. */
		if ( trans->action != 0 && trans->action->anyCurStateRef() )
			out << L"_ps = " << vCS() << L"'n";
		out << vCS() << L" = " << trans->targ->id << L"\n";

		if ( trans->action != 0 ) {
			/* Write out the transition func. */
			rbxGoto(out, label(L"f", trans->action->actListId)) << L"\n";
		}
		else {
			/* No code to execute, just loop around. */
			rbxGoto(out, L"_again") << L"\n";
		}
	}
	return out;
}

std::wostream &RbxGotoCodeGen::EXEC_FUNCS()
{
	/* Make labels that set acts and jump to execFuncs. Loop func indicies. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			rbxLabel(out, label(L"f", redAct->actListId)) << L"\n" <<
				L"_acts = " << itoa( redAct->location+1 ) << L"\n";
			rbxGoto(out, L"execFuncs") << L"\n";
		}
	}

	rbxLabel(out, L"execFuncs") <<
		L"\n"
		L"	_nacts = " << A() << L"[_acts]\n"
		L"	_acts += 1\n"
		L"	while ( _nacts > 0 ) \n"
		L"		_nacts -= 1\n"
		L"		_acts += 1\n"
		L"		case ( "<< A() << L"[_acts-1] ) \n";
	ACTION_SWITCH();
	out <<
		L"		end\n"
		L"	end \n";
	rbxGoto(out, L"_again");
	return out;
}

int RbxGotoCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	return act;
}

int RbxGotoCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	return act;
}

int RbxGotoCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	return act;
}

std::wostream &RbxGotoCodeGen::TO_STATE_ACTIONS()
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

std::wostream &RbxGotoCodeGen::FROM_STATE_ACTIONS()
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

std::wostream &RbxGotoCodeGen::EOF_ACTIONS()
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

std::wostream &RbxGotoCodeGen::FINISH_CASES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* States that are final and have an out action need a case. */
		if ( st->eofAction != 0 ) {
			/* Write the case label. */
			out << L"\t\twhen " << st->id << L" then\n";

			/* Write the goto func. */
			rbxGoto(out, label(L"f", st->eofAction->actListId)) << L"\n";
		}
	}
	
	return out;
}

void RbxGotoCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"begin\n" << vCS() << L" = " << gotoDest << L" ";
	rbxGoto(ret, L"_again") << 
		L"\nend\n";
}

void RbxGotoCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"begin\n" << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L")";
	rbxGoto(ret, L"_again") << 
		L"\nend\n";
}

void RbxGotoCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void RbxGotoCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void RbxGotoCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void RbxGotoCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}

void RbxGotoCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"begin\n" 
	    << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = " << 
		callDest << L"; ";
	rbxGoto(ret, L"_again") << 
		L"\nend\n";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void RbxGotoCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false );
	}

	ret << L"begin\n" << STACK() << L"[" << TOP() << L"++] = " << vCS() << L"; " << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, targState, inFinish );
	ret << L"); ";
	rbxGoto(ret, L"_again") << 
		L"\nend\n";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void RbxGotoCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"begin\n" << vCS() << L" = " << STACK() << L"[--" << TOP() << L"]; " ;

	if ( postPopExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, postPopExpr, 0, false );
		ret << L"}";
	}

	rbxGoto(ret, L"_again") << 
		L"\nend\n";
}

void RbxGotoCodeGen::BREAK( wostream &ret, int targState )
{
	outLabelUsed = true;

	out <<
		L"	begin\n"
		L"		" << P() << L" += 1\n"
		//L"		" << rbxGoto(ret, L"_out") << L"\n" 
		L"	end\n";
}

void RbxGotoCodeGen::writeData()
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

void RbxGotoCodeGen::writeExec()
{
	outLabelUsed = false;

	out << L"	begin\n";

	out << L"	Rubinius.asm { @labels = Hash.new { |h,k| h[k] = new_label } }\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = 0;\n";

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() 
	     || redFsm->anyFromStateActions() )
	{
                out <<  L" _acts, _nacts = nil\n";
	}

	if ( redFsm->anyConditions() )
		out << L"        _widec = nil\n";

	out << L"\n";

	if ( !noEnd ) {
		outLabelUsed = true;
		out << 
			L"	if ( " << P() << L" == " << PE() << L" )\n";
		rbxGoto(out << L"		", L"_out") << L"\n" <<
			L"	end\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			L"	if ( " << vCS() << L" == " << redFsm->errState->id << L" )\n";
		rbxGoto(out << L"		", L"_out") << L"\n" <<
			L"	end\n";
	}

	rbxLabel(out, L"_resume") << L"\n";

	if ( redFsm->anyFromStateActions() ) {
		out <<

			L"	_acts = " << ARR_OFF( A(), FSA() + L"[" + vCS() + L"]" ) << L";\n"
			L"	_nacts = " << L" *_acts++;\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( *_acts++ ) {\n";
		FROM_STATE_ACTION_SWITCH();
		out <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	out <<
		L"	case ( " << vCS() << L" )\n";
	STATE_GOTOS();
	out <<
		L"	end # case\n"
		L"\n";
	TRANSITIONS() <<
		L"\n";

	if ( redFsm->anyRegActions() )
		EXEC_FUNCS() << L"\n";


	rbxLabel(out, L"_again") << L"\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	_acts = " << ARR_OFF( A(), TSA() + L"[" + vCS() + L"]" ) << L";\n"
			L"	_nacts = " << L" *_acts++;\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( *_acts++ ) {\n";
		TO_STATE_ACTION_SWITCH();
		out <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			L"	if ( " << vCS() << L" == " << redFsm->errState->id << L" )\n";
		rbxGoto(out << L"		", L"_out") << L"\n" <<
			L"	end" << L"\n";
	}

	if ( !noEnd ) {
		out <<  L"	"  << P() << L" += 1\n"
			L"	if ( " << P() << L" != " << PE() << L" )\n";
		rbxGoto(out << L"		", L"_resume") << L"\n" <<
			L"	end" << L"\n";
	}
	else {
		out << 
			L"	" << P() << L" += 1;\n";
		rbxGoto(out << L"	", L"_resume") << L"\n";
	}

	if ( outLabelUsed )
		rbxLabel(out, L"_out") << L"\n";

	out << L"	end\n";
}

void RbxGotoCodeGen::writeEOF()
{
	if ( redFsm->anyEofActions() ) {
		out << 
			L"	{\n"
			L"	 _acts = " << 
			ARR_OFF( A(), EA() + L"[" + vCS() + L"]" ) << L";\n"
			L"	" << L" _nacts = " << L" *_acts++;\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( *_acts++ ) {\n";
		EOF_ACTION_SWITCH();
		out <<
			L"		}\n"
			L"	}\n"
			L"	}\n"
			L"\n";
	}
}

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */
