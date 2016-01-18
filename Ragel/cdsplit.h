/*
 *  Copyright 2006 Adrian Thurston <thurston@complang.org>
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

#ifndef _CDSPLIT_H
#define _CDSPLIT_H

#include "cdipgoto.h"

class SplitCodeGen : public IpGotoCodeGen
{
public:
	SplitCodeGen( wostream &out ) : FsmCodeGen(out), IpGotoCodeGen(out) {}

	bool ptOutLabelUsed;

	std::wostream &PART_MAP();
	std::wostream &EXIT_STATES( int partition );
	std::wostream &PART_TRANS( int partition );
	std::wostream &TRANS_GOTO( RedTransAp *trans, int level );
	void GOTO_HEADER( RedStateAp *state, bool stateInPartition );
	std::wostream &STATE_GOTOS( int partition );
	std::wostream &PARTITION( int partition );
	std::wostream &ALL_PARTITIONS();
	void writeData();
	void writeExec();
	void writeParts();

	void setLabelsNeeded( RedStateAp *fromState, GenInlineList *inlineList );
	void setLabelsNeeded( RedStateAp *fromState, RedTransAp *trans );
	void setLabelsNeeded();

	int currentPartition;
};

struct CSplitCodeGen
	: public SplitCodeGen, public CCodeGen
{
	CSplitCodeGen( wostream &out ) : 
		FsmCodeGen(out), SplitCodeGen(out), CCodeGen(out) {}
};

/*
 * class DIpGotoCodeGen
 */
struct DSplitCodeGen
	: public SplitCodeGen, public DCodeGen
{
	DSplitCodeGen( wostream &out ) : 
		FsmCodeGen(out), SplitCodeGen(out), DCodeGen(out) {}
};

/*
 * class D2SplitCodeGen
 */
struct D2SplitCodeGen
	: public SplitCodeGen, public D2CodeGen
{
	D2SplitCodeGen( wostream &out ) : 
		FsmCodeGen(out), SplitCodeGen(out), D2CodeGen(out) {}
};

#endif
