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
#include "mlfgoto.h"
#include "redfsm.h"
#include "gendata.h"
#include "bstmap.h"

std::wostream &OCamlFGotoCodeGen::EXEC_ACTIONS()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numTransRefs > 0 ) {
			/* 	We are at the start of a glob, write the case. */
			out << L"and f" << redAct->actListId << L" () =\n";
			out << L"\tbegin try\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

			out << L"\twith Goto_again -> () end;\n";
			out << L"\tdo_again ()\n";
		}
	}
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &OCamlFGotoCodeGen::TO_STATE_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numToStateRefs > 0 ) {
			/* Write the entry label. */
			out << L"\t|" << redAct->actListId+1 << L" ->\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

//			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

/* Write out the function switch. This switch is keyed on the values
 * of the func index. */
std::wostream &OCamlFGotoCodeGen::FROM_STATE_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numFromStateRefs > 0 ) {
			/* Write the entry label. */
			out << L"\t| " << redAct->actListId+1 << L" ->\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, false );

//			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}

std::wostream &OCamlFGotoCodeGen::EOF_ACTION_SWITCH()
{
	/* Loop the actions. */
	for ( GenActionTableMap::Iter redAct = redFsm->actionMap; redAct.lte(); redAct++ ) {
		if ( redAct->numEofRefs > 0 ) {
			/* Write the entry label. */
			out << L"\t| " << redAct->actListId+1 << L" ->\n";

			/* Write each action in the list of action items. */
			for ( GenActionTable::Iter item = redAct->key; item.lte(); item++ )
				ACTION( out, item->value, 0, true );

//			out << L"\tbreak;\n";
		}
	}

	genLineDirective( out );
	return out;
}


std::wostream &OCamlFGotoCodeGen::FINISH_CASES()
{
	for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
		/* States that are final and have an out action need a case. */
		if ( st->eofAction != 0 ) {
			/* Write the case label. */
			out << L"\t\t| " << st->id << L" -> ";

			/* Jump to the func. */
			out << L"f" << st->eofAction->actListId << L" ()\n";
		}
	}

	return out;
}

unsigned int OCamlFGotoCodeGen::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->actListId+1;
	return act;
}

unsigned int OCamlFGotoCodeGen::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->actListId+1;
	return act;
}

unsigned int OCamlFGotoCodeGen::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->actListId+1;
	return act;
}

void OCamlFGotoCodeGen::writeData()
{
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

  out << L"exception Goto_again" << TOP_SEP();
}

void OCamlFGotoCodeGen::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;

	out << L"	begin\n";

	if ( redFsm->anyRegCurStateRef() )
		out << L"	let _ps = ref 0 in\n";

	if ( redFsm->anyConditions() )
		out << L"	let _widec : " << WIDE_ALPH_TYPE() << L" = ref 0 in\n";

	out << L"\n";
  out << L"\tlet rec do_start () =\n";
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
			L"	begin match " << AT(FSA(),vCS()) << L" with\n";
			FROM_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"	end;\n"
			L"\n";
	}

	out << 
		L"	begin match " << vCS() << L" with\n";
		STATE_GOTOS();
		SWITCH_DEFAULT() <<
		L"	end\n"
		L"\n";
		TRANSITIONS() << 
		L"\n";

	if ( redFsm->anyRegActions() )
		EXEC_ACTIONS() << L"\n";

	out << L"\tand do_again () =\n";

	if ( redFsm->anyToStateActions() ) {
		out <<
			L" begin match " << AT(TSA(), vCS()) << L" with\n";
			TO_STATE_ACTION_SWITCH();
			SWITCH_DEFAULT() <<
			L"	end;\n"
			L"\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			L"	match " << vCS() << L" with\n" <<
      L"	| " << redFsm->errState->id << L" -> do_out ()\n"
      L"	| _ ->\n";
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
			L"	begin\n";

		if ( redFsm->anyEofTrans() ) {
			out <<
				L"	match " << vCS() << L" with\n";

			for ( RedStateList::Iter st = redFsm->stateList; st.lte(); st++ ) {
				if ( st->eofTrans != 0 )
					out << L"	| " << st->id << L" -> tr" << st->eofTrans->id << L" ()\n";
			}

			SWITCH_DEFAULT() << L";\n"; // fall through
		}

		if ( redFsm->anyEofActions() ) {
			out <<
				L"	try match " << AT(EA(), vCS()) << L" with\n";
				EOF_ACTION_SWITCH();
				SWITCH_DEFAULT() <<
				L"	\n"
				L"	with Goto_again -> do_again () \n";
		}

		out <<
			L"	end\n"
			L"\n";
	}
  else
    out << L"\t()\n";

	if ( outLabelUsed )
		out << L"\tand do_out () = ()\n";

  out << L"\tin do_start ()\n";
	out << L"	end;\n";
}
