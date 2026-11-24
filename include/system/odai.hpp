/***************************************************************************
 *            dynamics/odai.hpp
 *
 *  Copyright  2018-20  Luca Geretti
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

/** @file system/odai.hpp
 *  @brief Ordinary differential inclusions with explicit uncertainty semantics.
 */
#ifndef ARIADNE_ODAI_HPP
#define ARIADNE_ODAI_HPP

#include <optional>
#include <stdexcept>
#include <variant>

#include "function/function.hpp"
#include "function/taylor_multifunction.hpp"
#include "geometry/set_interface.hpp"
#include "symbolic/assignment.hpp"

#include "expression-set/expression_set.hpp"

namespace Ariadne {
class Grid;
class MissingInputException : public std::runtime_error {
  public:
    explicit MissingInputException(const String& message);
};
class UnusedInputException : public std::runtime_error {
  public:
    explicit UnusedInputException(const String& message);
};

/** @brief Explicit ordinary differential-inclusion declaration.
 *  Structured fields have arguments [state, optional time, constant parameters,
 *  free inputs]. Parameters stay fixed along a trajectory; inputs may vary
 *  measurably over time. The interval-Taylor form has pointwise set-valued
 *  semantics and does not silently freeze uncertain coefficients.
 *  Canonical TimeVariable is recognized when undeclared in another role.
 *  This type declares no solver, numerical lifting or implicit DAI constraints.
 */
class OrdinaryDifferentialAlgebraicInclusion {
  public:
    using TimeType = Real;
    using RealType = Real;
    using StateSpaceType = EuclideanSpace;
    using IntervalTaylorMultifunction = ValidatedVectorIntervalTaylorFunctionModel<FloatDP>;

    OrdinaryDifferentialAlgebraicInclusion(const DottedRealAssignments& dynamics,
                                           const RealVariablesBox& inputs);
    OrdinaryDifferentialAlgebraicInclusion(const DottedRealAssignments& dynamics,
                                           const RealVariablesBox& inputs, const RealVariablesBox& constant_parameters,
                                           std::optional<RealVariable> time = std::nullopt);
    OrdinaryDifferentialAlgebraicInclusion(const RealSpace& states,
                                           const IntervalTaylorMultifunction& field);
    virtual ~OrdinaryDifferentialAlgebraicInclusion() = default;
    virtual OrdinaryDifferentialAlgebraicInclusion* clone() const;

    SizeType dimension() const;
    SizeType number_of_inputs() const;
    const RealSpace& state_space() const;
    const RealSpace& argument_space() const;
    const RealSpace& input_space() const;
    const RealSpace& parameter_space() const;
    const BoxDomainType& constant_parameter_domain() const;
    bool has_explicit_time() const;
    const std::optional<RealVariable>& time_variable() const;
    Grid grid() const;

    bool has_input_function() const;
    /** @brief Normalised input coordinates, with state/time/parameters unchanged.
     *  This representation exists only for the structured constructor.
     */
    const EffectiveVectorMultivariateFunction& function() const;
    const BoxDomainType& inputs() const;
    /** @brief The owned interval model, including its domain and remainder. */
    const IntervalTaylorMultifunction& interval_taylor_multifunction() const;
    /** @brief Interval-Taylor mathematical view; does not certify a trajectory.
     *  Structured declarations retain their function/input-box pair instead.
     */
    ValidatedCompactVectorMultivariateMultifunction multifunction() const;

    Bool is_input_affine() const;
    Bool is_input_additive() const;
    EffectiveVectorMultivariateFunction noise_independent_component() const;
    Vector<EffectiveVectorMultivariateFunction> input_derivatives() const;
    friend OutputStream& operator<<(OutputStream&, const OrdinaryDifferentialAlgebraicInclusion&);

  private:
    Void _transform_and_assign(const EffectiveVectorMultivariateFunction&, const BoxDomainType&);
    std::variant<EffectiveVectorMultivariateFunction, IntervalTaylorMultifunction> _field;
    RealSpace _states;
    std::optional<RealVariable> _time;
    RealSpace _parameter_space;
    BoxDomainType _parameters;
    RealSpace _input_space;
    BoxDomainType _inputs;
    RealSpace _argument_space;
};
} // namespace Ariadne
#endif
