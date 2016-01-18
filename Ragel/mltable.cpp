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
#include "mltable.h"
#include "redfsm.h"
#include "gendata.h"

/* Determine if we should use indicies or not. */
void OCamlTabCodeGen::calcIndexSize()
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

std::wostream &OCamlTabCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	out << act;
	return out;
}

std::wostream &OCamlTabCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	out << act;
	return out;
}

std::wostream &OCamlTabCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	out << act;
	return out;
}


std::wostream &OCamlTabCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	out << act;
	return out;
}

std::wostream &OCamlTabCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" ->\n";
      ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlTabCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlTabCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" ->\n";
			ACTION( out, act, 0, true );
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &OCamlTabCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"\t| " << act->actionId << L" -> \n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlTabCodeGen::COND_OFFSETS()
{
	out << L"\t";
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		out << curKeyOffset;
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}

		/* Move the key offset ahead. */
		curKeyOffset += st->stateCondList.length();
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::KEY_OFFSETS()
{
	out << L"\t";
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		out << curKeyOffset;
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}

		/* Move the key offset ahead. */
		curKeyOffset += st->outSingle.length() + st->outRange.length()*2;
	}
	out << L"\n";
	return out;
}


std::wostream &OCamlTabCodeGen::INDEX_OFFSETS()
{
	out << L"\t";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset;
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}

		/* Move the index offset ahead. */
		curIndOffset += st->outSingle.length() + st->outRange.length();
		if ( st->defTrans != 0 )
			curIndOffset += 1;
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::COND_LENS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		out << st->stateCondList.length();
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


std::wostream &OCamlTabCodeGen::SINGLE_LENS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		out << st->outSingle.length();
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::RANGE_LENS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit length of range index. */
		out << st->outRange.length();
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::TO_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		TO_STATE_ACTION(st);
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::FROM_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		FROM_STATE_ACTION(st);
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::EOF_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		EOF_ACTION(st);
		if ( !st.last() ) {
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::EOF_TRANS()
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
			out << ARR_SEP();
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


std::wostream &OCamlTabCodeGen::COND_KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Lower key. */
			out << ALPHA_KEY( sc->lowKey ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";

			/* Upper key. */
			out << ALPHA_KEY( sc->highKey ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::COND_SPACES()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Cond Space id. */
			out << sc->condSpace->condSpaceId << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			out << ALPHA_KEY( stel->lowKey ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Loop the state's transitions. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			/* Lower key. */
			out << ALPHA_KEY( rtel->lowKey ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";

			/* Upper key. */
			out << ALPHA_KEY( rtel->highKey ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::INDICIES()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			out << stel->value->id << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			out << rtel->value->id << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			out << st->defTrans->id << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::TRANS_TARGS()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			out << trans->targ->id << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			out << trans->targ->id << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* The state's default target state. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			out << trans->targ->id << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			trans->pos = totalTrans;
			out << trans->targ->id << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}


	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}


std::wostream &OCamlTabCodeGen::TRANS_ACTIONS()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			TRANS_ACTION( trans ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			TRANS_ACTION( trans ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			TRANS_ACTION( trans ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			TRANS_ACTION( trans ) << ARR_SEP();
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &OCamlTabCodeGen::TRANS_TARGS_WI()
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
			out << ARR_SEP();
			if ( ++totalStates % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}


std::wostream &OCamlTabCodeGen::TRANS_ACTIONS_WI()
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
			out << ARR_SEP();
			if ( ++totalAct % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}

void OCamlTabCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- " << gotoDest << L"; " << 
			CTRL_FLOW() << L"raise Goto_again end";
}

void OCamlTabCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L"); " << CTRL_FLOW() << L"raise Goto_again end";
}

void OCamlTabCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void OCamlTabCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void OCamlTabCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" <- " << nextDest << L";";
}

void OCamlTabCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" <- (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}

void OCamlTabCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
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

void OCamlTabCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
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

void OCamlTabCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"begin " << vCS() << L" <- " << AT(STACK(), PRE_DECR(TOP()) ) << L"; ";

	if ( postPopExpr != 0 ) {
		ret << L"begin ";
		INLINE_LIST( ret, postPopExpr, 0, false );
		ret << L"end ";
	}

	ret << CTRL_FLOW() <<  L"raise Goto_again end";
}

void OCamlTabCodeGen::BREAK( wostream &ret, int targState )
{
	outLabelUsed = true;
	ret << L"begin " << P() << L" <- " << P() << L" + 1; " << CTRL_FLOW() << L"raise Goto_out end";
}

void OCamlTabCodeGen::writeData()
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
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondOffset), CO() );
		COND_OFFSETS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondLen), CL() );
		COND_LENS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( WIDE_ALPH_TYPE(), CK() );
		COND_KEYS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondSpaceId), C() );
		COND_SPACES();
		CLOSE_ARRAY() <<
		L"\n";
	}

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxKeyOffset), KO() );
	KEY_OFFSETS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( WIDE_ALPH_TYPE(), K() );
	KEYS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxSingleLen), SL() );
	SINGLE_LENS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxRangeLen), RL() );
	RANGE_LENS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndexOffset), IO() );
	INDEX_OFFSETS();
	CLOSE_ARRAY() <<
	L"\n";

	if ( useIndicies ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndex), I() );
		INDICIES();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxState), TT() );
		TRANS_TARGS_WI();
		CLOSE_ARRAY() <<
		L"\n";

		if ( redFsm->anyActions() ) {
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc), TA() );
			TRANS_ACTIONS_WI();
			CLOSE_ARRAY() <<
			L"\n";
		}
	}
	else {
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

  out << L"type " << TYPE_STATE() << L" = { mutable keys : int; mutable trans : int; mutable acts : int; mutable nacts : int; }"
    << TOP_SEP();

  out << L"exception Goto_match" << TOP_SEP();
  out << L"exception Goto_again" << TOP_SEP();
  out << L"exception Goto_eof_trans" << TOP_SEP();
}

void OCamlTabCodeGen::LOCATE_TRANS()
{
	out <<
		L"	state.keys <- " << AT( KO(), vCS() ) << L";\n"
		L"	state.trans <- " << CAST(transType) << AT( IO(), vCS() ) << L";\n"
		L"\n"
		L"	let klen = " << AT( SL(), vCS() ) << L" in\n"
		L"	if klen > 0 then begin\n"
		L"		let lower : " << signedKeysType << L" ref = ref state.keys in\n"
		L"		let upper : " << signedKeysType << L" ref = ref " << CAST(signedKeysType) << 
			L"(state.keys + klen - 1) in\n"
		L"		while !upper >= !lower do\n"
		L"			let mid = " << CAST(signedKeysType) << L" (!lower + ((!upper - !lower) / 2)) in\n"
		L"			if " << GET_WIDE_KEY() << L" < " << AT( K(), L"mid" ) << L" then\n"
		L"				upper := " << CAST(signedKeysType) << L" (mid - 1)\n"
		L"			else if " << GET_WIDE_KEY() << L" > " << AT( K(), L"mid" ) << L" then\n"
		L"				lower := " << CAST(signedKeysType) << L" (mid + 1)\n"
		L"			else begin\n"
		L"				state.trans <- state.trans + " << CAST(transType) << L" (mid - state.keys);\n"
		L"				raise Goto_match;\n"
		L"			end\n"
		L"		done;\n"
		L"		state.keys <- state.keys + " << CAST(keysType) << L" klen;\n"
		L"		state.trans <- state.trans + " << CAST(transType) << L" klen;\n"
		L"	end;\n"
		L"\n"
		L"	let klen = " << AT( RL(), vCS() ) << L" in\n"
		L"	if klen > 0 then begin\n"
		L"		let lower : " << signedKeysType << L" ref = ref state.keys in\n"
		L"		let upper : " << signedKeysType << L" ref = ref " << CAST(signedKeysType) <<
			L"(state.keys + (klen * 2) - 2) in\n"
		L"		while !upper >= !lower do\n"
		L"			let mid = " << CAST(signedKeysType) << L" (!lower + (((!upper - !lower) / 2) land (lnot 1))) in\n"
		L"			if " << GET_WIDE_KEY() << L" < " << AT( K() , L"mid" ) << L" then\n"
		L"				upper := " << CAST(signedKeysType) << L" (mid - 2)\n"
		L"			else if " << GET_WIDE_KEY() << L" > " << AT( K(), L"mid+1" ) << L" then\n"
		L"				lower := " << CAST(signedKeysType) << L" (mid + 2)\n"
		L"			else begin\n"
		L"				state.trans <- state.trans + " << CAST(transType) << L"((mid - state.keys) / 2);\n"
		L"				raise Goto_match;\n"
		L"		  end\n"
		L"		done;\n"
		L"		state.trans <- state.trans + " << CAST(transType) << L" klen;\n"
		L"	end;\n"
		L"\n";
}

void OCamlTabCodeGen::COND_TRANSLATE()
{
	out << 
		L"	_widec = " << GET_KEY() << L";\n"
		L"	_klen = " << CL() << L"[" << vCS() << L"];\n"
		L"	_keys = " << CAST(keysType) << L" ("<< CO() << L"[" << vCS() << L"]*2);\n"
		L"	if ( _klen > 0 ) {\n"
		L"		" << signedKeysType << L" _lower = _keys;\n"
		L"		" << signedKeysType << L" _mid;\n"
		L"		" << signedKeysType << L" _upper = " << CAST(signedKeysType) << 
			L" (_keys + (_klen<<1) - 2);\n"
		L"		while (true) {\n"
		L"			if ( _upper < _lower )\n"
		L"				break;\n"
		L"\n"
		L"			_mid = " << CAST(signedKeysType) << 
			L" (_lower + (((_upper-_lower) >> 1) & ~1));\n"
		L"			if ( " << GET_WIDE_KEY() << L" < " << CK() << L"[_mid] )\n"
		L"				_upper = " << CAST(signedKeysType) << L" (_mid - 2);\n"
		L"			else if ( " << GET_WIDE_KEY() << L" > " << CK() << L"[_mid+1] )\n"
		L"				_lower = " << CAST(signedKeysType) << L" (_mid + 2);\n"
		L"			else {\n"
		L"				switch ( " << C() << L"[" << CO() << L"[" << vCS() << L"]"
							L" + ((_mid - _keys)>>1)] ) {\n";

	for ( CondSpaceList::Iter csi = condSpaceList; csi.lte(); csi++ ) {
		GenCondSpace *condSpace = csi;
		out << L"	case " << condSpace->condSpaceId << L": {\n";
		out << TABS(2) << L"_widec = " << CAST(WIDE_ALPH_TYPE()) << L"(" <<
				KEY(condSpace->baseKey) << L" + (" << GET_KEY() << 
				L" - " << KEY(keyOps->minKey) << L"));\n";

		for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
			out << TABS(2) << L"if ( ";
			CONDITION( out, *csi );
			Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
			out << L" ) _widec += " << condValOffset << L";\n";
		}

		out << 
			L"		break;\n"
			L"	}\n";
	}

	SWITCH_DEFAULT();

	out << 
		L"				}\n"
		L"				break;\n"
		L"			}\n"
		L"		}\n"
		L"	}\n"
		L"\n";
}

void OCamlTabCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;
	initVarTypes();

	out <<
		L"	begin\n";
//		L"	" << klenType << L" _klen";

//	if ( redFsm->anyRegCurStateRef() )
//		out << L", _ps";

/*
	out << L"	" << transType << L" _trans;\n";

	if ( redFsm->anyConditions() )
		out << L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() 
			|| redFsm->anyFromStateActions() )
	{
		out << 
			L"	int _acts;\n"
			L"	int _nacts;\n";
	}

	out <<
		L"	" << keysType << L" _keys;\n"
		L"\n";
//		L"	" << PTR_CONST() << WIDE_ALPH_TYPE() << POINTER() << L"_keys;\n"
*/

  out <<
    L"	let state = { keys = 0; trans = 0; acts = 0; nacts = 0; } in\n"
    L"	let rec do_start () =\n";
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

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

  out << L"\tbegin try\n";
	LOCATE_TRANS();
  out << L"\twith Goto_match -> () end;\n";

  out << 
    L"\tdo_match ()\n";

	out << L"and do_match () =\n";

	if ( useIndicies )
		out << L"	state.trans <- " << CAST(transType) << AT( I(), L"state.trans" ) << L";\n";

  out << L"\tdo_eof_trans ()\n";
	
//	if ( redFsm->anyEofTrans() )
  out << L"and do_eof_trans () =\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	let ps = " << vCS() << L" in\n";

	out <<
		L"	" << vCS() << L" <- " << AT( TT() ,L"state.trans" ) << L";\n"
		L"\n";

	if ( redFsm->anyRegActions() ) {
		out <<
			L"\tbegin try\n"
      L"	match " << AT( TA(), L"state.trans" ) << L" with\n"
			L"\t| 0 -> raise Goto_again\n"
      L"\t| _ ->\n"
			L"	state.acts <- " << AT( TA(), L"state.trans" ) << L";\n"
			L"	state.nacts <- " << AT( A(), POST_INCR(L"state.acts") ) << L";\n"
			L"	while " << POST_DECR(L"state.nacts") << L" > 0 do\n"
			L"		begin match " << AT( A(), POST_INCR(L"state.acts") ) << L" with\n";
			ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		end;\n"
			L"	done\n"
      L"\twith Goto_again -> () end;\n";
	}
  out << L"\tdo_again ()\n";

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
			L"	begin try\n";

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"	if " << AT( ET(), vCS() ) << L" > 0 then\n"
				L"	begin\n"
        L"   state.trans <- " << CAST(transType) << L"(" << AT( ET(), vCS() ) << L" - 1);\n"
				L"		raise Goto_eof_trans;\n"
				L"	end;\n";
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	let __acts = ref " << AT( EA(), vCS() ) << L" in\n"
				L"	let __nacts = ref " << AT( A(), L"!__acts" ) << L" in\n"
        L" incr __acts;\n"
				L"	while !__nacts > 0 do\n"
        L"   decr __nacts;\n"
				L"		begin match " << AT( A(), POST_INCR(L"__acts.contents") ) << L" with\n";
				EOF_ACTION_SWITCH();
				SWITCH_DEFAULT() <<
				L"		end;\n"
				L"	done\n";
		}

		out << 
			L"	with Goto_again -> do_again ()\n"
			L"	| Goto_eof_trans -> do_eof_trans () end\n"
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

void OCamlTabCodeGen::initVarTypes()
{
	int klenMax = MAX(MAX(redFsm->maxCondLen, redFsm->maxRangeLen),
				redFsm->maxSingleLen);
	int keysMax = MAX(MAX(redFsm->maxKeyOffset, klenMax),
				redFsm->maxCondOffset);
	int transMax = MAX(MAX(redFsm->maxIndex+1, redFsm->maxIndexOffset), keysMax);
	transMax = MAX(transMax, klenMax);
	transType = ARRAY_TYPE(transMax);
	klenType = ARRAY_TYPE(klenMax);
	keysType = ARRAY_TYPE(keysMax);
	signedKeysType = ARRAY_TYPE(keysMax, true);
}
