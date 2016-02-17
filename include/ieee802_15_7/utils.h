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


#ifndef INCLUDED_IEEE802_15_7_UTILS_H
#define INCLUDED_IEEE802_15_7_UTILS_H

#include <ieee802_15_7/api.h>
#include <gnuradio/config.h>

#include <iostream>
#include <cinttypes>

#define dout d_debug && std::cout

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

enum LINE_CODE {LINE_OOK, LINE_VPPM};
enum RLL_CODE {RLL_MANCHESTER, RLL_4B6B, RLL_8B10B};
enum CONV_CODE {CC_NONE, CC_1_4, CC_1_3, CC_2_3};

const char rll4B6B[16] = {
  0b001110, 0b001101, 0b010011, 0b010110,
  0b010101, 0b100011, 0b100110, 0b100101,
  0b011001, 0b011010, 0b011100, 0b110001,
  0b110010, 0b101001, 0b101010, 0b101100
};
const char rll6B4B[64] = {
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1,  1,  0, -1,
  -1, -1, -1,  2, -1,  4,  3, -1,
  -1,  8,  9, -1, 10, -1, -1, -1,
  -1, -1, -1,  5, -1,  7,  6, -1,
  -1, 13, 14, -1, 15, -1, -1, -1,
  -1, 11, 12, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1
};

const char rll5B6BRdNeg[32] = {
  0b111001, 0b101110, 0b101101, 0b100011,
  0b101011, 0b100101, 0b100110, 0b000111,
  0b100111, 0b101001, 0b101010, 0b001011,
  0b101100, 0b001101, 0b001110, 0b111010,
  0b110110, 0b110001, 0b110010, 0b010011,
  0b110100, 0b010101, 0b010110, 0b010111,
  0b110011, 0b011001, 0b011010, 0b011011,
  0b011100, 0b011101, 0b011110, 0b110101
};
const char rll6B5BRdNeg[64] = {
  -1, -1, -1, -1, -1, -1, -1,  7,
  -1, -1, -1, 11, -1, 13, 14, -1,
  -1, -1, -1, 19, -1, 21, 22, 23,
  -1, 25, 26, 27, 28, 29, 30, -1,
  -1, -1, -1,  3, -1,  5,  6,  8,
  -1,  9, 10,  4, 12,  2,  1, -1,
  -1, 17, 18, 24, 20, 31, 16, -1,
  -1,  0, 15, -1, -1, -1, -1, -1
};

const char rll3B4BRdNeg[8] = {
  0b1101, 0b1001, 0b1010, 0b0011,
  0b1011, 0b0101, 0b0110, 0b0111
  // 0b0111: should avoid five consecutive 0s or 1s
  // i.e. 0b1110
};
const char rll4B3BRdNeg[16] = {
  -1, -1, -1,  3,
  -1,  5,  6,  7,
  -1,  1,  2,  4,
  -1,  0, -1, -1
};

struct mac_header {
  uint16_t frame_control;
  uint8_t seq_nr;
  uint8_t addr1[2];
  uint8_t addr2[2];
  uint8_t addr3[2];
  uint8_t addr4[2];
  // no auxiliary security header
}__attribute__((packed));

/**
 * PHY parameters
 */
class phy_param
{
 public:
  phy_param(OP_MODE o);

  void set_op_mode(OP_MODE o);

  OP_MODE op_mode;
  LINE_CODE line_code;
  RLL_CODE rll_code;
  CONV_CODE conv_code;
  int rs_n, rs_k, rs_t2, rs_m, rs_s;
};

/**
 * packet specific parameters
 */
class tx_param
{
 public:
  tx_param(phy_param &phy, int payload_length);

  void set_param(phy_param &phy, int payload_length);

  int payload_size, outer_size, payload_size_bits,
      inner_size, padded_size,
      rll_size, line_size;
  int tail_size;
};

char get_bit(long b, int i);
int n_ones(int n);

void unpack_bytes(const char *payload, char *payload_bits, tx_param &tx);
void pack_bits(const char *in_bits, char *out_bytes, tx_param &tx);

#define ARGS1 const char *input, char *out, tx_param &tx, phy_param &phy
#define ARGS2       char *input, char *out, tx_param &tx, phy_param &phy

void outer_encode(ARGS1);
int outer_decode(ARGS1);
void interleave(ARGS1);
void deinterleave(ARGS1);
void inner_encode(ARGS1);
void inner_decode(ARGS1);
void line_decode(ARGS2);
void rll_decode(ARGS2);

OP_MODE fallback_mcs(OP_MODE o);

#endif /* INCLUDED_IEEE802_15_7_UTILS_H */

