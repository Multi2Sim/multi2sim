/*
 *  Multi2Sim
 *  Copyright (C) 2016  Yifan Sun (yifansun@coe.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <iomanip>

#include <arch/hsa/disassembler/AsmService.h>

#include "VariablePrinter.h"
#include "Variable.h"

namespace HSA{

void VariablePrinter::PrintVariables(
    const std::map<std::string, std::unique_ptr<Variable>> *variables,
    std::ostream &os) const {
  for (auto it = variables->begin(); it != variables->end(); it++) {
    auto var = it->second.get();
    if (var->isInput()) {
      os << AsmService::TypeToString(var->getType()) << " ";
      os << var->getName();
      os << ", ";
    } else {
      os << "\t\t";
      os << AsmService::TypeToString(var->getType()) << " ";
      os << var->getName();
      os << "[" << var->getDim() << "]";
      os << " (0x" << std::setfill('0') << std::setw(16) << std::hex
          << var->getAddress() << ") ";
      os << "\n";
    }
  }
}

}  // namespace HSA
