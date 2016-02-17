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

#ifndef INCLUDED_IEEE802_11_generate_PHR_IMPL_H
#define INCLUDED_IEEE802_11_generate_PHR_IMPL_H

#include <ieee802_15_7/generate_PHR.h>

namespace gr {
namespace ieee802_15_7 {

class generate_PHR_impl : public generate_PHR
{
public:
	generate_PHR_impl(bool debug);
	~generate_PHR_impl();

	int work(int noutput_items, gr_vector_int& ninput_items,
				gr_vector_const_void_star& input_items,
				gr_vector_void_star& output_items );

private:
	bool d_debug;
	gr::thread::mutex d_mutex;
};

}
}

#endif /* INCLUDED_IEEE802_11_generate_PHR_IMPL_H */