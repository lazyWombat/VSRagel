/*
 *  Copyright 2007 Victor Hugo Borja <vic@rubyforge.org>
 *            2006-2007 Adrian Thurston <thurston@complang.org>
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

#ifndef _RBX_GOTOCODEGEN_H
#define _RBX_GOTOCODEGEN_H

#include <iostream>
#include <string>
#include "rubycodegen.h"

using std::wstring;

class RbxGotoCodeGen : public RubyCodeGen
{
public:
        RbxGotoCodeGen( wostream &out ) : RubyCodeGen(out) {}
        virtual ~RbxGotoCodeGen() {}

	std::wostream &TO_STATE_ACTION_SWITCH();
	std::wostream &FROM_STATE_ACTION_SWITCH();
	std::wostream &EOF_ACTION_SWITCH();
	std::wostream &ACTION_SWITCH();
	std::wostream &STATE_GOTOS();
	std::wostream &TRANSITIONS();
	std::wostream &EXEC_FUNCS();
	std::wostream &FINISH_CASES();

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

	int TO_STATE_ACTION( RedStateAp *state );
        int FROM_STATE_ACTION( RedStateAp *state );
        int EOF_ACTION( RedStateAp *state );

	void COND_TRANSLATE( GenStateCond *stateCond, int level );
	void emitCondBSearch( RedStateAp *state, int level, int low, int high );
	void STATE_CONDS( RedStateAp *state, bool genDefault ); 

	virtual std::wostream &TRANS_GOTO( RedTransAp *trans, int level );

	void emitSingleSwitch( RedStateAp *state );
	void emitRangeBSearch( RedStateAp *state, int level, int low, int high );

	/* Called from STATE_GOTOS just before writing the gotos */
	virtual void GOTO_HEADER( RedStateAp *state );
	virtual void STATE_GOTO_ERROR();

	virtual void writeData();
	virtual void writeEOF();
	virtual void writeExec();

        
        std::wostream &TO_STATE_ACTIONS();
        std::wostream &FROM_STATE_ACTIONS();
        std::wostream &EOF_ACTIONS();

private:
	wostream &rbxGoto(wostream &out, wstring label);
	wostream &rbxLabel(wostream &out, wstring label);	
};

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */

#endif 
