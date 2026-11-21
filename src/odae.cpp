/***************************************************************************
 *            dynamics/odae.cpp
 *
 *  Copyright  2008-20  Alberto Casagrande, Pieter Collins
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

/** @file src/odae.cpp
 *  @brief Implement explicit ordinary equation declarations.
 */
#include "system/odae.hpp"

#include <stdexcept>

#include "ordinary_dynamics.hpp"
#include "system/system_function.hpp"

namespace Ariadne {
OrdinaryDifferentialAlgebraicEquation::OrdinaryDifferentialAlgebraicEquation(
    const List<DottedRealAssignment>& dynamics)
    : OrdinaryDifferentialAlgebraicEquation(dynamics, RealVariablesBox()) {}

OrdinaryDifferentialAlgebraicEquation::OrdinaryDifferentialAlgebraicEquation(
    const List<DottedRealAssignment>& dynamics, const List<RealAssignment>& auxiliary)
    : OrdinaryDifferentialAlgebraicEquation(dynamics, RealVariablesBox(), std::nullopt, auxiliary) {}

OrdinaryDifferentialAlgebraicEquation::OrdinaryDifferentialAlgebraicEquation(
    const List<DottedRealAssignment>& dynamics, const RealVariablesBox& constant_parameters,
    std::optional<RealVariable> time, const List<RealAssignment>& auxiliary)
    : _dynamics(dynamics), _auxiliary(auxiliary), _time(std::move(time)),
      _parameter_space(Detail::variables_space(constant_parameters)),
      _parameters(Detail::variables_domain(constant_parameters)),
      _argument_space(Detail::ordinary_argument_space(state_space(), _time,
                                                      _parameter_space, RealSpace(), auxiliary_space())) {
    Set<UntypedVariable> allowed;
    for (SizeType i = 0u; i != _argument_space.size(); ++i)
        allowed.insert(_argument_space[i]);
    for (const auto& variable: left_hand_sides(auxiliary))
        allowed.insert(variable);
    auto validate = [&](const RealExpression& expression) {
        for (const auto& variable: expression.arguments()) {
            if (not allowed.contains(variable)) {
                throw std::invalid_argument("Undeclared variable in ordinary equation dynamics or auxiliary output");
            }
        }
    };
    for (const auto& assignment: dynamics)
        validate(assignment.rhs);
    for (const auto& assignment: auxiliary)
        validate(assignment.rhs);
    _dynamic_function = make_dynamic_function(_argument_space, auxiliary, dynamics);
    _auxiliary_function = make_auxiliary_function(_argument_space, auxiliary);
}

OrdinaryDifferentialAlgebraicEquation* OrdinaryDifferentialAlgebraicEquation::clone() const { return new OrdinaryDifferentialAlgebraicEquation(*this); }
DimensionType OrdinaryDifferentialAlgebraicEquation::dimension() const { return _dynamic_function.result_size(); }
const EffectiveVectorMultivariateFunction& OrdinaryDifferentialAlgebraicEquation::function() const { return _dynamic_function; }
const EffectiveVectorMultivariateFunction& OrdinaryDifferentialAlgebraicEquation::auxiliary_function() const { return _auxiliary_function; }
RealSpace OrdinaryDifferentialAlgebraicEquation::state_space() const { return RealSpace(left_hand_sides(_dynamics)); }
RealSpace OrdinaryDifferentialAlgebraicEquation::auxiliary_space() const { return RealSpace(left_hand_sides(_auxiliary)); }
RealSpace OrdinaryDifferentialAlgebraicEquation::state_auxiliary_space() const { return join(state_space(), auxiliary_space()); }
const RealSpace& OrdinaryDifferentialAlgebraicEquation::argument_space() const { return _argument_space; }
const RealSpace& OrdinaryDifferentialAlgebraicEquation::parameter_space() const { return _parameter_space; }
const BoxDomainType& OrdinaryDifferentialAlgebraicEquation::constant_parameter_domain() const { return _parameters; }
bool OrdinaryDifferentialAlgebraicEquation::has_explicit_time() const { return _time.has_value(); }
const std::optional<RealVariable>& OrdinaryDifferentialAlgebraicEquation::time_variable() const { return _time; }
OutputStream& operator<<(OutputStream& os, const OrdinaryDifferentialAlgebraicEquation& model) {
    return os << "OrdinaryDifferentialAlgebraicEquation( function=" << model.function()
              << ", auxiliary=" << model.auxiliary_function() << ", parameters=" << model.constant_parameter_domain() << " )";
}
} // namespace Ariadne
