# gr-ieee802_15_7

This is a GNU Radio out-of-tree (OOT) module which implements a transmitter and a receiver of IEEE 802.15.7 visible light communication standard.

**Note: This repository is for double blind peer review**

## Dependencies

GNU Radio: v.3.7.8 or later

ezpwd-reed-solomon: Reed-Solomon en/decoding

    # We are using a modified version of ezpwd-reed-solomon by pjkundert
    # git clone https://github.com/pjkundert/ezpwd-reed-solomon
    git clone https://github.com/gsongsong/ezpwd-reed-solomon
    cd ezpwd-reed-solomon/c++
    git checkout ret-1
    [sudo] cp ezpwd /usr/include/

gr-foo: Wireshark connector and periodic message source

    # We are using a modified version of gr-foo by bastibl
    # git clone https://github.com/bastibl/gr-foo
    git clone https://github.com/gsongsong/gr-foo
    cd gr-foo
    git checkout wireshark_802_15_7
    mkdir build && cd build
    cmake ..
    make
    [sudo] make install
    [sudo] ldconfig

IT++: Convolutional en/decoding

    [sudo] apt-get install libitpp
    
## Installation

    git clone https://gitlab.com/songsong/gr-ieee802_15_7.git
    cd gr-ieee802_15_7
    mkdir build && cd build
    cmake ..
    make
    [sudo] make install
    [sudo] ldconfig
