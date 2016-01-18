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

#include "rubyfflat.h"

void RubyFFlatCodeGen::GOTO( wostream &out, int gotoDest, bool inFinish )
{
	out << 
		L"	begin\n"
		L"		" << vCS() << L" = " << gotoDest << L"\n"
		L"		_goto_level = _again\n"
		L"		next\n"
		L"	end\n";
}

void RubyFFlatCodeGen::GOTO_EXPR( wostream &out, GenInlineItem *ilItem, bool inFinish )
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

void RubyFFlatCodeGen::CALL( wostream &out, int callDest, int targState, bool inFinish )
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

void RubyFFlatCodeGen::CALL_EXPR(wostream &out, GenInlineItem *ilItem, 
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

void RubyFFlatCodeGen::RET( wostream &out, bool inFinish )
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

void RubyFFlatCodeGen::BREAK( wostream &out, int targState )
{
	out << 
		L"	begin\n"
		L"		" << P() << L" += 1\n"
		L"		_goto_level = _out\n"
		L"		next\n"
		L"	end\n";
}


int RubyFFlatCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->actListId+1;
	return act;
}

int RubyFFlatCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->actListId+1;
	return act;
}

int RubyFFlatCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->actListId+1;
	return act;
}

/* Write out the function for a transition. */
int RubyFFlatCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	int action = 0;
	if ( trans->action != 0 )
		action = trans->action->actListId+1;
	return action;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &RubyFFlatCodeGen::TO_STATE_ACTION_SWITCH()
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
std::wostream &RubyFFlatCodeGen::FROM_STATE_ACTION_SWITCH()
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

std::wostream &RubyFFlatCodeGen::EOF_ACTION_SWITCH()
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
std::wostream &RubyFFlatCodeGen::ACTION_SWITCH()
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

void RubyFFlatCodeGen::writeData()
{
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
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActListId), TA() );
		TRANS_ACTIONS();
		CLOSE_ARRAY() <<
		L"\n";
	}

	if ( redFsm->anyToStateActions() ) {
		OPEN_ARRAY( ARRAY_TYPE(redFsm->maxActionLoc),  TSA() );
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

void RubyFFlatCodeGen::writeExec()
{
	out << 
		L"begin\n"
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
		/* break _again */
		out << 
			L"	if " << TA() << L"[_trans] != 0\n"
			L"	case " << TA() << L"[_trans]" << L"\n";
			ACTION_SWITCH() <<
			L"	end\n"
			L"	end\n";
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
				L"	  case " << EA() << L"[" << vCS() << L"]\n";
				EOF_ACTION_SWITCH() <<
				L"	  end\n";
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

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */

