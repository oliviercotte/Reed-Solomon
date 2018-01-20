// Copyright 1991-2014 Mentor Graphics Corporation

// All Rights Reserved.

// THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION
// WHICH IS THE PROPERTY OF MENTOR GRAPHICS CORPORATION
// OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.

#ifndef INCLUDED_RINGBUF
#define INCLUDED_RINGBUF

#include <systemc.h>
#include "control.h"
#include "store.h"
#include "retrieve.h"

SC_MODULE(ringbuf)
{
public:
    // Module ports
    sc_in<bool>                  clock;
    sc_in<bool>                  Reset;
    sc_in<bool>                  txda;
    sc_out<bool>                 rxda;
    sc_out<bool>                 outstrobe;
    sc_out<bool>                 txc;

    // Signals interconnecting child instances
    sc_signal<sc_uint<9> >       ramadrs;
    sc_signal<sc_uint<16> >      buffer;
    sc_signal<bool >             oeenable;

    // Child instances
    control*                     block1;
    store*                       block2;
    retrieve*                    block3;

    SC_CTOR(ringbuf)
        : clock("clock"),
          Reset("Reset"),
          txda("txda"),
          rxda("rxda"),
          outstrobe("outstrobe"),
          txc("txc"),
          ramadrs("ramadrs"),
          buffer("buffer"),
          oeenable("oeenable")
    {
        block1 = new control("block1");
        block1->clock(clock);
        block1->reset(Reset);
        block1->ramadrs(ramadrs);
        block1->txc(txc);
        block1->oeenable(oeenable);
        block1->outstrobe(outstrobe);

        block2 = new store("block2");
        block2->clock(clock);
        block2->reset(Reset);
        block2->oeenable(oeenable);
        block2->ramadrs(ramadrs);
        block2->buffer(buffer);
        block2->txda(txda);

        block3 = new retrieve("block3");
        block3->outstrobe(outstrobe);
        block3->ramadrs(ramadrs);
        block3->buffer(buffer);
        block3->rxda(rxda);
    }

    ~ringbuf()
    {
        delete block1; block1 = 0;
        delete block2; block2 = 0;
        delete block3; block3 = 0;
    }
};

#endif
