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
#pragma once
#ifndef _CDCODEGEN_HH
#define _CDCODEGEN_H

#include <iostream>
#include <string>
#include <stdio.h>
#include "common.h"
#include "gendata.h"

using std::wstring;
using std::wostream;

/* Integer array line length. */
#define IALL 8

/* Forwards. */
struct RedFsmAp;
struct RedStateAp;
struct CodeGenData;
struct GenAction;
struct NameInst;
struct GenInlineItem;
struct GenInlineList;
struct RedAction;
struct LongestMatch;
struct LongestMatchPart;

wstring itoa( int i );

/*
 * class FsmCodeGen
 */
class FsmCodeGen : public CodeGenData
{
public:
	FsmCodeGen( wostream &out );
	virtual ~FsmCodeGen() {}

	virtual void finishRagelDef();
	virtual void writeInit();
	virtual void writeStart();
	virtual void writeFirstFinal();
	virtual void writeError();

protected:
	wstring FSM_NAME();
	wstring START_STATE_ID();
	wostream &ACTIONS_ARRAY();
	wstring GET_WIDE_KEY();
	wstring GET_WIDE_KEY( RedStateAp *state );
	wstring TABS( int level );
	wstring KEY( Key key );
	wstring WIDE_KEY( RedStateAp *state, Key key );
	wstring LDIR_PATH( wchar_t *path );
	virtual void ACTION( wostream &ret, GenAction *action, int targState, 
			bool inFinish, bool csForced );
	void CONDITION( wostream &ret, GenAction *condition );
	wstring ALPH_TYPE();
	wstring WIDE_ALPH_TYPE();
	wstring ARRAY_TYPE( unsigned long maxVal );

	bool isAlphTypeSigned();
	bool isWideAlphTypeSigned();

	virtual wstring ARR_OFF( wstring ptr, wstring offset ) = 0;
	virtual wstring CAST( wstring type ) = 0;
	virtual wstring UINT() = 0;
	virtual wstring NULL_ITEM() = 0;
	virtual wstring POINTER() = 0;
	virtual wstring GET_KEY();
	virtual wostream &SWITCH_DEFAULT() = 0;

	wstring P();
	wstring PE();
	wstring vEOF();

	wstring ACCESS();
	wstring vCS();
	wstring STACK();
	wstring TOP();
	wstring TOKSTART();
	wstring TOKEND();
	wstring ACT();

	wstring DATA_PREFIX();
	wstring PM() { return L"_" + DATA_PREFIX() + L"partition_map"; }
	wstring C() { return L"_" + DATA_PREFIX() + L"cond_spaces"; }
	wstring CK() { return L"_" + DATA_PREFIX() + L"cond_keys"; }
	wstring K() { return L"_" + DATA_PREFIX() + L"trans_keys"; }
	wstring I() { return L"_" + DATA_PREFIX() + L"indicies"; }
	wstring CO() { return L"_" + DATA_PREFIX() + L"cond_offsets"; }
	wstring KO() { return L"_" + DATA_PREFIX() + L"key_offsets"; }
	wstring IO() { return L"_" + DATA_PREFIX() + L"index_offsets"; }
	wstring CL() { return L"_" + DATA_PREFIX() + L"cond_lengths"; }
	wstring SL() { return L"_" + DATA_PREFIX() + L"single_lengths"; }
	wstring RL() { return L"_" + DATA_PREFIX() + L"range_lengths"; }
	wstring A() { return L"_" + DATA_PREFIX() + L"actions"; }
	wstring TA() { return L"_" + DATA_PREFIX() + L"trans_actions"; }
	wstring TT() { return L"_" + DATA_PREFIX() + L"trans_targs"; }
	wstring TSA() { return L"_" + DATA_PREFIX() + L"to_state_actions"; }
	wstring FSA() { return L"_" + DATA_PREFIX() + L"from_state_actions"; }
	wstring EA() { return L"_" + DATA_PREFIX() + L"eof_actions"; }
	wstring ET() { return L"_" + DATA_PREFIX() + L"eof_trans"; }
	wstring SP() { return L"_" + DATA_PREFIX() + L"key_spans"; }
	wstring CSP() { return L"_" + DATA_PREFIX() + L"cond_key_spans"; }
	wstring START() { return DATA_PREFIX() + L"start"; }
	wstring ERROR() { return DATA_PREFIX() + L"error"; }
	wstring FIRST_FINAL() { return DATA_PREFIX() + L"first_final"; }
	wstring CTXDATA() { return DATA_PREFIX() + L"ctxdata"; }

	void INLINE_LIST( wostream &ret, GenInlineList *inlineList, 
			int targState, bool inFinish, bool csForced );
	virtual void GOTO( wostream &ret, int gotoDest, bool inFinish ) = 0;
	virtual void CALL( wostream &ret, int callDest, int targState, bool inFinish ) = 0;
	virtual void NEXT( wostream &ret, int nextDest, bool inFinish ) = 0;
	virtual void GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish ) = 0;
	virtual void NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish ) = 0;
	virtual void CALL_EXPR( wostream &ret, GenInlineItem *ilItem, 
			int targState, bool inFinish ) = 0;
	virtual void RET( wostream &ret, bool inFinish ) = 0;
	virtual void BREAK( wostream &ret, int targState, bool csForced ) = 0;
	virtual void CURS( wostream &ret, bool inFinish ) = 0;
	virtual void TARGS( wostream &ret, bool inFinish, int targState ) = 0;
	void EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish );
	void LM_SWITCH( wostream &ret, GenInlineItem *item, int targState, 
			int inFinish, bool csForced );
	void SET_ACT( wostream &ret, GenInlineItem *item );
	void INIT_TOKSTART( wostream &ret, GenInlineItem *item );
	void INIT_ACT( wostream &ret, GenInlineItem *item );
	void SET_TOKSTART( wostream &ret, GenInlineItem *item );
	void SET_TOKEND( wostream &ret, GenInlineItem *item );
	void GET_TOKEND( wostream &ret, GenInlineItem *item );
	virtual void SUB_ACTION( wostream &ret, GenInlineItem *item, 
			int targState, bool inFinish, bool csForced );
	void STATE_IDS();

	wstring ERROR_STATE();
	wstring FIRST_FINAL_STATE();

	virtual wstring PTR_CONST() = 0;
	virtual wstring PTR_CONST_END() = 0;
	virtual wostream &OPEN_ARRAY( wstring type, wstring name ) = 0;
	virtual wostream &CLOSE_ARRAY() = 0;
	virtual wostream &STATIC_VAR( wstring type, wstring name ) = 0;

	virtual wstring CTRL_FLOW() = 0;

	wostream &source_warning(const InputLoc &loc);
	wostream &source_error(const InputLoc &loc);

	unsigned int arrayTypeSize( unsigned long maxVal );

	bool outLabelUsed;
	bool testEofUsed;
	bool againLabelUsed;
	bool useIndicies;

	void genLineDirective( wostream &out );

public:
	/* Determine if we should use indicies. */
	virtual void calcIndexSize() {}
};

class CCodeGen : virtual public FsmCodeGen
{
public:
	CCodeGen( wostream &out ) : FsmCodeGen(out) {}

	virtual wstring NULL_ITEM();
	virtual wstring POINTER();
	virtual wostream &SWITCH_DEFAULT();
	virtual wostream &OPEN_ARRAY( wstring type, wstring name );
	virtual wostream &CLOSE_ARRAY();
	virtual wostream &STATIC_VAR( wstring type, wstring name );
	virtual wstring ARR_OFF( wstring ptr, wstring offset );
	virtual wstring CAST( wstring type );
	virtual wstring UINT();
	virtual wstring PTR_CONST();
	virtual wstring PTR_CONST_END();
	virtual wstring CTRL_FLOW();

	virtual void writeExports();
};

class DCodeGen : virtual public FsmCodeGen
{
public:
	DCodeGen( wostream &out ) : FsmCodeGen(out) {}

	virtual wstring NULL_ITEM();
	virtual wstring POINTER();
	virtual wostream &SWITCH_DEFAULT();
	virtual wostream &OPEN_ARRAY( wstring type, wstring name );
	virtual wostream &CLOSE_ARRAY();
	virtual wostream &STATIC_VAR( wstring type, wstring name );
	virtual wstring ARR_OFF( wstring ptr, wstring offset );
	virtual wstring CAST( wstring type );
	virtual wstring UINT();
	virtual wstring PTR_CONST();
	virtual wstring PTR_CONST_END();
	virtual wstring CTRL_FLOW();

	virtual void writeExports();
};

class D2CodeGen : virtual public FsmCodeGen
{
public:
	D2CodeGen( wostream &out ) : FsmCodeGen(out) {}

	virtual wstring NULL_ITEM();
	virtual wstring POINTER();
	virtual wostream &SWITCH_DEFAULT();
	virtual wostream &OPEN_ARRAY( wstring type, wstring name );
	virtual wostream &CLOSE_ARRAY();
	virtual wostream &STATIC_VAR( wstring type, wstring name );
	virtual wstring ARR_OFF( wstring ptr, wstring offset );
	virtual wstring CAST( wstring type );
	virtual wstring UINT();
	virtual wstring PTR_CONST();
	virtual wstring PTR_CONST_END();
	virtual wstring CTRL_FLOW();

	virtual void writeExports();
	virtual void SUB_ACTION( wostream &ret, GenInlineItem *item, 
			int targState, bool inFinish, bool csForced );
	virtual void ACTION( wostream &ret, GenAction *action, int targState, 
			bool inFinish, bool csForced );

};

#endif
