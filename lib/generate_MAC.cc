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

#include <ieee802_15_7/generate_MAC.h>

#include <gnuradio/io_signature.h>
#include <gnuradio/block_detail.h>

#include <ieee802_15_7/utils.h>

#if defined(__APPLE__)
#include <architecture/byte_order.h>
#define htole16(x) OSSwapHostToLittleInt16(x)
#else
#include <endian.h>
#endif

#include <boost/crc.hpp>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace gr::ieee802_15_7;

class generate_MAC_impl : public generate_MAC {

public:

generate_MAC_impl(std::vector<uint8_t> dst_vpan_id, std::vector<uint8_t> dst_addr,
		std::vector<uint8_t> src_vpan_id, std::vector<uint8_t> src_addr) :
		block("generate_MAC",
			gr::io_signature::make(0, 0, 0),
			gr::io_signature::make(0, 0, 0)),
		d_seq_nr(0) {

	message_port_register_out(pmt::mp("phy out"));

	message_port_register_in(pmt::mp("app in"));
	set_msg_handler(pmt::mp("app in"), boost::bind(&generate_MAC_impl::app_in, this, _1));

	if(!check_addr(dst_vpan_id)) throw std::invalid_argument("wrong destination VPAN ID size");
	if(!check_addr(dst_addr)) throw std::invalid_argument("wrong destination address size");
	if(!check_addr(src_vpan_id)) throw std::invalid_argument("wrong source VPAN ID size");
	if(!check_addr(src_addr)) throw std::invalid_argument("wrong source address size");

	for(int i = 0; i < 2; i++) {
		d_dst_vpan_id[i] = dst_vpan_id[i];
		d_dst_addr[i] = dst_addr[i];
		d_src_vpan_id[i] = src_vpan_id[i];
		d_src_addr[i] = src_addr[i];
	}
}

void app_in (pmt::pmt_t msg) {

	size_t       msg_len;
	const char   *msdu;

	if(pmt::is_eof_object(msg)) {
		message_port_pub(pmt::mp("phy out"), pmt::PMT_EOF);
		detail().get()->set_done(true);
		return;

	} else if(pmt::is_symbol(msg)) {

		std::string  str;
		str = pmt::symbol_to_string(msg);
		msg_len = str.length();
		msdu = str.data();

	} else if(pmt::is_pair(msg)) {

		msg_len = pmt::blob_length(pmt::cdr(msg));
		msdu = reinterpret_cast<const char *>(pmt::blob_data(pmt::cdr(msg)));

	} else {
		throw std::invalid_argument("IEEE 802.15.7 MAC expects PDUs or strings");
                return;
	}

	// make MAC frame
	int    psdu_length;
	char   *psdu;
	generate_mac_data_frame(msdu, msg_len, &psdu, &psdu_length);

	// dict
	pmt::pmt_t dict = pmt::make_dict();
	dict = pmt::dict_add(dict, pmt::mp("crc_included"), pmt::PMT_T);

	// blob
	pmt::pmt_t mac = pmt::make_blob(psdu, psdu_length);

	// pdu
	message_port_pub(pmt::mp("phy out"), pmt::cons(dict, mac));

	free(psdu);
}

void generate_mac_data_frame(const char *msdu, int msdu_size, char **psdu, int *psdu_size) {

	// mac header
	mac_header header;
	int header_size;
	header_size = sizeof(header);
	header.frame_control = 0xA040;

	header.seq_nr = d_seq_nr;
	d_seq_nr++;

	for(int i = 0; i < 2; i++) {
		header.addr1[i] = d_dst_vpan_id[i];
		header.addr2[i] = d_dst_addr[i];
		header.addr3[i] = d_src_vpan_id[i];
		header.addr4[i] = d_src_addr[i];
	}

	//header size is 11, plus 2 for FCS means 13 bytes
	*psdu_size = header_size + msdu_size + 2;
	*psdu = (char *) calloc(*psdu_size, sizeof(char));

	//copy mac header into psdu
	std::memcpy(*psdu, &header, header_size);
	//copy msdu into psdu
	memcpy(*psdu + header_size, msdu, msdu_size);
	//compute and store fcs
	boost::crc_16_type result;
	result.process_bytes(*psdu, header_size + msdu_size);

	uint16_t fcs = result.checksum();
	memcpy(*psdu + header_size + msdu_size, &fcs, sizeof(uint16_t));
}

bool check_addr(std::vector<uint8_t> mac) {
	if(mac.size() != 2) return false;
	return true;
}

private:
	uint8_t d_seq_nr;
	uint8_t d_dst_vpan_id[2];
	uint8_t d_dst_addr[2];
	uint8_t d_src_vpan_id[2];
	uint8_t d_src_addr[2];
};

generate_MAC::sptr
generate_MAC::make(std::vector<uint8_t> dst_vpan_id, std::vector<uint8_t> dst_addr,
		std::vector<uint8_t> src_vpan_id, std::vector<uint8_t> src_addr) {
	return gnuradio::get_initial_sptr(new generate_MAC_impl(dst_vpan_id, dst_addr, src_vpan_id, src_addr));
}

