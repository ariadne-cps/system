# Shared structure and explicit continuous hybrid automata (v6.29)

The public `HybridAutomatonInterface` describes time-independent structural
queries: `name`, `has_mode`, `events`, `has_transition`, `target`
and `dimension`.

`ContinuousTimeHybridAutomatonInterface` inherits this contract and retains
the continuous-time capabilities of the former `HybridAutomatonInterface`,
including `HybridTime`, ODAE/ODAI metadata, guard, invariant, reset, event
kind, auxiliary and dynamics access. The two concrete implementations are
`ContinuousTimeHybridAutomaton` and
`CompositeContinuousTimeHybridAutomaton`. No old-name compatibility aliases
are introduced. Continuous-time event and numerical semantics are unchanged.

This first coordinated migration renames continuous System types, headers,
source files, existing System Python bindings and tests. Evolution must update
its consumer types and its System gitlink to this commit before attempting a
joint build. Use `ContinuousTimeHybridAutomatonInterface` as the Evolution
`SystemType` for the continuous hybrid family.

Structural inheritance alone does not provide full implementation reuse.
The next extraction should isolate common mode/event/transition graph storage,
validation and composition in reusable internal components, without imposing
continuous-time guards, crossing semantics or `HybridTime` on discrete systems.

## Planned discrete model

`DiscreteTimeHybridAutomaton` and
`DiscreteTimeHybridAutomatonInterface` will share the structural interface
but use integer time and native per-location iteration maps/relations.
Before implementation specify the tick transition relation: order of map
application and guard evaluation, preemption, reset time, simultaneous enabled
events, priorities, and whether transitions without consuming a tick are
permitted. Continuous `URGENT`, `PERMISSIVE` and `IMPACT` have no
automatic discrete analogues.

Discrete validated and approximate evolvers should reuse compatible scheduling
and publication infrastructure, not continuous integration, reach tubes,
Lie derivatives, or contact certification. Keep paving work on its separate
`decoupling` branch. New Python capabilities and tutorial parity remain the
final Evolution phase.

## Validation

Source-level migration only; no numerical algorithms are intentionally changed.
The prior System `20d4994` CI covered extraction of the structural contract.
The coordinated migration requires fresh compile/test/format CI and a joint
Evolution consumer build before it can be called validated.

## Reusable transition topology (second implementation slice)

The public `HybridModeStructure` owns the discrete mode location and its
`DiscreteEvent -> DiscreteLocation` transition-target map. This model is
independent of continuous flows and discrete-time tick policies. It exposes
mode location, event lookup, event labels, target lookup with explicit
fallback, insertion of an already validated target and product-mode topology
composition. The existing continuous `ContinuousTimeHybridMode` derives from it.

The continuous atomic and composite automata now delegate target storage,
transition membership, transition-event enumeration and labelled product-target
joining to the shared structure. Existing guards, resets, invariant semantics,
ODE dynamics and event priorities are deliberately left in the continuous
model. The composite retains the same target join and conflict behaviour.

A dedicated System unit test covers the generic mode structure and existing
hybrid automaton tests cover the unchanged continuous semantics. Neither the
new discrete model nor any discrete-time transition semantics are implemented.

## Shared mode registry and location matching (third implementation slice)

`HybridModeRegistry<Mode>` owns the mode map and shares exact location-key
queries, location equivalence, compatible partial-location lookup and
indistinguishability detection. `ContinuousTimeHybridAutomaton` now inherits
this storage and delegates the corresponding queries, collision check and
mode lookup. The existing exception types, error paths, insertion order and
continuous dynamics, event, guard and reset handling remain unchanged.

Tests cover an empty registry, multiple modes for one named location variable,
exact versus partial matching, extended coordinate queries and incompatible
location variables. The existing atomic and composite System tests continue
checking public behaviour. The registry does not prescribe how a discrete-time
mode's update map, guards or tick transition relation should work.

The registry and topology have separate responsibilities: the registry owns a
map of modes; `HybridModeStructure` owns one mode's location and outgoing
targets. The planned discrete-time model can use both without inheriting any
continuous-time numerical or event contracts.

## Reusable structural transition preconditions (fourth implementation slice)

`HybridModeRegistry<Mode>` now exposes a protected, time-independent
`_transition_status(source,event,target)` check. It checks, in this
order: exact source location existence, exact target location existence,
and duplicate outgoing event on the resolved source mode. It returns
`VALID`, `MISSING_SOURCE`, `MISSING_TARGET` or `DUPLICATE_EVENT`.
It does not throw or mutate either the registry or the transition map.

`ContinuousTimeHybridAutomaton::_new_update` uses this check and retains its
existing public exception types, messages and precedence, followed by
the unchanged checks for reset variables and the unchanged target/reset
insertion. Guards, invariants, ODAE/ODAI, contact semantics and tick rules
are outside the shared contract.

System structural tests cover every status and precedence, including
multiple invalid preconditions; existing continuous API tests exercise the
same public exception behaviour. No discrete hybrid model has been added.
Format changes remain deferred by design. New System and consuming Evolution
CI runs are required to validate this extraction.

## Explicit mode types (fifth implementation slice)

The concrete continuous-time mode is named `ContinuousTimeHybridMode`
(previously `DiscreteMode`). This identifies the temporal model rather than
confusing the discrete *location* with discrete *time*. The corresponding
writers are `CompactContinuousTimeHybridModeWriter` and
`VerboseContinuousTimeHybridModeWriter`. No legacy type aliases are retained.

`HybridModeRegistry<Mode>` retains a template parameter because it owns
complete mode objects, not only their locations. `Mode` must derive from
`HybridModeStructure`, which supplies labelled transition topology.
This requirement is now enforced explicitly with a compile-time assertion.
The continuous automaton instantiates
`HybridModeRegistry<ContinuousTimeHybridMode>`, and a future discrete-time
automaton may instantiate `HybridModeRegistry<DiscreteTimeHybridMode>`.
No discrete-time semantics are introduced in this change.
