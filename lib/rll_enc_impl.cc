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

#include "rll_enc_impl.h"
#include <ieee802_15_7/utils.h>
#include <gnuradio/tag_checker.h>

using namespace std;
using namespace gr::ieee802_15_7;

rll_enc::sptr
rll_enc::make()
{
	return gnuradio::get_initial_sptr(new rll_enc_impl());
}

rll_enc_impl::rll_enc_impl()
	: tagged_stream_block("RLL encoder",
			   io_signature::make(1, 1, sizeof(char)),
			   io_signature::make(1, 1, sizeof(char)),
			   "packet_len") {
	set_tag_propagation_policy(TPP_DONT);
}

rll_enc_impl::~rll_enc_impl() { }


int
rll_enc_impl::work(int noutput_items,
		gr_vector_int &ninput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items) {

	const unsigned long nread = nitems_read(0);
	const int ninput = ninput_items[0];
	const unsigned char *in = (unsigned char*)input_items[0];
	const int noutput = noutput_items;
	char *out = (char*)output_items[0];

	vector<tag_t> tags;
	get_tags_in_range(tags, 0, nread,
			nread + ninput,
			pmt::mp("op_mode"));
	if(!tags.size()) {
		throw runtime_error("no operating mode tag in input stream");
	}

	OP_MODE op_mode = (OP_MODE)pmt::to_long(tags[0].value);
	add_item_tag(0, nitems_written(0), pmt::mp("op_mode"),
		pmt::from_long(op_mode));

	phy_param phy(op_mode);

	int i = 0;
	int o = 0;

	switch (phy.rll_code) {
		case RLL_MANCHESTER:
			while (i < ninput && o + 1 < noutput) {
				int bit = in[i++];

				out[o++] = !bit;
				out[o++] = !!bit;
			}

			return o;

		case RLL_4B6B:
			while (i + 3 < ninput && o + 5 < noutput) {
				int bits = 0;
				for (int j = 0; j < 4; j++) {
					bits |= !!in[i++] << j;
				}

				for (int j = 0; j < 6; j++) {
					out[o++] = !!(rll4B6B[bits] & (1 << j));
				}
			}

			return o;

		case RLL_8B10B: {
			int RD = -1;
			while (i + 7 < ninput && o + 9 < noutput) {
				int bits = 0;
				for (int j = 0; j < 5; j++) {
					bits |= !!in[i++] << j;
				}
				int rll6B = rll5B6BRdNeg[bits];
				if (n_ones(rll6B) != 3 && RD == 1) {
					rll6B = ~rll6B & 0b111111;
				} else if (n_ones(rll6B & 0b111) % 3 == 0 && RD == 1) {
					rll6B = ~rll6B & 0b111111;
				}

				for (int j = 0; j < 6; j++) {
					out[o++] = !!(rll6B & (1 << j));
				}

				if (2 * n_ones(rll6B) - 6) { // codeword disparity
					RD *= -1;
				}

				bits = 0;
				for (int j = 0; j < 3; j++) {
					bits |= !!in[i++] << j;
				}
				int rll4B = rll3B4BRdNeg[bits];
				if (n_ones(rll4B) != 2) {
					if (RD == 1) {
						rll4B = ~rll4B & 0b1111;
					}
					int run = n_ones(rll6B & 0b111) + n_ones(rll4B & 0b1100);
					if (run % 5 == 0) {
						rll4B = ((rll4B & 1) << 3) | ((rll4B & 0b1110) >> 1);
					}
				} else if (n_ones(rll4B & 0b11) % 2 == 0 && RD == 1) {
					rll4B = ~rll4B & 0b1111;
				}

				for (int j = 0; j < 4; j++) {
					out[o++] = !!(rll4B & (1 << j));
				}

				if (2 * n_ones(rll4B) - 4) {
					RD *= -1;
				}
			}

			return o;
		}
	}
}