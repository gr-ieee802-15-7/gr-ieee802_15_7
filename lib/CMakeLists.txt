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

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND ieee802_15_7_sources
	utils.cc
	
	generate_MAC.cc
	encode_PSDU.cc

	generate_PHR_impl.cc
	generate_preamble_impl.cc

	rll_enc_impl.cc
	rll_dec_impl.cc

	line_enc_impl.cc
	line_dec_impl.cc
	
	detect_FLP_impl.cc
	decode_PHR_impl.cc

	decode_PSDU_impl.cc
)

list(APPEND ieee802_15_7_libs
	${Boost_LIBRARIES}
	${GNURADIO_ALL_LIBRARIES}
	${ITPP_LIBRARIES}
)

MESSAGE(STATUS "ieee802_15_7_libs: " ${ieee802_15_7_libs})

set(ieee802_15_7_sources "${ieee802_15_7_sources}" PARENT_SCOPE)
if(NOT ieee802_15_7_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT ieee802_15_7_sources)

add_library(gnuradio-ieee802_15_7 SHARED ${ieee802_15_7_sources})
target_link_libraries(gnuradio-ieee802_15_7 ${ieee802_15_7_libs})
set_target_properties(gnuradio-ieee802_15_7 PROPERTIES DEFINE_SYMBOL "gnuradio_ieee802_15_7_EXPORTS")

if(APPLE)
		set_target_properties(gnuradio-ieee802_15_7 PROPERTIES
				INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
		)
endif(APPLE)

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-ieee802_15_7
		LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
		ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
		RUNTIME DESTINATION bin              # .dll file
)

########################################################################
# Build and register unit test
########################################################################
# include(GrTest)

# include_directories(${CPPUNIT_INCLUDE_DIRS})

# add_executable(test-ieee802_15_7 ${test_ieee802_15_7_sources})

# target_link_libraries(
#   test-ieee802_15_7
#   ${GNURADIO_RUNTIME_LIBRARIES}
#   ${Boost_LIBRARIES}
#   ${CPPUNIT_LIBRARIES}
#   gnuradio-ieee802_15_7
# )

# GR_ADD_TEST(test_ieee802_15_7 test-ieee802_15_7)
