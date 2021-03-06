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

#ifndef _MLFLAT_H
#define _MLFLAT_H

#include <iostream>
#include "mlcodegen.h"

/* Forwards. */
//struct CodeGenData;
//struct NameInst;
//struct RedTransAp;
//struct RedStateAp;

/*
 * OCamlFlatCodeGen
 */
class OCamlFlatCodeGen : public OCamlCodeGen
{
public:
	OCamlFlatCodeGen( wostream &out ) : OCamlCodeGen(out) {}
	virtual ~OCamlFlatCodeGen() { }

protected:
	std::wostream &TO_STATE_ACTION_SWITCH();
	std::wostream &FROM_STATE_ACTION_SWITCH();
	std::wostream &EOF_ACTION_SWITCH();
	std::wostream &ACTION_SWITCH();
	std::wostream &KEYS();
	std::wostream &INDICIES();
	std::wostream &FLAT_INDEX_OFFSET();
	std::wostream &KEY_SPANS();
	std::wostream &TO_STATE_ACTIONS();
	std::wostream &FROM_STATE_ACTIONS();
	std::wostream &EOF_ACTIONS();
	std::wostream &EOF_TRANS();
	std::wostream &TRANS_TARGS();
	std::wostream &TRANS_ACTIONS();
	void LOCATE_TRANS();

	std::wostream &COND_INDEX_OFFSET();
	void COND_TRANSLATE();
	std::wostream &CONDS();
	std::wostream &COND_KEYS();
	std::wostream &COND_KEY_SPANS();

	void GOTO( wostream &ret, int gotoDest, bool inFinish );
	void CALL( wostream &ret, int callDest, int targState, bool inFinish );
	void NEXT( wostream &ret, int nextDest, bool inFinish );
	void GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );
	void NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );
	void CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish );
	void CURS( wostream &ret, bool inFinish );
	void TARGS( wostream &ret, bool inFinish, int targState );
	void RET( wostream &ret, bool inFinish );
	void BREAK( wostream &ret, int targState );

	virtual std::wostream &TO_STATE_ACTION( RedStateAp *state );
	virtual std::wostream &FROM_STATE_ACTION( RedStateAp *state );
	virtual std::wostream &EOF_ACTION( RedStateAp *state );
	virtual std::wostream &TRANS_ACTION( RedTransAp *trans );

	virtual void writeData();
	virtual void writeExec();

	void initVarTypes();
	wstring slenType, transType, indsType, condsType;
};

#endif
