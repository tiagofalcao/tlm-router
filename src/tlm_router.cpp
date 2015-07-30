//===-- tlm_router.cpp - router module implementation -----------*- C++ -*-===//
//
//                              ArchC Project
//
// This file is distributed under the GNU Affero General Public License v3.0.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation of the router module for TLM2,
/// which will be contruct a Mesh conection modules and routing payloads.
///
//===----------------------------------------------------------------------===//
#include "tlm_router.h"

using tlm_router::router;
typedef tlm_router::router SC_CURRENT_USER_MODULE;

router::router(sc_module_name module_name)
    : sc_module(module_name), peq("peq") {

  SC_THREAD(proc);

  LOCAL_init_socket.register_nb_transport_bw(this, &router::nb_transport_bw);
  LOCAL_target_socket.register_nb_transport_fw(this, &router::nb_transport_fw);

  N_init_socket.register_nb_transport_bw(this, &router::nb_transport_bw);
  N_target_socket.register_nb_transport_fw(this, &router::nb_transport_fw);
  S_init_socket.register_nb_transport_bw(this, &router::nb_transport_bw);
  S_target_socket.register_nb_transport_fw(this, &router::nb_transport_fw);
  W_init_socket.register_nb_transport_bw(this, &router::nb_transport_bw);
  W_target_socket.register_nb_transport_fw(this, &router::nb_transport_fw);
  E_init_socket.register_nb_transport_bw(this, &router::nb_transport_bw);
  E_target_socket.register_nb_transport_fw(this, &router::nb_transport_fw);

  delay_req = 1;
  delay_resp = 0;
}

router::~router() {}

tlm_sync_enum router::nb_transport_fw(tlm_generic_payload &payload,
                                      tlm_phase &phase, sc_time &delay_time) {
  sc_time delay;
  delay = sc_time(delay_req, SC_NS); // TODO

  // Received request
  peq.notify(payload, delay);

  phase = END_REQ;
  return TLM_UPDATED;
}

tlm_sync_enum router::nb_transport_bw(tlm_generic_payload &payload,
                                      tlm_phase &phase, sc_time &delay_time) {
  sc_time delay;
  delay = sc_time(delay_resp, SC_NS); // TODO

  peq.notify(payload, delay);

  phase = END_REQ;
  return TLM_COMPLETED;
}

void router::proc() {
  sc_time delay;
  delay = sc_time(0, SC_NS); // TODO

  while (true) {
    tlm_generic_payload *payload = NULL;

    wait(peq.get_event());

    while ((payload = peq.get_next_transaction())) {
      tlm_response_status status = payload->get_response_status();
      router_payload *ext;
      payload->get_extension(ext);

      if (status) {
        simple_target_socket<router> *socket;

        if (!ext) {
          perror("Response without router payload\n");
          exit(1);
        }

        ext->incNumberOfHops();

        unsigned int targetX, targetY;
        targetX = ext->getInitX();
        targetY = ext->getInitY();

        if ((targetX == posX) && (targetY == posY)) {
          // Routing response to LOCAL"
          socket = &LOCAL_target_socket;
          payload->clear_extension(ext);
          delete ext;
        } else if (targetY < posY) {
          // Routing response to West
          socket = &W_target_socket;
        } else if (targetY > posY) {
          // Routing response to East
          socket = &E_target_socket;
        } else if (targetX > posX) {
          // Routing response to North
          socket = &N_target_socket;
        } else if (targetX < posX) {
          // Routing response to South
          socket = &S_target_socket;
        }

        tlm_phase phase = BEGIN_RESP;
        tlm_sync_enum resp = (*socket)->nb_transport_bw(*payload, phase, delay);
        if (resp != TLM_COMPLETED) {
          fprintf(stderr, "Response not completed: %d.", resp);
        }
        // Response completed.
      } else {
        simple_initiator_socket<router> *socket;

        if (!ext) {
          ext = payload_extension_create(payload);
        }

        ext->incNumberOfHops();

        unsigned int targetX, targetY;
        targetX = ext->getTargetX();
        targetY = ext->getTargetY();

        if ((targetX == posX) && (targetY == posY)) {
          // Routing request to LOCAL
          socket = &LOCAL_init_socket;
        } else if (targetY < posY) {
          // Routing request to West
          socket = &W_init_socket;
        } else if (targetY > posY) {
          // Routing request to East
          socket = &E_init_socket;
        } else if (targetX > posX) {
          // Routing request to North
          socket = &N_init_socket;
        } else if (targetX < posX) {
          // Routing request to South
          socket = &S_init_socket;
        }

        tlm_phase phase = BEGIN_REQ;
        tlm_sync_enum resp = (*socket)->nb_transport_fw(*payload, phase, delay);
        if (resp != TLM_UPDATED) {
          fprintf(stderr, "Response not updated: %d.", resp);
        }
        // Response updated
      }
    }
  }
}

router_payload *router::payload_extension_create(tlm_generic_payload *payload) {
  sc_dt::uint64 addr;
  addr = payload->get_address();

  unsigned int targetX, targetY;
  tableOfRouts.returnsTargetPosition(addr, targetX, targetY);

  router_payload *ext = new router_payload(posX, posY, targetX, targetY);
  payload->set_extension(ext);

  return ext;
}

inline int router::getX() { return posX; }

inline int router::getY() { return posY; }

void router::setXY(unsigned int x, unsigned int y) {
  posX = x;
  posY = y;
}
