# -*- coding: utf-8 -*-
#
# Copyright (C) 1998 Dinu C. Gherman <gherman@europemail.com>
# Copyright (C) 2005-2007 Christopher Lenz <cmlenz@gmx.de>
# Copyright (C) 2007 Edgewall Software
# 
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://bitten.edgewall.org/wiki/License.
# 
# This module is based on the pycount.py script written by Dinu C.
# Gherman, and is used here under the following license:
# 
#     Permission to use, copy, modify, and distribute this software
#     and its documentation without fee and for any purpose, except
#     direct commerial advantage, is hereby granted, provided that
#     the above copyright notice appear in all copies and that both
#     that copyright notice and this  permission notice appear in
#     supporting documentation.

"""Support for counting the lines of code in Python programs."""

import re

__all__ = ['BLANK', 'CODE', 'COMMENT', 'DOC', 'count']
__docformat__ = 'restructuredtext en'

# Reg. exps. to find the end of a triple quote, given that
# we know we're in one; use the "match" method; .span()[1]
# will be the index of the character following the final
# quote.
_squote3_finder = re.compile(
    r"([^\']|"
    r"\.|"
    r"'[^\']|"
    r"'\.|"
    r"''[^\']|"
    r"''\.)*'''")

_dquote3_finder = re.compile(
    r'([^\"]|'
    r'\.|'
    r'"[^\"]|'
    r'"\.|'
    r'""[^\"]|'
    r'""\.)*"""')

# Reg. exps. to find the leftmost one-quoted string; use the
# "search" method; .span()[0] bounds the string found.
_dquote1_finder = re.compile(r'"([^"]|\.)*"')
_squote1_finder = re.compile(r"'([^']|\.)*'")

# _is_comment matches pure comment line.
_is_comment = re.compile(r"^[ \t]*#").match

# _is_blank matches empty line.
_is_blank = re.compile(r"^[ \t]*$").match

# find leftmost splat or quote.
_has_nightmare = re.compile(r"""[\"'#]""").search

# _is_doc_candidate matches lines that start with a triple quote.
_is_doc_candidate = re.compile(r"^[ \t]*('''|\"\"\")")

BLANK, CODE, COMMENT, DOC  = 0, 1, 2, 3

def count(source):
    """Parse the given file-like object as Python source code.
    
    For every line in the code, this function yields a ``(lineno, type, line)``
    tuple, where ``lineno`` is the line number (starting at 0), ``type`` is
    one of `BLANK`, `CODE`, `COMMENT` or `DOC`, and ``line`` is the actual
    content of the line.
    
    :param source: a file-like object containing Python code
    """

    quote3_finder = {'"': _dquote3_finder, "'": _squote3_finder}
    quote1_finder = {'"': _dquote1_finder, "'": _squote1_finder }

    in_doc = False
    in_triple_quote = None

    for lineno, line in enumerate(source):
        classified = False

        if in_triple_quote:
            if in_doc:
                yield lineno, DOC, line
            else:
                yield lineno, CODE, line
            classified = True
            m = in_triple_quote.match(line)
            if m == None:
                continue
            # Get rid of everything through the end of the triple.
            end = m.span()[1]
            line = line[end:]
            in_doc = in_triple_quote = False

        if _is_blank(line):
            if not classified:
                yield lineno, BLANK, line
            continue

        if _is_comment(line):
            if not classified:
                yield lineno, COMMENT, line
            continue

        # Now we have a code line, a doc start line, or crap left
        # over following the close of a multi-line triple quote; in
        # (& only in) the last case, classified==1.
        if not classified:
            if _is_doc_candidate.match(line):
                yield lineno, DOC, line
                in_doc = True
            else:
                yield lineno, CODE, line

        # The only reason to continue parsing is to make sure the
        # start of a multi-line triple quote isn't missed.
        while True:
            m = _has_nightmare(line)
            if not m:
                break
            else:
                i = m.span()[0]

            ch = line[i]    # splat or quote
            if ch == '#':
                # Chop off comment; and there are no quotes
                # remaining because splat was leftmost.
                break
            # A quote is leftmost.
            elif ch * 3 == line[i:i + 3]:
                # at the start of a triple quote
                in_triple_quote = quote3_finder[ch]
                m = in_triple_quote.match(line, i + 3)
                if m:
                    # Remove the string & continue.
                    end = m.span()[1]
                    line = line[:i] + line[end:]
                    in_doc = in_triple_quote = False
                else:
                    # Triple quote doesn't end on this line.
                    break
            else:
                # At a single quote; remove the string & continue.
                prev_line = line[:]
                line = re.sub(quote1_finder[ch], ' ', line, 1)
                # No more change detected, so be quiet or give up.
                if prev_line == line:
                    # Let's be quiet and hope only one line is affected.
                    line = ''
