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

#include <cassert>
#include <cstring>
// #include <math.h>
#include <itpp/itcomm.h>
#include <ezpwd/rs>

#include <ieee802_15_7/utils.h>

#define ITPPCC 1

using namespace std;
using namespace itpp;
using namespace ezpwd;


phy_param::phy_param(OP_MODE o) {
	set_op_mode(o);
}


void phy_param::set_op_mode(OP_MODE o) {
	op_mode = o;

	line_code = LINE_OOK;
	rll_code = RLL_8B10B;
	rs_n = 0;
	rs_k = 0;
	conv_code = CC_NONE;

	switch(op_mode) {
		case OOK_MANCHESTER_RS_15_7_CC_1_4:
			rll_code = RLL_MANCHESTER;
			rs_n = 15;
			rs_k = 7;
			conv_code = CC_1_4;
			break;
		case OOK_MANCHESTER_RS_15_11_CC_1_3:
			rll_code = RLL_MANCHESTER;
			rs_n = 15;
			rs_k = 11;
			conv_code = CC_1_3;
			break;
		case OOK_MANCHESTER_RS_15_11_CC_2_3:
			rll_code = RLL_MANCHESTER;
			rs_n = 15;
			rs_k = 11;
			conv_code = CC_2_3;
			break;
		case OOK_MANCHESTER_RS_15_11_CC_NONE:
			rll_code = RLL_MANCHESTER;
			rs_n = 15;
			rs_k = 11;
			break;
		case OOK_MANCHESTER_RS_NONE_CC_NONE:
			rll_code = RLL_MANCHESTER;
			break;
		case VPPM_4B6B_RS_15_2_CC_NONE:
			line_code = LINE_VPPM;
			rll_code = RLL_4B6B;
			rs_n = 15;
			rs_k = 2;
			break;
		case VPPM_4B6B_RS_15_4_CC_NONE:
			line_code = LINE_VPPM;
			rll_code = RLL_4B6B;
			rs_n = 15;
			rs_k = 4;
			break;
		case VPPM_4B6B_RS_15_7_CC_NONE:
			line_code = LINE_VPPM;
			rll_code = RLL_4B6B;
			rs_n = 15;
			rs_k = 7;
			break;
		case VPPM_4B6B_RS_NONE_CC_NONE:
			line_code = LINE_VPPM;
			rll_code = RLL_4B6B;
			break;

		case VPPM_4B6B_3_75_RS_64_32:
		case VPPM_4B6B_7_5_RS_64_32:
			line_code = LINE_VPPM;
			rll_code = RLL_4B6B;
			rs_n = 64;
			rs_k = 32;
			break;
        case VPPM_4B6B_7_5_RS_NONE:
            line_code = LINE_VPPM;
            rll_code = RLL_4B6B;
            break;

		case OOK_8B10B_15_RS_64_32:
		case OOK_8B10B_30_RS_64_32:
		case OOK_8B10B_60_RS_64_32:
		case OOK_8B10B_120_RS_64_32:
			rs_n = 64;
			rs_k = 32;
			break;

		case VPPM_4B6B_3_75_RS_160_128:
		case VPPM_4B6B_7_5_RS_160_128:
			line_code = LINE_VPPM;
			rll_code = RLL_4B6B;
			rs_n = 160;
			rs_k = 128;
			break;

		case OOK_8B10B_15_RS_160_128:
		case OOK_8B10B_30_RS_160_128:
		case OOK_8B10B_60_RS_160_128:
		case OOK_8B10B_120_RS_160_128:
			rs_n = 160;
			rs_k = 128;
			break;
        case OOK_8B10B_120_RS_NONE:
            break;

		default:
			throw runtime_error("phy_param::set_op_mode(): invalid op_mode");
			break;
	}
	rs_t2 = rs_n - rs_k;
	rs_m = (int)log2(rs_n + 1);

}


tx_param::tx_param(phy_param &phy, int payload_length) {
	set_param(phy, payload_length);
}


void tx_param::set_param(phy_param &phy, int payload_length) {

	payload_size = payload_length;

	if (phy.rs_n == 0) {
		phy.rs_s = 0;
		outer_size = payload_size;
	} else {
		int n_code = payload_size / phy.rs_k + (payload_size % phy.rs_k > 0);
//		phy.rs_s = (phy.rs_k - payload_size % phy.rs_k) % phy.rs_k;
		phy.rs_s = n_code * phy.rs_k - payload_size;
		outer_size = n_code * phy.rs_n - phy.rs_s;
	}

	payload_size_bits = outer_size * 8;

	tail_size = 0;
    if (phy.conv_code != CC_NONE) {
        tail_size = 6;
    }
	inner_size = payload_size_bits + tail_size;
	switch (phy.conv_code) {
		case CC_1_4:
			inner_size *= 4;
			break;
		case CC_1_3:
			inner_size *= 3;
			break;
		case CC_2_3:
			inner_size *= 3;
			if (inner_size % 2) {
				inner_size += 1;
			}
			inner_size /= 2;
			break;
        default:
            break;
	}

	padded_size = inner_size;
	if (phy.rll_code == RLL_4B6B && inner_size % 4) {
		padded_size += 4 - (inner_size % 4);
	} else if (phy.rll_code == RLL_8B10B && inner_size % 8) {
		padded_size += 8 - (inner_size % 8);
	}

    switch(phy.rll_code) {
        case RLL_MANCHESTER:
            rll_size =  padded_size * 2;
            break;
        case RLL_4B6B:
            rll_size = padded_size / 4;
            rll_size *= 6;
            break;
        case RLL_8B10B:
            rll_size = padded_size / 8;
            rll_size *= 10;
            break;
    }

	line_size = rll_size * 2;
}


char get_bit(long b, int i){
	return (char)(b & (1 << i) ? 1 : 0);
}


int n_ones(int n) {
	int sum = 0;
	for(int i = 0; i < 8; i++) {
		sum += !!(n & (1 << i));
	}
	return sum;
}

RS<15,2> rs_15_2;
RS<15,4> rs_15_4;
RS<15,7> rs_15_7;
RS<15,11> rs_15_11;
RS<255,223> rs_64_32;

void outer_encode(const char *input, char *out,
					tx_param &tx, phy_param &phy)
{
	if (phy.rs_n != 0)
	{
		int block_size = tx.payload_size + phy.rs_s;

		vector<char> uncoded_bits, coded_bits, codeword;
		uncoded_bits.assign(input, input + tx.payload_size);
		uncoded_bits.insert(uncoded_bits.end(), phy.rs_s, 0);
		coded_bits.clear();

		// It's quite dirty...
		switch (phy.rs_n) {
			case 15: {
				switch (phy.rs_k) {
					case 2: {
						RS<15,2> rs;
						for (int i = 0; i < uncoded_bits.size(); i += phy.rs_k) {
							codeword.assign(uncoded_bits.begin() + i,
											uncoded_bits.begin() + i + phy.rs_k);
							rs.encode(codeword);
							coded_bits.insert(coded_bits.end(),
											  codeword.begin(), codeword.end());
						}
						break;
					}
					case 4: {
						RS<15,4> rs;
						for (int i = 0; i < uncoded_bits.size(); i += phy.rs_k) {
							codeword.assign(uncoded_bits.begin() + i,
											uncoded_bits.begin() + i + phy.rs_k);
							rs.encode(codeword);
							coded_bits.insert(coded_bits.end(),
											  codeword.begin(), codeword.end());
						}
						break;
					}
					case 7: {
						RS<15,7> rs;
						for (int i = 0; i < uncoded_bits.size(); i += phy.rs_k) {
							codeword.assign(uncoded_bits.begin() + i,
											uncoded_bits.begin() + i + phy.rs_k);
							rs.encode(codeword);
							coded_bits.insert(coded_bits.end(),
											  codeword.begin(), codeword.end());
						}
						break;
					}
					case 11: {
						RS<15,11> rs;
						for (int i = 0; i < uncoded_bits.size(); i += phy.rs_k) {
							codeword.assign(uncoded_bits.begin() + i,
											uncoded_bits.begin() + i + phy.rs_k);
							rs.encode(codeword);
							coded_bits.insert(coded_bits.end(),
											  codeword.begin(), codeword.end());
						}
						break;
					}
					default:
						break;
				}
				break;
			}
			case 64: {
				RS<255,223> rs; // RS<64,32> = RS<255-191,223-191>
				vector<char> pad(191, 0);
				for (int i = 0; i < uncoded_bits.size(); i += phy.rs_k) {
					codeword.assign(uncoded_bits.begin() + i,
									uncoded_bits.begin() + i + phy.rs_k);
					codeword.insert(codeword.end(), pad.begin(), pad.end());
					rs.encode(codeword);
					coded_bits.insert(coded_bits.end(),
									  codeword.begin(), codeword.begin() + phy.rs_k);
					coded_bits.insert(coded_bits.end(),
									  codeword.begin() + 223, codeword.end());
				}
				break;
			}
			case 160: {
				RS<255,223> rs; // RS<160,128> = RS<255-95,223-95>
				vector<char> pad(95, 0);
				for (int i = 0; i < uncoded_bits.size(); i += phy.rs_k) {
					codeword.assign(uncoded_bits.begin() + i,
									uncoded_bits.begin() + i + phy.rs_k);
					codeword.insert(codeword.end(), pad.begin(), pad.end());
					rs.encode(codeword);
					coded_bits.insert(coded_bits.end(),
									  codeword.begin(), codeword.begin() + phy.rs_k);
					coded_bits.insert(coded_bits.end(),
									  codeword.begin() + 223, codeword.end());
				}
				break;
			}
			default:
				break;
		}

		int o = 0;
		int ncode = block_size / phy.rs_k;
		int p_start = phy.rs_n * (ncode - 1) + tx.payload_size % phy.rs_k;
		for (int i = 0; i < coded_bits.size(); i++) {
			if (phy.rs_s && i == p_start) {
				i += phy.rs_s;
			}
			out[o++] = coded_bits[i];
		}

	} else {
		memcpy(out, input, (size_t)tx.outer_size);
	}
}


int outer_decode(const char *input, char *out,
				  tx_param &tx, phy_param &phy)
{
	int ret = 0;
	if (phy.rs_n != 0) {
		int block_size = tx.outer_size + phy.rs_s;

		vector<char> depunctured_bits, decoded_bits, codeword;
		depunctured_bits.assign(input, input + tx.outer_size);
		depunctured_bits.insert(depunctured_bits.end() - phy.rs_t2, phy.rs_s, 0);

		decoded_bits.clear();

		// It's quite dirty...
		switch (phy.rs_n) {
			case 15: {
				switch (phy.rs_k) {
					case 2: {
						RS<15,2> rs;
						for (int i = 0; i < depunctured_bits.size(); i += phy.rs_n) {
							codeword.assign(depunctured_bits.begin() + i,
											depunctured_bits.begin() + i + phy.rs_n);
							ret = rs.decode(codeword);
							if (ret == -1) {
								return -1;
							}
							decoded_bits.insert(decoded_bits.end(),
												codeword.begin(), codeword.begin() + phy.rs_k);
						}
						break;
					}
					case 4: {
						RS<15,4> rs;
						for (int i = 0; i < depunctured_bits.size(); i += phy.rs_n) {
							codeword.assign(depunctured_bits.begin() + i,
											depunctured_bits.begin() + i + phy.rs_n);
							ret = rs.decode(codeword);
							if (ret == -1) {
								return -1;
							}
							decoded_bits.insert(decoded_bits.end(),
												codeword.begin(), codeword.begin() + phy.rs_k);
						}
						break;
					}
					case 7: {
						RS<15,7> rs;
						for (int i = 0; i < depunctured_bits.size(); i += phy.rs_n) {
							codeword.assign(depunctured_bits.begin() + i,
											depunctured_bits.begin() + i + phy.rs_n);
							ret = rs.decode(codeword);
							if (ret == -1) {
								return -1;
							}
							decoded_bits.insert(decoded_bits.end(),
												codeword.begin(), codeword.begin() + phy.rs_k);
						}
						break;
					}
					case 11: {
						RS<15,11> rs;
						for (int i = 0; i < depunctured_bits.size(); i += phy.rs_n) {
							codeword.assign(depunctured_bits.begin() + i,
											depunctured_bits.begin() + i + phy.rs_n);
							ret = rs.decode(codeword);
							if (ret == -1) {
								return -1;
							}
							decoded_bits.insert(decoded_bits.end(),
												codeword.begin(), codeword.begin() + phy.rs_k);
						}
						break;
					}
					default:
						break;
				}
				break;
			}
			case 64: {
				RS<255,223> rs; // RS<64,32> = RS<255-191,223-191>
				vector<char> pad(191, 0);
				for (int i = 0; i < depunctured_bits.size(); i += phy.rs_n) {
					codeword.assign(depunctured_bits.begin() + i,
									depunctured_bits.begin() + i + phy.rs_n);
					codeword.insert(codeword.begin() + phy.rs_k, pad.begin(), pad.end());
					ret = rs.decode(codeword);
					if (ret == -1) {
						return -1;
					}
					decoded_bits.insert(decoded_bits.end(),
										codeword.begin(), codeword.begin() + phy.rs_k);
				}
				break;
			}
			case 160: {
				RS<255,223> rs; // RS<160,128> = RS<255-95,223-95>
				vector<char> pad(95, 0);
				for (int i = 0; i < depunctured_bits.size(); i += phy.rs_n) {
					codeword.assign(depunctured_bits.begin() + i,
									depunctured_bits.begin() + i + phy.rs_n);
					codeword.insert(codeword.begin() + phy.rs_k, pad.begin(), pad.end());
					ret = rs.decode(codeword);
					if (ret == -1) {
						return -1;
					}
					decoded_bits.insert(decoded_bits.end(),
										codeword.begin(), codeword.begin() + phy.rs_k);
				}
				break;
			}
			default:
				break;
		}


		int o = 0;
		int ncode = block_size / phy.rs_n;
		int p_start = phy.rs_k * (ncode - 1) + tx.payload_size % phy.rs_k;

		for (int i = 0; i < decoded_bits.size(); i++) {
			if (phy.rs_s && i == p_start) {
				i += phy.rs_s;
			}
			out[o++] = decoded_bits[i];
		}

	} else {
		// no RS used, just copy
		memcpy(out, input, (size_t)tx.outer_size);
	}

	return ret;
}


void interleave(const char *input, char *out,
				tx_param &tx, phy_param &phy)
{
	if (phy.conv_code != CC_NONE) {
		int block_size = tx.outer_size + phy.rs_s;
		int depth = (tx.outer_size + phy.rs_s) / phy.rs_n;

		char *interleaved_punctured = (char*)calloc((size_t)tx.outer_size,
													sizeof(char));

		int i_out = 0;
		for (int i = 0; i < block_size; i++) {
			int idx = (i % depth) * phy.rs_n + (i / depth);
			if (idx < tx.outer_size) {
				interleaved_punctured[i_out++] = input[idx];
			}
		}

		memcpy(out, interleaved_punctured, (size_t)tx.outer_size);

		free(interleaved_punctured);
	} else {
		memcpy(out, input, (size_t)tx.outer_size);
	}
}


void deinterleave(const char *input, char *out,
				  tx_param &tx, phy_param &phy)
{
	if (phy.conv_code != CC_NONE) {
		int block_size = tx.outer_size + phy.rs_s;
		int depth = (tx.outer_size + phy.rs_s) / phy.rs_n;

		char *depunctured = (char*)calloc((size_t)block_size, sizeof(char));
		char *deinterleaved_depunctured = (char*)calloc((size_t)block_size, sizeof(char));

		int t = 0;
		int i = 0;
		int i_out = 0;
		for (i = 0; i < tx.outer_size; i++) {
			int zidx = (phy.rs_n - (block_size - tx.outer_size) + 1)
					   * depth + t * depth - 1;
			if (i_out == zidx) {
				i_out++;
				t++;
			}
			depunctured[i_out++] = input[i];
		}

		int idx;
		for (i = 0; i < block_size; i++) {
			idx = (i % phy.rs_n) * depth + i / phy.rs_n;
			deinterleaved_depunctured[i] = depunctured[idx];
		}
		memcpy(out, deinterleaved_depunctured, (size_t)tx.outer_size);

		free(depunctured);
		free(deinterleaved_depunctured);
	} else {
		memcpy(out, input, (size_t)tx.outer_size);
	}
}


void unpack_bytes(const char *in_bytes, char *out_bits, tx_param &tx) {
	int o = 0;
	for(int i = 0; i < tx.outer_size; i++) {
		for(int b = 0; b < 8; b++) {
			out_bits[o++] = !!(in_bytes[i] & (1 << b));
		}
	}
}


void pack_bits(const char *in_bits, char *out_bytes, tx_param &tx) {
	for (int o = 0; o < tx.outer_size; o++) {
		char byte = 0;
		for (int i = 0; i < 8; i++) {
			byte |= (in_bits[o * 8 + i] << i);
		}
		out_bytes[o] = byte;
	}
}


void inner_encode(const char *input, char *out,
				  tx_param &tx, phy_param &phy)
{
	if (phy.conv_code != CC_NONE) {
		int o = 0;
#if ITPPCC == 1
		Punctured_Convolutional_Code code;
		ivec generator(3);
		generator(0)= 0133;
		generator(1) = 0171;
		generator(2) = 0165;
		code.set_generator_polynomials(generator, 7);
		bmat puncture_matrix;

		bvec bits, encoded_bits;
		bits.set_size(tx.payload_size_bits);
		for (int i = 0; i < tx.payload_size_bits; i++) {
			bits[i] = input[i];
		}

		int rep = 1;
		switch (phy.conv_code) {
			case CC_1_4:
				rep = 2;
				puncture_matrix = "1 1;1 1;0 0";
				break;
			case CC_1_3:
				puncture_matrix = "1 1;1 1;1 1";
				break;
			case CC_2_3:
				puncture_matrix = "1 0;1 1;0 0";
				break;
			default:
				assert(false);
				break;
		}
		code.set_puncture_matrix(puncture_matrix);
		code.reset();
		code.encode_tail(bits, encoded_bits);
		for (int i = 0; i < encoded_bits.size(); i++) {
			for (int j = 0; j < rep; j++) {
				out[o++] = encoded_bits[i].value();
			}
		}
#else
        int g0 = 0133, g1 = 0171, g2 = 0165;
        int state = 0;

		char b;
		for (int i = 0; i < tx.payload_size_bits + tx.tail_size; i++) {
			if (i < tx.payload_size_bits) {
				b = input[i];
			} else {
				b = 0;
			}
			state = ((state << 1) & 0x7e) | b;
			switch (phy.conv_code) {
				case CC_1_4:
					out[o++] = n_ones(state & g0) % 2;
					out[o++] = n_ones(state & g0) % 2;
					out[o++] = n_ones(state & g1) % 2;
					out[o++] = n_ones(state & g1) % 2;
					break;
				case CC_1_3:
					out[o++] = n_ones(state & g0) % 2;
					out[o++] = n_ones(state & g1) % 2;
					out[o++] = n_ones(state & g2) % 2;
					break;
				case CC_2_3:
					if (i % 2 == 0) {
						out[o++] = n_ones(state & g0) % 2;
					}
					out[o++] = n_ones(state & g1) % 2;
					break;
				default:
					assert(false);
					break;
			}
		}
#endif
	} else {
		memcpy(out, input, (size_t)tx.inner_size);
	}
}


void inner_decode(const char *input, char *out,
				  tx_param &tx, phy_param &phy)
{
	if (phy.conv_code != CC_NONE) {
		Punctured_Convolutional_Code code;
		ivec generator(3);
		generator(0)= 0133;
		generator(1) = 0171;
		generator(2) = 0165;
		code.set_generator_polynomials(generator, 7);
		bmat puncture_matrix;

		vec rx_signal;
		bvec decoded_bits;

		int rep = 1;
		if (phy.conv_code == CC_1_4) {
			rep = 2;
		}
		rx_signal.set_size(tx.inner_size / rep);
		int i = 0;
		for (i = 0; i < tx.inner_size / rep; i++) {
			// IT++ convolutional decoder interprets
			// -1 as bit 0
			//  1 as bit 1
			rx_signal[i] = (input[i * rep] == 0 ? 1 : -1);
		}

		switch (phy.conv_code) {
			case CC_1_4:
				puncture_matrix = "1 1;1 1;0 0";
				break;
			case CC_1_3:
				puncture_matrix = "1 1;1 1;1 1";
				break;
			case CC_2_3:
				puncture_matrix = "1 0;1 1;0 0";
				break;
			default:
				assert(false);
				break;
		}
		code.set_puncture_matrix(puncture_matrix);
		code.reset();
		code.decode_tail(rx_signal, decoded_bits);
		for (int j = 0; j < decoded_bits.size(); j++) {
			out[j] = decoded_bits[j].value();
		}

	} else {
		memcpy(out, input, (size_t)tx.payload_size_bits);
	}
}


void line_decode(char *line, char *rll,
				 tx_param &tx, phy_param &phy) {
	int o = 0;
	int rep = 2; // it is because of limited sampling rate of USRP
	int duty = 1; // rep = 2, duty = 1 means 50 % duty cycle

	switch(phy.line_code) {
		case LINE_OOK: {

			for (int i = 0; i < tx.line_size;) {
				int bit = 0;
				for (int j = 0; j < rep; j++) {
					bit += line[i++];
				}
				rll[o++] = (char) (bit > 0 ? 1: -1);
			}
			break;
		}

		case LINE_VPPM: {

			for (int i = 0; i < tx.line_size;) {
				int bit0 = 0;
				for (int j = 0; j < duty; j++) {
					bit0 += line[i++];
				}
				int bit1 = 0;
				for (int j = 0; j < duty; j++) {
					bit1 = line[i++];
				}
				rll[o++] = (char) ((bit1 - bit0) > 0 ? 1 : -1);
			}
			break;
		}

		default:
			assert(false);
			break;
	}
}


void rll_decode(char *rll, char *punctured,
				tx_param &tx, phy_param &phy) {
	int o = 0;

	switch(phy.rll_code) {
		case RLL_MANCHESTER: {

			for (int i = 0; i < tx.rll_size;) {
				int bit = -rll[i++];
				bit += rll[i++];
				punctured[o++] = (char) (bit > 0 ? 1 : 0);
			}
			break;
		}

		case RLL_4B6B: {

			for (int i = 0; i < tx.rll_size;) {
				int bits = 0;
				for (int j = 0; j < 6; j++) {
					bits |= (rll[i++] == 1) << j;
				}

				for (int j = 0; j < 4; j++) {
					if (o < tx.inner_size) {
						punctured[o++] = !!(rll6B4B[bits] & (1 << j));
					}
				}
			}
			break;
		}

		case RLL_8B10B: {

			int RD = -1;
			for (int i = 0; i < tx.rll_size;) {
				int bits = 0;
				for (int j = 0; j < 6; j++) {
					bits |= (rll[i++] == 1) << j;
				}
				if (n_ones(bits) != 3 && RD == 1) {
					bits = ~bits & 0b111111;
				} else if (n_ones(bits & 0b111) % 3 == 0 && RD == 1) {
					bits = ~bits & 0b111111;
				}

				for (int j = 0; j < 5; j++) {
					punctured[o++] = !!(rll6B5BRdNeg[bits] & (1 << j));
				}

				if (2 * n_ones(bits) - 6) {
					RD *= -1;
				}

				bits = 0;
				for (int j = 0; j < 4; j++) {
					bits |= (rll[i++] == 1) << j;
				}
				if (n_ones(bits) != 2) {
					if (RD == 1) {
						bits = ~bits & 0b1111;
					}
					if (rll4B3BRdNeg[bits] == -1) {
						bits = ((bits & 0b111) << 1) | ((bits & 0b1000) >> 3);
					}
				} else if (n_ones(bits & 0b11) % 2 == 0 && RD == 1) {
					bits = ~bits & 0b1111;
				}

				for (int j = 0; j < 3; j++) {
					punctured[o++] = !!(rll4B3BRdNeg[bits] & (1 << j));
				}

				if (2 * n_ones(bits) - 4) {
					RD *= -1;
				}
			}
			break;
		}

		default:
			assert(false);
			break;
	}

}

OP_MODE fallback_mcs(OP_MODE o) {
	switch (o) {
		case OOK_MANCHESTER_RS_15_7_CC_1_4:
		case OOK_MANCHESTER_RS_15_11_CC_1_3:
		case OOK_MANCHESTER_RS_15_11_CC_2_3:
		case OOK_MANCHESTER_RS_15_11_CC_NONE:
		case OOK_MANCHESTER_RS_NONE_CC_NONE:
			return OOK_MANCHESTER_RS_15_7_CC_1_4;

		case VPPM_4B6B_RS_15_2_CC_NONE:
		case VPPM_4B6B_RS_15_4_CC_NONE:
		case VPPM_4B6B_RS_15_7_CC_NONE:
		case VPPM_4B6B_RS_NONE_CC_NONE:
			return VPPM_4B6B_RS_15_2_CC_NONE;

		case VPPM_4B6B_3_75_RS_64_32:
		case VPPM_4B6B_3_75_RS_160_128:
			return VPPM_4B6B_3_75_RS_64_32;

		case VPPM_4B6B_7_5_RS_64_32:
		case VPPM_4B6B_7_5_RS_160_128:
        case VPPM_4B6B_7_5_RS_NONE:
			return VPPM_4B6B_7_5_RS_64_32;

		case OOK_8B10B_15_RS_64_32:
		case OOK_8B10B_15_RS_160_128:
			return OOK_8B10B_15_RS_64_32;

		case OOK_8B10B_30_RS_64_32:
		case OOK_8B10B_30_RS_160_128:
			return OOK_8B10B_30_RS_64_32;

		case OOK_8B10B_60_RS_64_32:
		case OOK_8B10B_60_RS_160_128:
			return OOK_8B10B_60_RS_64_32;

		case OOK_8B10B_120_RS_64_32:
		case OOK_8B10B_120_RS_160_128:
        case OOK_8B10B_120_RS_NONE:
			return OOK_8B10B_120_RS_64_32;
	}
}
