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

#ifndef _RUBY_FLATCODEGEN_H
#define _RUBY_FLATCODEGEN_H

#include <iostream>
#include "rubycodegen.h"

using std::wstring;
using std::wostream;


/*
 * FlatCodeGen
 */
class RubyFlatCodeGen : public RubyCodeGen
{
public:
	RubyFlatCodeGen( wostream &out ) :
		RubyCodeGen(out) {};
	virtual ~RubyFlatCodeGen() {}
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


	virtual int TO_STATE_ACTION( RedStateAp *state );
	virtual int FROM_STATE_ACTION( RedStateAp *state );
	virtual int EOF_ACTION( RedStateAp *state );
	virtual int TRANS_ACTION( RedTransAp *trans );

	virtual void writeData();
	virtual void writeExec();

};

#endif


/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */
