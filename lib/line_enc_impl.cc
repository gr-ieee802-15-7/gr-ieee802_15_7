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

#include "line_enc_impl.h"
#include <ieee802_15_7/utils.h>
#include <gnuradio/tag_checker.h>

using namespace std;
using namespace gr::ieee802_15_7;

line_enc::sptr
line_enc::make()
{
	return gnuradio::get_initial_sptr(new line_enc_impl());
}

line_enc_impl::line_enc_impl()
	: tagged_stream_block("line encoder",
			   io_signature::make(1, 1, sizeof(char)),
			   io_signature::make(1, 1, sizeof(char)),
			   "packet_len") {
	set_tag_propagation_policy(TPP_DONT);
}

line_enc_impl::~line_enc_impl() { }


int
line_enc_impl::work(int noutput_items,
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

	phy_param phy(op_mode);

	int i = 0;
	int o = 0;
	int rep = 2; // it is because of limited sampling rate of USRP
	int duty = 1; // rep = 2, duty = 1 means 50 % duty cycle

	switch (phy.line_code) {
		case LINE_OOK:
			while (i < ninput && o + rep - 1 < noutput) {
				for (int j = 0; j < rep; j++) {
					out[o++] = in[i];
				}
				i++;
			}

			return o;

		case LINE_VPPM: {
			while (i < ninput && o + rep - 1 < noutput) {
				int bit = in[i++];

				if (bit == 0) {
					for (int j = 0; j < duty; j++) {
						out[o++] = 1;
					}
					for (int j = 0; j < rep - duty; j++) {
						out[o++] = 0;
					}
				} else {
					for (int j = 0; j < rep - duty; j++) {
						out[o++] = 0;
					}
					for (int j = 0; j < duty; j++) {
						out[o++] = 1;
					}
				}
			}

			return o;
		}
	}
}