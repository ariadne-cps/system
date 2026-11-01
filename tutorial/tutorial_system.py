#!/usr/bin/env python3
"""The same two-mode system as tutorial_system.cpp, using pyariadne."""

from pyariadne import (DiscreteEvent, DiscreteLocation, HybridAutomaton,
                       RealVariable, VectorField, IteratedMap, DifferentialInclusion,\n                       RealVariablesBox, dot, next)


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
    automaton = HybridAutomaton()
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
    decay = VectorField([dot(x) << -x])
    contraction = IteratedMap([next(x) << x / 2])
    assert decay.dimension() == 1
    assert contraction.dimension() == 1
    print(f"vector field: {decay}")
    print(f"iterated map: {contraction}")

    section("Differential inclusion with bounded input")
    u = RealVariable("u")
    inclusion = DifferentialInclusion(
        [dot(x) << (-x + u)], RealVariablesBox({u: (-1, 1)}))
    assert inclusion.dimension() == 1
    assert inclusion.number_of_inputs() == 1
    print(f"differential inclusion: {inclusion}")
    # PDE construction currently requires Matrix<Real>, DiagonalMatrix<Real>
    # and function bindings not provided by the installed Python module.


if __name__ == "__main__":
    tutorial_system()
