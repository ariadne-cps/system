# Common structure of hybrid automata (first implementation slice)

Status: initial System-side implementation toward cross-repository design v6.29.
This increment intentionally preserves the existing public continuous-time names
and numerical behaviour until dependent repositories can be migrated together.

## Implemented

`HybridAutomatonStructureInterface` provides six pure structural queries:
`name`, `has_mode`, `events`, `has_transition`, `target` and
`dimension`. The existing `HybridAutomatonInterface` derives from it;
`HybridAutomaton` and `CompositeHybridAutomaton` therefore implement it
without adapters or duplicated dispatch. Continuous-time operations and event
semantics remain in the existing interface.

The umbrella header exports the structural contract, and compile-time checks
cover both concrete implementations. The implementation does not change any
continuous dynamics, guards, resets, or time handling.

## Planned coordinated migration (not implemented by this commit)

1. Rename `HybridAutomaton` to `ContinuousTimeHybridAutomaton` and
   `HybridAutomatonInterface` to `ContinuousTimeHybridAutomatonInterface`.
2. Rename the continuous composite, migrate C++/Python users in System and
   Evolution and preserve complete continuous regression behaviour. Afterwards
   the structural interface can take the shorter name `HybridAutomatonInterface`.
   No permanent compatibility aliases are intended.
3. Introduce `DiscreteTimeHybridAutomaton` with its own time-specific interface
   and precise tick/event semantics. Reuse structural graph, validation and
   composition logic only when contracts are equivalent.
4. Extract shared hybrid orchestration in Evolution while keeping the discrete
   iteration kernel and continuous flow/crossing kernels separate.

A tick policy must specify guard evaluation order, simultaneous events, reset
timing and whether zero-time transition chains are allowed. Do not import
URGENT/PERMISSIVE/IMPACT semantics implicitly from the continuous model.

## Verification boundary

This is a compile-time API extraction with no numerical algorithm change.
Integrated System/Evolution compilation and CI must be checked after publication.
