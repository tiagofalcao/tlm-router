//===-- tlm_router_table.h - router table definition ------------*- C++ -*-===//
//
//                              ArchC Project
//
// This file is distributed under the GNU Affero General Public License v3.0.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the declaration of the router table class to routers,
/// which will be used to memory mapping.
///
//===----------------------------------------------------------------------===//
#ifndef TLM_ROUTER_TABLE_H
#define TLM_ROUTER_TABLE_H

#include <systemc>

namespace tlm_router {
class router_table {
public:
  router_table();
  router_table(unsigned int);
  ~router_table();

  void newEntryAbsolute(unsigned int, unsigned int, sc_dt::uint64,
                        sc_dt::uint64);
  void newEntry(unsigned int, unsigned int, sc_dt::uint64);
  void returnsTargetPosition(sc_dt::uint64 addr, unsigned int &targetPosX,
                             unsigned int &targetPosY);

  void print();

  void appendTo(router_table &other);

private:
  sc_dt::uint64 **m;
  unsigned int numberOfActiveLines;
  unsigned int numberOfAllocatedLines;
  sc_dt::uint64 topMemoryAddress;
};
};

#endif
