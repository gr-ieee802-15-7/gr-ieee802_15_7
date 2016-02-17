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
#include <stdint.h>
#include <boost/crc.hpp>
#include <gnuradio/io_signature.h>
#include <itpp/itcomm.h>
#include <iomanip>

#include "decode_PSDU_impl.h"
#include <ieee802_15_7/utils.h>

using namespace std;
using namespace gr::ieee802_15_7;
using namespace itpp;

decode_PSDU_impl::decode_PSDU_impl(bool debug) :
	tagged_stream_block("decode PSDU",
			gr::io_signature::make(1, 1, sizeof(char)),
			gr::io_signature::make(0, 0, 0),
			"packet_len"),
			d_debug(debug) {

	message_port_register_out(pmt::mp("out"));

	set_tag_propagation_policy(TPP_DONT);

	gettimeofday(&tv_start, NULL);
	us_start = tv_start.tv_sec * 1000000 + tv_start.tv_usec;
	exceed_time = false;
	nbytes = 0;
}

decode_PSDU_impl::~decode_PSDU_impl() {
}


int decode_PSDU_impl::work (int noutput_items,
		gr_vector_int& ninput_items,
		gr_vector_const_void_star& input_items,
		gr_vector_void_star& output_items) {

	const char *in = (const char*)input_items[0];
	char *out = (char*)output_items[0];

	vector<tag_t> tags;
	get_tags_in_range(tags, 0, nitems_read(0),
					  nitems_read(0) + ninput_items[0]);
	if(!tags.size()) {
		throw runtime_error("no tags in input stream");
	}

	int o = 0;

	OP_MODE op_mode;
	int psdu_len;

	for (int i = 0; i < tags.size(); i++) {
		if (pmt::symbol_to_string(tags[i].key) == "op_mode") {
			op_mode = (OP_MODE)pmt::to_long(tags[i].value);
		}
		if (pmt::symbol_to_string(tags[i].key) == "psdu_len") {
			psdu_len = (int)pmt::to_long(tags[i].value);
		}
	}

	phy_param phy(op_mode);
	tx_param tx(phy, psdu_len);

	char *inner_data = (char *)calloc((size_t)tx.inner_size, sizeof(char));
	char *unpacked_data = (char *)calloc((size_t)tx.payload_size_bits, sizeof(char));
	char *interleaved_data = (char *)calloc((size_t)tx.outer_size, sizeof(char));
	char *outer_data = (char *)calloc((size_t)tx.outer_size, sizeof(char));
	char *payload = (char *)calloc((size_t)tx.payload_size, sizeof(char));

	memcpy(inner_data, in, (size_t)tx.inner_size);
	inner_decode(inner_data, unpacked_data, tx, phy);
	pack_bits(unpacked_data, interleaved_data, tx);
	deinterleave(interleaved_data, outer_data, tx, phy);
	int ret = outer_decode(outer_data, payload, tx, phy);
	if (ret != -1) {

		int frame_ver = payload[0] & 0b11;
		int reserved = (payload[0] & 0b111100) >> 2;
		int frame_type = (payload[1] & 1) | ((payload[0] & 0b11000000) >> 6);
		int security_enabled = (payload[1] & 0b10) >> 1;
		int frame_pending = (payload[1] & 0b100) >> 2;
		int ack_req = (payload[1] & 0b1000) >> 3;
		int dst_addr_mode = (payload[1] & 0b110000) >> 4;
		int src_addr_mode = (payload[1] & 0b11000000) >> 6;

		uint8_t seq_no = payload[2];
		uint8_t dst_vpan_id[2], dst_addr[2], src_vpan_id[2], src_addr[2];
		for (int i = 0; i < 2; i++) {
			dst_vpan_id[i] = payload[i + 3];
			dst_addr[i] = payload[i + 5];
			src_vpan_id[i] = payload[i + 7];
			src_addr[i] = payload[i + 9];
		}

		uint16_t fcs_payload = (payload[psdu_len - 1] << 8) | (payload[psdu_len - 2] & 0xFF);

		boost::crc_16_type result;
		result.process_bytes(payload, psdu_len - 2);
		uint16_t fcs = result.checksum();

		dout << "Payload checksum: ";
		if (fcs_payload == fcs) {
			pmt::pmt_t pmt_length = pmt::make_dict();
			pmt_length = pmt::dict_add(pmt_length, pmt::mp("length"), pmt::from_long(tx.payload_size));
			pmt::pmt_t pmt_payload = pmt::make_blob(payload, tx.payload_size);
			message_port_pub(pmt::mp("out"), pmt::cons(pmt_length, pmt_payload));

			dout << "OK" << endl;
			dout << "Frame version: " << frame_ver;
			dout << "\tReserved: " << reserved;
			dout << "\tFrame type: " << frame_type;
			dout << "\tSecurity Enabled: " << security_enabled;
			dout << "\tFrame pending: " << frame_pending;
			dout << "\tAck request: " << ack_req;
			dout << "\tDest addressing mode: " << dst_addr_mode;
			dout << "\tSource addressing mode: " << src_addr_mode << endl;
			dout << "Sequence number: " << +seq_no;
			dout << "\tDest VPAN ID: " << +dst_vpan_id[0] << ":" << +dst_vpan_id[1];
			dout << "\tDest address: " << +dst_addr[0] << ":" << +dst_addr[1];
			dout << "\tSource VPAN ID: " << +src_vpan_id[0] << ":" << +src_vpan_id[1];
			dout << "\tSource address: " << +src_addr[0] << ":" << +src_addr[1] << endl;
			dout << "Content: ";
			for (int i = 0; i < psdu_len - 13; i++) {
				dout << payload[i + 11];
			}
			dout << endl;

		} else {
			dout << "Failed" << endl;
		}
	}

	free(inner_data);
	free(unpacked_data);
	free(interleaved_data);
	free(outer_data);
	free(payload);

	return o;
}


decode_PSDU::sptr
decode_PSDU::make(bool debug) {
	return gnuradio::get_initial_sptr(new decode_PSDU_impl(debug));
}

