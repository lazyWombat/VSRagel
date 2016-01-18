/*
 *  Copyright 2004-2006 Adrian Thurston <thurston@complang.org>
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
#include "mlfflat.h"
#include "redfsm.h"
#include "gendata.h"

std::wostream &OCamlFFlatCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->actListId+1;
	out << act;
	return out;
}

std::wostream &OCamlFFlatCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->actListId+1;
	out << act;
	return out;
}

std::wostream &OCamlFFlatCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->actListId+1;
	out << act;
	return out;
}

/* Write out the function for a transition. */
std::wostream &OCamlFFlatCodeGen::TRANS_ACTION( RedTransAp *trans )
{
	int action = 0;
	if ( trans->action != 0 )
		action = trans->action->actListId+1;
	out << action;
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &OCamlFFlatCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numToStateRefs > 0 ) {
			/* Write the entry label. */
			out << L"\t| " << redAct->actListId+1 << L" ->\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &OCamlFFlatCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numFromStateRefs > 0 ) {
			/* Write the entry label. */
			out << L"\t| " << redAct->actListId+1 << L" ->\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlFFlatCodeGen::EOF_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numEofRefs > 0 ) {
			/* Write the entry label. */
			out << L"\t| " << redAct->actListId+1 << L" ->\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, true );

			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &OCamlFFlatCodeGen::ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			/* Write the entry label. */
			out << L"\t| " << redAct->actListId+1 << L" ->\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

			out << L"\t()\n";
		}
	}

	genLineDirective( out );
	return out;
}

void OCamlFFlatCodeGen::writeData()
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

  out << L"type " << TYPE_STATE() << L" = { mutable keys : int; mutable trans : int; }"
    << TOP_SEP();

  out << L"exception Goto_match" << TOP_SEP();
  out << L"exception Goto_again" << TOP_SEP();
  out << L"exception Goto_eof_trans" << TOP_SEP();
}

void OCamlFFlatCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;
	initVarTypes();

	out << 
		L"	begin\n";
//		L"	" << slenType << L" _slen";

//	if ( redFsm->anyRegCurStateRef() )
//		out << L", _ps";
	
//	out << L";\n";
//	out << L"	" << transType << L" _trans";

//	if ( redFsm->anyConditions() )
//		out << L", _cond";

//	out << L";\n";

//	out <<
//		L"	" << L"int _keys;\n"
//		L"	" << indsType << L" _inds;\n";
		/*
		L"	" << PTR_CONST() << WIDE_ALPH_TYPE() << POINTER() << L"_keys;\n"
		L"	" << PTR_CONST() << ARRAY_TYPE(redFsm->maxIndex) << POINTER() << L"_inds;\n";*/

  out <<
    L"	let state = { keys = 0; trans = 0; } in\n"
    L"	let rec do_start () =\n";

//	if ( redFsm->anyConditions() ) {
//		out << 
//			L"	" << condsType << L" _conds;\n"
//			L"	" << WIDE_ALPH_TYPE() << L" _widec;\n";
//	}

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
			L"	begin match " << AT( FSA(), vCS() ) << L" with\n";
			FROM_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"	end;\n"
			L"\n";
	}

	if ( redFsm->anyConditions() )
		COND_TRANSLATE();

  out << L"\tbegin try\n";
	LOCATE_TRANS();
  out << L"\twith Goto_match -> () end;\n";

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
			L"	begin try if " << AT( TA() , L"state.trans" ) << L" = 0 then\n"
			L"		raise Goto_again;\n"
			L"\n"
			L"	match " << AT( TA(), L"state.trans" ) << L" with\n";
			ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"	with Goto_again -> () end;\n"
			L"\n";
	}
  out << L"\tdo_again ()\n";

//	if ( redFsm->anyRegActions() || redFsm->anyActionGotos() || 
//			redFsm->anyActionCalls() || redFsm->anyActionRets() )
  out << L"\tand do_again () =\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L"	begin match " << AT( TSA(), vCS() ) << L" with\n";
			TO_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"	end;\n"
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
				L"	begin match " << AT( EA(), vCS() ) << L" with\n";
				EOF_ACTION_SWITCH();
				SWITCH_DEFAULT() <<
				L"	end\n";
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

