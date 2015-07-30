//===-- tlm_router_test.h - test module for router --------------*- C++ -*-===//
//
//                              ArchC Project
//
// This file is distributed under the GNU Affero General Public License v3.0.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the module to test router module with TLM2 payloads.
///
//===----------------------------------------------------------------------===//
#ifndef TLM_ROUTER_TEST_H_
#define TLM_ROUTER_TEST_H_

#include <string>
using std::string;

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
using namespace sc_core;
#include "tlm.h"
using namespace tlm;
#include "tlm_utils/simple_target_socket.h"
#include "tlm_utils/simple_initiator_socket.h"
using namespace tlm_utils;

#include "tlm_router.h"
using tlm_router::router;
using tlm_router::router_table;

#include "tlm_init_dummy.h"
using tlm_dummy::init_dummy;

#include "tlm_target_dummy.h"
using tlm_dummy::target_dummy;

class router_test : sc_core::sc_module {
  typedef router_test SC_CURRENT_USER_MODULE;

private:
  sc_event transaction_done;
  sc_event confirmation_done;
  uint32_t delay_req;
  uint32_t delay_resp;

  unsigned int msgs;
  unsigned int bucket_size;
  bool wait_done;

  int target;
  unsigned int receive;
  unsigned int *receive_tiles;

  int source;
  unsigned int send;
  unsigned int *send_tiles;
  unsigned int confirmations;
  unsigned int *confirmations_tiles;

  unsigned int id;
  unsigned int ntiles;
  bool *tiles;
  unsigned int exit_count;

public:
  simple_initiator_socket<router_test> isocket;
  simple_target_socket<router_test> tsocket;
  peq_with_get<tlm_generic_payload> peq;

  tlm_sync_enum nb_transport_fw(tlm_generic_payload &payload, tlm_phase &phase,
                                sc_time &delay_time) {
    sc_time delay;
    unsigned int expected_value;

    tlm_command cmd;
    unsigned int addr;
    unsigned int *ptr;
    unsigned int len;
    router_payload *ext;
    unsigned int pkgsource = 0;

    cmd = payload.get_command();
    addr = payload.get_address();
    ptr = (unsigned int *)payload.get_data_ptr();
    len = payload.get_data_length();
    payload.get_extension(ext);

    switch (cmd) {
    case TLM_WRITE_COMMAND:
      if ((!this->id) && (!addr) && (len == 2)) {
        unsigned short int *iptr = (unsigned short int *)payload.get_data_ptr();
        if (this->tiles[*iptr]) {
          fprintf(stderr, "Receveid 2 end request from %03d.\n", *iptr);
          exit(1);
        }
        this->tiles[*iptr] = true;
        this->exit_count++;
        printf("Request exit from %u (%u/%u)\n", *iptr, this->exit_count,
               this->ntiles);
        if (this->exit_count == this->ntiles) {
          printf("Complete\n");
          sc_stop();
        }
        break;
      }

      pkgsource = addr & 0xFFFF;
      printf("Receiving at %d (%u/%u).\n", this->id,
             this->receive_tiles[pkgsource] + 1, this->msgs);
      if (this->source >= 0 && pkgsource != this->source) {
        fprintf(stderr, "Invalid source: %u != %u\n", pkgsource, this->source);
        exit(1);
      }

      expected_value = ~addr;
      if (expected_value != *ptr) {
        fprintf(stderr, "Expected Value: %u != %u.\n", expected_value, *ptr);
        exit(1);
      }

      this->receive++;
      this->receive_tiles[pkgsource]++;

      if (this->receive_tiles[pkgsource] > this->msgs) {
        fprintf(stderr, "Too many writes received at %03u from %03u\n",
                this->id, pkgsource);
        exit(1);
      }

      break;
    default:
      fprintf(stderr, "Command %d is not implemented.\n", cmd);
      exit(1);
    }
    delay_time = sc_time(delay_req, SC_NS); // TODO

    payload.set_response_status(TLM_OK_RESPONSE);
    peq.notify(payload, delay_time);

    phase = END_REQ;
    return TLM_UPDATED;
  }

  tlm_sync_enum nb_transport_bw(tlm_generic_payload &payload, tlm_phase &phase,
                                sc_time &delay_time) {
    sc_time delay;
    unsigned int expected_value;

    tlm_command cmd;
    unsigned int addr;
    unsigned int *ptr;
    unsigned int len;

    cmd = payload.get_command();
    addr = payload.get_address();
    ptr = (unsigned int *)payload.get_data_ptr();
    len = payload.get_data_length();

    if (payload.get_response_status() != TLM_OK_RESPONSE) {
      fprintf(stderr, "Wrong response status (%d).\n",
              payload.get_response_status());
      exit(1);
    }

    switch (cmd) {
    case TLM_WRITE_COMMAND:
      if (len == 4) {

        expected_value = ~addr;
        if (expected_value != *ptr) {
          fprintf(stderr, "Expected Value: %u != %u.\n", expected_value, *ptr);
          exit(1);
        }

        // Write confirmed
        this->confirmations++;
        if (this->confirmations > this->send) {
          fprintf(stderr, "More confirmations than sent: %u != %u.\n",
                  this->confirmations, this->send);
          exit(1);
        }

        unsigned int pkgtarget = addr >> 16;
        this->confirmations_tiles[pkgtarget]++;
        printf("Confirmation from %d (%u/%u).\n", pkgtarget,
               this->confirmations_tiles[pkgtarget],
               this->send_tiles[pkgtarget]);
        if (this->confirmations_tiles[pkgtarget] >
            this->send_tiles[pkgtarget]) {
          fprintf(stderr, "More confirmations than sent to %u: %u != %u.\n",
                  pkgtarget, this->confirmations_tiles[pkgtarget],
                  this->send_tiles[pkgtarget]);
          exit(1);
        }

        unsigned int expected_confirmations =
            this->target < 0 ? this->ntiles : 1;
        expected_confirmations *= this->msgs;

        if (!(this->confirmations % this->bucket_size)) {
          if (this->wait_done)
            transaction_done.notify(delay);
        }

        printf("Confirmation received at %u (%u/%u).\n", this->id,
               this->confirmations, expected_confirmations);
        if (this->confirmations == expected_confirmations) {
          confirmation_done.notify(delay);
        }
      } else if (addr) {
        fprintf(stderr, "Wrong length: %u != %u.\n", len, 4);
        exit(1);
      }
      free(ptr);
      delete (&payload);
      break;
    default:
      fprintf(stderr, "Command %d is not implemented.\n", cmd);
      exit(1);
    }
    delay = sc_time(delay_resp, SC_NS); // TODO

    phase = END_REQ;
    return TLM_COMPLETED;
  }

  void proc() {
    sc_time delay;
    delay = sc_time(0, SC_NS); // TODO

    while (true) {
      tlm_generic_payload *payload;

      wait(peq.get_event());
      while ((payload = peq.get_next_transaction())) {
        tlm_phase phase = BEGIN_RESP;

        printf("Send confirmation from %u.\n", this->id);
        tlm_sync_enum resp = tsocket->nb_transport_bw(*payload, phase, delay);
        if (resp != TLM_COMPLETED) {
          fprintf(stderr, "Response not completed: %d.\n", resp);
          exit(1);
        }
      }
    }
  }

  void generator() {
    tlm_generic_payload payload;
    sc_time delay;
    tlm_phase phase;
    tlm_sync_enum resp;

    // Generator Begin
    unsigned int target_now = 0;

    if ((this->source < 0) || (this->source == this->id)) {

      unsigned int expected_send = this->target < 0 ? this->ntiles : 1;
      expected_send *= this->msgs;

      while (true) {
        unsigned int addr;
        bool type = false;
        tlm::tlm_generic_payload *payload = new tlm_generic_payload();

        if (this->target < 0) {
          unsigned int target_proposed = target_now;
          while (this->send_tiles[target_now] >= this->msgs) {
            // Skiping write
            target_now++;
            target_now %= this->ntiles;
            if (target_now == target_proposed) {
              printf("All sent at %u (%u/%u).\n", this->id, this->send,
                     this->msgs);
              goto request_end;
            }
          };
        } else {
          target_now = this->target;
          if (this->send_tiles[target_now] >= this->msgs) {
            printf("All sent at %u (%u/%u).\n", this->id, this->send,
                   this->msgs);
            goto request_end;
          }
        }
        // Sending write
        addr = target_now;
        addr <<= 16;
        addr |= this->id & 0xFFFF;

        payload->set_address(addr);
        payload->set_data_length(4);
        payload->set_write(); // command

        unsigned int *ptr = (unsigned int *)malloc(sizeof(unsigned int));
        *ptr = ~addr;
        payload->set_data_ptr((unsigned char *)ptr);
        delay = sc_time(delay_req, SC_NS); // TODO
        phase = BEGIN_REQ;

        this->send++;
        this->send_tiles[target_now]++;
        printf("Sending from %u to %d (%u/%u).\n", this->id, target_now,
               this->send_tiles[target_now], this->msgs);
        resp = isocket->nb_transport_fw(std::ref(*payload), phase, delay);

        if (resp != TLM_UPDATED) {
          fprintf(stderr, "Response not updated: %d.\n", resp);
          exit(1);
        };

        target_now++;
        target_now %= this->ntiles;

        if (this->send >= expected_send) {
          goto request_end;
        } else if (!(this->send % this->bucket_size)) {
          if (this->wait_done)
            wait(transaction_done);
          else
            wait(1, SC_NS);
        }
      }

    request_end:
      if (this->confirmations < this->send) {
        printf("Wait confirmations at %u.\n", this->id);
        wait(confirmation_done);
      }
    }

    { // Request End
      unsigned int addr = 0;
      tlm::tlm_generic_payload *payload = new tlm_generic_payload();

      payload->set_address(addr);
      payload->set_data_length(sizeof(unsigned short int));
      payload->set_write(); // command

      unsigned short int *ptr =
          (unsigned short int *)malloc(sizeof(unsigned short int));
      *ptr = this->id;
      payload->set_data_ptr((unsigned char *)ptr);
      delay = sc_time(delay_req, SC_NS); // TODO
      phase = BEGIN_REQ;

      // Sending END signal
      printf("Requesting exit at %u\n", this->id);
      resp = isocket->nb_transport_fw(std::ref(*payload), phase, delay);

      if (resp != TLM_UPDATED) {
        fprintf(stderr, "Response not updated: %d.\n", resp);
        exit(1);
      };
    }
  }

  router_test(sc_module_name module_name, unsigned int id, unsigned int ntiles,
              int source, int target, unsigned int msgs,
              unsigned int bucket_size, bool wait_done)
      : sc_module(module_name), peq("peq") {

    isocket.register_nb_transport_bw(this, &router_test::nb_transport_bw);
    tsocket.register_nb_transport_fw(this, &router_test::nb_transport_fw);

    delay_req = 0;
    delay_resp = 0;

    this->id = id;
    this->ntiles = ntiles;

    this->tiles = NULL;
    if (!this->id) {
      this->tiles = (bool *)calloc(ntiles, sizeof(bool));
      this->exit_count = 0;
    }

    this->msgs = msgs;
    this->bucket_size = bucket_size;
    this->wait_done = wait_done;

    this->source = source;
    this->target = target;

    this->receive_tiles = NULL;
    if ((this->target < 0) || (this->target == this->id)) {
      printf("Alloc recv structure at %u.\n", this->id);
      this->receive = 0;
      this->receive_tiles =
          (unsigned int *)calloc(ntiles, sizeof(unsigned int));
    }

    this->send_tiles = NULL;
    this->confirmations_tiles = NULL;
    if ((this->source < 0) || (this->source == this->id)) {
      printf("Alloc send structure at %u.\n", this->id);
      this->send = 0;
      this->send_tiles = (unsigned int *)calloc(ntiles, sizeof(unsigned int));
      this->confirmations = 0;
      this->confirmations_tiles =
          (unsigned int *)calloc(ntiles, sizeof(unsigned int));
    }

    SC_THREAD(proc);
    SC_THREAD(generator);
  }

  ~router_test() {
    if (this->tiles)
      free(this->tiles);
    if (this->send_tiles)
      free(this->send_tiles);
    if (this->confirmations_tiles)
      free(this->confirmations_tiles);
    if (this->receive_tiles)
      free(this->receive_tiles);
  }
};
#endif // TLM_ROUTER_TEST_H_
