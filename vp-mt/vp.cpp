#include "vp.hpp"

Vp::Vp(sc_module_name name):
    sc_module(name),
    ip_0("ip_0"),
    ip_1("ip_1"),
    cpu("cpu"),
    bram("bram"),
    interconnect("interconnect")
{
    ip_0.bram_socket(bram.socket);
    ip_1.bram_socket(bram.socket);
    
    /*sockets to be added*/

    SC_REPORT_INFO( "VP", "Constructed");
}

Vp::~Vp()
{
    SC_REPORT_INFO( "VP", "Destroyed");
}
