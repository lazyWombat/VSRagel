/*
 *  Copyright 2007 Victor Hugo Borja <vic@rubyforge.org>
 *            2007 Adrian Thurston <thurston@complang.org>
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

#include <iomanip>
#include <sstream>
#include "redfsm.h"
#include "gendata.h"
#include "ragel.h"
#include "rubytable.h"

using std::wostream;
using std::wostringstream;
using std::wstring;
using std::endl;



void RubyTabCodeGen::GOTO( wostream &out, int gotoDest, bool inFinish )
{
	out << 
		L"	begin\n"
		L"		" << vCS() << L" = " << gotoDest << L"\n"
		L"		_trigger_goto = true\n"
		L"		_goto_level = _again\n"
		L"		break\n"
		L"	end\n";
}

void RubyTabCodeGen::GOTO_EXPR( wostream &out, GenInlineItem *ilItem, bool inFinish )
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

void RubyTabCodeGen::CALL( wostream &out, int callDest, int targState, bool inFinish )
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

void RubyTabCodeGen::CALL_EXPR(wostream &out, GenInlineItem *ilItem, int targState, bool inFinish )
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

void RubyTabCodeGen::RET( wostream &out, bool inFinish )
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

void RubyTabCodeGen::BREAK( wostream &out, int targState )
{
	out << 
		L"	begin\n"
		L"		" << P() << L" += 1\n"
		L"		_trigger_goto = true\n"
		L"		_goto_level = _out\n"
		L"		break\n"
		L"	end\n";
}

void RubyTabCodeGen::COND_TRANSLATE()
{
	out <<
		L"	_widec = " << GET_KEY() << L"\n"
		L"	_keys = " << CO() << L"[" << vCS() << L"]*2\n"
		L"	_klen = " << CL() << L"[" << vCS() << L"]\n"
		L"	if _klen > 0\n"
		L"		_lower = _keys\n"
		L"		_upper = _keys + (_klen<<1) - 2\n"
		L"		loop do\n"
		L"			break if _upper < _lower\n"
		L"			_mid = _lower + (((_upper-_lower) >> 1) & ~1)\n"
		L"			if " << GET_WIDE_KEY() << L" < " << CK() << L"[_mid]\n"
		L"				_upper = _mid - 2\n"
		L"			elsif " << GET_WIDE_KEY() << L" > " << CK() << L"[_mid+1]\n"
		L"				_lower = _mid + 2\n"
		L"			else\n"
		L"				case " << C() << L"[" << CO() << L"[" << vCS() << L"]"
							L" + ((_mid - _keys)>>1)]\n";

	for ( CondSpaceList::Iter csi = condSpaceList; csi.lte(); csi++ ) {
		GenCondSpace *condSpace = csi;
		out << L"	when " << condSpace->condSpaceId << L" then" ;
		out << L"	_widec = " << KEY(condSpace->baseKey) << 
				L"+ (" << GET_KEY() << L" - " << KEY(keyOps->minKey) << L")\n";

		for ( GenCondSet::Iter csi = condSpace->condSet; csi.lte(); csi++ ) {
			Size condValOffset = ((1 << csi.pos()) * keyOps->alphSize());
			out << L"	_widec += " << condValOffset << L" if ( ";
			CONDITION( out, *csi );
			out << L" )\n";
		}
	}

	out <<
		L"				end # case\n"
		L"			end\n"
		L"		end # loop\n"
		L"	end\n";
}


void RubyTabCodeGen::LOCATE_TRANS()
{
	out <<
		L"	_keys = " << KO() << L"[" << vCS() << L"]\n"
		L"	_trans = " << IO() << L"[" << vCS() << L"]\n"
		L"	_klen = " << SL() << L"[" << vCS() << L"]\n"
		L"	_break_match = false\n"
		L"	\n"
		L"	begin\n"
		L"	  if _klen > 0\n"
		L"	     _lower = _keys\n"
		L"	     _upper = _keys + _klen - 1\n"
		L"\n"
		L"	     loop do\n"
		L"	        break if _upper < _lower\n"
		L"	        _mid = _lower + ( (_upper - _lower) >> 1 )\n"
		L"\n"
		L"	        if " << GET_WIDE_KEY() << L" < " << K() << L"[_mid]\n"
		L"	           _upper = _mid - 1\n"
		L"	        elsif " << GET_WIDE_KEY() << L" > " << K() << L"[_mid]\n"
		L"	           _lower = _mid + 1\n"
		L"	        else\n"
		L"	           _trans += (_mid - _keys)\n"
		L"	           _break_match = true\n"
		L"	           break\n"
		L"	        end\n"
		L"	     end # loop\n"
		L"	     break if _break_match\n"
		L"	     _keys += _klen\n"
		L"	     _trans += _klen\n"
		L"	  end"
		L"\n"
		L"	  _klen = " << RL() << L"[" << vCS() << L"]\n"
		L"	  if _klen > 0\n"
		L"	     _lower = _keys\n"
		L"	     _upper = _keys + (_klen << 1) - 2\n"
		L"	     loop do\n"
		L"	        break if _upper < _lower\n"
		L"	        _mid = _lower + (((_upper-_lower) >> 1) & ~1)\n"
		L"	        if " << GET_WIDE_KEY() << L" < " << K() << L"[_mid]\n"
		L"	          _upper = _mid - 2\n"
		L"	        elsif " << GET_WIDE_KEY() << L" > " << K() << L"[_mid+1]\n"
		L"	          _lower = _mid + 2\n"
		L"	        else\n"
		L"	          _trans += ((_mid - _keys) >> 1)\n"
		L"	          _break_match = true\n"
		L"	          break\n"
		L"	        end\n"
		L"	     end # loop\n"
		L"	     break if _break_match\n"
		L"	     _trans += _klen\n"
		L"	  end\n"
		L"	end while false\n";
}

void RubyTabCodeGen::writeExec()
{
	out << 
		L"begin\n"
		L"	_klen, _trans, _keys";

	if ( redFsm->anyRegCurStateRef() )
		out << L", _ps";
	if ( redFsm->anyConditions() ) 
		out << L", _widec";
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

	if ( useIndicies )
		out << L"	_trans = " << I() << L"[_trans]\n";

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
		TO_STATE_ACTION_SWITCH();
		out <<
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
				L"	__acts = " << EA() << L"[" << vCS() << L"]\n"
				L"	__nacts = " << L" " << A() << L"[__acts]\n"
				L"	__acts += 1\n"
				L"	while __nacts > 0\n"
				L"		__nacts -= 1\n"
				L"		__acts += 1\n"
				L"		case " << A() << L"[__acts - 1]\n";
			EOF_ACTION_SWITCH() <<
				L"		end # eof action switch\n"
				L"	end\n"
				L"	if _trigger_goto\n"
				L"		next\n"
				L"	end\n";
		}

		out << 
			L"end\n";
	}

	out << 
		L"	end\n"
		L"	if _goto_level <= _out\n"
		L"		break\n"
		L"	end\n";

	/* The loop for next. */
	out << 
		L"	end\n";

	/* Wrapping the execute block. */
	out << 
		L"	end\n";
}

std::wostream &RubyTabCodeGen::FROM_STATE_ACTION_SWITCH() 
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action */
			out << L"			when " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &RubyTabCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"when " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RubyTabCodeGen::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"when " << act->actionId << L" then\n";
			ACTION( out, act, 0, true );
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RubyTabCodeGen::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << L"when " << act->actionId << L" then\n";
			ACTION( out, act, 0, false );
		}
	}

	genLineDirective( out );
	return out;
}


void RubyTabCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
	ret << vCS() << L" = " << nextDest << L";";
}

void RubyTabCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
	ret << vCS() << L" = (";
	INLINE_LIST( ret, ilItem->children, 0, inFinish );
	ret << L");";
}


int RubyTabCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	return act;
}

int RubyTabCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	return act;
}

int RubyTabCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	return act;
}


std::wostream &RubyTabCodeGen::COND_OFFSETS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		ARRAY_ITEM( INT(curKeyOffset), ++totalStateNum, st.last() );

		/* Move the key offset ahead. */
		curKeyOffset += st->stateCondList.length();
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::KEY_OFFSETS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0, curKeyOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the key offset. */
		ARRAY_ITEM( INT(curKeyOffset), ++totalStateNum, st.last() );

		/* Move the key offset ahead. */
		curKeyOffset += st->outSingle.length() + st->outRange.length()*2;
	}
	END_ARRAY_LINE();
	return out;
}


std::wostream &RubyTabCodeGen::INDEX_OFFSETS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0, curIndOffset = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write the index offset. */
		ARRAY_ITEM( INT(curIndOffset), ++totalStateNum, st.last() );

		/* Move the index offset ahead. */
		curIndOffset += st->outSingle.length() + st->outRange.length();
		if ( st->defTrans != 0 )
			curIndOffset += 1;
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::COND_LENS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		ARRAY_ITEM( INT(st->stateCondList.length()), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}


std::wostream &RubyTabCodeGen::SINGLE_LENS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write singles length. */
		ARRAY_ITEM( INT(st->outSingle.length()), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::RANGE_LENS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Emit length of range index. */
		ARRAY_ITEM( INT(st->outRange.length()), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::TO_STATE_ACTIONS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT(TO_STATE_ACTION(st)), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::FROM_STATE_ACTIONS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT(FROM_STATE_ACTION(st)), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::EOF_ACTIONS()
{
	START_ARRAY_LINE();
	int totalStateNum = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Write any eof action. */
		ARRAY_ITEM( INT(EOF_ACTION(st)), ++totalStateNum, st.last() );
	}
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::EOF_TRANS()
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

std::wostream &RubyTabCodeGen::COND_KEYS()
{
	START_ARRAY_LINE();
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Lower key. */
			ARRAY_ITEM( KEY( sc->lowKey ), ++totalTrans, false );
			ARRAY_ITEM( KEY( sc->highKey ), ++totalTrans, false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::COND_SPACES()
{
	START_ARRAY_LINE();
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the state's transitions. */
		for ( GenStateCondList::Iter sc = st->stateCondList; sc.lte(); sc++ ) {
			/* Cond Space id. */
			ARRAY_ITEM( KEY( sc->condSpace->condSpaceId ), ++totalTrans, false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::KEYS()
{
	START_ARRAY_LINE();
	int totalTrans = 0;
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Loop the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			ARRAY_ITEM( KEY( stel->lowKey ), ++totalTrans, false );
		}

		/* Loop the state's transitions. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			/* Lower key. */
			ARRAY_ITEM( KEY( rtel->lowKey ), ++totalTrans, false );

			/* Upper key. */
			ARRAY_ITEM( KEY( rtel->highKey ), ++totalTrans, false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::INDICIES()
{
	int totalTrans = 0;
	START_ARRAY_LINE();
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			ARRAY_ITEM( KEY( stel->value->id ), ++totalTrans, false );
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			ARRAY_ITEM( KEY( rtel->value->id ), ++totalTrans, false );
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			ARRAY_ITEM( KEY( st->defTrans->id ), ++totalTrans, false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::TRANS_TARGS()
{
	int totalTrans = 0;
	START_ARRAY_LINE();
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			ARRAY_ITEM( KEY( trans->targ->id ), ++totalTrans, false );
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			ARRAY_ITEM( KEY( trans->targ->id ), ++totalTrans, false );
		}

		/* The state's default target state. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			ARRAY_ITEM( KEY( trans->targ->id ), ++totalTrans, false );
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			trans->pos = totalTrans;
			ARRAY_ITEM( KEY( trans->targ->id ), ++totalTrans, false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}


std::wostream &RubyTabCodeGen::TRANS_ACTIONS()
{
	int totalTrans = 0;
	START_ARRAY_LINE();
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* Walk the singles. */
		for ( RedTransList::Iter stel = st->outSingle; stel.lte(); stel++ ) {
			RedTransAp *trans = stel->value;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), ++totalTrans, false );
		}

		/* Walk the ranges. */
		for ( RedTransList::Iter rtel = st->outRange; rtel.lte(); rtel++ ) {
			RedTransAp *trans = rtel->value;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), ++totalTrans, false );
		}

		/* The state's default index goes next. */
		if ( st->defTrans != 0 ) {
			RedTransAp *trans = st->defTrans;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), ++totalTrans, false );
		}
	}

	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		if ( st->eofTrans != 0 ) {
			RedTransAp *trans = st->eofTrans;
			ARRAY_ITEM( INT(TRANS_ACTION( trans )), ++totalTrans, false );
		}
	}

	/* Output one last number so we don't have to figure out when the last
	 * entry is and avoid writing a comma. */
	ARRAY_ITEM( INT(0), ++totalTrans, true );
	END_ARRAY_LINE();
	return out;
}

std::wostream &RubyTabCodeGen::TRANS_TARGS_WI()
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
		ARRAY_ITEM( INT(trans->targ->id), ++totalStates, ( t >= redFsm->transSet.length()-1 ) );
	}
	END_ARRAY_LINE();
	delete[] transPtrs;
	return out;
}


std::wostream &RubyTabCodeGen::TRANS_ACTIONS_WI()
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
		ARRAY_ITEM( INT(TRANS_ACTION( trans )), ++totalAct, 
				( t >= redFsm->transSet.length()-1 ) );
	}
	END_ARRAY_LINE();
	delete[] transPtrs;
	return out;
}


void RubyTabCodeGen::writeData()
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

/*
 Local Variables:
 mode: c++
 indent-tabs-mode: 1
 c-file-style: L"bsd"
 End:
 */
