//===-- tlm_router_payload.cpp - router payload implementation --*- C++ -*-===//
//
//                              ArchC Project
//
// This file is distributed under the GNU Affero General Public License v3.0.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation of the payload extension for TLM2
/// routers which contains the source and target from each payload and the
/// number of hops where the payload passed.
///
//===----------------------------------------------------------------------===//
#include "tlm_router_payload.h"

using tlm_router::router_payload;

router_payload::router_payload(unsigned int initX, unsigned int initY,
                               unsigned int targetX, unsigned int targetY,
                               unsigned int numberOfHops) {
  iX = initX;
  iY = initY;
  tX = targetX;
  tY = targetY;
  hops = numberOfHops;
}

router_payload::~router_payload() {}

unsigned int router_payload::getTargetX() { return tX; }

unsigned int router_payload::getTargetY() { return tY; }

unsigned int router_payload::getInitX() { return iX; }

unsigned int router_payload::getInitY() { return iY; }

void router_payload::setNumberOfHops(unsigned int hops) { this->hops = hops; }

void router_payload::incNumberOfHops() { hops++; }

unsigned int router_payload::getNumberOfHops() { return hops; }

void router_payload::print() {
  printf("Init %u,%u\n", iX, iY);
  printf("Target %u,%u\n", tX, tY);
  printf("Hops %u\n", hops);
}

tlm_extension_base *router_payload::clone() const {
  return new router_payload(iX, iY, tX, tY, hops);
}

void router_payload::copy_from(tlm_extension_base const &ext) {
  perror("Payload extension copy_from() isn't implemented - clone() can be "
         "used intead, if necessary.");
  exit(1);
}
