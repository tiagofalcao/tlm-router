//===-- tlm_router.h - router module definition -----------------*- C++ -*-===//
//
//                              ArchC Project
//
// This file is distributed under the GNU Affero General Public License v3.0.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the router module for TLM2,
/// which will be contruct a Mesh conection modules and routing payloads.
///
//===----------------------------------------------------------------------===//
#ifndef TLM_ROUTER_H_
#define TLM_ROUTER_H_

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
using namespace sc_core;
#include <tlm.h>
using namespace tlm;
#include <tlm_utils/simple_target_socket.h>
#include <tlm_utils/simple_initiator_socket.h>
using namespace tlm_utils;

#include "tlm_router_table.h"
using tlm_router::router_table;

#include "tlm_router_payload.h"
using tlm_router::router_payload;

namespace tlm_router {
class router : sc_core::sc_module {
public:
  simple_initiator_socket<router> LOCAL_init_socket;
  simple_target_socket<router> LOCAL_target_socket;

  simple_initiator_socket<router> N_init_socket;
  simple_target_socket<router> N_target_socket;

  simple_initiator_socket<router> S_init_socket;
  simple_target_socket<router> S_target_socket;

  simple_initiator_socket<router> W_init_socket;
  simple_target_socket<router> W_target_socket;

  simple_initiator_socket<router> E_init_socket;
  simple_target_socket<router> E_target_socket;

  router_table tableOfRouts;

  tlm_sync_enum nb_transport_fw(tlm_generic_payload &payload, tlm_phase &phase,
                                sc_time &delay_time);
  tlm_sync_enum nb_transport_bw(tlm_generic_payload &payload, tlm_phase &phase,
                                sc_time &delay_time);
  peq_with_get<tlm_generic_payload> peq;
  void proc();

  int getX();
  int getY();
  void setXY(unsigned int x, unsigned int y);

  router(sc_module_name module_name);

  ~router();

private:
  unsigned int delay_req;
  unsigned int delay_resp;

  unsigned int posX;
  unsigned int posY;

  router_payload *payload_extension_create(tlm_generic_payload *payload);
  bool local_response(tlm_generic_payload *payload);
};
};
#endif // TLM_ROUTER_H_
