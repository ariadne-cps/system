"""Smoke tests for real dynamical system definitions in the composed pyariadne_system extension."""
import unittest
from pyariadne_system import DiscreteEvent, DiscreteLocation, HybridAutomaton


def test_discrete_event():
    event = DiscreteEvent("tick")
    copy = DiscreteEvent("tick")
    other = DiscreteEvent("tock")
    assert event.name() == "tick"
    assert event == copy
    assert event != other
    assert str(event)


def test_hybrid_automaton_mode():
    automaton = HybridAutomaton()
    location = DiscreteLocation(1)
    assert not automaton.has_mode(location)
    automaton.new_mode(location)
    assert automaton.has_mode(location)
    assert str(automaton)


def test_unguarded_transition():
    idle, active = DiscreteLocation(0), DiscreteLocation(1)
    activate, deactivate = DiscreteEvent("activate"), DiscreteEvent("deactivate")
    automaton = HybridAutomaton()
    automaton.new_mode(idle)
    automaton.new_mode(active)
    automaton.new_transition(idle, activate, active)
    automaton.new_transition(active, deactivate, idle)

    assert automaton.number_of_modes() == 2
    assert automaton.has_transition(idle, activate)
    assert automaton.target(idle, activate) == active
    assert automaton.has_transition(active, deactivate)
    assert automaton.target(active, deactivate) == idle
    assert not automaton.has_transition(idle, deactivate)
    assert automaton.target(idle, deactivate) == idle


def test_odae_and_iterated_map():
    from pyariadne_system import RealVariable, OrdinaryDifferentialAlgebraicEquation, IteratedMap, dot, next
    x = RealVariable("x")
    field = OrdinaryDifferentialAlgebraicEquation([dot(x) << -x])
    mapping = IteratedMap([next(x) << x / 2])
    assert field.dimension() == 1
    assert mapping.dimension() == 1
    assert str(field) and str(mapping)

def test_odai_inputs():
    from pyariadne_system import OrdinaryDifferentialAlgebraicInclusion, RealVariable, RealVariablesBox, dot
    x, u = RealVariable("x"), RealVariable("u")
    model = OrdinaryDifferentialAlgebraicInclusion([dot(x) << (-x + u)], RealVariablesBox({u: (-1, 1)}))
    assert model.dimension() == 1
    assert model.number_of_inputs() == 1
    assert model.is_input_affine()
    assert model.is_input_additive()


def test_neural_set_dependency():
    from pyariadne_system import NeuralValidatedEvaluator, neural_validated_bounds
    assert callable(NeuralValidatedEvaluator)
    assert callable(neural_validated_bounds)


def test_ordinary_equation_time_parameters_and_auxiliary_output():
    from pyariadne_system import (OrdinaryDifferentialAlgebraicEquation,
        RealVariable, RealVariablesBox, RealSpace, dot, let)
    x, t, p, a = (RealVariable(name) for name in ("x", "t", "p", "a"))
    equation = OrdinaryDifferentialAlgebraicEquation(
        [dot(x) << a], RealVariablesBox({p: (1, 2)}), time=t,
        auxiliary=[let(a) << (x + t + p)])
    assert equation.dimension() == 1
    assert equation.state_space().size() == 1 and equation.state_space().contains(x)
    assert [equation.argument_space().index(v) for v in (x, t, p)] == [0, 1, 2]
    assert equation.auxiliary_space().size() == 1 and equation.auxiliary_space().contains(a)
    assert equation.function().argument_size() == 3
    assert equation.has_explicit_time()
    assert equation.auxiliary_function().result_size() == 1
    with unittest.TestCase().assertRaises(ValueError):
        OrdinaryDifferentialAlgebraicEquation([dot(x) << x], RealVariablesBox({}), time=x)


def test_ordinary_inclusion_distinguishes_fixed_parameters_and_inputs():
    from pyariadne_system import (OrdinaryDifferentialAlgebraicInclusion,
        RealVariable, RealVariablesBox, RealSpace, dot)
    x, t, p, u = (RealVariable(name) for name in ("x", "t", "p", "u"))
    inclusion = OrdinaryDifferentialAlgebraicInclusion(
        [dot(x) << (t + p + 2*u)], RealVariablesBox({u: (1, 2)}),
        RealVariablesBox({p: (1, 2)}), time=t)
    assert inclusion.dimension() == 1
    assert inclusion.state_space().size() == 1 and inclusion.state_space().contains(x)
    assert [inclusion.argument_space().index(v) for v in (x, t, p, u)] == [0, 1, 2, 3]
    assert inclusion.parameter_space().size() == 1 and inclusion.parameter_space().contains(p)
    assert inclusion.has_explicit_time()
    assert inclusion.has_input_function()
    assert inclusion.is_input_additive()
    assert inclusion.function().argument_size() == 4


def test_owned_interval_taylor_inclusion():
    import gc
    from pyariadne_system import (OrdinaryDifferentialAlgebraicInclusion,
        ValidatedIntervalTaylorFunctionModelDP, ValidatedVectorIntervalTaylorFunctionModelDP,
        RealVariable, RealSpace, BoxDomainType, FloatDPExactBox, FloatDPUpperInterval,
        FloatDPBounds, exact, dp, refines)
    x, y = RealVariable("x"), RealVariable("y")
    domain = BoxDomainType(FloatDPExactBox([{exact(-1): exact(1)}]))
    # The temporary function model is owned by the system declaration.
    inclusion = OrdinaryDifferentialAlgebraicInclusion(RealSpace([x]),
        ValidatedVectorIntervalTaylorFunctionModelDP([
            ValidatedIntervalTaylorFunctionModelDP.constant(domain, FloatDPUpperInterval("-1", "1", dp))]))
    gc.collect()
    assert inclusion.dimension() == 1
    assert not inclusion.has_input_function()
    assert not inclusion.has_explicit_time()
    field = inclusion.interval_taylor_multifunction()
    value = field.evaluate(FloatDPExactBox([{exact(0): exact(0)}]))
    assert refines(FloatDPBounds("-1", "1", dp), FloatDPBounds(value[0].lower_bound(), value[0].upper_bound()))
    with unittest.TestCase().assertRaises(RuntimeError):
        inclusion.function()
    with unittest.TestCase().assertRaises(ValueError):
        OrdinaryDifferentialAlgebraicInclusion(RealSpace([x, y]), field)
    del inclusion
    gc.collect()
    assert field.domain() == domain


def test_public_names_have_no_compatibility_aliases():
    import pyariadne_system as system
    assert hasattr(system, "OrdinaryDifferentialAlgebraicEquation")
    assert hasattr(system, "OrdinaryDifferentialAlgebraicInclusion")
    assert not hasattr(system, "VectorField")
    assert not hasattr(system, "DifferentialInclusion")
