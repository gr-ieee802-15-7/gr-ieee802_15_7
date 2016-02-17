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

#include "line_dec_impl.h"
#include <ieee802_15_7/utils.h>
#include <gnuradio/tag_checker.h>

#define DECISION(X) ((X) > 0 ? 1 : 0)

using namespace std;
using namespace gr::ieee802_15_7;

line_dec::sptr
line_dec::make()
{
	return gnuradio::get_initial_sptr(new line_dec_impl());
}

line_dec_impl::line_dec_impl()
	: tagged_stream_block("line decoder",
			   io_signature::make(1, 1, sizeof(char)),
			   io_signature::make(1, 1, sizeof(char)),
				"packet_len") {
	set_tag_propagation_policy(TPP_DONT);
}

line_dec_impl::~line_dec_impl() { }


int
line_dec_impl::work(int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items) {

	const unsigned long nread = nitems_read(0);
	const int ninput = ninput_items[0];
	const char *in = (char*)input_items[0];
	char *out = (char*)output_items[0];

	vector<tag_t> tags;
	get_tags_in_range(tags, 0, nitems_read(0),
			nitems_read(0) + ninput_items[0]);
	if(!tags.size()) {
		throw runtime_error("no tags in input stream");
	}

	get_tags_in_range(tags, 0, nread,
					  nread + ninput,
					  pmt::mp("op_mode"));
	if(!tags.size()) {
		throw runtime_error("no operating mode tag in input stream");
	}
	OP_MODE op_mode = (OP_MODE)pmt::to_long(tags[0].value);

	get_tags_in_range(tags, 0, nread,
					  nread + ninput,
					  pmt::mp("psdu_len"));
	if(!tags.size()) {
		throw runtime_error("no psdu length tag in input stream");
	}
	int psdu_len = (int)pmt::to_long(tags[0].value);

	add_item_tag(0, nitems_written(0), pmt::mp("op_mode"),
				 pmt::from_long(op_mode));
	add_item_tag(0, nitems_written(0), pmt::mp("psdu_len"),
				 pmt::from_long(psdu_len));

	phy_param phy(op_mode);
	tx_param tx(phy, psdu_len);

	char *encoded = (char *)calloc((size_t)tx.line_size, (sizeof(char)));
	char *decoded = (char *)calloc((size_t)tx.rll_size, (sizeof(char)));

	memcpy(encoded, in, (size_t)tx.line_size);
	line_decode(encoded, decoded, tx, phy);
	memcpy(out, decoded, (size_t)tx.rll_size);

	return tx.rll_size;
}