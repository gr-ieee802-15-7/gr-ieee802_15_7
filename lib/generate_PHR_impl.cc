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

#include "generate_PHR_impl.h"
#include <ieee802_15_7/utils.h>
#include <stdint.h>
#include <string.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/tag_checker.h>
#include <boost/crc.hpp>

using namespace std;
using namespace gr::ieee802_15_7;


generate_PHR_impl::generate_PHR_impl(bool debug) :
	tagged_stream_block ("generate_PHR",
			gr::io_signature::make(1, 1, sizeof(char)),
			gr::io_signature::make(1, 1, sizeof(char)), "packet_len"),
			d_debug(debug) {
	set_tag_propagation_policy(TPP_DONT);
}

generate_PHR_impl::~generate_PHR_impl() {
}


int generate_PHR_impl::work(int noutput_items, gr_vector_int& ninput_items,
			gr_vector_const_void_star& input_items,
			gr_vector_void_star& output_items ) {


	unsigned char *out = (unsigned char*)output_items[0];

	vector<tag_t> tags;
	long psdu_len;
	OP_MODE header_op_mode;
	OP_MODE op_mode;
	get_tags_in_range(tags, 0, nitems_read(0),
			nitems_read(0) + ninput_items[0]);
	if(!tags.size()) {
		throw runtime_error("no tags in input stream");
	}

	for (int i = 0; i < tags.size(); i++) {
		if (pmt::symbol_to_string(tags[i].key) == "psdu_len") {
			psdu_len = pmt::to_long(tags[i].value);
		}
		if (pmt::symbol_to_string(tags[i].key) == "op_mode") {
			op_mode = (OP_MODE)pmt::to_long(tags[i].value);
		}
	}

	header_op_mode = fallback_mcs(op_mode);

	phy_param header_phy(header_op_mode);
	tx_param header_tx(header_phy, (32 + 16) / 8);
	// 32: header
	// 16: HCS

	char *outer_data = (char *) calloc((size_t)header_tx.outer_size, sizeof(char));
	char *interleaved_data = (char *) calloc((size_t)header_tx.outer_size, sizeof(char));
	char *header_data_bits = (char *) calloc((size_t)header_tx.payload_size_bits, sizeof(char));
	char *inner_data = (char *) calloc((size_t)header_tx.inner_size, sizeof(char));
	char *padded_data = (char *) calloc((size_t)header_tx.padded_size, sizeof(char));

	add_item_tag(0, nitems_written(0), pmt::mp("op_mode"),
		pmt::from_long(header_op_mode));

	//data bits of the signal header
	char header[6] = {0};

	int o = 0;

	// burst mode
	header[(o++ / 8)] |= 0 << 0;
	// channel number
	header[(o++ / 8)] |= 1 << 1;
	header[(o++ / 8)] |= 1 << 2;
	header[(o++ / 8)] |= 1 << 3;
	// MCS ID
	for (int i = 0; i < 6; i++) {
		header[(o++ / 8)] |= get_bit(op_mode,  i) << ((i + 4) % 8);
	}
	// PSDU length
	for (int i = 0; i < 16; i += 4) {
		header[(o++ / 8)] |= get_bit(psdu_len,  i    ) << ((i + 10) % 8);
		header[(o++ / 8)] |= get_bit(psdu_len,  i + 1) << ((i + 11) % 8);
		header[(o++ / 8)] |= get_bit(psdu_len,  i + 2) << ((i + 12) % 8);
		header[(o++ / 8)] |= get_bit(psdu_len,  i + 3) << ((i + 13) % 8);
	}
	// dimmed OOK
	header[(o++ / 8)] |= 0 << (26 % 8);
	// reserved
	for (int i = 27; i < 32; i++) {
		header[(o++ / 8)] |= 0 << (i % 8);
	}

	boost::crc_16_type result;
	result.process_bytes(&header, 4);
	uint16_t hcs = result.checksum();
	for (int i = 0; i < 16; i++) {
		header[(o++ / 8)] |= get_bit(hcs, i) << (i % 8);
	}

	outer_encode(header, outer_data, header_tx, header_phy);
	interleave(outer_data, interleaved_data, header_tx, header_phy);
	unpack_bytes(interleaved_data, header_data_bits, header_tx);
	inner_encode(header_data_bits, inner_data, header_tx, header_phy);
	memcpy(padded_data, inner_data, (size_t)header_tx.inner_size);
	memcpy(out, padded_data, (size_t)header_tx.padded_size);


	free(header_data_bits);
	free(outer_data);
	free(interleaved_data);
	free(inner_data);
	free(padded_data);

	return header_tx.padded_size;
}


generate_PHR::sptr
generate_PHR::make(bool debug) {
	return gnuradio::get_initial_sptr(new generate_PHR_impl(debug));
}