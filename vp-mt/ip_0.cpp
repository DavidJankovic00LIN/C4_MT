#include "ip_0.hpp"

SC_HAS_PROCESS(IP_0);

IP_0::IP_0(sc_module_name name):
sc_module(name),
ready(1) {
    interconnect_socket.register_b_transport(this, &IP_0::b_transport);
    SC_REPORT_INFO("IP_0","Constructed.");
}
IP_0::~IP_0()
{
    SC_REPORT_INFO("IP_0","Destructed.");
}

void IP_0::b_transport(pl_t& pl, sc_time& offset)
{
    tlm_command cmd = pl.get_command();
    sc_dt::uint64 addr = pl.get_address();
    unsigned int len = pl.get_data_length();
    unsigned char *buf = pl.get_data_ptr();
    pl.set_response_status(TLM_OK_RESPONSE);
    switch (cmd)
    {
        case TLM_WRITE_COMMAND:
            switch (addr)
            {
                case ADDR_START:
                    start = toInt(buf);
                    //cout << "start: " << start << endl;
                    if (start == 1 && ready == 1)
                    {
                        ready = 0;
                        // winning(offset);
                        ready = 1;
                    }
                    break;
                default:
                    pl.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            }
            break;

        case TLM_READ_COMMAND:
            switch (addr)
            {
                // preuzimanje vrednosti sa winninga
                case ADDR_WIN_VAL:
                    // win_value=winning(offset);
                    toUchar(buf, win_val);
                    break;

                case ADDR_READY:
                    toUchar(buf, ready);
                    break;
                default:
                    pl.set_response_status(TLM_ADDRESS_ERROR_RESPONSE);
            }
            break;

        default:
            pl.set_response_status(tlm::TLM_COMMAND_ERROR_RESPONSE);
            cout << "Wrong command" << endl;
    }
}


void IP_0::write_bram(sc_uint<64> addr,unsigned char val)
{
    // upitno da li je neohodna??
	/*pl_t pl;
	unsigned char buf;
	buf = val;
	pl.set_address(addr);
	pl.set_data_length(1); 
	pl.set_data_ptr(&buf);
	pl.set_command( tlm::TLM_WRITE_COMMAND );
	pl.set_response_status ( tlm::TLM_INCOMPLETE_RESPONSE );
	bram_socket->b_transport(pl, offset);*/

}

void IP_01::read_bram(sc_uint<64> addr)
{
    pl_t pl;
    unsigned char buf;
    pl.set_address(VP_ADDR_BRAM_L+addr);
    pl.set_data_length(1);
    pl.set_data_ptr(&buf);
    pl.set_command(tlm::TLM_READ_COMMAND);
    pl.set_response_status(tlm::TLM_INCOMPLETE_RESPONSE);

    bram_socket->b_transport(pl, offset);

    return buf;
}