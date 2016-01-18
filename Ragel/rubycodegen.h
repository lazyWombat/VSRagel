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

#ifndef _RUBY_CODEGEN_H
#define _RUBY_CODEGEN_H

#include "common.h"
#include "gendata.h"

/* Integer array line length. */
#define IALL 8


class RubyCodeGen : public CodeGenData
{
public:
   RubyCodeGen( wostream &out ) : CodeGenData(out) { }
   virtual ~RubyCodeGen() {}
protected:
	wostream &START_ARRAY_LINE();
	wostream &ARRAY_ITEM( wstring item, int count, bool last );
	wostream &END_ARRAY_LINE();
  

	wstring FSM_NAME();

        wstring START_STATE_ID();
	wstring ERROR_STATE();
	wstring FIRST_FINAL_STATE();
        void INLINE_LIST(wostream &ret, GenInlineList *inlineList, int targState, bool inFinish);
        wstring ACCESS();

        void ACTION( wostream &ret, GenAction *action, int targState, bool inFinish );
	wstring GET_KEY();
        wstring GET_WIDE_KEY();
	wstring GET_WIDE_KEY( RedStateAp *state );
	wstring KEY( Key key );
	wstring TABS( int level );
	wstring INT( int i );
	void CONDITION( wostream &ret, GenAction *condition );
	wstring ALPH_TYPE();
	wstring WIDE_ALPH_TYPE();
  	wstring ARRAY_TYPE( unsigned long maxVal );
	wostream &ACTIONS_ARRAY();
	void STATE_IDS();


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

public:
	wstring NULL_ITEM();
	wostream &OPEN_ARRAY( wstring type, wstring name );
	wostream &CLOSE_ARRAY();
	wostream &STATIC_VAR( wstring type, wstring name );
	wstring ARR_OFF( wstring ptr, wstring offset );

	wstring P();
	wstring PE();
	wstring vEOF();

	wstring vCS();
	wstring TOP();
	wstring STACK();
	wstring ACT();
	wstring TOKSTART();
	wstring TOKEND();
	wstring DATA();


	void finishRagelDef();
	unsigned int arrayTypeSize( unsigned long maxVal );

protected:
	virtual void writeExports();
	virtual void writeInit();
	virtual void writeStart();
	virtual void writeFirstFinal();
	virtual void writeError();

	/* Determine if we should use indicies. */
	virtual void calcIndexSize();

	virtual void BREAK( wostream &ret, int targState ) = 0;
	virtual void GOTO( wostream &ret, int gotoDest, bool inFinish ) = 0;
	virtual void GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish ) = 0;
	virtual void CALL( wostream &ret, int callDest, int targState, bool inFinish ) = 0;
	virtual void CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish ) = 0;
	virtual void RET( wostream &ret, bool inFinish ) = 0;


	virtual void NEXT( wostream &ret, int nextDest, bool inFinish ) = 0;
	virtual void NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish ) = 0;

	virtual int TO_STATE_ACTION( RedStateAp *state ) = 0;
	virtual int FROM_STATE_ACTION( RedStateAp *state ) = 0;
	virtual int EOF_ACTION( RedStateAp *state ) = 0;

	virtual int TRANS_ACTION( RedTransAp *trans );

        void EXEC( wostream &ret, GenInlineItem *item, int targState, int inFinish );
	void LM_SWITCH( wostream &ret, GenInlineItem *item, int targState, int inFinish );
	void SET_ACT( wostream &ret, GenInlineItem *item );
	void INIT_TOKSTART( wostream &ret, GenInlineItem *item );
	void INIT_ACT( wostream &ret, GenInlineItem *item );
	void SET_TOKSTART( wostream &ret, GenInlineItem *item );
	void SET_TOKEND( wostream &ret, GenInlineItem *item );
	void GET_TOKEND( wostream &ret, GenInlineItem *item );
	void SUB_ACTION( wostream &ret, GenInlineItem *item, int targState, bool inFinish );

protected:
	wostream &source_warning(const InputLoc &loc);
	wostream &source_error(const InputLoc &loc);


        /* fields */
	bool outLabelUsed;
	bool againLabelUsed;
	bool useIndicies;

	void genLineDirective( wostream &out );
};

/*
 * Local Variables:
 * mode: c++
 * indent-tabs-mode: 1
 * c-file-style: L"bsd"
 * End:
 */

#endif
