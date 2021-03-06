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

#ifndef _GOIPGOTO_H
#define _GOIPGOTO_H

#include <iostream>
#include "gogoto.h"

/* Forwards. */
struct CodeGenData;

class GoIpGotoCodeGen
	: public GoGotoCodeGen
{
public:
	GoIpGotoCodeGen( wostream &out )
			: GoGotoCodeGen(out) {}

	std::wostream &EXIT_STATES();
	std::wostream &TRANS_GOTO( RedTransAp *trans, int level );
	int TRANS_NR( RedTransAp *trans );
	std::wostream &FINISH_CASES( int level );
	std::wostream &AGAIN_CASES( int level );
	std::wostream &STATE_GOTOS_SWITCH( int level );

	void GOTO( wostream &ret, int gotoDest, bool inFinish );
	void CALL( wostream &ret, int callDest, int targState, bool inFinish );
	void NEXT( wostream &ret, int nextDest, bool inFinish );
	void GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );
	void NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );
	void CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish );
	void RET( wostream &ret, bool inFinish );
	void CURS( wostream &ret, bool inFinish );
	void TARGS( wostream &ret, bool inFinish, int targState );
	void BREAK( wostream &ret, int targState, bool csForced );

	virtual void writeData();
	virtual void writeExec();

protected:
	bool useAgainLabel();

	/* Called from GotoCodeGen::STATE_GOTOS just before writing the gotos for
	 * each state. */
	bool IN_TRANS_ACTIONS( RedStateAp *state );
	void GOTO_HEADER( RedStateAp *state, int level );
	void STATE_GOTO_ERROR( int level );

	/* Set up labelNeeded flag for each state. */
	void setLabelsNeeded( GenInlineList *inlineList );
	void setLabelsNeeded();
};

#endif
