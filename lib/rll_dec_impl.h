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

#ifndef INCLUDED_IEEE802_15_7_RLL_DEC_IMPL_H
#define INCLUDED_IEEE802_15_7_RLL_DEC_IMPL_H

#include <ieee802_15_7/rll_dec.h>
#include <ieee802_15_7/utils.h>

namespace gr {
namespace ieee802_15_7 {

class rll_dec_impl : public rll_dec
{
	public:
		rll_dec_impl();
		~rll_dec_impl();

		int work(int noutput_items,
				gr_vector_int &ninput_itmes,
				gr_vector_const_void_star &input_items,
				gr_vector_void_star &output_items);
};

} /* namespace ieee802_15_7 */
} /* namespace gr */

#endif /* INCLUDED_IEEE802_15_7_RLL_DEC_IMPL_H */
