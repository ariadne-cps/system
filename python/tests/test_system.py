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
