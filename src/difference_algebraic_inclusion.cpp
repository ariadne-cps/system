/***************************************************************************
 *            difference_algebraic_inclusion.cpp
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

/** @file src/difference_algebraic_inclusion.cpp
 *  @brief Structured bounded-input difference inclusion declarations.
 */

#include "system/difference_algebraic_inclusion.hpp"

#include <stdexcept>

#include "ordinary_dynamics.hpp"
#include "system/system_function.hpp"

namespace Ariadne {

DifferenceAlgebraicInclusion::DifferenceAlgebraicInclusion(
    const List<PrimedRealAssignment>& updates, const RealVariablesBox& inputs)
    : DifferenceAlgebraicInclusion(updates, List<RealAssignment>(), inputs) {}

DifferenceAlgebraicInclusion::DifferenceAlgebraicInclusion(
    const List<PrimedRealAssignment>& updates, const List<RealAssignment>& auxiliary,
    const RealVariablesBox& inputs)
    : _updates(updates), _auxiliary(auxiliary),
      _states(left_hand_sides(updates)), _auxiliary_space(left_hand_sides(auxiliary)),
      _input_space(Detail::variables_space(inputs)), _inputs(Detail::variables_domain(inputs)),
      _argument_space(Detail::ordinary_argument_space(_states, std::nullopt, RealSpace(),
                                                      _input_space, _auxiliary_space)) {
    if (_input_space.size() == 0u)
        throw std::invalid_argument("DifferenceAlgebraicInclusion requires a nonempty input box");
    if (!unique_elements(left_hand_sides(_updates)))
        throw std::invalid_argument("Difference inclusion defines a next-state variable more than once");
    if (!unique_elements(left_hand_sides(_auxiliary)))
        throw std::invalid_argument("Difference inclusion defines an auxiliary variable more than once");
    Set<UntypedVariable> available;
    for (const auto& variable: _argument_space.variables())
        available.insert(variable);
    for (const auto& variable: _auxiliary_space.variables())
        available.insert(variable);
    Set<UntypedVariable> used;
    for (const auto& assignment: _updates)
        used.adjoin(assignment.rhs.arguments());
    for (const auto& assignment: _auxiliary)
        used.adjoin(assignment.rhs.arguments());
    if (!subset(used, available))
        throw std::invalid_argument("Difference inclusion refers to undeclared state, input or auxiliary variables");
    for (const auto& variable: _input_space.variables())
        if (!used.contains(variable))
            throw std::invalid_argument("Difference inclusion declares an unused bounded input");

    _update_function = make_reset_function(_argument_space, _auxiliary, _updates);
    _auxiliary_function = make_auxiliary_function(_argument_space, _auxiliary);
}

OutputStream& operator<<(OutputStream& os, const DifferenceAlgebraicInclusion& model) {
    return os << "DifferenceAlgebraicInclusion( updates=" << model.updates()
              << ", auxiliary=" << model.auxiliary_assignments()
              << ", inputs=" << model.inputs() << " )";
}

} // namespace Ariadne
