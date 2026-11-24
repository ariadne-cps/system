/***************************************************************************
 *            difference_algebraic_equation.hpp
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

/** @file system/difference_algebraic_equation.hpp
 *  @brief Explicit algebraic outputs and deterministic discrete-time updates.
 */

#ifndef ARIADNE_DIFFERENCE_ALGEBRAIC_EQUATION_HPP
#define ARIADNE_DIFFERENCE_ALGEBRAIC_EQUATION_HPP

#include "function/function.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

namespace Ariadne {

/** @brief Deterministic discrete-time dynamics with explicit algebraic outputs.
 *  @details A state update x[n+1] = f(x[n], a[n]) and explicit algebraic
 *  assignments a[n] = g(x[n]) define an autonomous discrete-time model.
 *  Algebraic outputs are assignments, not implicit equations to be solved.
 *  Symbolic assignments and compiled functions are owned directly by
 *  the equation; no legacy iterated-map wrapper is exposed.
 *  Inputs, time-varying parameters and set-valued updates are not modelled
 *  by this deterministic contract.
 */
class DifferenceAlgebraicEquation {
  public:
    using TimeType = Integer;
    using RealType = Real;
    using StateSpaceType = EuclideanSpace;

    explicit DifferenceAlgebraicEquation(const List<PrimedRealAssignment>& updates);
    DifferenceAlgebraicEquation(const List<PrimedRealAssignment>& updates,
                                 const List<RealAssignment>& auxiliary);

    /** @brief Clone this declaration, preserving its explicit assignments. */
    DifferenceAlgebraicEquation* clone() const { return new DifferenceAlgebraicEquation(*this); }

    /** @brief State update function after substitution of auxiliary outputs. */
    const EffectiveVectorMultivariateFunction& update_function() const { return _update_function; }
    /** @brief Alias for the state update function. */
    const EffectiveVectorMultivariateFunction& function() const { return _update_function; }
    /** @brief Explicit auxiliary output function. */
    const EffectiveVectorMultivariateFunction& auxiliary_function() const { return _auxiliary_function; }

    /** @brief Symbolic next-state assignments. */
    const List<PrimedRealAssignment>& updates() const { return _updates; }
    /** @brief Explicit algebraic assignments, not implicit constraints. */
    const List<RealAssignment>& auxiliary_assignments() const { return _auxiliary; }

    DimensionType dimension() const { return _update_function.result_size(); }
    RealSpace state_space() const { return RealSpace(left_hand_sides(_updates)); }
    RealSpace auxiliary_space() const { return RealSpace(left_hand_sides(_auxiliary)); }
    RealSpace state_auxiliary_space() const { return join(state_space(), auxiliary_space()); }

    friend OutputStream& operator<<(OutputStream&, const DifferenceAlgebraicEquation&);

  private:
    List<PrimedRealAssignment> _updates;
    List<RealAssignment> _auxiliary;
    EffectiveVectorMultivariateFunction _update_function;
    EffectiveVectorMultivariateFunction _auxiliary_function;
};

} // namespace Ariadne

#endif // ARIADNE_DIFFERENCE_ALGEBRAIC_EQUATION_HPP
