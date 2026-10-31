"""Smoke tests for real dynamical system definitions in the composed pyariadne extension."""
from pyariadne import DiscreteEvent, DiscreteLocation, HybridAutomaton


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
