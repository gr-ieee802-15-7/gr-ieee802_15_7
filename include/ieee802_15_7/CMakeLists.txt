# Copyright 2011,2012 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.
include(GrPython)

########################################################################
# Install public header files
########################################################################
list(APPEND include_sources
	api.h

	utils.h

	generate_MAC.h
	encode_PSDU.h

	generate_PHR.h
	generate_preamble.h

	rll_enc.h
	rll_dec.h

	line_enc.h
	line_dec.h
	
	detect_FLP.h
	decode_PHR.h

	decode_PSDU.h
)

install(
	FILES ${include_sources}
	DESTINATION ${GR_INCLUDE_DIR}/gnuradio/ieee802_15_7
	COMPONENT "ieee802_15_7_devel"
)