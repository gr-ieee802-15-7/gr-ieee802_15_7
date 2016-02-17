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

#include <ieee802_15_7/encode_PSDU.h>
#include <ieee802_15_7/utils.h>
#include <gnuradio/io_signature.h>


using namespace std;
using namespace gr::ieee802_15_7;


class encode_PSDU_impl : public encode_PSDU {
public:

encode_PSDU_impl(OP_MODE o, bool debug) :
	block ("encode_PSDU",
			gr::io_signature::make(0, 0, 0),
			gr::io_signature::make(1, 1, sizeof(char))),
			d_symbols_offset(0),
			d_symbols(NULL),
			d_debug(debug),
			d_phyparam(o) {

	message_port_register_in(pmt::mp("in"));
	set_op_mode(o);
}

~encode_PSDU_impl() {
	free(d_symbols);
}


int general_work(int noutput, gr_vector_int& ninput_items,
			gr_vector_const_void_star& input_items,
			gr_vector_void_star& output_items ) {

	unsigned char *out = (unsigned char*)output_items[0];

	while(!d_symbols_offset) {

		pmt::pmt_t msg(delete_head_blocking(pmt::intern("in")));

		if(pmt::is_eof_object(msg)) {
			return -1;
		}

		if(pmt::is_pair(msg)) {
			gr::thread::scoped_lock lock(d_mutex);

			int payload_length = (int)pmt::blob_length(pmt::cdr(msg));
			if(payload_length > 1023) {
				// aMaxPHYFrameSize: 1023 for PHY I, 65535 for PHY II in octets
				// aMinMPDUOverhead: 9 in octets
				cout << "packet too large" <<endl;
				return 0;
			}
			const char *payload = static_cast<const char*>(pmt::blob_data(pmt::cdr(msg)));

			// ############ INSERT MAC STUFF
			tx_param tx(d_phyparam, payload_length); // 1: byte-scale

			char *outer_data = (char*)calloc((size_t)tx.outer_size, sizeof(char));
			char *interleaved_data = (char*)calloc((size_t)tx.outer_size, sizeof(char));
			char *payload_bits = (char*)calloc((size_t)tx.payload_size_bits, sizeof(char));
			char *inner_data = (char*)calloc((size_t)tx.inner_size, sizeof(char));
			char *padded_data = (char*)calloc((size_t)tx.padded_size, sizeof(char));

			outer_encode(payload, outer_data, tx, d_phyparam);
			interleave(outer_data, interleaved_data, tx, d_phyparam);
			unpack_bytes(interleaved_data, payload_bits, tx);
			inner_encode(payload_bits, inner_data, tx, d_phyparam);
			memcpy(padded_data, inner_data, (size_t)tx.inner_size);

			pmt::pmt_t srcid = pmt::string_to_symbol(alias());

			pmt::pmt_t psdu_len = pmt::from_long(tx.payload_size);
			add_item_tag(0, nitems_written(0), pmt::mp("psdu_len"),
				psdu_len, srcid);

			pmt::pmt_t packet_len = pmt::from_long(tx.inner_size);
			add_item_tag(0, nitems_written(0), pmt::mp("packet_len"),
				packet_len, srcid);

			pmt::pmt_t op_mode = pmt::from_long(d_phyparam.op_mode);
			add_item_tag(0, nitems_written(0), pmt::mp("op_mode"),
				op_mode, srcid);

			d_symbols_len = tx.padded_size;
			d_symbols = (char*)calloc((size_t)d_symbols_len, 1);
			memcpy(d_symbols, padded_data, (size_t)d_symbols_len);

			free(outer_data);
			free(interleaved_data);
			free(payload_bits);
			free(inner_data);
			free(padded_data);

			break;
		}
	}

	int i = min(noutput, d_symbols_len - d_symbols_offset);
	memcpy(out, d_symbols + d_symbols_offset, (size_t)i);
	d_symbols_offset += i;

	if(d_symbols_offset == d_symbols_len) {
		d_symbols_offset = 0;
		free(d_symbols);
		d_symbols = 0;
	}

	return i;
}

void set_op_mode(OP_MODE op_mode) {
	gr::thread::scoped_lock lock(d_mutex);
	d_phyparam = phy_param(op_mode);
}

private:
	bool				d_debug;
	char*				d_symbols;
	int					d_symbols_offset;
	int					d_symbols_len;
	phy_param			d_phyparam;
	gr::thread::mutex	d_mutex;

};

encode_PSDU::sptr
encode_PSDU::make(OP_MODE op_mode, bool debug) {
	return gnuradio::get_initial_sptr(new encode_PSDU_impl(op_mode, debug));
}
