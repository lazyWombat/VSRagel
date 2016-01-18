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

#ifndef _GOTABLISH_H
#define _GOTABLISH_H

#include "gocodegen.h"

class GoTablishCodeGen
    : public GoCodeGen
{
public:
    GoTablishCodeGen( wostream &out )
        : GoCodeGen(out) {}
protected:
    virtual void GOTO( wostream &ret, int gotoDest, bool inFinish );
    virtual void CALL( wostream &ret, int callDest, int targState, bool inFinish );
    virtual void NEXT( wostream &ret, int nextDest, bool inFinish );
    virtual void GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );
    virtual void NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish );
    virtual void CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish );
    virtual void CURS( wostream &ret, bool inFinish );
    virtual void TARGS( wostream &ret, bool inFinish, int targState );
    virtual void RET( wostream &ret, bool inFinish );
    virtual void BREAK( wostream &ret, int targState, bool csForced );
};

#endif
