/*
 *  Copyright 2006 Adrian Thurston <thurston@complang.org>
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
#include "cssplit.h"
#include "gendata.h"
#include <assert.h>

using std::wostream;
using std::ios;
using std::endl;

/* Emit the goto to take for a given transition. */
std::wostream &CSharpSplitCodeGen::TRANS_GOTO( RedTransAp *trans, int level )
{
	if ( trans->targ->partition == currentPartition ) {
		if ( trans->action != 0 ) {
			/* Go to the transition which will go to the state. */
			out << TABS(level) << L"goto tr" << trans->id << L";";
		}
		else {
			/* Go directly to the target state. */
			out << TABS(level) << L"goto st" << trans->targ->id << L";";
		}
	}
	else {
		if ( trans->action != 0 ) {
			/* Go to the transition which will go to the state. */
			out << TABS(level) << L"goto ptr" << trans->id << L";";
			trans->partitionBoundary = true;
		}
		else {
			/* Go directly to the target state. */
			out << TABS(level) << L"goto pst" << trans->targ->id << L";";
			trans->targ->partitionBoundary = true;
		}
	}
	return out;
}

/* Called from before writing the gotos for each state. */
void CSharpSplitCodeGen::GOTO_HEADER( RedStateAp *state, bool stateInPartition )
{
	bool anyWritten = IN_TRANS_ACTIONS( state );

	if ( state->labelNeeded ) 
		out << L"st" << state->id << L":\n";

	if ( state->toStateAction != 0 ) {
		/* Remember that we wrote an action. Write every action in the list. */
		anyWritten = true;
		for ( GenActionTable::Iter item = state->toStateAction->key; item.lte(); item++ )
			ACTION( out, item->value, state->id, false );
	}

	/* Advance and test buffer pos. */
	if ( state->labelNeeded ) {
		if ( !noEnd ) {
			out <<
				L"	if ( ++" << P() << L" == " << PE() << L" )\n"
				L"		goto _out" << state->id << L";\n";
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
		for ( GenActionTable::Iter item = state->fromStateAction->key; item.lte(); item++ )
			ACTION( out, item->value, state->id, false );
	}

	if ( anyWritten )
		genLineDirective( out );

	/* Record the prev state if necessary. */
	if ( state->anyRegCurStateRef() )
		out << L"	_ps = " << state->id << L";\n";
}

std::wostream &CSharpSplitCodeGen::STATE_GOTOS( int partition )
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->partition == partition ) {
			if ( st == redFsm->errState )
				STATE_GOTO_ERROR();
			else {
				/* We call into the base of the goto which calls back into us
				 * using virtual functions. Set the current partition rather
				 * than coding parameter passing throughout. */
				currentPartition = partition;

				/* Writing code above state gotos. */
				GOTO_HEADER( st, st->partition == partition );

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
	}
	return out;
}


std::wostream &CSharpSplitCodeGen::PART_TRANS( int partition )
{
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ ) {
		if ( trans->partitionBoundary ) {
			out << 
				L"ptr" << trans->id << L":\n";

			if ( trans->action != 0 ) {
				/* If the action contains a next, then we must preload the current
				 * state since the action may or may not set it. */
				if ( trans->action->anyNextStmt() )
					out << L"	" << vCS() << L" = " << trans->targ->id << L";\n";

				/* Write each action in the list. */
				for ( GenActionTable::Iter item = trans->action->key; item.lte(); item++ )
					ACTION( out, item->value, trans->targ->id, false );
			}

			out <<
				L"	goto pst" << trans->targ->id << L";\n";
			trans->targ->partitionBoundary = true;
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->partitionBoundary ) {
			out << 
				L"	pst" << st->id << L":\n" 
				L"	" << vCS() << L" = " << st->id << L";\n";

			if ( st->toStateAction != 0 ) {
				/* Remember that we wrote an action. Write every action in the list. */
				for ( GenActionTable::Iter item = st->toStateAction->key; item.lte(); item++ )
					ACTION( out, item->value, st->id, false );
				genLineDirective( out );
			}

			ptOutLabelUsed = true;
			out << L"	goto _pt_out; \n";
		}
	}
	return out;
}

std::wostream &CSharpSplitCodeGen::EXIT_STATES( int partition )
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->partition == partition && st->outNeeded ) {
			outLabelUsed = true;
			out << L"	_out" << st->id << L": " << vCS() << L" = " << 
					st->id << L"; goto _out; \n";
		}
	}
	return out;
}


std::wostream &CSharpSplitCodeGen::PARTITION( int partition )
{
	outLabelUsed = false;
	ptOutLabelUsed = false;

	/* Initialize the partition boundaries, which get set during the writing
	 * of states. After the state writing we will */
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		trans->partitionBoundary = false;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		st->partitionBoundary = false;

	out << L"	" << ALPH_TYPE() << L" *p = *_pp, *pe = *_ppe;\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	int _ps = 0;\n";

	if ( redFsm->anyConditions() )
		out << L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";

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
			outLabelUsed = true;
			out << 
				L"	if ( ++" << P() << L" == " << PE() << L" )\n"
				L"		goto _out;\n";

		}
		else {
			out << 
				L"	" << P() << L" += 1;\n";
		}

		out <<
			L"_resume:\n";
	}

	out << 
		L"	switch ( " << vCS() << L" )\n	{\n";
		STATE_GOTOS( partition );
		SWITCH_DEFAULT() <<
		L"	}\n";
		PART_TRANS( partition );
		EXIT_STATES( partition );

	if ( outLabelUsed ) {
		out <<
			L"\n"
			L"	_out:\n"
			L"	*_pp = p;\n"
			L"	*_ppe = pe;\n"
			L"	return 0;\n";
	}

	if ( ptOutLabelUsed ) {
		out <<
			L"\n"
			L"	_pt_out:\n"
			L"	*_pp = p;\n"
			L"	*_ppe = pe;\n"
			L"	return 1;\n";
	}

	return out;
}

std::wostream &CSharpSplitCodeGen::PART_MAP()
{
	int *partMap = new int[redFsm->stateList.length()];
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
		partMap[st->id] = st->partition;

	out << L"\t";
	int totalItem = 0;
	for ( int i = 0; i < redFsm->stateList.length(); i++ ) {
		out << partMap[i];
		if ( i != redFsm->stateList.length() - 1 ) {
			out << L", ";
			if ( ++totalItem % IALL == 0 )
				out << L"\n\t";
		}
	}

	delete[] partMap;
	return out;
}

void CSharpSplitCodeGen::writeData()
{
	out <<
		L"const int " << START() << L" = " << START_STATE_ID() << L";\n"
		L"\n";

	if ( !noFinal ) {
		out <<
			L"const int " << FIRST_FINAL() << L" = " << FIRST_FINAL_STATE() << L";\n"
			L"\n";
	}

	if ( !noError ) {
		out <<
			L"const int " << ERROR() << L" = " << ERROR_STATE() << L";\n"
			L"\n";
	}


	OPEN_ARRAY( ARRAY_TYPE(numSplitPartitions), PM() );
	PART_MAP();
	CLOSE_ARRAY() <<
	L"\n";

	for ( int p = 0; p < redFsm->nParts; p++ ) {
		out << L"int partition" << p << L"( " << ALPH_TYPE() << L" **_pp, " << ALPH_TYPE() << 
			L" **_ppe, struct " << FSM_NAME() << L" *fsm );\n";
	}
	out << L"\n";
}

std::wostream &CSharpSplitCodeGen::ALL_PARTITIONS()
{
	/* compute the format wstring. */
	int width = 0, high = redFsm->nParts - 1;
	while ( high > 0 ) {
		width++;
		high /= 10;
	}
	assert( width <= 8 );
	wchar_t suffFormat[] = L"_%6.6d.c";
	suffFormat[2] = suffFormat[4] = ( L'0' + width );

	for ( int p = 0; p < redFsm->nParts; p++ ) {
		wchar_t suffix[10];
		swprintf( suffix, 10, suffFormat, p );
		const wchar_t *fn = fileNameFromStem( sourceFileName, suffix );
		const wchar_t *include = fileNameFromStem( sourceFileName, L".h" );

		/* Create the filter on the output and open it. */
		output_filter *partFilter = new output_filter( fn );
		partFilter->open( fn, ios::out|ios::trunc );
		if ( !partFilter->is_open() ) {
			error() << L"error opening " << fn << L" for writing" << endl;
			exit(1);
		}

		/* Attach the new file to the output stream. */
		std::wstreambuf *prev_rdbuf = out.rdbuf( partFilter );

		out << 
			L"#include \"" << include << "\"\n"
			L"int partition" << p << L"( " << ALPH_TYPE() << L" **_pp, " << ALPH_TYPE() << 
					L" **_ppe, struct " << FSM_NAME() << L" *fsm )\n"
			L"{\n";
			PARTITION( p ) <<
			L"}\n\n";
		out.flush();

		/* Fix the output stream. */
		out.rdbuf( prev_rdbuf );
	}
	return out;
}


void CSharpSplitCodeGen::writeExec()
{
	/* Must set labels immediately before writing because we may depend on the
	 * noend write option. */
	setLabelsNeeded();
	out << 
		L"	{\n"
		L"	int _stat = 0;\n";

	if ( !noEnd ) {
		out <<
			L"	if ( " << P() << L" == " << PE() << L" )\n"
			L"		goto _out;\n";
	}

	out << L"	goto _resume;\n";
	
	/* In this reentry, to-state actions have already been executed on the
	 * partition-switch exit from the last partition. */
	out << L"_reenter:\n";

	if ( !noEnd ) {
		out <<
			L"	if ( ++" << P() << L" == " << PE() << L" )\n"
			L"		goto _out;\n";
	}
	else {
		out << 
			L"	" << P() << L" += 1;\n";
	}

	out << L"_resume:\n";

	out << 
		L"	switch ( " << PM() << L"[" << vCS() << L"] ) {\n";
	for ( int p = 0; p < redFsm->nParts; p++ ) {
		out <<
			L"	case " << p << L":\n"
			L"		_stat = partition" << p << L"( &p, &pe, fsm );\n"
			L"		break;\n";
	}
	out <<
		L"	}\n"
		L"	if ( _stat )\n"
		L"		goto _reenter;\n";
	
	if ( !noEnd )
		out << L"	_out: {}\n";

	out <<
		L"	}\n";
	
	ALL_PARTITIONS();
}

void CSharpSplitCodeGen::setLabelsNeeded( RedStateAp *fromState, GenInlineList *inlineList )
{
	for ( GenInlineList::Iter item = *inlineList; item.lte(); item++ ) {
		switch ( item->type ) {
		case GenInlineItem::Goto: case GenInlineItem::Call: {
			/* In split code gen we only need labels for transitions across
			 * partitions. */
			if ( fromState->partition == item->targState->partition ){
				/* Mark the target as needing a label. */
				item->targState->labelNeeded = true;
			}
			break;
		}
		default: break;
		}

		if ( item->children != 0 )
			setLabelsNeeded( fromState, item->children );
	}
}

void CSharpSplitCodeGen::setLabelsNeeded( RedStateAp *fromState, RedTransAp *trans )
{
	/* In the split code gen we don't need labels for transitions across
	 * partitions. */
	if ( fromState->partition == trans->targ->partition ) {
		/* If there is no action with a next statement, then the label will be
		 * needed. */
		trans->labelNeeded = true;
		if ( trans->action == 0 || !trans->action->anyNextStmt() )
			trans->targ->labelNeeded = true;
	}

	/* Need labels for states that have goto or calls in action code
	 * invoked on characters (ie, not from out action code). */
	if ( trans->action != 0 ) {
		/* Loop the actions. */
		for ( GenActionTable::Iter act = trans->action->key; act.lte(); act++ ) {
			/* Get the action and walk it's tree. */
			setLabelsNeeded( fromState, act->value->inlineList );
		}
	}
}

/* Set up labelNeeded flag for each state. */
void CSharpSplitCodeGen::setLabelsNeeded()
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
		for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
			trans->labelNeeded = false;

		/* Walk all transitions and set only those that have targs. */
		for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
			for ( RedTransList::Iter tel = st->outRange; tel.lte(); tel++ )
				setLabelsNeeded( st, tel->value );

			for ( RedTransList::Iter tel = st->outSingle; tel.lte(); tel++ )
				setLabelsNeeded( st, tel->value );

			if ( st->defTrans != 0 )
				setLabelsNeeded( st, st->defTrans );
		}
	}

	if ( !noEnd ) {
		for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ )
			st->outNeeded = st->labelNeeded;
	}
	else {
		if ( redFsm->errState != 0 )
			redFsm->errState->outNeeded = true;

		for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ ) {
			/* Any state with a transition in that has a break will need an
			 * out label. */
			if ( trans->action != 0 && trans->action->anyBreakStmt() )
				trans->targ->outNeeded = true;
		}
	}
}

