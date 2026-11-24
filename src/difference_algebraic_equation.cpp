/***************************************************************************
 *            difference_algebraic_equation.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne System.
 *
 *  Ariadne System is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne System is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file src/difference_algebraic_equation.cpp
 *  @brief Construct declarations of explicit discrete-time algebraic dynamics.
 */

#include "system/difference_algebraic_equation.hpp"
#include "system/system_function.hpp"

namespace Ariadne {

DifferenceAlgebraicEquation::DifferenceAlgebraicEquation(
    const List<PrimedRealAssignment>& updates)
    : DifferenceAlgebraicEquation(updates, List<RealAssignment>()) {}

DifferenceAlgebraicEquation::DifferenceAlgebraicEquation(
    const List<PrimedRealAssignment>& updates, const List<RealAssignment>& auxiliary)
    : _updates(updates), _auxiliary(auxiliary),
      _update_function(make_reset_function(left_hand_sides(updates), auxiliary, updates)),
      _auxiliary_function(make_auxiliary_function(left_hand_sides(updates), auxiliary)) {}

OutputStream& operator<<(OutputStream& os, const DifferenceAlgebraicEquation& model) {
    return os << "DifferenceAlgebraicEquation( updates=" << model.updates()
              << ", auxiliary=" << model.auxiliary_assignments() << " )";
}

} // namespace Ariadne
