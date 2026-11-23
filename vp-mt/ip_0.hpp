#ifndef IP_0_HPP
#define IP_0_HPP

#include "defines.hpp"

class IP_0: public sc_module
{

    public:

    IP_0 (sc_core::sc_module_name name);
    ~IP_0 ();

    tlm_utils::simple_target_socket<IP_0> interconnect_socket;
    tlm_utils::simple_initiator_socket<IP_0> bram_socket;

    protected:
    pl_t pl;
    sc_time delay;

    //system control signals
    sc_uint<1> start;
    sc_uint<1> ready;
    sc_uint<3> win_val;

    //internal signals

    void b_transport(pl_t&, sc_time&);
    void write_bram(sc_uint<64> addr, unsigned char val);
    unsigned char read_bram(sc_uint<64> addr);
};

#endif // IP_0_HPP
