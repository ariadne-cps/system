#include <cassert>
#include <iostream>
#include <string>

#include "ariadne-system.hpp"

using namespace Ariadne;

namespace {

void section(const std::string& title) {
    std::cout << "\n== " << title << " ==\n";
}

} // namespace

int main() {
    section("Discrete modes and events");
    // A DiscreteLocation is supplied by Hybrid; a DiscreteEvent belongs to System.
    const DiscreteLocation idle(0);
    const DiscreteLocation active(1);
    const DiscreteEvent activate("activate");
    const DiscreteEvent deactivate("deactivate");

    assert(idle != active);
    assert(activate != deactivate);

    std::cout << "idle: " << idle << "\n"
              << "active: " << active << "\n"
              << "activation event: " << activate << "\n";

    section("A two-mode hybrid automaton");
    HybridAutomaton automaton;
    assert(!automaton.has_mode(idle));
    automaton.new_mode(idle);
    automaton.new_mode(active);

    assert(automaton.has_mode(idle));
    assert(automaton.has_mode(active));
    assert(automaton.locations().size() == 2u);

    // An unguarded transition has no reset: both modes have no
    // continuous state variables. No evolution is being simulated.
    automaton.new_transition(idle, activate, active);
    automaton.new_transition(active, deactivate, idle);

    section("Inspecting the transition relation");
    assert(automaton.has_transition(idle, activate));
    assert(automaton.target(idle, activate) == active);
    assert(automaton.has_transition(active, deactivate));
    assert(automaton.target(active, deactivate) == idle);

    // A missing transition leaves the location unchanged in target().
    assert(!automaton.has_transition(idle, deactivate));
    assert(automaton.target(idle, deactivate) == idle);

    std::cout << "number of modes: " << automaton.locations().size() << "\n"
              << "idle --" << activate << "--> " << automaton.target(idle, activate) << "\n"
              << "active --" << deactivate << "--> " << automaton.target(active, deactivate) << "\n";
    std::cout << "automaton: " << automaton << "\n";
    section("Continuous and discrete-time systems");
    RealVariable x("x");
    // A typed assignment list avoids ambiguous single-element brace initialization.
    List<DottedRealAssignment> dynamics;
    dynamics.append(dot(x)=-x);
    VectorField decay(dynamics);
    List<PrimedRealAssignment> updates;
    updates.append(next(x)=x/2);
    IteratedMap contraction(updates);
    assert(decay.dimension()==1u);
    assert(contraction.dimension()==1u);
    std::cout << "vector field: " << decay << "\n";
    std::cout << "iterated map: " << contraction << "\n";
    return 0;
}
