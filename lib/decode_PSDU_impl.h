/* -*- c++ -*- */
/* 
 * Copyright 2015 Seokseong Jeon <songsong@monet.postech.ac.kr>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_IEEE802_11_DECODE_PHY_IMPL_H
#define INCLUDED_IEEE802_11_DECODE_PHY_IMPL_H

#include <ieee802_15_7/decode_PSDU.h>
#include <ieee802_15_7/utils.h>

#include <stdint.h>
#include <boost/crc.hpp>
#include <gnuradio/io_signature.h>
#include <itpp/itcomm.h>
#include <iomanip>

#include <ctime>
#include <sys/time.h>

using namespace itpp;
namespace gr {
namespace ieee802_15_7 {

class decode_PSDU_impl : public decode_PSDU
{
public:
	decode_PSDU_impl(bool debug);
	~decode_PSDU_impl();

	int work (int noutput_items, gr_vector_int& ninput_items,
			gr_vector_const_void_star& input_items,
			gr_vector_void_star& output_items);	

private:
	bool d_debug;
	struct timeval tv_start;
	long us_start;
	bool exceed_time;
	int nbytes;

};

}
}

#endif /* INCLUDED_IEEE802_11_DECODE_PHY_IMPL_H */
