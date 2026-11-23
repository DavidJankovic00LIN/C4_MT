#ifndef BRAM_HPP
#define BRAM_HPP


#include "defines.hpp"

class Bram: public sc_core::sc_module {

public:

    Bram (sc_core::sc_module_name name);
    ~Bram ();
    tlm_utils::simple_target_socket<Bram> bram_socket_1;
    tlm_utils::simple_target_socket<Bram> bram_socket_2;


    protected:

    void b_transport(pl_t &, sc_core::sc_time &);
    std::vector<unsigned char> mem;



};

#endif