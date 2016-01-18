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
#include "cdipgoto.h"
#include "redfsm.h"
#include "gendata.h"
#include "bstmap.h"

bool IpGotoCodeGen::useAgainLabel()
{
	return redFsm->anyRegActionRets() || 
			redFsm->anyRegActionByValControl() || 
			redFsm->anyRegNextStmt();
}

void IpGotoCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"{" << CTRL_FLOW() << L"goto st" << gotoDest << L";}";
}

void IpGotoCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << targState << 
			L"; " << CTRL_FLOW() << L"goto st" << callDest << L";}";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void IpGotoCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, prePushExpr, 0, false, false );
	}

	ret << L"{" << STACK() << L"[" << TOP() << L"++] = " << targState << L"; " << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
	ret << L"); " << CTRL_FLOW() << L"goto _again;}";

	if ( prePushExpr != 0 )
		ret << L"}";
}

void IpGotoCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << STACK() << L"[--" << TOP() << L"];";

	if ( postPopExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, postPopExpr, 0, false, false );
		ret << L"}";
	}

	ret << CTRL_FLOW() << L"goto _again;}";
}

void IpGotoCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"{" << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
	ret << L"); " << CTRL_FLOW() << L"goto _again;}";
}

void IpGotoCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void IpGotoCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
	ret << L");";
}

void IpGotoCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void IpGotoCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << targState;
}

void IpGotoCodeGen::BREAK( wostream &ret, int targState, bool csForced )
{
	outLabelUsed = true;
	ret << L"{" << P() << L"++; ";
	if ( !csForced ) 
		ret << vCS() << L" = " << targState << L"; ";
	ret << CTRL_FLOW() << L"goto _out;}";
}

bool IpGotoCodeGen::IN_TRANS_ACTIONS( RedStateAp *state )
{
	bool anyWritten = false;

	/* Emit any transitions that have actions and that go to this state. */
	for ( int it = 0; it < state->numInTrans; it++ ) {
		RedTransAp *trans = state->inTrans[it];
		if ( trans->action != 0 && trans->labelNeeded ) {
			/* Remember that we wrote an action so we know to write the
			 * line directive for going back to the output. */
			anyWritten = true;

			/* Write the label for the transition so it can be jumped to. */
			out << L"tr" << trans->id << L":\n";

			/* If the action contains a next, then we must preload the current
			 * state since the action may or may not set it. */
			if ( trans->action->anyNextStmt() )
				out << L"	" << vCS() << L" = " << trans->targ->id << L";\n";

			/* Write each action in the list. */
			for ( GenActionTable::Iter item = trans->action->key; item.lte(); item++ ) {
				ACTION( out, item->value, trans->targ->id, false, 
						trans->action->anyNextStmt() );
			}

			/* If the action contains a next then we need to reload, otherwise
			 * jump directly to the target state. */
			if ( trans->action->anyNextStmt() )
				out << L"\tgoto _again;\n";
			else
				out << L"\tgoto st" << trans->targ->id << L";\n";
		}
	}

	return anyWritten;
}

/* Called from GotoCodeGen::STATE_GOTOS just before writing the gotos for each
 * state. */
void IpGotoCodeGen::GOTO_HEADER( RedStateAp *state )
{
	bool anyWritten = IN_TRANS_ACTIONS( state );

	if ( state->labelNeeded ) 
		out << L"st" << state->id << L":\n";

	if ( state->toStateAction != 0 ) {
		/* Remember that we wrote an action. Write every action in the list. */
		anyWritten = true;
		for ( GenActionTable::Iter item = state->toStateAction->key; item.lte(); item++ ) {
			ACTION( out, item->value, state->id, false, 
					state->toStateAction->anyNextStmt() );
		}
	}

	/* Advance and test buffer pos. */
	if ( state->labelNeeded ) {
		if ( !noEnd ) {
			out <<
				L"	if ( ++" << P() << L" == " << PE() << L" )\n"
				L"		goto _test_eof" << state->id << L";\n";
		}
		else {
			out << 
				L"	" << P() << L" += 1;\n";
		}
	}

	/* Give the state a switch case. */
	out << L"case " << state->id << L":\n";

	if ( state->fromStateAction != 0 ) {
		/* Remember that we wrote an action. Write every action in the list. */
		anyWritten = true;
		for ( GenActionTable::Iter item = state->fromStateAction->key; item.lte(); item++ ) {
			ACTION( out, item->value, state->id, false,
					state->fromStateAction->anyNextStmt() );
		}
	}

	if ( anyWritten )
		genLineDirective( out );

	/* Record the prev state if necessary. */
	if ( state->anyRegCurStateRef() )
		out << L"	_ps = " << state->id << L";\n";
}

void IpGotoCodeGen::STATE_GOTO_ERROR()
{
	/* In the error state we need to emit some stuff that usually goes into
	 * the header. */
	RedStateAp *state = redFsm->errState;
	bool anyWritten = IN_TRANS_ACTIONS( state );

	/* No case label needed since we don't switch on the error state. */
	if ( anyWritten )
		genLineDirective( out );

	if ( state->labelNeeded ) 
		out << L"st" << state->id << L":\n";

	/* Break out here. */
	outLabelUsed = true;
	out << vCS() << L" = " << state->id << L";\n";
	out << L"	goto _out;\n";
}


/* Emit the goto to take for a given transition. */
std::wostream &IpGotoCodeGen::TRANS_GOTO( RedTransAp *trans, int level )
{
	if ( trans->action != 0 ) {
		/* Go to the transition which will go to the state. */
		out << TABS(level) << L"goto tr" << trans->id << L";";
	}
	else {
		/* Go directly to the target state. */
		out << TABS(level) << L"goto st" << trans->targ->id << L";";
	}
	return out;
}

std::wostream &IpGotoCodeGen::EXIT_STATES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->outNeeded ) {
			testEofUsed = true;
			out << L"	_test_eof" << st->id << L": " << vCS() << L" = " << 
					st->id << L"; goto _test_eof; \n";
		}
	}
	return out;
}

std::wostream &IpGotoCodeGen::AGAIN_CASES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		out << 
			L"		case " << st->id << L": goto st" << st->id << L";\n";
	}
	return out;
}

std::wostream &IpGotoCodeGen::FINISH_CASES()
{
	bool anyWritten = false;

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofAction != 0 ) {
			if ( st->eofAction->eofRefs == 0 )
				st->eofAction->eofRefs = new IntSet;
			st->eofAction->eofRefs->insert( st->id );
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 )
			out << L"	case " << st->id << L": goto tr" << st->eofTrans->id << L";\n";
	}

	for ( GenActionTableMap::Iter act = redFsm->actionMap; act.lte(); act++ ) {
		if ( act->eofRefs != 0 ) {
			for ( IntSet::Iter pst = *act->eofRefs; pst.lte(); pst++ )
				out << L"	case " << *pst << L": \n";

			/* Remember that we wrote a trans so we know to write the
			 * line directive for going back to the output. */
			anyWritten = true;

			/* Write each action in the eof action list. */
			for ( GenActionTable::Iter item = act->key; item.lte(); item++ )
				ACTION( out, item->value, STATE_ERR_STATE, true, false );
			out << L"\tbreak;\n";
		}
	}

	if ( anyWritten )
		genLineDirective( out );
	return out;
}

void IpGotoCodeGen::setLabelsNeeded( GenInlineList *inlineList )
{
	for ( GenInlineList::Iter item = *inlineList; item.lte(); item++ ) {
		switch ( item->type ) {
		case GenInlineItem::Goto: case GenInlineItem::Call: {
			/* Mark the target as needing a label. */
			item->targState->labelNeeded = true;
			break;
		}
		default: break;
		}

		if ( item->children != 0 )
			setLabelsNeeded( item->children );
	}
}

/* Set up labelNeeded flag for each state. */
void IpGotoCodeGen::setLabelsNeeded()
{
	/* If we use the _again label, then we the _again switch, which uses all
	 * labels. */
	if ( useAgainLabel() ) {
		for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
			st->labelNeeded = true;
	}
	else {
		/* Do not use all labels by default, init all labelNeeded vars to false. */
		for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
			st->labelNeeded = false;

		/* Walk all transitions and set only those that have targs. */
		for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ ) {
			/* If there is no action with a next statement, then the label will be
			 * needed. */
			if ( trans->action == 0 || !trans->action->anyNextStmt() )
				trans->targ->labelNeeded = true;

			/* Need labels for states that have goto or calls in action code
			 * invoked on characters (ie, not from out action code). */
			if ( trans->action != 0 ) {
				/* Loop the actions. */
				for ( GenActionTable::Iter act = trans->action->key; act.lte(); act++ ) {
					/* Get the action and walk it's tree. */
					setLabelsNeeded( act->value->inlineList );
				}
			}
		}
	}

	if ( !noEnd ) {
		for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
			if ( st != redFsm->errState )
				st->outNeeded = st->labelNeeded;
		}
	}
}

void IpGotoCodeGen::writeData()
{
	STATE_IDS();
}

void IpGotoCodeGen::writeExec()
{
	/* Must set labels immediately before writing because we may depend on the
	 * noend write option. */
	setLabelsNeeded();
	testEofUsed = false;
	outLabelUsed = false;

	out << L"	{\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	int _ps = 0;\n";

	if ( redFsm->anyConditions() )
		out << L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";

	if ( !noEnd ) {
		testEofUsed = true;
		out << 
			L"	if ( " << P() << L" == " << PE() << L" )\n"
			L"		goto _test_eof;\n";
	}

	if ( useAgainLabel() ) {
		out << 
			L"	goto _resume;\n"
			L"\n"
			L"_again:\n"
			L"	switch ( " << vCS() << L" ) {\n";
			AGAIN_CASES() <<
			L"	default: break;\n"
			L"	}\n"
			L"\n";

		if ( !noEnd ) {
			testEofUsed = true;
			out << 
				L"	if ( ++" << P() << L" == " << PE() << L" )\n"
				L"		goto _test_eof;\n";
		}
		else {
			out << 
				L"	" << P() << L" += 1;\n";
		}

		out << L"_resume:\n";
	}

	out << 
		L"	switch ( " << vCS() << L" )\n	{\n";
		STATE_GOTOS();
		SWITCH_DEFAULT() <<
		L"	}\n";
		EXIT_STATES() << 
		L"\n";

	if ( testEofUsed ) 
		out << L"	_test_eof: {}\n";

	if ( redFsm->anyEofTrans() || redFsm->anyEofActions() ) {
		out <<
			L"	if ( " << P() << L" == " << vEOF() << L" )\n"
			L"	{\n"
			L"	switch ( " << vCS() << L" ) {\n";
			FINISH_CASES();
			SWITCH_DEFAULT() <<
			L"	}\n"
			L"	}\n"
			L"\n";
	}

	if ( outLabelUsed ) 
		out << L"	_out: {}\n";

	out <<
		L"	}\n";
}
