/* -*- c++ -*- */

#define IEEE802_15_7_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "ieee802_15_7_swig_doc.i"

%{
#include "ieee802_15_7/generate_MAC.h"
#include "ieee802_15_7/encode_PSDU.h"
#include "ieee802_15_7/generate_PHR.h"
#include "ieee802_15_7/generate_preamble.h"
#include "ieee802_15_7/rll_enc.h"
#include "ieee802_15_7/rll_dec.h"
#include "ieee802_15_7/line_enc.h"
#include "ieee802_15_7/line_dec.h"
#include "ieee802_15_7/detect_FLP.h"
#include "ieee802_15_7/decode_PHR.h"
#include "ieee802_15_7/decode_PSDU.h"
%}

%include "ieee802_15_7/generate_MAC.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, generate_MAC);
%include "ieee802_15_7/encode_PSDU.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, encode_PSDU);
%include "ieee802_15_7/generate_PHR.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, generate_PHR);
%include "ieee802_15_7/generate_preamble.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, generate_preamble);
%include "ieee802_15_7/rll_enc.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, rll_enc);
%include "ieee802_15_7/rll_dec.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, rll_dec);
%include "ieee802_15_7/line_enc.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, line_enc);
%include "ieee802_15_7/line_dec.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, line_dec);
%include "ieee802_15_7/detect_FLP.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, detect_FLP);
%include "ieee802_15_7/decode_PHR.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, decode_PHR);
%include "ieee802_15_7/decode_PSDU.h"
GR_SWIG_BLOCK_MAGIC2(ieee802_15_7, decode_PSDU);

