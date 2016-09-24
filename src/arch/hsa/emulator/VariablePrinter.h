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

#ifndef ARCH_HSA_EMULATOR_VARIABLEPRINTER_H
#define ARCH_HSA_EMULATOR_VARIABLEPRINTER_H

#include <map>
#include <memory>
#include <string>
#include <iostream>

namespace HSA {

class Variable;

/// A VariablePinters prints variable list 
class VariablePrinter {
public:
  void PrintVariables(
      const std::map<std::string, std::unique_ptr<Variable>> *variables, 
      std::ostream &os = std::cout) const;
};

}  // namespace HSA

#endif  // ARCH_HSA_EMULATOR_VARIABLEPRINTER_H
