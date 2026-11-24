#!/usr/bin/env python3
"""The same two-mode system as tutorial_system.cpp, using pyariadne_system."""

from pyariadne_system import (DiscreteEvent, DiscreteLocation, ContinuousTimeHybridAutomaton,
                       RealVariable, OrdinaryDifferentialAlgebraicEquation, DifferenceAlgebraicEquation, OrdinaryDifferentialAlgebraicInclusion,
                       RealVariablesBox, dot, next)


def section(title):
    print(f"\n== {title} ==")


def tutorial_system():
    section("Discrete modes and events")
    idle = DiscreteLocation(0)
    active = DiscreteLocation(1)
    activate = DiscreteEvent("activate")
    deactivate = DiscreteEvent("deactivate")

    assert idle != active
    assert activate != deactivate
    print(f"idle: {idle}")
    print(f"active: {active}")
    print(f"activation event: {activate}")

    section("A two-mode hybrid automaton")
    automaton = ContinuousTimeHybridAutomaton()
    assert not automaton.has_mode(idle)
    automaton.new_mode(idle)
    automaton.new_mode(active)

    assert automaton.has_mode(idle)
    assert automaton.has_mode(active)
    assert automaton.number_of_modes() == 2

    # Unguarded transitions without resets between discrete-only modes.
    automaton.new_transition(idle, activate, active)
    automaton.new_transition(active, deactivate, idle)

    section("Inspecting the transition relation")
    assert automaton.has_transition(idle, activate)
    assert automaton.target(idle, activate) == active
    assert automaton.has_transition(active, deactivate)
    assert automaton.target(active, deactivate) == idle

    assert not automaton.has_transition(idle, deactivate)
    assert automaton.target(idle, deactivate) == idle

    print(f"number of modes: {automaton.number_of_modes()}")
    print(f"idle --{activate}--> {automaton.target(idle, activate)}")
    print(f"active --{deactivate}--> {automaton.target(active, deactivate)}")
    print(f"automaton: {automaton}")

    section("Continuous and discrete-time systems")
    x = RealVariable("x")
    decay = OrdinaryDifferentialAlgebraicEquation([dot(x) << -x])
    contraction = DifferenceAlgebraicEquation([next(x) << x / 2])
    assert decay.dimension() == 1
    assert contraction.dimension() == 1
    print(f"vector field: {decay}")
    print(f"difference equation: {contraction}")

    section("Differential inclusion with bounded input")
    u = RealVariable("u")
    inclusion = OrdinaryDifferentialAlgebraicInclusion(
        [dot(x) << (-x + u)], RealVariablesBox({u: (-1, 1)}))
    assert inclusion.dimension() == 1
    assert inclusion.number_of_inputs() == 1
    print(f"differential inclusion: {inclusion}")
    section("Time, constant parameters and variable inputs")
    t, p = RealVariable("t"), RealVariable("p")
    parameters = RealVariablesBox({p: (1, 2)})
    timed_equation = OrdinaryDifferentialAlgebraicEquation([dot(x) << (-x + t + p)], parameters, t)
    timed_inclusion = OrdinaryDifferentialAlgebraicInclusion(
        [dot(x) << (-x + t + p + u)], RealVariablesBox({u: (-1, 1)}), parameters, t)
    print("equation arguments:", timed_equation.argument_space())
    print("inclusion arguments:", timed_inclusion.argument_space())
    assert timed_inclusion.dimension() == 1

    section("An interval-Taylor multifunction declaration")
    from pyariadne_system import (ValidatedIntervalTaylorFunctionModelDP,
        ValidatedVectorIntervalTaylorFunctionModelDP, BoxDomainType,
        FloatDPExactBox, FloatDPUpperInterval, RealSpace, exact, dp)
    domain = BoxDomainType(FloatDPExactBox([{exact(-1): exact(1)}]))
    field = ValidatedVectorIntervalTaylorFunctionModelDP([
        ValidatedIntervalTaylorFunctionModelDP.constant(domain, FloatDPUpperInterval("-1", "1", dp))])
    set_valued = OrdinaryDifferentialAlgebraicInclusion(RealSpace([x]), field)
    print(set_valued)
    assert not set_valued.has_input_function()

    # PDE construction currently requires Matrix<Real>, DiagonalMatrix<Real>
    # and function bindings not provided by the installed Python module.


if __name__ == "__main__":
    tutorial_system()
