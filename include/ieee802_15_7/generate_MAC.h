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
 
#ifndef INCLUDED_IEEE802_15_7_GENERATE_MAC_H
#define INCLUDED_IEEE802_15_7_GENERATE_MAC_H

#include <ieee802_15_7/api.h>
#include <gnuradio/block.h>

using namespace std;

namespace gr {
  namespace ieee802_15_7 {

    class IEEE802_15_7_API generate_MAC : virtual public block
    {
     public:
      typedef boost::shared_ptr<generate_MAC> sptr;

      static sptr make(
       vector<uint8_t> dst_vpan_id, vector<uint8_t> dst_addr,
       vector<uint8_t> src_vpan_id, vector<uint8_t> src_addr
      );
    };

  } // namespace ieee802_15_7
} // namespace gr

#endif /* INCLUDED_IEEE802_15_7_GENERATE_MAC_H */

