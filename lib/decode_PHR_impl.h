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

#ifndef INCLUDED_IEEE802_15_7_decode_PHR_IMPL_H
#define INCLUDED_IEEE802_15_7_decode_PHR_IMPL_H
#include <list>

#include <gnuradio/filter/fir_filter.h>

#include <ieee802_15_7/decode_PHR.h>
#include <ieee802_15_7/utils.h>

using namespace std;

namespace gr {
namespace ieee802_15_7 {


class decode_PHR_impl : public decode_PHR
{

public:
	decode_PHR_impl(OP_MODE op_mode, bool debug);
	~decode_PHR_impl();

	int general_work (int noutput, gr_vector_int& ninput_items,
			gr_vector_const_void_star& input_items,
			gr_vector_void_star& output_items);

	void forecast (int noutput_items, gr_vector_int &ninput_items_required);
	bool search_frame_start();

private:
	enum {SEARCH, SYNC, COPY, PARSE, PASS, RESET} d_state;
	int		d_offset;
	int		d_frame_start;
	float *d_correlation;
	int d_to_copy;
	int d_i;
	OP_MODE d_op_mode;
	phy_param phy;
	tx_param tx;

	char *line_data;
	
	list<pair<double, int> > d_cor;
	vector<gr::tag_t> d_tags;
	gr::filter::kernel::fir_filter_fff *d_fir;

	const bool d_debug;
	struct timeval tv_start;
	long us_start;
	bool exceed_time;
	int nbytes;

protected:
	static const vector<float> TDP;
};

} /* namespace ieee802_15_7 */
} /* namespace gr */

#endif /* INCLUDED_IEEE802_15_7_decode_PHR_IMPL_H */
