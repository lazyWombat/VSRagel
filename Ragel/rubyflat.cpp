/*
 *  Copyright 2001-2007 Adrian Thurston <thurston@complang.org>
 *  Copyright 2007 Victor Hugo Borja <vic@rubyforge.org>
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

#include "rubyflat.h"
#include "ragel.h"
#include "redfsm.h"
#include "gendata.h"

using std::wostream;
using std::wstring;

std::wostream &RubyFlatCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RubyFlatCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RubyFlatCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, true );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RubyFlatCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break */
			out << L"\twhen " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &RubyFlatCodeGen::KEYS()
{
	START_ARRAY_LINE();
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just low key and high key. */
		ARRAY_ITEM( KEY( st->lowKey ), ++totalTrans, false );
		ARRAY_ITEM( KEY( st->highKey ), ++totalTrans, false );
		if ( ++totalTrans % IALL == 0 )
			out << L"\n\t";

	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT( 0 ), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::INDICIES()
{
	int totalTrans = 0;
	START_ARRAY_LINE();
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->transList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->lowKey, st->highKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				ARRAY_ITEM( KEY( st->transList[pos]->id ), ++totalTrans, false );
			}
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 )
			ARRAY_ITEM( KEY( st->defTrans->id ), ++totalTrans, false );
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT( 0 ), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::FLAT_INDEX_OFFSET()
{
	START_ARRAY_LINE();
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		ARRAY_ITEM( INT( curIndOffset ), ++totalStateNum, st.last() );
		/* Move the index offset ahead. */
		if ( st->transList != 0 )
			curIndOffset += keyOps->span( st->lowKey, st->highKey );

		if ( st->defTrans != 0 )
			curIndOffset += 1;
	}

	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::KEY_SPANS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		unsigned long long span = 0;
		if ( st->transList != 0 )
			span = keyOps->span( st->lowKey, st->highKey );
		ARRAY_ITEM( INT( span ), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::TO_STATE_ACTIONS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT( TO_STATE_ACTION(st) ), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::FROM_STATE_ACTIONS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT( FROM_STATE_ACTION(st) ), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::EOF_ACTIONS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT( EOF_ACTION(st) ), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::EOF_TRANS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		long trans = 0;
		if ( st->eofTrans != 0 ) {
			assert( st->eofTrans->pos >= 0 );
			trans = st->eofTrans->pos+1;
		}

		/* Write any eof action. */
		ARRAY_ITEM( INT(trans), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::TRANS_TARGS()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	START_ARRAY_LINE();

	int totalStates = 0;
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Save the position. Needed for eofTargs. */
		RedTransAp *trans = transPtrs[t];
		trans->pos = t;

		/* Write out the target state. */
		ARRAY_ITEM( INT( trans->targ->id ), ++totalStates, t >= redFsm->transSet.length()-1  );
	}
	END_ARRAY_LINE();
	delete[] transPtrs;
	return out;
}


std::wostream &RubyFlatCodeGen::TRANS_ACTIONS()
{
	/* Transitions must be written ordered by their id. */
	RedTransAp **transPtrs = new RedTransAp*[redFsm->transSet.length()];
	for ( TransApSet::Iter trans = redFsm->transSet; trans.lte(); trans++ )
		transPtrs[trans->id] = trans;

	/* Keep a count of the num of items in the array written. */
	START_ARRAY_LINE();
	int totalAct = 0;
	for ( int t = 0; t < redFsm->transSet.length(); t++ ) {
		/* Write the function for the transition. */
		RedTransAp *trans = transPtrs[t];
		ARRAY_ITEM( INT( TRANS_ACTION( trans ) ), ++totalAct, t >= redFsm->transSet.length()-1 );
	}
	END_ARRAY_LINE();
	delete[] transPtrs;
	return out;
}


void RubyFlatCodeGen::LOCATE_TRANS()
{
	out <<
		L"	_keys = " << vCS() << L" << 1\n"
		L"	_inds = " << IO() << L"[" << vCS() << L"]\n"
		L"	_slen = " << SP() << L"[" << vCS() << L"]\n"
		L"	_wide = " << GET_WIDE_KEY() << L"\n"
		L"	_trans = if (   _slen > 0 && \n"
		L"			" << K() << L"[_keys] <= _wide && \n"
		L"			" << L"_wide <= " << K() << L"[_keys + 1] \n"
		L"		    ) then\n"
		L"			" << I() << L"[ _inds + _wide - " << K() << L"[_keys] ] \n"
		L"		 else \n"
		L"			" << I() << L"[ _inds + _slen ]\n"
		L"		 end\n"
		"";

}

std::wostream &RubyFlatCodeGen::COND_INDEX_OFFSET()
{
	START_ARRAY_LINE();
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		ARRAY_ITEM( INT( curIndOffset ), ++totalStateNum, st.last() );
		/* Move the index offset ahead. */
		if ( st->condList != 0 )
			curIndOffset += keyOps->span( st->condLowKey, st->condHighKey );
	}
	END_ARRAY_LINE();
	return out;
}

void RubyFlatCodeGen::COND_TRANSLATE()
{
	out <<
		L"	_widec = " << GET_KEY() << L"\n"
		L"	_keys = " << vCS() << L" << 1\n"
		L"	_conds = " << CO() << L"[" << vCS() << L"]\n"
		L"	_slen = " << CSP() << L"[" << vCS() << L"]\n"
		L"	_wide = " << GET_WIDE_KEY() << L"\n"
		L"	_cond = if ( _slen > 0 && \n"
		L"		     " << CK() << L"[_keys] <= _wide &&\n"
		L"		     " << L"_wide <= " << CK() << L"[_keys + 1]\n"
		L"		   ) then \n"
		L"			" << C() << L"[ _conds + _wide - " << CK() << L"[_keys]" << L" ]\n"
		L"		else\n"
		L"		       0\n"
		L"		end\n";
	out <<
		L"	case _cond \n";
	for ( CondSpaceList::Iter csi = condSpaceList; csi.lte(); csi++ ) {
		GenCondSpace *condSpace = csi;
		out << L"	when " << condSpace->condSpaceId + 1 << L" then\n";
		out << TABS(2) << L"_widec = " << L"(" <<
				KEY(condSpace->baseKey) << L" + (" << GET_KEY() << 
				L" - " << KEY(keyOps->minKey) << L"))\n";

		for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
			out << TABS(2) << L"if ( ";
			CONDITION( out, *csi );
			Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
			out << 
				L" ) then \n" <<
				TABS(3) << L"  _widec += " << condValOffset << L"\n"
				L"end\n";
		}
	}

	out <<
		L"	end # _cond switch \n";
}

std::wostream &RubyFlatCodeGen::CONDS()
{
	int totalTrans = 0;
	START_ARRAY_LINE();
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->condList != 0 ) {
			/* Walk the singles. */
			unsigned long long span = keyOps->span( st->condLowKey, st->condHighKey );
			for ( unsigned long long pos = 0; pos < span; pos++ ) {
				if ( st->condList[pos] != 0 )
					ARRAY_ITEM( INT( st->condList[pos]->condSpaceId + 1 ), ++totalTrans, false );
				else
					ARRAY_ITEM( INT( 0 ), ++totalTrans, false );
			}
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT( 0 ), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::COND_KEYS()
{
	START_ARRAY_LINE();
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit just cond low key and cond high key. */
		ARRAY_ITEM( KEY( st->condLowKey ), ++totalTrans, false );
		ARRAY_ITEM( KEY( st->condHighKey ), ++totalTrans, false );
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT( 0 ), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyFlatCodeGen::COND_KEY_SPANS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		unsigned long long span = 0;
		if ( st->condList != 0 )
			span = keyOps->span( st->condLowKey, st->condHighKey );
		ARRAY_ITEM( INT( span ), ++totalStateNum, false );
	}
	END_ARRAY_LINE();
	return out;
}


void RubyFlatCodeGen::GOTO( wostream &out, int gotoDest, bool inFinish )
{
	out << 
		L"	begin\n"
		L"		" << vCS() << L" = " << gotoDest << L"\n"
		L"		_trigger_goto = true\n"
		L"		_goto_level = _again\n"
		L"		break\n"
		L"	end\n";
}

void RubyFlatCodeGen::CALL( wostream &out, int callDest, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		out << L"begin\n";
		INLINE_LIST( out, prePushExpr, 0, false );
	}

	out <<
		L"	begin\n"
		L"		" << STACK() << L"[" << TOP() << L"] = " << vCS() << L"\n"
		L"		" << TOP() << L"+= 1\n"
		L"		" << vCS() << L" = " << callDest << L"\n"
		L"		_trigger_goto = true\n"
		L"		_goto_level = _again\n"
		L"		break\n"
		L"	end\n";

	if ( prePushExpr != 0 )
		out << L"end\n";
}

void RubyFlatCodeGen::CALL_EXPR(wostream &out, GenInlineItem *ilItem, int targState, bool inFinish )
{
	if ( prePushExpr != 0 ) {
		out << L"begin\n";
		INLINE_LIST( out, prePushExpr, 0, false );
	}

	out <<
		L"	begin\n"
		L"		" << STACK() << L"[" << TOP() << L"] = " << vCS() << L"\n"
		L"		" << TOP() << L" += 1\n"
		L"		" << vCS() << L" = (";
	INLINE_LIST( out, ilItem->children, targState, inFinish );
	out << L")\n";

	out << 
		L"		_trigger_goto = true\n"
		L"		_goto_level = _again\n"
		L"		break\n"
		L"	end\n";

	if ( prePushExpr != 0 )
		out << L"end\n";
}

void RubyFlatCodeGen::RET( wostream &out, bool inFinish )
{
	out <<
		L"	begin\n"
		L"		" << TOP() << L" -= 1\n"
		L"		" << vCS() << L" = " << STACK() << L"[" << TOP() << L"]\n";

	if ( postPopExpr != 0 ) {
		out << L"begin\n";
		INLINE_LIST( out, postPopExpr, 0, false );
		out << L"end\n";
	}

	out <<
		L"		_trigger_goto = true\n"
		L"		_goto_level = _again\n"
		L"		break\n"
		L"	end\n";
}

void RubyFlatCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void RubyFlatCodeGen::GOTO_EXPR( wostream &out, GenInlineItem *ilItem, bool inFinish )
{
	out << 
		L"	begin\n"
		L"		" << vCS() << L" = (";
	INLINE_LIST( out, ilItem->children, 0, inFinish );
	out << L")\n";
	out <<
		L"		_trigger_goto = true\n"
		L"		_goto_level = _again\n"
		L"		break\n"
		L"	end\n";
}

void RubyFlatCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}


void RubyFlatCodeGen::CURS( wostream &ret, bool inFinish )
{
	ret << L"(_ps)";
}

void RubyFlatCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
	ret << L"(" << vCS() << L")";
}

void RubyFlatCodeGen::BREAK( wostream &out, int targState )
{
	out << 
		L"	begin\n"
		L"		" << P() << L" += 1\n"
		L"		_trigger_goto = true\n"
		L"		_goto_level = _out\n"
		L"		break\n"
		L"	end\n";
}

int RubyFlatCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	return act;
}

int RubyFlatCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	return act;
}

int RubyFlatCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	return act;
}

int RubyFlatCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	/* If there are actions, emit them. Otherwise emit zero. */
	int act = 0;
	if ( trans->action != 0 )
		act = trans->action->location+1;
	return act;
}

void RubyFlatCodeGen::writeData()
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
		OPEN_ARRAY( WIDE_ALPH_TYPE(), CK() );
		COND_KEYS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondSpan), CSP() );
		COND_KEY_SPANS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCond), C() );
		CONDS();
		CLOSE_ARRAY() <<
		L"\n";

		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxCondIndexOffset), CO() );
		COND_INDEX_OFFSET();
		CLOSE_ARRAY() <<
		L"\n";
	}

	OPEN_ARRAY( WIDE_ALPH_TYPE(), K() );
	KEYS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxSpan), SP() );
	KEY_SPANS();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxFlatIndexOffset), IO() );
	FLAT_INDEX_OFFSET();
	CLOSE_ARRAY() <<
	L"\n";

	OPEN_ARRAY( ARRAY_TYPE(redFsm->maxIndex), I() );
	INDICIES();
	CLOSE_ARRAY() <<
	L"\n";

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

void RubyFlatCodeGen::writeExec()
{
	out << 
		L"begin # ragel flat\n"
		L"	testEof = false\n"
		L"	_slen, _trans, _keys, _inds";
	if ( redFsm->anyRegCurStateRef() )
		out << L", _ps";
	if ( redFsm->anyConditions() )
		out << L", _cond, _conds, _widec";
	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() 
			|| redFsm->anyFromStateActions() )
		out << L", _acts, _nacts";
	
	out << L" = nil\n";

	out << 
		L"	_goto_level = 0\n"
		L"	_resume = 10\n"
		L"	_eof_trans = 15\n"
		L"	_again = 20\n"
		L"	_test_eof = 30\n"
		L"	_out = 40\n";

	out << 
		L"	while true\n"
		L"	_trigger_goto = false\n"
		L"	if _goto_level <= 0\n";
	
	if ( !noEnd ) {
		out << 
			L"	if " << P() << L" == " << PE() << L"\n"
			L"		_goto_level = _test_eof\n"
			L"		next\n"
			L"	end\n";
	}

	if ( redFsm->errState != 0 ) {
		out << 
			L"	if " << vCS() << L" == " << redFsm->errState->id << L"\n"
			L"		_goto_level = _out\n"
			L"		next\n"
			L"	end\n";
	}
	
	/* The resume label. */
	out << 
		L"	end\n"
		L"	if _goto_level <= _resume\n";

	if ( redFsm->anyFromStateActions() ) {
		out << 
			L"	_acts = " << FSA() << L"[" << vCS() << L"]\n"
			L"	_nacts = " << A() << L"[_acts]\n"
			L"	_acts += 1\n"
			L"	while _nacts > 0\n"
			L"		_nacts -= 1\n"
			L"		_acts += 1\n"
			L"		case " << A() << L"[_acts - 1]\n";
		FROM_STATE_ACTION_SWITCH();
		out <<
			L"		end # from state action switch\n"
			L"	end\n"
			L"	if _trigger_goto\n"
			L"		next\n"
			L"	end\n";
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();
	
	LOCATE_TRANS();

	if ( redFsm->anyEofTrans() ) {
		out << 
			L"	end\n"
			L"	if _goto_level <= _eof_trans\n";
	}

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = " << vCS() << L"\n";

	out << L"	" << vCS() << L" = " << TT() << L"[_trans]\n";

	if ( redFsm->anyRegActions() ) {
		out << 
			L"	if " << TA() << L"[_trans] != 0\n"
			L"		_acts = " << TA() << L"[_trans]\n"
			L"		_nacts = " << A() << L"[_acts]\n"
			L"		_acts += 1\n"
			L"		while _nacts > 0\n"
			L"			_nacts -= 1\n"
			L"			_acts += 1\n"
			L"			case " << A() << L"[_acts - 1]\n";
		ACTION_SWITCH();
		out <<
			L"			end # action switch\n"
			L"		end\n"
			L"	end\n"
			L"	if _trigger_goto\n"
			L"		next\n"
			L"	end\n";
	}
	
	/* The again label. */
	out <<
		L"	end\n"
		L"	if _goto_level <= _again\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	_acts = " << TSA() << L"["  << vCS() << L"]\n"
			L"	_nacts = " << A() << L"[_acts]\n"
			L"	_acts += 1\n"
			L"	while _nacts > 0\n"
			L"		_nacts -= 1\n"
			L"		_acts += 1\n"
			L"		case " << A() << L"[_acts - 1]\n";
			TO_STATE_ACTION_SWITCH() <<
			L"		end # to state action switch\n"
			L"	end\n"
			L"	if _trigger_goto\n"
			L"		next\n"
			L"	end\n";
	}

	if ( redFsm->errState != 0 ) {
		out << 
			L"	if " << vCS() << L" == " << redFsm->errState->id << L"\n"
			L"		_goto_level = _out\n"
			L"		next\n"
			L"	end\n";
	}

	out << L"	" << P() << L" += 1\n";

	if ( !noEnd ) {
		out << 
			L"	if " << P() << L" != " << PE() << L"\n"
			L"		_goto_level = _resume\n"
			L"		next\n"
			L"	end\n";
	}
	else {
		out <<
			L"	_goto_level = _resume\n"
			L"	next\n";
	}

	/* The test_eof label. */
	out <<
		L"	end\n"
		L"	if _goto_level <= _test_eof\n";

	if ( redFsm->anyEofTrans() || redFsm->anyEofActions() ) {
		out << 
			L"	if " << P() << L" == " << vEOF() << L"\n";

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"	if " << ET() << L"[" << vCS() << L"] > 0\n"
				L"		_trans = " << ET() << L"[" << vCS() << L"] - 1;\n"
				L"		_goto_level = _eof_trans\n"
				L"		next;\n"
				L"	end\n";
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	begin\n"
				L"	__acts = " << EA() << L"[" << vCS() << L"]\n"
				L"	__nacts = " << A() << L"[__acts]\n" << 
				L"	__acts += 1\n"
				L"	while ( __nacts > 0 ) \n"
				L"		__nacts -= 1\n"
				L"		__acts += 1\n"
				L"		case ( "<< A() << L"[__acts-1] ) \n";
				EOF_ACTION_SWITCH() <<
				L"		end\n"
				L"	end\n"
				L"	if _trigger_goto\n"
				L"		next\n"
				L"	end\n"
				L"	end\n";
		}

		out <<
			L"	end\n";
	}

	out << 
		L"	end\n"
		L"	if _goto_level <= _out\n"
		L"		break\n"
		L"	end\n";

	/* The loop for faking goto. */
	out <<
		L"	end\n";

	/* Wrapping the execute block. */
	out << 
		L"	end\n";
}


/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */
