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

#include <ieee802_15_7/detect_FLP.h>
#include <gnuradio/io_signature.h>
#include <ieee802_15_7/utils.h>

#include <iostream>

#define DECISION(X) ((X) > 0 ? 1 : -1)

using namespace std;
using namespace gr::ieee802_15_7;

class detect_FLP_impl : public detect_FLP {

public:
detect_FLP_impl(double threshold, unsigned int min_plateau,
				bool debug) : block("detect_FLP",
			gr::io_signature::make2(2, 2, sizeof(float), sizeof(float)),
			gr::io_signature::make(1, 1, sizeof(float))),
			d_debug(debug),
			d_count(0),
			d_cumulative_val(0),
			d_found(false),
			d_plateau(0),
			MIN_PLATEAU(min_plateau),
			d_threshold(threshold) {

	set_tag_propagation_policy(block::TPP_DONT);
}

~detect_FLP_impl(){
}

int general_work (int noutput_items, gr_vector_int& ninput_items,
		gr_vector_const_void_star& input_items,
		gr_vector_void_star& output_items) {

	const float *in = (const float*)input_items[0];
	const float *in2 = (const float*)input_items[1];
	float *out = (float*)output_items[0];

	int noutput = noutput_items;
	int ninput = min(ninput_items[0], ninput_items[1]);


	int i = 0;
	int o = 0;
	bool tag_added = false;

	while (i < ninput && o < noutput) {
		bool reset = false;
		if (in2[i] > d_threshold) {
			if (d_plateau < MIN_PLATEAU) {
				d_cumulative_val += in[i];
				d_count++;
				d_plateau++;
				// dout << "plateau: " << d_plateau << endl;
			} else {
				d_found = true;
				d_decision_threshold = d_cumulative_val / d_count;
				if (!tag_added) {
					dout << endl;
					dout << "plateau exceed!" << endl;
					pmt::pmt_t srcid = pmt::string_to_symbol(name());
					add_item_tag(0, nitems_written(0),
									pmt::string_to_symbol("frame_start"),
									pmt::from_bool(true),
									srcid);
					tag_added = true;
				}
				reset = true;
			}
		} else {
			reset = true;
		}
		if (reset) {
			d_cumulative_val = 0;
			d_count = 0;
			d_plateau = 0;
		}
		if (d_found) {
			int bit = DECISION(in[i] - d_decision_threshold);
			if (i % 2)	dout << (int)(bit > 0);
			out[o++] = bit;
		}
		i++;
	}

	dout << endl;

	consume_each(i);
	return o;
}

private:
	int d_plateau;
	int d_count;
	float d_cumulative_val;
	float d_decision_threshold;
	bool d_found;

	const double d_threshold;
	const bool d_debug;
	const unsigned int MIN_PLATEAU;
};

detect_FLP::sptr
detect_FLP::make(double threshold, unsigned int min_plateau, bool debug) {
	return gnuradio::get_initial_sptr(new detect_FLP_impl(threshold, min_plateau, debug));
}
