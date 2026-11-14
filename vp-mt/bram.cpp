#include "bram.hpp"
#include "utilities.hpp"

Bram::Bram(sc_core::sc_module_name name): sc_module(name) {

    bram_socket_1.register_b_transport(this, &Bram::b_transport);
    bram_socket_2.register_b_transport(this, &Bram::b_transport);
    mem.resize(BRAM_SIZE, ' ');

    SC_REPORT_INFO("Bram","Construced.");
}

Bram::~Bram() {
SC_REPORT_INFO("Bram","Destructed.");
}


void Bram::b_transport(pl_t &pl, sc_core::sc_time &offset) {

    tlm::tlm_command cmd= pl.get_command();
    sc_dt::uint64 addr= pl.get_address();
    unsigned int len= pl.get_data_length();
    unsigned char *buf= pl.get_data_ptr();

    switch (cmd) {
        case tlm::TLM_WRITE_COMMAND:
            for(unsigned int i=0; i<len; i++){

                mem[addr++]=buf[i];
                //cout <<"read from bram:";
                //cout <<"ADDR: " <<static_cast<int>(addr-1)<< ", MEM: " <<(int)mem[addr-1]<<endl;
            }

            pl.set_response_status(tlm::TLM_OK_RESPONSE);

            offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
            break;

            case tlm::TLM_READ_COMMAND:
                for(unsigned int i=0; i<len; i++){

                    buf[i]=mem[addr++];
                    //cout <<"read from bram: ";
                    //cout <<"ADDR: " <<static_cast<int>(addr-1)<< ", MEM: " <<(int)mem[addr-1]<<endl;
             }

            pl.set_response_status(tlm::TLM_OK_RESPONSE);

            offset += sc_core::sc_time(DELAY, sc_core::SC_NS);
            break;

            default:
                pl.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE );
                offset += sc_core::sc_time(DELAY,sc_core::SC_NS);

    }



}