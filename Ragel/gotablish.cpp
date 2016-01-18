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

#include "ragel.h"
#include "gotablish.h"

using std::endl;

void GoTablishCodeGen::GOTO( wostream &ret, int gotoDest, bool inFinish )
{
    ret << vCS() << L" = " << gotoDest << endl <<
            L"goto _again" << endl;
}

void GoTablishCodeGen::GOTO_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
    ret << vCS() << L" = (";
    INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
    ret << L")" << endl << L"goto _again" << endl;
}

void GoTablishCodeGen::CURS( wostream &ret, bool inFinish )
{
    ret << L"(_ps)";
}

void GoTablishCodeGen::TARGS( wostream &ret, bool inFinish, int targState )
{
    ret << L"(" << vCS() << L")";
}

void GoTablishCodeGen::NEXT( wostream &ret, int nextDest, bool inFinish )
{
    ret << vCS() << L" = " << nextDest << endl;
}

void GoTablishCodeGen::NEXT_EXPR( wostream &ret, GenInlineItem *ilItem, bool inFinish )
{
    ret << vCS() << L" = (";
    INLINE_LIST( ret, ilItem->children, 0, inFinish, false );
    ret << L")" << endl;
}

void GoTablishCodeGen::CALL( wostream &ret, int callDest, int targState, bool inFinish )
{
    if ( prePushExpr != 0 ) {
        ret << L"{ ";
        INLINE_LIST( ret, prePushExpr, 0, false, false );
    }

    ret << STACK() << L"[" << TOP() << L"] = " << vCS() << L"; " << TOP() << L"++; " <<
            vCS() << L" = " << callDest << L"; " << L"goto _again" << endl;

    if ( prePushExpr != 0 )
        ret << L" }";
}

void GoTablishCodeGen::CALL_EXPR( wostream &ret, GenInlineItem *ilItem, int targState, bool inFinish )
{
    if ( prePushExpr != 0 ) {
        ret << L"{";
        INLINE_LIST( ret, prePushExpr, 0, false, false );
    }

    ret << STACK() << L"[" << TOP() << L"] = " << vCS() << L"; " << TOP() << L"++; " << vCS() << L" = (";
    INLINE_LIST( ret, ilItem->children, targState, inFinish, false );
    ret << L"); " << L"goto _again" << endl;

    if ( prePushExpr != 0 )
        ret << L"}";
}

void GoTablishCodeGen::RET( wostream &ret, bool inFinish )
{
    ret << TOP() << L"--; " << vCS() << L" = " << STACK() << L"[" <<
            TOP() << L"]" << endl;

    if ( postPopExpr != 0 ) {
        ret << L"{ ";
        INLINE_LIST( ret, postPopExpr, 0, false, false );
        ret << L" }" << endl;
    }

    ret << L"goto _again" << endl;
}

void GoTablishCodeGen::BREAK( wostream &ret, int targState, bool csForced )
{
    outLabelUsed = true;
    ret << P() << L"++; " << L"goto _out" << endl;
}
