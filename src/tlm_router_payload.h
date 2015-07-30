//===-- tlm_router_payload.h - router payload definition --------*- C++ -*-===//
//
//                              ArchC Project
//
// This file is distributed under the GNU Affero General Public License v3.0.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the definition of the payload extension for TLM2 routers
/// which contains the source and target from each payload and the number of
/// hops where the payload passed.
///
//===----------------------------------------------------------------------===//
#ifndef TLM_ROUTER_PAYLOAD_H
#define TLM_ROUTER_PAYLOAD_H

#include <tlm.h>
using namespace tlm;

namespace tlm_router {
class router_payload : public tlm_extension<router_payload> {
public:
  router_payload(unsigned int initX, unsigned int initY, unsigned int targetX,
                 unsigned int targetY, unsigned int numberOfHops = 0);
  ~router_payload();

  unsigned int getTargetX();
  unsigned int getTargetY();
  unsigned int getInitX();
  unsigned int getInitY();

  void setNumberOfHops(unsigned int);
  void incNumberOfHops();
  unsigned int getNumberOfHops();

  void print();

  tlm_extension_base *clone() const;
  void copy_from(tlm_extension_base const &ext);

private:
  unsigned int tX;
  unsigned int tY;
  unsigned int iX;
  unsigned int iY;
  unsigned int hops;
};
};
/***********************************************************/

#endif
