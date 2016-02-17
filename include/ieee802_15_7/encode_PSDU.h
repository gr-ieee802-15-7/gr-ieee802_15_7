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
 
#ifndef INCLUDED_IEEE802_15_7_ENCODE_PSDU_H
#define INCLUDED_IEEE802_15_7_ENCODE_PSDU_H

#include <ieee802_15_7/api.h>
#include <gnuradio/block.h>
//#include <ieee802_15_7/utils.h>

#ifndef OP_MODE_H
#define OP_MODE_H
enum OP_MODE {
  // PHY I
  // optical clock rate: 200 kHz
  OOK_MANCHESTER_RS_15_7_CC_1_4 = 0,
  OOK_MANCHESTER_RS_15_11_CC_1_3 = 1,
  OOK_MANCHESTER_RS_15_11_CC_2_3 = 2,
  OOK_MANCHESTER_RS_15_11_CC_NONE = 3,
  OOK_MANCHESTER_RS_NONE_CC_NONE = 4,

  // optical clock rate: 400 kHz
  VPPM_4B6B_RS_15_2_CC_NONE = 5,
  VPPM_4B6B_RS_15_4_CC_NONE = 6,
  VPPM_4B6B_RS_15_7_CC_NONE = 7,
  VPPM_4B6B_RS_NONE_CC_NONE = 8,

  // PHY II
  VPPM_4B6B_3_75_RS_64_32 = 9,
  VPPM_4B6B_3_75_RS_160_128 = 10,
  VPPM_4B6B_7_5_RS_64_32 = 11,
  VPPM_4B6B_7_5_RS_160_128 = 12,
  VPPM_4B6B_7_5_RS_NONE = 13,

  OOK_8B10B_15_RS_64_32 = 14,
  OOK_8B10B_15_RS_160_128 = 15,
  OOK_8B10B_30_RS_64_32= 16,
  OOK_8B10B_30_RS_160_128 = 17,
  OOK_8B10B_60_RS_64_32 = 18,
  OOK_8B10B_60_RS_160_128 = 19,
  OOK_8B10B_120_RS_64_32 = 20,
  OOK_8B10B_120_RS_160_128 = 21,
  OOK_8B10B_120_RS_NONE = 22
};
#endif

namespace gr {
  namespace ieee802_15_7 {

    class IEEE802_15_7_API encode_PSDU : virtual public block
    {
     public:
      typedef boost::shared_ptr<encode_PSDU> sptr;

      static sptr make(OP_MODE op_mode, bool debug = false);
      virtual void set_op_mode(OP_MODE op_mode) = 0;
    };

  } // namespace ieee802_15_7
} // namespace gr

#endif /* INCLUDED_IEEE802_15_7_ENCODE_PSDU_H */

