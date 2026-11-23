#ifndef VP_HPP
#define VP_HPP


#include "defines.hpp"

#include "ip_0.hpp"
#include "ip_1.hpp"
#include "cpu.hpp"
#include "bram.hpp"
#include "interconnect.hpp"

class VP: public sc_module

{
    public:
        Vp(sc_core::sc_module_name name);
        ~VP();

    protected:
        IP_0 ip_0;
        IP_1 ip_1;
        CPU cpu;
        BRAM bram;
        Interconnect interconnect;
};

#endif // VP_HPP