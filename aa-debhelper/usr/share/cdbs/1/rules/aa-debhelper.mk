# -*- mode: makefile; coding: utf-8 -*-
# Copyright ©2005 Torsten Werner <twerner@debian.org>
# Description: Uses aa-debhelper to implement the binary package building stage
#  Note that we require aa-debhelper (>= 0.2).
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# 02111-1307 USA.

ifndef _cdbs_bootstrap
_cdbs_scripts_path ?= /usr/lib/cdbs
_cdbs_rules_path ?= /usr/share/cdbs/1/rules
_cdbs_class_path ?= /usr/share/cdbs/1/class
endif

ifndef _cdbs_rules_aa_debhelper
_cdbs_rules_aa_debhelper := 1

include $(_cdbs_rules_path)/debhelper.mk$(_cdbs_makefile_suffix)

CDBS_BUILD_DEPENDS	:= $(CDBS_BUILD_DEPENDS), aa-debhelper (>= 0.2)

DH_COMPAT=4

ifeq ($(DEB_VERBOSE_ALL), yes)
DH_VERBOSE = 1
endif

$(patsubst %,binary-install/%,$(DEB_PACKAGES)) :: binary-install/%:
	dh_divert -p$(cdbs_curpkg)

endif
