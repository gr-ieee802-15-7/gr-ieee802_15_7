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
#include <boost/crc.hpp>

#include <gnuradio/io_signature.h>
#include <gnuradio/filter/fir_filter.h>
#include <gnuradio/fft/fft.h>

#include "decode_PHR_impl.h"
#include <ieee802_15_7/utils.h>

#define SAMP_PER_SYM 2
#define DECISION(X) ((X) > 0 ? 1 : -1)

using namespace std;
using namespace gr::ieee802_15_7;

decode_PHR::sptr
decode_PHR::make(OP_MODE op_mode, bool debug)
{
	return gnuradio::get_initial_sptr(new decode_PHR_impl(op_mode, debug));
}


decode_PHR_impl::decode_PHR_impl(OP_MODE op_mode, bool debug)
	: block("decode_PHR",
		io_signature::make(1, 1, sizeof(float)),
		io_signature::make(1, 1, sizeof(char))),
		d_debug(debug),
		d_offset(0),
		d_to_copy(0),
		d_i(0),
		d_state(SEARCH),
		d_op_mode(op_mode),
		phy((OP_MODE)0),
		tx(phy, 0),
		line_data(NULL) {

	set_tag_propagation_policy(block::TPP_DONT);

	d_fir = new gr::filter::kernel::fir_filter_fff(1, TDP); 
	d_correlation = gr::fft::malloc_float(480 * 2);
	gettimeofday(&tv_start, NULL);
	us_start = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
	exceed_time = false;
	nbytes = 0;
}


decode_PHR_impl::~decode_PHR_impl(){
	delete d_fir;
	gr::fft::free(d_correlation);
}


int
decode_PHR_impl::general_work (int noutput, gr_vector_int& ninput_items,
		gr_vector_const_void_star& input_items,
		gr_vector_void_star& output_items) {

	const float *in = (const float*)input_items[0];
	char *out = (char*)output_items[0];

	int ninput = ninput_items[0];

	const unsigned int nread = nitems_read(0);

	std::vector<tag_t> tags;
	get_tags_in_range(tags, 0, nread,
						nread + ninput,
						pmt::mp("frame_start"));

	int i = 0;
	int o = 0;

	switch(d_state) {
		case SEARCH: {
			int to_consume = ninput;
			if (tags.size()) {
				d_state = SYNC;
				to_consume = tags[0].offset - nread;
				to_consume = 0;
			}

			consume_each(to_consume);
			return 0;
		}

		case SYNC: {
			int len_cor = min(120 * SAMP_PER_SYM, max(ninput - (60 * SAMP_PER_SYM - 1), 0));
			d_fir->filterN(d_correlation, in, len_cor);

			int to_consume;
			while(i < len_cor) {
				d_cor.push_back(pair<double, int>(d_correlation[i++], d_offset++));
			}

			to_consume = max(len_cor - 60 * SAMP_PER_SYM, 0);
			if (search_frame_start()) {
				to_consume = d_frame_start;

				d_op_mode = fallback_mcs(d_op_mode);
				phy.set_op_mode(d_op_mode);
				tx.set_param(phy, (32 + 16) / 8);

				d_i = 0;
				line_data = (char *) calloc((size_t) tx.line_size, sizeof(char));
				d_to_copy = tx.line_size;

				d_state = COPY;

				dout << "TDP found at " << d_frame_start << endl;
			} else {
				dout << "no TDP found" << endl;
			}
			d_offset = 0;

			consume_each(to_consume);
			return 0;
		}

		case COPY:
			while(i < ninput) {
				if (d_to_copy--) {
					line_data[d_i++] = (char)DECISION(in[i]);
				}

				i++;
				d_offset++;

				if (!d_to_copy) {
					d_state = PARSE;

					break;
				}
			}

			consume_each(i);
			return 0;

		case PARSE: {
			char *rll_data = (char *)calloc((size_t)tx.rll_size, sizeof(char));
			char *inner_data = (char *)calloc((size_t)tx.inner_size, sizeof(char));
			char *unpacked_data = (char *)calloc((size_t)tx.payload_size_bits, sizeof(char));
			char *interleaved_data = (char *)calloc((size_t)tx.outer_size, sizeof(char));
			char *outer_data = (char *)calloc((size_t)(tx.outer_size), sizeof(char));
			char *header_data = (char *)calloc((size_t)tx.payload_size, sizeof(char));

			line_decode(line_data, rll_data, tx, phy);
			rll_decode(rll_data, inner_data, tx, phy);
			inner_decode(inner_data, unpacked_data, tx, phy);
			pack_bits(unpacked_data, interleaved_data, tx);
			deinterleave(interleaved_data, outer_data, tx, phy);
			int ret = outer_decode(outer_data, header_data, tx, phy);
			if (ret != -1) {

				char header_data_bits[48];
				for (int j = 0; j < 6; j++) {
					for (int k = 0; k <  8; k++) {
						header_data_bits[j * 8 + k] = get_bit(header_data[j], k);
					}
				}

				int j = 0, k = 0;
				int header = 0;
				int burst_mode = header_data_bits[j];
				header |= burst_mode << j++;
				int chan = 0;
				for (k = 0; k < 3; k++) {
					chan |= header_data_bits[j++] << k;
				}
				header |= chan << (j - k);
				int mcs = 0;
				for (k = 0; k < 6; k++) {
					mcs |= header_data_bits[j++] << k;
				}
				header |= mcs << (j - k);
				int psdu_len = 0;
				for (k = 0; k < 16; k++) {
					psdu_len |= header_data_bits[j++] << k;
				}
				header |= psdu_len << (j - k);
				int dimmed_OOK = header_data_bits[j++];
				header |= dimmed_OOK << (j - k);
				int reserved = 0;
				for (k = 0; k < 5; k++) {
					reserved |= header_data_bits[j++] << k;
				}
				header |= reserved << (j - k);

				uint16_t hcs_header = 0;
				for (k = 0; k < 16; k++) {
					hcs_header |= header_data_bits[j++] << k;
				}

				boost::crc_16_type result;
				result.process_bytes(&header, 4);
				uint16_t hcs = result.checksum();

				dout << "Header checksum: ";
				if (hcs_header == hcs) {
					dout << "OK" << endl;
					dout << "Burst mode: " << burst_mode;
					dout << "\tChannel: " << chan;
					dout << "\tMCS ID: " << mcs;
					dout << "\tPSDU length: " << psdu_len;
					dout << "\tDimmed OOK: " << dimmed_OOK;
					dout << "\tReserved: " << reserved << endl;

					phy.set_op_mode((OP_MODE)mcs);
					tx.set_param(phy, psdu_len);
					d_to_copy = tx.line_size;
					pmt::pmt_t srcid = pmt::string_to_symbol(name());
					add_item_tag(0, nitems_written(0),
								 pmt::string_to_symbol("packet_len"),
								 pmt::from_long(d_to_copy),
								 srcid);
					add_item_tag(0, nitems_written(0),
								 pmt::string_to_symbol("psdu_len"),
								 pmt::from_long(psdu_len),
								 srcid);
					add_item_tag(0, nitems_written(0),
								 pmt::string_to_symbol("op_mode"),
								 pmt::from_long(phy.op_mode),
								 srcid);
					d_state = PASS;
				} else {
					d_state = SEARCH;
					dout << "Failed" << endl;
				}
			} else {
				d_state = SEARCH;
			}

			free(line_data);
			line_data = NULL;
			free(rll_data);
			free(inner_data);
			free(interleaved_data);
			free(outer_data);
			free(header_data);

			consume_each(0);
			return 0;
		}

		case PASS: {
			int to_consume;

			while(i < ninput && o < noutput) {

				if (d_to_copy--){
					out[o++] = (char)DECISION(in[i]);
				}

				i++;

				if (!d_to_copy) {
					d_offset = 0;
					d_state = SEARCH;

					if (!tags.size()) {
						to_consume = i;
					} else {
						to_consume = tags[0].offset - nread;
					}
					break;
				} else {
					to_consume = i;
				}
			}

			consume_each(to_consume);
			return o;
		}

		default:
			runtime_error("bad state");

			consume_each(0);
			return 0;
	}

	return 0;
}

void
decode_PHR_impl::forecast (int noutput_items,
							gr_vector_int &ninput_items_required) {
	// in sync state we need at least a symbol to correlate
	// with the pattern
	if(d_state == SYNC) {
		ninput_items_required[0] = 240 * SAMP_PER_SYM;
	} else {
		ninput_items_required[0] = noutput_items;
	}
}


bool
decode_PHR_impl::search_frame_start() {
	d_cor.sort();
	d_cor.reverse();

	// copy list in vector for nicer access
	vector<pair<double, int> > vec(d_cor.begin(), d_cor.end());
	d_cor.clear();

	d_frame_start = -1;

	double max_cor = 0;
	for (int i = 0; i < vec.size(); i++) {
		double cor = get<0>(vec[i]);
		int pos = get<1>(vec[i]);
		if (cor > 54 * SAMP_PER_SYM && cor > max_cor) {
			d_frame_start = pos + 60 * SAMP_PER_SYM;
			max_cor = cor;
		}
	}

	return d_frame_start != -1;
}


const vector<float> decode_PHR_impl::TDP = {
	// it is in a reverse order

	// ~TDP
	+1, +1, -1, -1, -1, -1, -1, -1, -1, -1,
 	-1, -1, -1, -1, +1, +1, -1, -1, -1, -1,
 	-1, -1, +1, +1, -1, -1, +1, +1, +1, +1,
	// TDP
 	-1, -1, +1, +1, +1, +1, +1, +1, +1, +1,
 	+1, +1, +1, +1, -1, -1, +1, +1, +1, +1,
 	+1, +1, -1, -1, +1, +1, -1, -1, -1, -1,
	// ~TDP
	+1, +1, -1, -1, -1, -1, -1, -1, -1, -1,
 	-1, -1, -1, -1, +1, +1, -1, -1, -1, -1,
 	-1, -1, +1, +1, -1, -1, +1, +1, +1, +1,
	// TDP
 	-1, -1, +1, +1, +1, +1, +1, +1, +1, +1,
 	+1, +1, +1, +1, -1, -1, +1, +1, +1, +1,
 	+1, +1, -1, -1, +1, +1, -1, -1, -1, -1
};
