#ifndef TYPEDEFS_HPP
#define TYPEDEFS_HPP

#define SC_INCLUDE_FX
#include<tlm>
#include<systemc>
#include<string>
#include<vector>
#include<iostream>
#include<bitset>
#include<cmath>

typedef tlm::tlm_base_protocol_types::tlm_payload_type pl_t;
typedef tlm::tlm_base_protocol_types::tlm_phase_type ph_t;

using namespace std;
using namespace sc_core;
using namespace tlm;
using namespace sc_dt;

//registers in IP01 - NEGAMAX -TBD


//registers in IP02 - MCTS - TBD

//AXI & BRAM -TBD
#define VP_ADDR_BRAM_L 
#define VP_ADDR_BRAM_H 

#define VP_ADDR_IP01_HARD_L
#define VP_ADDR_IP01_HARD_H

#define VP_ADDR_IP02_HARD_L
#define VP_ADDR_IP02_HARD_H

#endif // TYPEDEFS_HPP