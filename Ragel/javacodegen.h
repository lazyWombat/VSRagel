/*
 *  Copyright 2006-2007 Adrian Thurston <thurston@complang.org>
 *            2007 Colin Fleming <colin.fleming@caverock.com>
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

#ifndef _JAVACODEGEN_H
#define _JAVACODEGEN_H

#include <iostream>
#include <string>
#include <stdio.h>
#include "common.h"
#include "gendata.h"

using std::wstring;
using std::wostream;

/*
 * JavaTabCodeGen
 */
struct JavaTabCodeGen : public CodeGenData
{
	JavaTabCodeGen( wostream &out ) : 
		CodeGenData(out) {}

	std::wostream &TO_STATE_ACTION_SWITCH();
	std::wostream &FROM_STATE_ACTION_SWITCH();
	std::wostream &EOF_ACTION_SWITCH();
	std::wostream &ACTION_SWITCH();

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
	virtual void writeInit();
	virtual void writeExports();
	virtual void writeStart();
	virtual void writeFirstFinal();
	virtual void writeError();
	virtual void finishRagelDef();

	void NEXT( wostream &ret, int nextDest, bool inFinish );
	void NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );

	int TO_STATE_ACTION( RedStateAp *state );
	int FROM_STATE_ACTION( RedStateAp *state );
	int EOF_ACTION( RedStateAp *state );
	int TRANS_ACTION( RedTransAp *trans );

	/* Determine if we should use indicies. */
	void calcIndexSize();

private:
	wstring array_type;
	wstring array_name;
	int item_count;
	int div_count;

public:

	virtual wstring NULL_ITEM();
	virtual wostream &OPEN_ARRAY( wstring type, wstring name );
	virtual wostream &ARRAY_ITEM( wstring item, bool last );
	virtual wostream &CLOSE_ARRAY();
	virtual wostream &STATIC_VAR( wstring type, wstring name );
	virtual wstring ARR_OFF( wstring ptr, wstring offset );
	virtual wstring CAST( wstring type );
	virtual wstring GET_KEY();
	virtual wstring CTRL_FLOW();

	wstring FSM_NAME();
	wstring START_STATE_ID();
	wostream &ACTIONS_ARRAY();
	wstring GET_WIDE_KEY();
	wstring GET_WIDE_KEY( RedStateAp *state );
	wstring TABS( int level );
	wstring KEY( Key key );
	wstring INT( int i );
	void ACTION( wostream &ret, GenAction *action, int targState, bool inFinish );
	void CONDITION( wostream &ret, GenAction *condition );
	wstring ALPH_TYPE();
	wstring WIDE_ALPH_TYPE();
	wstring ARRAY_TYPE( unsigned long maxVal );

	wstring ACCESS();

	wstring P();
	wstring PE();
	wstring vEOF();

	wstring vCS();
	wstring STACK();
	wstring TOP();
	wstring TOKSTART();
	wstring TOKEND();
	wstring ACT();
	wstring DATA();

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

	void INLINE_LIST( wostream &ret, GenInlineList *inlineList, int targState, bool inFinish );
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

	wstring ERROR_STATE();
	wstring FIRST_FINAL_STATE();

	wostream &source_warning(const InputLoc &loc);
	wostream &source_error(const InputLoc &loc);

	unsigned int arrayTypeSize( unsigned long maxVal );

	bool outLabelUsed;
	bool againLabelUsed;
	bool useIndicies;

	void genLineDirective( wostream &out );
};

#endif
