/***************************************************************************
 *            dynamics/odae.hpp
 *
 *  Copyright  2004-20  Alberto Casagrande, Pieter Collins
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

/** @file system/odae.hpp
 *  @brief Ordinary differential equations with explicit algebraic outputs.
 */
#ifndef ARIADNE_ODAE_HPP
#define ARIADNE_ODAE_HPP

#include <optional>

#include "function/function.hpp"
#include "geometry/set_interface.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

#include "expression-set/expression_set.hpp"

namespace Ariadne {

/** @brief A declaration of explicit ordinary differential dynamics.
 *  Auxiliary assignments are explicit outputs, not implicit DAE constraints.
 *  Function arguments are [state, optional time, constant parameters].
 *  Canonical TimeVariable is recognized when undeclared in another role.
 *  Time and parameters are not state variables. This model chooses no solver.
 */
class OrdinaryDifferentialAlgebraicEquation {
    friend class ContinuousTimeHybridAutomaton;

  public:
    using TimeType = Real;
    using RealType = Real;
    using StateSpaceType = EuclideanSpace;

    explicit OrdinaryDifferentialAlgebraicEquation(const List<DottedRealAssignment>& dynamics);
    OrdinaryDifferentialAlgebraicEquation(const List<DottedRealAssignment>& dynamics,
                                          const List<RealAssignment>& auxiliary);
    OrdinaryDifferentialAlgebraicEquation(const List<DottedRealAssignment>& dynamics,
                                          const RealVariablesBox& constant_parameters,
                                          std::optional<RealVariable> time = std::nullopt,
                                          const List<RealAssignment>& auxiliary = {});

    OrdinaryDifferentialAlgebraicEquation* clone() const;
    DimensionType dimension() const;
    const EffectiveVectorMultivariateFunction& function() const;
    const EffectiveVectorMultivariateFunction& auxiliary_function() const;
    RealSpace state_space() const;
    RealSpace auxiliary_space() const;
    RealSpace state_auxiliary_space() const;
    const RealSpace& argument_space() const;
    const RealSpace& parameter_space() const;
    const BoxDomainType& constant_parameter_domain() const;
    bool has_explicit_time() const;
    const std::optional<RealVariable>& time_variable() const;

    friend OutputStream& operator<<(OutputStream&, const OrdinaryDifferentialAlgebraicEquation&);

  private:
    List<DottedRealAssignment> _dynamics;
    List<RealAssignment> _auxiliary;
    std::optional<RealVariable> _time;
    RealSpace _parameter_space;
    BoxDomainType _parameters;
    RealSpace _argument_space;
    EffectiveVectorMultivariateFunction _dynamic_function;
    EffectiveVectorMultivariateFunction _auxiliary_function;
};
} // namespace Ariadne
#endif
