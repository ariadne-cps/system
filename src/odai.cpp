/***************************************************************************
 *            dynamics/odai.cpp
 *
 *  Copyright  2008-20  Luca Geretti
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

/** @file src/odai.cpp
 *  @brief Implement differential inclusion dynamics.
 */

#include "system/odai.hpp"

#include <limits>

#include "function/symbolic_function.hpp"
#include "ordinary_dynamics.hpp"

#include "expression-set/expression_set.hpp"

namespace Ariadne {
namespace {

// Formula variable indices are Nat, while container sizes are SizeType.
static Nat formula_index(SizeType index) {
    return static_cast<Nat>(index);
}

List<Nat> input_indices(SizeType num_variables, SizeType num_inputs) {
    List<Nat> result;
    for (SizeType i: range(num_variables, num_variables + num_inputs)) {
        result.append(formula_index(i));
    }
    return result;
}

Pair<CoordinateFormulaPair, ExactIntervalType> centered_coordinate_transformation(const Nat& i, const ExactIntervalType& bounds) {
    if (bounds.lower_bound() == -bounds.upper_bound()) {
        return Pair<CoordinateFormulaPair, ExactIntervalType>({i, EffectiveFormula::coordinate(i)}, bounds);
    } else {
        return Pair<CoordinateFormulaPair, ExactIntervalType>({i, EffectiveFormula::coordinate(i) + EffectiveFormula::constant(EffectiveNumber(bounds.midpoint()))}, ExactIntervalType(cast_exact(bounds.lower_bound() - bounds.midpoint()), cast_exact(bounds.upper_bound() - bounds.midpoint())));
    }
}

Pair<CoordinateFormulaPairs, BoxDomainType> centered_coordinates_transformation(SizeType dim, const BoxDomainType& inputs) {
    CoordinateFormulaPairs assignments;
    BoxDomainType new_bounds(inputs.size());
    for (auto i: range(inputs.size())) {
        auto tr = centered_coordinate_transformation(formula_index(i + dim), inputs[i]);
        assignments.push_back(tr.first);
        new_bounds[i] = tr.second;
    }
    return Pair<CoordinateFormulaPairs, BoxDomainType>(assignments, new_bounds);
}

Void incorporate_additive_inputs_coefficients(Vector<EffectiveFormula>& transformed_formulae, BoxDomainType& transformed_inputs, SizeType input_offset) {
    SizeType n = transformed_formulae.size();
    SizeType m = transformed_inputs.size();
    for (SizeType j: range(m)) {
        for (SizeType i: range(n)) {
            EffectiveFormula der = simplify(derivative(transformed_formulae[i], formula_index(input_offset + j)));
            if (not identical(der, EffectiveFormula::zero()) and not identical(der, EffectiveFormula::constant(1))) {
                transformed_inputs[j] = cast_exact(transformed_inputs[j] * der.val());
                transformed_formulae[i] = simplify(substitute(transformed_formulae[i], formula_index(input_offset + j), EffectiveFormula::zero()) + EffectiveFormula::coordinate(formula_index(input_offset + j)));
            }
        }
    }
}

} // namespace

MissingInputException::MissingInputException(const String& message) : std::runtime_error(message) {}
UnusedInputException::UnusedInputException(const String& message) : std::runtime_error(message) {}

Void OrdinaryDifferentialAlgebraicInclusion::_transform_and_assign(
    const EffectiveVectorMultivariateFunction& function, const BoxDomainType& inputs) {
    const auto& ff = dynamic_handle_extract<const EffectiveVectorFormulaFunction>(function);
    const SizeType offset = function.argument_size() - inputs.size();
    auto transformation = centered_coordinates_transformation(offset, inputs);
    auto transformed_inputs = transformation.second;
    auto formulae = substitute(ff.formulae(), transformation.first);
    if (is_additive_in(formulae, input_indices(offset, inputs.size()))) {
        incorporate_additive_inputs_coefficients(formulae, transformed_inputs, offset);
    }
    _field = EffectiveVectorFormulaFunction(function.argument_size(), formulae);
    _inputs = transformed_inputs;
}

OrdinaryDifferentialAlgebraicInclusion::OrdinaryDifferentialAlgebraicInclusion(
    const DottedRealAssignments& dynamics, const RealVariablesBox& inputs)
    : OrdinaryDifferentialAlgebraicInclusion(dynamics, inputs, RealVariablesBox()) {}

OrdinaryDifferentialAlgebraicInclusion::OrdinaryDifferentialAlgebraicInclusion(
    const DottedRealAssignments& dynamics, const RealVariablesBox& inputs,
    const RealVariablesBox& constant_parameters, std::optional<RealVariable> time)
    : _states(left_hand_sides(dynamics)), _time(std::move(time)),
      _parameter_space(Detail::variables_space(constant_parameters)),
      _parameters(Detail::variables_domain(constant_parameters)),
      _input_space(Detail::variables_space(inputs)),
      _argument_space(Detail::ordinary_argument_space(_states, _time, _parameter_space, _input_space)) {
    Set<UntypedVariable> arguments;
    for (const auto& expression: right_hand_sides(dynamics))
        arguments.adjoin(expression.arguments());
    for (const auto& variable: inputs.variables()) {
        if (not arguments.contains(variable)) {
            throw UnusedInputException("Declared input does not occur in ordinary inclusion dynamics");
        }
    }
    Set<UntypedVariable> allowed;
    for (SizeType i = 0u; i != _argument_space.size(); ++i)
        allowed.insert(_argument_space[i]);
    for (const auto& variable: arguments) {
        if (not allowed.contains(variable)) {
            throw MissingInputException("Ordinary inclusion contains an undeclared state, time, parameter or input");
        }
    }
    _transform_and_assign(make_function(_argument_space,
                                        Vector<RealExpression>(right_hand_sides(dynamics))),
                          Detail::variables_domain(inputs));
}

OrdinaryDifferentialAlgebraicInclusion::OrdinaryDifferentialAlgebraicInclusion(
    const RealSpace& states, const IntervalTaylorMultifunction& field)
    : _field(field), _states(states), _parameters(0u), _inputs(0u), _argument_space(states) {
    // This is a field on physical states, not a multiflow with an extra time
    // coordinate. The model is owned; no temporary or external handle can dangle.
    Detail::ordinary_argument_space(states, std::nullopt, RealSpace(), RealSpace());
    if (field.argument_size() != states.size() || field.result_size() != states.size()) {
        throw std::invalid_argument("Interval Taylor inclusion field must match its declared state space");
    }
}

OrdinaryDifferentialAlgebraicInclusion* OrdinaryDifferentialAlgebraicInclusion::clone() const { return new OrdinaryDifferentialAlgebraicInclusion(*this); }
SizeType OrdinaryDifferentialAlgebraicInclusion::dimension() const { return _states.size(); }
SizeType OrdinaryDifferentialAlgebraicInclusion::number_of_inputs() const { return _inputs.size(); }
const RealSpace& OrdinaryDifferentialAlgebraicInclusion::state_space() const { return _states; }
const RealSpace& OrdinaryDifferentialAlgebraicInclusion::argument_space() const { return _argument_space; }
const RealSpace& OrdinaryDifferentialAlgebraicInclusion::input_space() const { return _input_space; }
const RealSpace& OrdinaryDifferentialAlgebraicInclusion::parameter_space() const { return _parameter_space; }
const BoxDomainType& OrdinaryDifferentialAlgebraicInclusion::constant_parameter_domain() const { return _parameters; }
bool OrdinaryDifferentialAlgebraicInclusion::has_explicit_time() const { return _time.has_value(); }
const std::optional<RealVariable>& OrdinaryDifferentialAlgebraicInclusion::time_variable() const { return _time; }
bool OrdinaryDifferentialAlgebraicInclusion::has_input_function() const { return std::holds_alternative<EffectiveVectorMultivariateFunction>(_field); }
const EffectiveVectorMultivariateFunction& OrdinaryDifferentialAlgebraicInclusion::function() const {
    const auto* field = std::get_if<EffectiveVectorMultivariateFunction>(&_field);
    if (field == nullptr)
        throw std::logic_error("Interval Taylor inclusion has no single-valued input function");
    return *field;
}
const BoxDomainType& OrdinaryDifferentialAlgebraicInclusion::inputs() const { return _inputs; }
const OrdinaryDifferentialAlgebraicInclusion::IntervalTaylorMultifunction& OrdinaryDifferentialAlgebraicInclusion::interval_taylor_multifunction() const {
    const auto* field = std::get_if<IntervalTaylorMultifunction>(&_field);
    if (field == nullptr)
        throw std::logic_error("Structured inclusion has no interval Taylor field representation");
    return *field;
}
ValidatedCompactVectorMultivariateMultifunction OrdinaryDifferentialAlgebraicInclusion::multifunction() const {
    return ValidatedCompactVectorMultivariateMultifunction(interval_taylor_multifunction());
}
Bool OrdinaryDifferentialAlgebraicInclusion::is_input_affine() const {
    return is_affine_in(function(), input_indices(_argument_space.size() - number_of_inputs(), number_of_inputs()));
}
Bool OrdinaryDifferentialAlgebraicInclusion::is_input_additive() const {
    return is_additive_in(function(), input_indices(_argument_space.size() - number_of_inputs(), number_of_inputs()));
}
EffectiveVectorMultivariateFunction OrdinaryDifferentialAlgebraicInclusion::noise_independent_component() const {
    const auto& ff = dynamic_handle_extract<const EffectiveVectorFormulaFunction>(function());
    CoordinateFormulaPairs substitutions;
    for (auto index: input_indices(_argument_space.size() - number_of_inputs(), number_of_inputs())) {
        substitutions.append({index, EffectiveFormula::zero()});
    }
    return EffectiveVectorFormulaFunction(_argument_space.size(), simplify(substitute(ff.formulae(), substitutions)));
}
Vector<EffectiveVectorMultivariateFunction> OrdinaryDifferentialAlgebraicInclusion::input_derivatives() const {
    const auto& ff = dynamic_handle_extract<const EffectiveVectorFormulaFunction>(function());
    Vector<EffectiveVectorMultivariateFunction> result(number_of_inputs());
    for (SizeType j = 0u; j != number_of_inputs(); ++j) {
        Vector<EffectiveFormula> formulae(dimension());
        for (SizeType i = 0u; i != dimension(); ++i) {
            formulae[i] = simplify(derivative(ff.formulae()[i], formula_index(_argument_space.size() - number_of_inputs() + j)));
        }
        result[j] = EffectiveVectorFormulaFunction(_argument_space.size(), formulae);
    }
    return result;
}
OutputStream& operator<<(OutputStream& os, const OrdinaryDifferentialAlgebraicInclusion& model) {
    os << "OrdinaryDifferentialAlgebraicInclusion( ";
    if (model.has_input_function())
        os << model.function() << ", inputs=" << model.inputs();
    else
        os << model.interval_taylor_multifunction();
    return os << ", parameters=" << model.constant_parameter_domain() << " )";
}

} // namespace Ariadne
