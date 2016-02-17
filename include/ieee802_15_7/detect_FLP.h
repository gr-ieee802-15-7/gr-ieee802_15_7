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


#ifndef INCLUDED_IEEE802_15_7_DETECT_FLP_H
#define INCLUDED_IEEE802_15_7_DETECT_FLP_H

#include <ieee802_15_7/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace ieee802_15_7 {

    class IEEE802_15_7_API detect_FLP : virtual public block
    {
     public:
      typedef boost::shared_ptr<detect_FLP> sptr;

      static sptr make(double threshold, unsigned int min_plateau, bool debug = false);
    };

  } // namespace ieee802_15_7
} // namespace gr

#endif /* INCLUDED_IEEE802_15_7_DETECT_FLP_H */

