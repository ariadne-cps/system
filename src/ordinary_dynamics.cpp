/***************************************************************************
 *            src/ordinary_dynamics.cpp
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "ordinary_dynamics.hpp"

#include <stdexcept>

namespace Ariadne::Detail {
RealSpace variables_space(const RealVariablesBox& variables) {
    List<RealVariable> ordered;
    for (const auto& variable: variables.variables()) {
        ordered.append(variable);
    }
    return RealSpace(ordered);
}

BoxDomainType variables_domain(const RealVariablesBox& variables) {
    List<IntervalDomainType> bounds;
    for (const auto& variable: variables.variables()) {
        bounds.append(IntervalDomainType(
            cast_exact(variables[variable].lower_bound().get(dp)),
            cast_exact(variables[variable].upper_bound().get(dp))));
    }
    if (bounds.empty())
        return BoxDomainType(0u);
    return BoxDomainType(Vector<IntervalDomainType>(bounds));
}

RealSpace ordinary_argument_space(const RealSpace& states,
                                  const std::optional<RealVariable>& time, const RealSpace& parameters,
                                  const RealSpace& inputs, const RealSpace& auxiliary) {
    List<RealVariable> arguments;
    Set<Identifier> names;
    auto append = [&](const RealVariable& variable, bool argument) {
        if (names.contains(variable.name())) {
            throw std::invalid_argument("Ordinary dynamics variables must have distinct names and roles");
        }
        names.insert(variable.name());
        if (argument)
            arguments.append(variable);
    };
    for (SizeType i = 0u; i != states.size(); ++i)
        append(states[i], true);
    if (time)
        append(*time, true);
    for (SizeType i = 0u; i != parameters.size(); ++i)
        append(parameters[i], true);
    for (SizeType i = 0u; i != inputs.size(); ++i)
        append(inputs[i], true);
    for (SizeType i = 0u; i != auxiliary.size(); ++i)
        append(auxiliary[i], false);
    return RealSpace(arguments);
}
} // namespace Ariadne::Detail
