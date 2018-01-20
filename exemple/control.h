// Copyright 1991-2014 Mentor Graphics Corporation

// All Rights Reserved.

// THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION
// WHICH IS THE PROPERTY OF MENTOR GRAPHICS CORPORATION
// OR ITS LICENSORS AND IS SUBJECT TO LICENSE TERMS.

#ifndef INCLUDED_CONTROL
#define INCLUDED_CONTROL

#include <systemc.h>

class control : public sc_module
{
public:
    sc_in<bool>               clock;
    sc_in<bool>               reset;
    sc_out<sc_uint<9> >       ramadrs;
    sc_out<bool>              oeenable;
    sc_out<bool>              txc;
    sc_out<bool>              outstrobe;

    // Class methods
    void incrementer();
    void enable_gen();
    void outstrobe_gen();


    // Constructor
    SC_CTOR(control)
        : clock("clock"),
          reset("reset"),
          ramadrs("ramadrs"),
          oeenable("oeenable"),
          txc("txc"),
          outstrobe("outstrobe")
    {
        SC_METHOD(incrementer);
        sensitive << clock.pos();
        dont_initialize();

        SC_METHOD(enable_gen);
        sensitive << clock.pos();
        dont_initialize();

        SC_METHOD(outstrobe_gen);
        sensitive << ramadrs;
        dont_initialize();
    }


    // Empty Destructor
    ~control()
    {
    }
};


//
// This process increments the ram address.
//
inline void control::incrementer()
{
    if (reset.read() == 0)
        ramadrs.write(0);
    else
        ramadrs.write(ramadrs.read() + 1);
}


//
// This process generates the output enable signal.
//
inline void control::enable_gen()
{
    if (reset.read() == 0)
        oeenable.write(0);
    else
    {
        if (ramadrs.read().range(4, 0) == 0)
            oeenable.write(1);
        else
            oeenable.write(0);
    }
}


//
// This process generates the outstrobe and the txc data line.
//
inline void control::outstrobe_gen()
{
    bool x = 1;
    for (int i = 4; i < 9; i++)
        x &= (bool)ramadrs.read()[i];

    outstrobe.write(x);
    txc.write((bool)ramadrs.read()[4]);
}

#endif
