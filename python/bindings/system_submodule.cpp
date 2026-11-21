/***************************************************************************
 *            python/bindings/system_submodule.cpp
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

/** @file python/bindings/system_submodule.cpp
 *  @brief Expose System classes and functions to Python.
 */

#include <vector>

#include "pybind11.hpp"
#include "system/discrete_event.hpp"
#include "system/hybrid_automaton.hpp"
#include "system/iterated_map.hpp"
#include "system/odae.hpp"
#include "system/odai.hpp"

#include "function-utilities.hpp"

using namespace Ariadne;

namespace {
void export_interval_taylor_fields(pybind11::module& module) {
    using Scalar = ValidatedIntervalTaylorFunctionModel<FloatDP>;
    using Field = OrdinaryDifferentialAlgebraicInclusion::IntervalTaylorMultifunction;
    using ScalarPatch = ScaledFunctionPatch<ValidatedIntervalTaylorModel<FloatDP>>;
    pybind11::class_<Scalar>(module, "ValidatedIntervalTaylorFunctionModelDP")
        .def_static("constant", [](const BoxDomainType& domain, const FloatDPUpperInterval& value) {
            return Scalar(Scalar::constant(domain, value, ThresholdSweeper<FloatDP>(dp, 1e-12)));
        })
        .def_static("coordinate", [](const BoxDomainType& domain, SizeType index) {
            return Scalar(Scalar::coordinate(domain, index, ThresholdSweeper<FloatDP>(dp, 1e-12)));
        })
        .def("__add__", [](const Scalar& a, const Scalar& b) { return Scalar(a + b); })
        .def("__sub__", [](const Scalar& a, const Scalar& b) { return Scalar(a - b); })
        .def("__mul__", [](const Scalar& a, const Scalar& b) { return Scalar(a * b); })
        .def("__neg__", [](const Scalar& a) { return Scalar(-a); });
    pybind11::class_<Field>(module, "ValidatedVectorIntervalTaylorFunctionModelDP")
        .def(pybind11::init([](const std::vector<Scalar>& components) {
            if (components.empty())
                throw std::invalid_argument("A vector interval field needs at least one component");
            const Vector<ScalarPatch> patches(components.size(), [&](SizeType i) { return ScalarPatch(components[i]); });
            return Field(patches);
        }))
        .def("domain", &Field::domain)
        .def("argument_size", &Field::argument_size)
        .def("result_size", &Field::result_size)
        .def("evaluate", [](const Field& field, const BoxDomainType& states) {
            if (states.dimension() != field.argument_size() || not definitely(subset(states, field.domain()))) {
                throw std::invalid_argument("Interval field evaluation is outside its declared domain");
            }
            const Vector<FloatDPBounds> arguments(states.dimension(), [&](SizeType i) {
                return FloatDPBounds(states[i].lower_bound(), states[i].upper_bound());
            });
            return field(arguments);
        });
}
} // namespace

void system_submodule(pybind11::module& module) {
    export_interval_taylor_fields(module);
    pybind11::class_<DiscreteEvent> event_class(module, "DiscreteEvent");
    event_class.def(pybind11::init<>());
    event_class.def(pybind11::init<const StringType&>());
    event_class.def("name", &DiscreteEvent::name);
    event_class.def("__eq__", [](const DiscreteEvent& a, const DiscreteEvent& b) { return a == b; });
    event_class.def("__ne__", [](const DiscreteEvent& a, const DiscreteEvent& b) { return a != b; });
    event_class.def("__str__", &__cstr__<DiscreteEvent>);
    event_class.def("__repr__", &__cstr__<DiscreteEvent>);

    pybind11::class_<OrdinaryDifferentialAlgebraicInclusion> inclusion_class(module, "OrdinaryDifferentialAlgebraicInclusion");
    inclusion_class.def(pybind11::init([](
                                           const std::vector<DottedRealAssignment>& dynamics, const RealVariablesBox& inputs) {
        DottedRealAssignments assignments;
        for (const auto& equation: dynamics) {
            assignments.append(equation);
        }
        return OrdinaryDifferentialAlgebraicInclusion(assignments, inputs);
    }));
    inclusion_class.def(pybind11::init([](const std::vector<DottedRealAssignment>& dynamics,
                                          const RealVariablesBox& inputs, const RealVariablesBox& parameters, std::optional<RealVariable> time) {
                            return OrdinaryDifferentialAlgebraicInclusion(DottedRealAssignments(dynamics), inputs, parameters, time);
                        }),
                        pybind11::arg("dynamics"), pybind11::arg("inputs"), pybind11::arg("constant_parameters") = RealVariablesBox(), pybind11::arg("time") = pybind11::none());
    inclusion_class.def(pybind11::init<const RealSpace&, const OrdinaryDifferentialAlgebraicInclusion::IntervalTaylorMultifunction&>());
    inclusion_class.def("state_space", &OrdinaryDifferentialAlgebraicInclusion::state_space, pybind11::return_value_policy::copy);
    inclusion_class.def("argument_space", &OrdinaryDifferentialAlgebraicInclusion::argument_space, pybind11::return_value_policy::copy);
    inclusion_class.def("parameter_space", &OrdinaryDifferentialAlgebraicInclusion::parameter_space, pybind11::return_value_policy::copy);
    inclusion_class.def("constant_parameter_domain", &OrdinaryDifferentialAlgebraicInclusion::constant_parameter_domain, pybind11::return_value_policy::copy);
    inclusion_class.def("has_explicit_time", &OrdinaryDifferentialAlgebraicInclusion::has_explicit_time);
    inclusion_class.def("time_variable", &OrdinaryDifferentialAlgebraicInclusion::time_variable, pybind11::return_value_policy::copy);
    inclusion_class.def("has_input_function", &OrdinaryDifferentialAlgebraicInclusion::has_input_function);
    inclusion_class.def("function", &OrdinaryDifferentialAlgebraicInclusion::function, pybind11::return_value_policy::copy);
    inclusion_class.def("inputs", &OrdinaryDifferentialAlgebraicInclusion::inputs, pybind11::return_value_policy::copy);
    inclusion_class.def("interval_taylor_multifunction", &OrdinaryDifferentialAlgebraicInclusion::interval_taylor_multifunction, pybind11::return_value_policy::copy);
    inclusion_class.def("dimension", &OrdinaryDifferentialAlgebraicInclusion::dimension);
    inclusion_class.def("number_of_inputs", &OrdinaryDifferentialAlgebraicInclusion::number_of_inputs);
    inclusion_class.def("is_input_affine", &OrdinaryDifferentialAlgebraicInclusion::is_input_affine);
    inclusion_class.def("is_input_additive", &OrdinaryDifferentialAlgebraicInclusion::is_input_additive);
    inclusion_class.def("__str__", &__cstr__<OrdinaryDifferentialAlgebraicInclusion>);

    pybind11::class_<HybridAutomaton> automaton_class(module, "HybridAutomaton");
    automaton_class.def(pybind11::init<>());
    automaton_class.def("new_mode", [](HybridAutomaton& a, DiscreteLocation q) { a.new_mode(q); });
    automaton_class.def("new_mode", [](HybridAutomaton& a, DiscreteLocation q,
                                       const std::vector<DottedRealAssignment>& dynamics) {
        a.new_mode(q, List<DottedRealAssignment>(dynamics));
    });
    automaton_class.def("has_mode", &HybridAutomaton::has_mode);
    // Expose the unguarded discrete transition overload, with no reset.
    // This form is intended for systems without continuous state variables.
    automaton_class.def("new_transition",
                        [](HybridAutomaton& a, DiscreteLocation source, DiscreteEvent event, DiscreteLocation target) {
                            a.new_transition(source, event, target);
                        });
    automaton_class.def("has_transition", &HybridAutomaton::has_transition);
    automaton_class.def("target", &HybridAutomaton::target);
    automaton_class.def("number_of_modes",
                        [](const HybridAutomaton& a) { return a.locations().size(); });
    automaton_class.def("__str__", &__cstr__<HybridAutomaton>);
    automaton_class.def("__repr__", &__cstr__<HybridAutomaton>);

    // Construct system models from the symbolic assignments already bound by
    // symbolic_submodule.  The vector conversion is handled by pybind11.
    pybind11::class_<OrdinaryDifferentialAlgebraicEquation> odae_class(module, "OrdinaryDifferentialAlgebraicEquation");
    odae_class.def(pybind11::init([](const std::vector<DottedRealAssignment>& dynamics) {
        return OrdinaryDifferentialAlgebraicEquation(List<DottedRealAssignment>(dynamics));
    }));
    odae_class.def(pybind11::init([](const std::vector<DottedRealAssignment>& dynamics,
                                     const RealVariablesBox& parameters, std::optional<RealVariable> time, const std::vector<RealAssignment>& auxiliary) {
                       return OrdinaryDifferentialAlgebraicEquation(List<DottedRealAssignment>(dynamics), parameters, time, List<RealAssignment>(auxiliary));
                   }),
                   pybind11::arg("dynamics"), pybind11::arg("constant_parameters") = RealVariablesBox(), pybind11::arg("time") = pybind11::none(), pybind11::arg("auxiliary") = std::vector<RealAssignment>());
    odae_class.def(pybind11::init([](const std::vector<DottedRealAssignment>& dynamics, const std::vector<RealAssignment>& auxiliary) {
        return OrdinaryDifferentialAlgebraicEquation(List<DottedRealAssignment>(dynamics), List<RealAssignment>(auxiliary));
    }));
    odae_class.def("state_space", &OrdinaryDifferentialAlgebraicEquation::state_space);
    odae_class.def("argument_space", &OrdinaryDifferentialAlgebraicEquation::argument_space, pybind11::return_value_policy::copy);
    odae_class.def("auxiliary_space", &OrdinaryDifferentialAlgebraicEquation::auxiliary_space);
    odae_class.def("parameter_space", &OrdinaryDifferentialAlgebraicEquation::parameter_space, pybind11::return_value_policy::copy);
    odae_class.def("constant_parameter_domain", &OrdinaryDifferentialAlgebraicEquation::constant_parameter_domain, pybind11::return_value_policy::copy);
    odae_class.def("has_explicit_time", &OrdinaryDifferentialAlgebraicEquation::has_explicit_time);
    odae_class.def("time_variable", &OrdinaryDifferentialAlgebraicEquation::time_variable, pybind11::return_value_policy::copy);
    odae_class.def("function", &OrdinaryDifferentialAlgebraicEquation::function, pybind11::return_value_policy::copy);
    odae_class.def("auxiliary_function", &OrdinaryDifferentialAlgebraicEquation::auxiliary_function, pybind11::return_value_policy::copy);
    odae_class.def("dimension", &OrdinaryDifferentialAlgebraicEquation::dimension);
    odae_class.def("__str__", &__cstr__<OrdinaryDifferentialAlgebraicEquation>);

    pybind11::class_<IteratedMap> iterated_map_class(module, "IteratedMap");
    iterated_map_class.def(pybind11::init([](const std::vector<PrimedRealAssignment>& updates) {
        return IteratedMap(List<PrimedRealAssignment>(updates));
    }));
    iterated_map_class.def("dimension", &IteratedMap::dimension);
    iterated_map_class.def("__str__", &__cstr__<IteratedMap>);
}
