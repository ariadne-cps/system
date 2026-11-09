// Hybrid paving operations depending on a system specification.
#include "function/functional.hpp"
#include "paving/binary_tree_paving.hpp"
#include "system/hybrid_paving.hpp"
#include "system/hybrid_automaton_interface.hpp"

namespace Ariadne {

HybridPaving extend_auxiliary(const HybridPaving& hybrid_paving,
                                      const HybridAutomatonInterface& hybrid_automaton) {
    HybridGrid hybrid_state_grid = hybrid_paving.grid();
    HybridSpace hybrid_state_space = hybrid_state_grid.space();
    HybridScalings scalings = hybrid_state_grid.scalings();

    HybridSpace hybrid_state_auxiliary_space = hybrid_automaton.state_auxiliary_space();
    HybridGrid hybrid_state_auxiliary_grid(hybrid_state_auxiliary_space,scalings);

    HybridPaving result=hybrid_paving.with_grid(hybrid_state_auxiliary_grid);
    for (auto iter = hybrid_paving.locations_begin(); iter!=hybrid_paving.locations_end(); ++iter) {
        DiscreteLocation const& location=iter->first;
        Paving const& state_paving=iter->second;
        RealSpace auxiliary_space=hybrid_automaton.continuous_auxiliary_space(location);
        EffectiveVectorMultivariateFunction auxiliary_function=hybrid_automaton.auxiliary_function(location);
        Grid auxiliary_grid=scalings.grid(location,auxiliary_space);
        // The skew product is a derived geometric view. Preserve the
        // selected state backend without changing authoritative storage.
        BinaryTreePaving tree(state_paving.grid());
        for(auto cell=state_paving.begin(),end=state_paving.end();cell!=end;++cell)
            tree.adjoin(*cell);
        BinaryTreePaving product=outer_skew_product(tree,auxiliary_grid,auxiliary_function);
        // A paving-level union requires backend compatibility. Import the
        // derived geometry cell by cell into the chosen backend instead:
        // Tree and ROBDD both support direct GridCell insertion.
        Paving& destination=result[location];
        for(auto const& cell:product)
            destination.adjoin(cell);
    }
    return result;
}

} // namespace Ariadne
