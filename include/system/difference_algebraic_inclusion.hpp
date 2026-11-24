/***************************************************************************
 *            difference_algebraic_inclusion.hpp
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

/** @file system/difference_algebraic_inclusion.hpp
 *  @brief Bounded-input difference inclusions with explicit algebraic outputs.
 */

#ifndef ARIADNE_DIFFERENCE_ALGEBRAIC_INCLUSION_HPP
#define ARIADNE_DIFFERENCE_ALGEBRAIC_INCLUSION_HPP

#include "expression-set/expression_set.hpp"
#include "function/function.hpp"
#include "symbolic/assignment.hpp"
#include "symbolic/space.hpp"

namespace Ariadne {

/** @brief A structured, set-valued discrete-time update declaration.
 *  @details At each discrete step x[n+1] = f(x[n],a[n],u[n]),
 *  a[n] = g(x[n],u[n]), where u[n] can independently take any value
 *  in the declared input box at that step. No input is frozen along a path.
 *  The exposed function has arguments [state, inputs]; it is not an
 *  autonomous IteratedMap and does not choose an input sample.
 *  Auxiliary equations are explicit assignments, not implicit constraints.
 *  Numerical image enclosure and alternative multifunction representations
 *  belong to later stages; this type declares no validated evolver.
 */
class DifferenceAlgebraicInclusion {
  public:
    using TimeType = Integer;
    using RealType = Real;
    using StateSpaceType = EuclideanSpace;

    DifferenceAlgebraicInclusion(const List<PrimedRealAssignment>& updates,
                                 const RealVariablesBox& inputs);
    DifferenceAlgebraicInclusion(const List<PrimedRealAssignment>& updates,
                                 const List<RealAssignment>& auxiliary,
                                 const RealVariablesBox& inputs);

    DifferenceAlgebraicInclusion* clone() const { return new DifferenceAlgebraicInclusion(*this); }

    DimensionType dimension() const { return _states.size(); }
    SizeType number_of_inputs() const { return _input_space.size(); }
    const RealSpace& state_space() const { return _states; }
    const RealSpace& auxiliary_space() const { return _auxiliary_space; }
    RealSpace state_auxiliary_space() const { return join(_states, _auxiliary_space); }
    const RealSpace& input_space() const { return _input_space; }
    const RealSpace& argument_space() const { return _argument_space; }
    const BoxDomainType& inputs() const { return _inputs; }
    const List<PrimedRealAssignment>& updates() const { return _updates; }
    const List<RealAssignment>& auxiliary_assignments() const { return _auxiliary; }

    /** @brief Input-parameterized next-state function; not a single-valued state map. */
    const EffectiveVectorMultivariateFunction& update_function() const { return _update_function; }
    const EffectiveVectorMultivariateFunction& function() const { return _update_function; }
    const EffectiveVectorMultivariateFunction& auxiliary_function() const { return _auxiliary_function; }

    friend OutputStream& operator<<(OutputStream&, const DifferenceAlgebraicInclusion&);

  private:
    List<PrimedRealAssignment> _updates;
    List<RealAssignment> _auxiliary;
    RealSpace _states;
    RealSpace _auxiliary_space;
    RealSpace _input_space;
    BoxDomainType _inputs;
    RealSpace _argument_space;
    EffectiveVectorMultivariateFunction _update_function;
    EffectiveVectorMultivariateFunction _auxiliary_function;
};

} // namespace Ariadne

#endif // ARIADNE_DIFFERENCE_ALGEBRAIC_INCLUSION_HPP
