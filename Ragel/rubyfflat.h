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

#ifndef _RUBY_FFLATCODEGEN_H
#define _RUBY_FFLATCODEGEN_H

#include <iostream>
#include "rubyflat.h"

class RubyFFlatCodeGen : public RubyFlatCodeGen 
{
public:
	RubyFFlatCodeGen( wostream &out ) : 
		RubyFlatCodeGen(out) {}
protected:
	
	std::wostream &TO_STATE_ACTION_SWITCH();
	std::wostream &FROM_STATE_ACTION_SWITCH();
	std::wostream &EOF_ACTION_SWITCH();
	std::wostream &ACTION_SWITCH();

	void GOTO( wostream &out, int gotoDest, bool inFinish );
	void GOTO_EXPR( wostream &out, GenInlineItem *ilItem, bool inFinish );
	void CALL( wostream &out, int callDest, int targState, bool inFinish );
	void CALL_EXPR(wostream &out, GenInlineItem *ilItem, int targState, bool inFinish );
	void RET( wostream &out, bool inFinish );
	void BREAK( wostream &out, int targState );

	virtual int TO_STATE_ACTION( RedStateAp *state );
	virtual int FROM_STATE_ACTION( RedStateAp *state );
	virtual int EOF_ACTION( RedStateAp *state );
	virtual int TRANS_ACTION( RedTransAp *trans );

	virtual void writeData();
	virtual void writeExec();
};

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */

#endif
