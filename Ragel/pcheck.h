/*
 *  Copyright 2001, 2002 Adrian Thurston <thurston@complang.org>
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

#ifndef _PCHECK_H
#define _PCHECK_H

class ParamCheck
{
public:
	ParamCheck( const wchar_t *paramSpec, int argc, const wchar_t **argv);

	bool check();

	const wchar_t *paramArg;     /* The argument to the parameter. */
	wchar_t parameter;     /* The parameter matched. */
	enum { match, invalid, noparam } state;

	const wchar_t *argOffset;    /* If we are reading params inside an
	                     * arg this points to the offset. */

	const wchar_t *curArg;       /* Pointer to the current arg. */
	int iCurArg;        /* Index to the current arg. */

private:
	const wchar_t *paramSpec;    /* Parameter spec supplied by the coder. */
	int argc;           /* Arguement data from the command line. */
	const wchar_t **argv;
};

#endif
