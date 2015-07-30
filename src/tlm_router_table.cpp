//===-- tlm_router_table.cpp - router table implementation ------*- C++ -*-===//
//
//                              ArchC Project
//
// This file is distributed under the GNU Affero General Public License v3.0.
// See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains the implementation of the router table class to routers,
/// which will be used to memory mapping.
///
//===----------------------------------------------------------------------===//
#include "tlm_router_table.h"

using tlm_router::router_table;

router_table::router_table(unsigned int lines = 0) {
  this->m = (sc_dt::uint64 **)malloc(lines * sizeof(sc_dt::uint64 *));
  for (int i = 0; i < lines; i++)
    this->m[i] = (sc_dt::uint64 *)malloc(4 * sizeof(sc_dt::uint64));
  this->numberOfAllocatedLines = lines;
  this->topMemoryAddress = 0;
  this->numberOfActiveLines = 0;
}

router_table::router_table() {
  this->m = NULL;
  this->numberOfAllocatedLines = 0;
  this->topMemoryAddress = 0;
  this->numberOfActiveLines = 0;
}

router_table::~router_table() {
  unsigned int i = this->numberOfAllocatedLines;
  while (i--)
    free(this->m[i]);
  free(m);
}

void router_table::newEntryAbsolute(unsigned int i, unsigned int j,
                                    sc_dt::uint64 start, sc_dt::uint64 size) {
  if (this->numberOfActiveLines == this->numberOfAllocatedLines) {
    this->numberOfAllocatedLines++;
    this->m = (sc_dt::uint64 **)realloc(this->m, this->numberOfAllocatedLines *
                                                     sizeof(sc_dt::uint64 *));
    this->m[numberOfActiveLines] =
        (sc_dt::uint64 *)malloc(4 * sizeof(sc_dt::uint64));
  }

  m[numberOfActiveLines][0] = start;
  m[numberOfActiveLines][1] = i;
  m[numberOfActiveLines][2] = j;
  m[numberOfActiveLines][3] = size;

  sc_dt::uint64 top = start + size;
  if (top > this->topMemoryAddress)
    this->topMemoryAddress = top;

  numberOfActiveLines++;
}

void router_table::newEntry(unsigned int i, unsigned int j,
                            sc_dt::uint64 size) {
  this->newEntryAbsolute(i, j, this->topMemoryAddress, size);
}

void router_table::print() {
  printf("Number of Active Lines in the table of routs: %d \n",
         numberOfActiveLines);
  for (unsigned int i = 0; i < this->numberOfActiveLines; i++) {
    printf("[%u]: 0x%llx - 0x%llx (%03llu, %03llu)\n ", i, m[i][0],
           m[i][0] + m[i][3], m[i][1], m[i][2]);
  }
}

void router_table::appendTo(router_table &other) {
  for (int i = 0; i < this->numberOfActiveLines; i++)
    other.newEntryAbsolute(m[i][1], m[i][2], m[i][0], m[i][3]);
}

void router_table::returnsTargetPosition(sc_dt::uint64 addr,
                                         unsigned int &targetPosX,
                                         unsigned int &targetPosY) {
  unsigned int i = this->numberOfActiveLines;

  if (addr >= this->topMemoryAddress) {
    fprintf(stderr, "Address (0x%llx) outside the range (0x%llx - 0x%llx)",
            addr, (unsigned long long)0, this->topMemoryAddress);
    exit(1);
  }

  while (i--) {
    sc_dt::uint64 offset = addr - m[i][0];
    if (offset >= 0 && offset < m[i][3]) {
      targetPosX = m[i][1];
      targetPosY = m[i][2];
      return;
    }
  }

  fprintf(stderr, "Invalid Address (0x%llx)", addr);
  exit(1);
}
