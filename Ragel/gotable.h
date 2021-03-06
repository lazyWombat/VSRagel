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

#ifndef _GOTABLE_H
#define _GOTABLE_H

#include <iostream>
#include "gotablish.h"

class GoTabCodeGen
    : public GoTablishCodeGen
{
public:
	GoTabCodeGen( wostream &out )
        : GoTablishCodeGen(out) {}

    virtual ~GoTabCodeGen() { }
    virtual void writeData();
    virtual void writeExec();

protected:
    std::wostream &TO_STATE_ACTION_SWITCH( int level );
    std::wostream &FROM_STATE_ACTION_SWITCH( int level );
    std::wostream &EOF_ACTION_SWITCH( int level );
    std::wostream &ACTION_SWITCH( int level );

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
    void LOCATE_TRANS();

    void COND_TRANSLATE();

    virtual std::wostream &TO_STATE_ACTION( RedStateAp *state );
    virtual std::wostream &FROM_STATE_ACTION( RedStateAp *state );
    virtual std::wostream &EOF_ACTION( RedStateAp *state );
    virtual std::wostream &TRANS_ACTION( RedTransAp *trans );
    virtual void calcIndexSize();
};

#endif
