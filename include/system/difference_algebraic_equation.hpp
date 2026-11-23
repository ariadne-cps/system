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

#include "system/iterated_map.hpp"

namespace Ariadne {

/** @brief Deterministic discrete-time dynamics with explicit algebraic outputs.
 *  @details A state update x[n+1] = f(x[n], a[n]) and explicit algebraic
 *  assignments a[n] = g(x[n]) define an autonomous discrete-time model.
 *  Algebraic outputs are assignments, not implicit equations to be solved.
 *  This declaration stores an IteratedMap as its operational representation;
 *  symbolic assignments are retained by that map, without duplication.
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

    /** @brief Resulting deterministic iteration operator. */
    const IteratedMap& iterated_map() const { return _map; }
    /** @brief State update function after substitution of auxiliary outputs. */
    const EffectiveVectorMultivariateFunction& update_function() const { return _map.update_function(); }
    /** @brief Alias for the state update function. */
    const EffectiveVectorMultivariateFunction& function() const { return _map.function(); }
    /** @brief Explicit auxiliary output function. */
    const EffectiveVectorMultivariateFunction& auxiliary_function() const { return _map.auxiliary_function(); }

    /** @brief Symbolic next-state assignments. */
    const List<PrimedRealAssignment>& updates() const { return _map.updates(); }
    /** @brief Explicit algebraic assignments, not implicit constraints. */
    const List<RealAssignment>& auxiliary_assignments() const { return _map.auxiliary_assignments(); }

    DimensionType dimension() const { return _map.dimension(); }
    RealSpace state_space() const { return _map.state_space(); }
    RealSpace auxiliary_space() const { return _map.auxiliary_space(); }
    RealSpace state_auxiliary_space() const { return _map.state_auxiliary_space(); }

    friend OutputStream& operator<<(OutputStream&, const DifferenceAlgebraicEquation&);

  private:
    IteratedMap _map;
};

} // namespace Ariadne

#endif // ARIADNE_DIFFERENCE_ALGEBRAIC_EQUATION_HPP
