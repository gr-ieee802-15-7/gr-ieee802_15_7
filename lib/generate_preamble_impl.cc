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

#include "generate_preamble_impl.h"
#include <ieee802_15_7/utils.h>
#include <gnuradio/io_signature.h>

using namespace gr::ieee802_15_7;


generate_preamble_impl::generate_preamble_impl(bool debug) :
	tagged_stream_block ("generate_preamble",
			gr::io_signature::make(1, 1, sizeof(char)),
			gr::io_signature::make(1, 1, sizeof(char)), "packet_len"),
			d_debug(debug) {
	set_tag_propagation_policy(TPP_DONT);
}

generate_preamble_impl::~generate_preamble_impl() {
}


int generate_preamble_impl::work(int noutput_items, gr_vector_int& ninput_items,
			gr_vector_const_void_star& input_items,
			gr_vector_void_star& output_items ) {


	unsigned char *out = (unsigned char*)output_items[0];
	int o = 0;

	int rep = 2;

	std::vector<tag_t> tags;
	get_tags_in_range(tags, 0, nitems_read(0),
			nitems_read(0) + ninput_items[0],
			pmt::mp("op_mode"));
	if(tags.size() != 1) {
		throw std::runtime_error("no operating mode in input stream");
	}

	for (int i = 0; i < 128; i++) {
		for (int j = 0; j < rep; j++) {
			out[o++] = (i + 1) % 2;
		}
	}
	int TDP = 0b011111101110100; // LSB will be transmitted first
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 15; j++) {
			for (int k = 0; k < rep; k++) {
				out[o++] = get_bit(TDP, j);
			}
		}
		TDP = ~TDP;
	}

	return o;
}


generate_preamble::sptr
generate_preamble::make(bool debug) {
	return gnuradio::get_initial_sptr(new generate_preamble_impl(debug));
}