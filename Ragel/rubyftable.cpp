/*
 *  2007 Victor Hugo Borja <vic@rubyforge.org>
 *  Copyright 2001-2007 Adrian Thurston <thurston@complang.org>
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
#include "rubyftable.h"

using std::wostream;
using std::wostringstream;
using std::wstring;
using std::endl;

void RubyFTabCodeGen::GOTO( wostream &out, int gotoDest, bool inFinish )
{
	out << 
		L"	begin\n"
		L"		" << vCS() << L" = " << gotoDest << L"\n"
		L"		_goto_level = _again\n"
		L"		next\n"
		L"	end\n";
}

void RubyFTabCodeGen::GOTO_EXPR( wostream &out, GenInlineItem *ilItem, bool inFinish )
{
	out << 
		L"	begin\n"
		L"		" << vCS() << L" = (";
	INLINE_LIST( out, ilItem->children, 0, inFinish );
	out << L")\n";
	out <<
		L"		_goto_level = _again\n"
		L"		next\n"
		L"	end\n";
}

void RubyFTabCodeGen::CALL( wostream &out, int callDest, int targState, bool inFinish )
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
		L"		_goto_level = _again\n"
		L"		next\n"
		L"	end\n";

	if ( prePushExpr != 0 )
		out << L"end\n";
}

void RubyFTabCodeGen::CALL_EXPR(wostream &out, GenInlineItem *ilItem, 
		int targState, bool inFinish )
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
		L"		_goto_level = _again\n"
		L"		next\n"
		L"	end\n";

	if ( prePushExpr != 0 )
		out << L"end\n";
}

void RubyFTabCodeGen::RET( wostream &out, bool inFinish )
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
		L"		_goto_level = _again\n"
		L"		next\n"
		L"	end\n";
}

void RubyFTabCodeGen::BREAK( wostream &out, int targState )
{
	out << 
		L"	begin\n"
		L"		" << P() << L" += 1\n"
		L"		_goto_level = _out\n"
		L"		next\n"
		L"	end\n";
}


std::wostream &RubyFTabCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numToStateRefs > 0 ) {
			/* Write the entry label. */
			out << L"\twhen " << redAct->actListId+1 << L" then\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

		}
	}

	genLineDirective( out );
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &RubyFTabCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numFromStateRefs > 0 ) {
			/* Write the entry label. */
			out << L"\twhen " << redAct->actListId+1 << L" then\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &RubyFTabCodeGen::EOF_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numEofRefs > 0 ) {
			/* Write the entry label. */
			out << L"\twhen " << redAct->actListId+1 << L" then\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, true );

		}
	}

	genLineDirective( out );
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &RubyFTabCodeGen::ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			/* Write the entry label. */
			out << L"\twhen " << redAct->actListId+1 << L" then\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

		}
	}

	genLineDirective( out );
	return out;
}


int RubyFTabCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->actListId+1;
	return act;
}

int RubyFTabCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->actListId+1;
	return act;
}

int RubyFTabCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->actListId+1;
        return act;
}


/* Write out the function for a transition. */
int RubyFTabCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	int action = 0;
	if ( trans->action != 0 )
		action = trans->action->actListId+1;
	return action;
}

void RubyFTabCodeGen::writeData()
{

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
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActListId), TA() );
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
			OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActListId), TA() );
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
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActListId), EA() );
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

void RubyFTabCodeGen::writeExec()
{
	out << 
		L"begin\n"
		L"	testEof = false\n"
		L"	_klen, _trans, _keys";

	if ( redFsm->anyRegCurStateRef() )
		out << L", _ps";

	if ( redFsm->anyConditions() )
		out << L", _widec";

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
			L"	case " << FSA() << L"[" << vCS() << L"] \n";
			FROM_STATE_ACTION_SWITCH() <<
			L"	end # from state action switch \n"
			L"\n";
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

	LOCATE_TRANS();

	if ( useIndicies )
		out << L"	_trans = " << I() << L"[_trans];\n";

	if ( redFsm->anyEofTrans() ) {
		out << 
			L"	end\n"
			L"	if _goto_level <= _eof_trans\n";
	}

	if ( redFsm->anyRegCurStateRef() )
		out << L"	_ps = " << vCS() << L";\n";

	out <<
		L"	" << vCS() << L" = " << TT() << L"[_trans];\n"
		L"\n";

	if ( redFsm->anyRegActions() ) {
		out << 
			L"	if " << TA() << L"[_trans] != 0\n"
			L"\n"
			L"		case " << TA() << L"[_trans] \n";
			ACTION_SWITCH() <<
			L"		end # action switch \n"
			L"	end\n"
			L"\n";
	}
        
	/* The again label. */
	out <<
		L"	end\n"
		L"	if _goto_level <= _again\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	case " << TSA() << L"[" << vCS() << L"] \n";
			TO_STATE_ACTION_SWITCH() <<
			L"	end\n"
			L"\n";
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

	/* The test eof label. */
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
				L"		case ( " << EA() << L"[" << vCS() << L"] )\n";
				EOF_ACTION_SWITCH() <<
				L"		end\n"
				L"	end\n";
		}

		out << 
			L"	end\n"
			L"\n";
	}

	out << 
		L"	end\n"
		L"	if _goto_level <= _out\n"
		L"		break\n"
		L"	end\n"
		L"end\n";

	/* Wrapping the execute block. */
	out << L"	end\n";
}


void RubyFTabCodeGen::calcIndexSize()
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
		sizeWithInds += arrayTypeSize(redFsm->maxActListId) * redFsm->transSet.length();

	/* Calculate the cost of not using indicies. */
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		int totalIndex = st->outSingle.length() + st->outRange.length() + 
				(st->defTrans == 0 ? 0 : 1);
		sizeWithoutInds += arrayTypeSize(redFsm->maxState) * totalIndex;
		if ( redFsm->anyActions() )
			sizeWithoutInds += arrayTypeSize(redFsm->maxActListId) * totalIndex;
	}

	/* If using indicies reduces the size, use them. */
	useIndicies = sizeWithInds < sizeWithoutInds;
}

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */
