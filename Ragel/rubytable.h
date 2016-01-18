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

#ifndef _RUBY_TABCODEGEN_H
#define _RUBY_TABCODEGEN_H

#include <iostream>
#include <string>
#include <stdio.h>
#include "common.h"
#include "gendata.h"
#include "rubycodegen.h"


using std::wstring;
using std::wostream;

/*
 * RubyCodeGen
 */
class RubyTabCodeGen : public RubyCodeGen
{
public:
	RubyTabCodeGen( wostream &out ) : 
          RubyCodeGen(out) {}
        virtual ~RubyTabCodeGen() {}

public:
	void BREAK( wostream &ret, int targState );
	void GOTO( wostream &ret, int gotoDest, bool inFinish );
	void GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );
	void CALL( wostream &ret, int callDest, int targState, bool inFinish );
	void CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish );
	void RET( wostream &ret, bool inFinish );

	void COND_TRANSLATE();
	void LOCATE_TRANS();

	virtual void writeExec();
	virtual void writeData();

 protected:
	virtual std::wostream &TO_STATE_ACTION_SWITCH();
	virtual std::wostream &FROM_STATE_ACTION_SWITCH();
	virtual std::wostream &EOF_ACTION_SWITCH();
	virtual std::wostream &ACTION_SWITCH();

	std::wostream &COND_KEYS();
	std::wostream &COND_SPACES();
	std::wostream &KEYS();
	std::wostream &INDICIES();
	std::wostream &COND_OFFSETS();
	std::wostream &KEY_OFFSETS();
	std::wostream &INDEX_OFFSETS();
	std::wostream &COND_LENS();
	std::wostream &SINGLE_LENS();
	std::wostream &RANGE_LENS();
	std::wostream &TO_STATE_ACTIONS();
	std::wostream &FROM_STATE_ACTIONS();
	std::wostream &EOF_ACTIONS();
	std::wostream &EOF_TRANS();
	std::wostream &TRANS_TARGS();
	std::wostream &TRANS_ACTIONS();
	std::wostream &TRANS_TARGS_WI();
	std::wostream &TRANS_ACTIONS_WI();


	void NEXT( wostream &ret, int nextDest, bool inFinish );
	void NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );

	virtual int TO_STATE_ACTION( RedStateAp *state );
	virtual int FROM_STATE_ACTION( RedStateAp *state );
	virtual int EOF_ACTION( RedStateAp *state );

private:
	wstring array_type;
	wstring array_name;

public:

	void EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish );
	void EXECTE( wostream &ret, GenInlineItem *item, int targState, int inFinish );
	void LM_SWITCH( wostream &ret, GenInlineItem *item, int targState, int inFinish );
	void SET_ACT( wostream &ret, GenInlineItem *item );
	void INIT_TOKSTART( wostream &ret, GenInlineItem *item );
	void INIT_ACT( wostream &ret, GenInlineItem *item );
	void SET_TOKSTART( wostream &ret, GenInlineItem *item );
	void SET_TOKEND( wostream &ret, GenInlineItem *item );
	void GET_TOKEND( wostream &ret, GenInlineItem *item );
	void SUB_ACTION( wostream &ret, GenInlineItem *item, 
			int targState, bool inFinish );


};


#endif

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */
