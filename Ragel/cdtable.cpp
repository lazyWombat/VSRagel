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
#include "cdtable.h"
#include "redfsm.h"
#include "gendata.h"

/* Determine if we should use indicies or not. */
void TabCodeGen::calcIndexSize()
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

std::wostream &TabCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	out << act;
	return out;
}

std::wostream &TabCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	out << act;
	return out;
}

std::wostream &TabCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	out << act;
	return out;
}


std::wostream &TabCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	out << act;
	return out;
}

std::wostream &TabCodeGen::TO_STATE_ACTION_SWITCH()
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

std::wostream &TabCodeGen::FROM_STATE_ACTION_SWITCH()
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

std::wostream &TabCodeGen::EOF_ACTION_SWITCH()
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


std::wostream &TabCodeGen::ACTION_SWITCH()
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

std::wostream &TabCodeGen::COND_OFFSETS()
{
	out << L"\t";
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		out << curKeyOffset;
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}

		/* Move the key offset ahead. */
		curKeyOffset += st->stateCondList.length();
	}
	out << L"\n";
	return out;
}

std::wostream &TabCodeGen::KEY_OFFSETS()
{
	out << L"\t";
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		out << curKeyOffset;
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}

		/* Move the key offset ahead. */
		curKeyOffset += st->outSingle.length() + st->outRange.length()*2;
	}
	out << L"\n";
	return out;
}


std::wostream &TabCodeGen::INDEX_OFFSETS()
{
	out << L"\t";
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		out << curIndOffset;
		if ( !st.last() ) {
			out << L", ";
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

std::wostream &TabCodeGen::COND_LENS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		out << st->stateCondList.length();
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


std::wostream &TabCodeGen::SINGLE_LENS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		out << st->outSingle.length();
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &TabCodeGen::RANGE_LENS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit length of range index. */
		out << st->outRange.length();
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &TabCodeGen::TO_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		TO_STATE_ACTION(st);
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &TabCodeGen::FROM_STATE_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		FROM_STATE_ACTION(st);
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &TabCodeGen::EOF_ACTIONS()
{
	out << L"\t";
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		EOF_ACTION(st);
		if ( !st.last() ) {
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}

std::wostream &TabCodeGen::EOF_TRANS()
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
			out << L", ";
			if ( ++totalStateNum % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	return out;
}


std::wostream &TabCodeGen::COND_KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Lower key. */
			out << KEY( sc->lowKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";

			/* Upper key. */
			out << KEY( sc->highKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &TabCodeGen::COND_SPACES()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Cond Space id. */
			out << sc->condSpace->condSpaceId << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &TabCodeGen::KEYS()
{
	out << L'\t';
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			out << KEY( stel->lowKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Loop the state's transitions. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			/* Lower key. */
			out << KEY( rtel->lowKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";

			/* Upper key. */
			out << KEY( rtel->highKey ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &TabCodeGen::INDICIES()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			out << stel->value->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			out << rtel->value->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			out << st->defTrans->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &TabCodeGen::TRANS_TARGS()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* The state's default target state. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Add any eof transitions that have not yet been written out above. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			trans->pos = totalTrans;
			out << trans->targ->id << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}


	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}


std::wostream &TabCodeGen::TRANS_ACTIONS()
{
	int totalTrans = 0;
	out << L'\t';
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}

	/* Add any eof transitions that have not yet been written out above. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			TRANS_ACTION( trans ) << L", ";
			if ( ++totalTrans % IALL == 0 )
				out << L"\n\t";
		}
	}


	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	out << 0 << L"\n";
	return out;
}

std::wostream &TabCodeGen::TRANS_TARGS_WI()
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
			out << L", ";
			if ( ++totalStates % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}


std::wostream &TabCodeGen::TRANS_ACTIONS_WI()
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
			out << L", ";
			if ( ++totalAct % IALL == 0 )
				out << L"\n\t";
		}
	}
	out << L"\n";
	delete[] transPtrs;
	return out;
}

void TabCodeGen::LOCATE_TRANS()
{
	out <<
		L"	_keys = " << ARR_OFF( K(), KO() + L"[" + vCS() + L"]" ) << L";\n"
		L"	_trans = " << IO() << L"[" << vCS() << L"];\n"
		L"\n"
		L"	_klen = " << SL() << L"[" << vCS() << L"];\n"
		L"	if ( _klen > 0 ) {\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_lower = _keys;\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_mid;\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_upper = _keys + _klen - 1;\n"
		L"		while (1) {\n"
		L"			if ( _upper < _lower )\n"
		L"				break;\n"
		L"\n"
		L"			_mid = _lower + ((_upper-_lower) >> 1);\n"
		L"			if ( " << GET_WIDE_KEY() << L" < *_mid )\n"
		L"				_upper = _mid - 1;\n"
		L"			else if ( " << GET_WIDE_KEY() << L" > *_mid )\n"
		L"				_lower = _mid + 1;\n"
		L"			else {\n"
		L"				_trans += " << CAST(UINT()) << L"(_mid - _keys);\n"
		L"				goto _match;\n"
		L"			}\n"
		L"		}\n"
		L"		_keys += _klen;\n"
		L"		_trans += _klen;\n"
		L"	}\n"
		L"\n"
		L"	_klen = " << RL() << L"[" << vCS() << L"];\n"
		L"	if ( _klen > 0 ) {\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_lower = _keys;\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_mid;\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_upper = _keys + (_klen<<1) - 2;\n"
		L"		while (1) {\n"
		L"			if ( _upper < _lower )\n"
		L"				break;\n"
		L"\n"
		L"			_mid = _lower + (((_upper-_lower) >> 1) & ~1);\n"
		L"			if ( " << GET_WIDE_KEY() << L" < _mid[0] )\n"
		L"				_upper = _mid - 2;\n"
		L"			else if ( " << GET_WIDE_KEY() << L" > _mid[1] )\n"
		L"				_lower = _mid + 2;\n"
		L"			else {\n"
		L"				_trans += " << CAST(UINT()) << L"((_mid - _keys)>>1);\n"
		L"				goto _match;\n"
		L"			}\n"
		L"		}\n"
		L"		_trans += _klen;\n"
		L"	}\n"
		L"\n";
}

void TabCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << gotoDest << L"; " << 
			CTRL_FLOW() << L"goto _again;}";
}

void TabCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << L"{" << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
	ret << L"); " << CTRL_FLOW() << L"goto _again;}";
}

void TabCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void TabCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void TabCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void TabCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
	ret << L");";
}

void TabCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
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

void TabCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
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

void TabCodeGen::RET( wostream &ret, bool inFinish )
{
	ret << L"{" << vCS() << L" = " << STACK() << L"[--" << 
			TOP() << L"]; ";

	if ( postPopExpr != 0 ) {
		ret << L"{";
		INLINE_LIST( ret, postPopExpr, 0, false, false );
		ret << L"}";
	}

	ret << CTRL_FLOW() <<  L"goto _again;}";
}

void TabCodeGen::BREAK( wostream &ret, int targState, bool csForced )
{
	outLabelUsed = true;
	ret << L"{" << P() << L"++; " << CTRL_FLOW() << L"goto _out; }";
}

void TabCodeGen::writeData()
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
}

void TabCodeGen::COND_TRANSLATE()
{
	out << 
		L"	_widec = " << GET_KEY() << L";\n"
		L"	_klen = " << CL() << L"[" << vCS() << L"];\n"
		L"	_keys = " << ARR_OFF( CK(), L"(" + CO() + L"[" + vCS() + L"]*2)" ) << L";\n"
		L"	if ( _klen > 0 ) {\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_lower = _keys;\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_mid;\n"
		L"		" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_upper = _keys + (_klen<<1) - 2;\n"
		L"		while (1) {\n"
		L"			if ( _upper < _lower )\n"
		L"				break;\n"
		L"\n"
		L"			_mid = _lower + (((_upper-_lower) >> 1) & ~1);\n"
		L"			if ( " << GET_WIDE_KEY() << L" < _mid[0] )\n"
		L"				_upper = _mid - 2;\n"
		L"			else if ( " << GET_WIDE_KEY() << L" > _mid[1] )\n"
		L"				_lower = _mid + 2;\n"
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

void TabCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out <<
		L"	{\n"
		L"	int _klen";

	if ( redFsm->anyRegCurStateRef() )
		out << L", _ps";

	out << 
		L";\n"
		L"	" << UINT() << L" _trans;\n";

	if ( redFsm->anyConditions() )
		out << L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() 
			|| redFsm->anyFromStateActions() )
	{
		out << 
			L"	" << PTR_CONST() << ARRAY_TYPE(redFsm->maxActArrItem) << PTR_CONST_END() << 
					POINTER() << L"_acts;\n"
			L"	" << UINT() << L" _nacts;\n";
	}

	out <<
		L"	" << PTR_CONST() << WIDE_ALPH_TYPE() << PTR_CONST_END() << POINTER() << L"_keys;\n"
		L"\n";

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
			L"	_acts = " << ARR_OFF( A(),  FSA() + L"[" + vCS() + L"]" ) << L";\n"
			L"	_nacts = " << CAST(UINT()) << L" *_acts++;\n"
			L"	while ( _nacts-- > 0 ) {\n"
			L"		switch ( *_acts++ ) {\n";
			FROM_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

	LOCATE_TRANS();

	out << L"_match:\n";

	if ( useIndicies )
		out << L"	_trans = " << I() << L"[_trans];\n";
	
	if ( redFsm->anyEofTrans() )
		out << L"_eof_trans:\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = " << vCS() << L";\n";

	out <<
		L"	" << vCS() << L" = " << TT() << L"[_trans];\n"
		L"\n";

	if ( redFsm->anyRegActions() ) {
		out <<
			L"	if ( " << TA() << L"[_trans] == 0 )\n"
			L"		goto _again;\n"
			L"\n"
			L"	_acts = " << ARR_OFF( A(), TA() + L"[_trans]" ) << L";\n"
			L"	_nacts = " << CAST(UINT()) << L" *_acts++;\n"
			L"	while ( _nacts-- > 0 )\n	{\n"
			L"		switch ( *_acts++ )\n		{\n";
			ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"		}\n"
			L"	}\n"
			L"\n";
	}

	if ( redFsm->anyRegActions() || redFsm->anyActionGotos() || 
			redFsm->anyActionCalls() || redFsm->anyActionRets() )
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
				L"	if ( " << ET() << L"[" << vCS() << L"] > 0 ) {\n"
				L"		_trans = " << ET() << L"[" << vCS() << L"] - 1;\n"
				L"		goto _eof_trans;\n"
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
