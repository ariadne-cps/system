#include "pybind11.hpp"
#include <vector>
#include "function-utilities.hpp"
#include "system/discrete_event.hpp"
#include "system/differential_inclusion.hpp"
#include "system/hybrid_automaton.hpp"
#include "system/iterated_map.hpp"
#include "system/vector_field.hpp"

using namespace Ariadne;

void system_submodule(pybind11::module& module) {
    pybind11::class_<DiscreteEvent> event_class(module, "DiscreteEvent");
    event_class.def(pybind11::init<>());
    event_class.def(pybind11::init<const StringType&>());
    event_class.def("name", &DiscreteEvent::name);
    event_class.def("__eq__", [](DiscreteEvent const& a, DiscreteEvent const& b) { return a == b; });
    event_class.def("__ne__", [](DiscreteEvent const& a, DiscreteEvent const& b) { return a != b; });
    event_class.def("__str__", &__cstr__<DiscreteEvent>);
    event_class.def("__repr__", &__cstr__<DiscreteEvent>);

    pybind11::class_<DifferentialInclusion> inclusion_class(module,"DifferentialInclusion");
    inclusion_class.def(pybind11::init([](
        std::vector<DottedRealAssignment> const& dynamics, RealVariablesBox const& inputs) {
        DottedRealAssignments assignments;
        for (auto const& equation : dynamics) { assignments.append(equation); }
        return DifferentialInclusion(assignments,inputs);
    }));
    inclusion_class.def("dimension",&DifferentialInclusion::dimension);
    inclusion_class.def("number_of_inputs",&DifferentialInclusion::number_of_inputs);
    inclusion_class.def("is_input_affine",&DifferentialInclusion::is_input_affine);
    inclusion_class.def("is_input_additive",&DifferentialInclusion::is_input_additive);
    inclusion_class.def("__str__",&__cstr__<DifferentialInclusion>);

    pybind11::class_<HybridAutomaton> automaton_class(module, "HybridAutomaton");
    automaton_class.def(pybind11::init<>());
    automaton_class.def("new_mode", [](HybridAutomaton& a, DiscreteLocation q) { a.new_mode(q); });
    automaton_class.def("new_mode", [](HybridAutomaton& a, DiscreteLocation q,
                                       std::vector<DottedRealAssignment> const& dynamics) {
        a.new_mode(q,List<DottedRealAssignment>(dynamics));
    });
    automaton_class.def("has_mode", &HybridAutomaton::has_mode);
    // Expose the unguarded discrete transition overload, with no reset.
    // This form is intended for systems without continuous state variables.
    automaton_class.def("new_transition",
        [](HybridAutomaton& a, DiscreteLocation source, DiscreteEvent event, DiscreteLocation target) {
            a.new_transition(source,event,target);
        });
    automaton_class.def("has_transition", &HybridAutomaton::has_transition);
    automaton_class.def("target", &HybridAutomaton::target);
    automaton_class.def("number_of_modes",
        [](HybridAutomaton const& a) { return a.locations().size(); });
    automaton_class.def("__str__", &__cstr__<HybridAutomaton>);
    automaton_class.def("__repr__", &__cstr__<HybridAutomaton>);
    
    // Construct system models from the symbolic assignments already bound by
    // symbolic_submodule.  The vector conversion is handled by pybind11.
    pybind11::class_<VectorField> vector_field_class(module,"VectorField");
    vector_field_class.def(pybind11::init([](std::vector<DottedRealAssignment> const& dynamics) {
        return VectorField(List<DottedRealAssignment>(dynamics));
    }));
    vector_field_class.def("dimension", &VectorField::dimension);
    vector_field_class.def("__str__", &__cstr__<VectorField>);

    pybind11::class_<IteratedMap> iterated_map_class(module,"IteratedMap");
    iterated_map_class.def(pybind11::init([](std::vector<PrimedRealAssignment> const& updates) {
        return IteratedMap(List<PrimedRealAssignment>(updates));
    }));
    iterated_map_class.def("dimension", &IteratedMap::dimension);
    iterated_map_class.def("__str__", &__cstr__<IteratedMap>);

}
