// Construct the default Euclidean grid associated with a differential inclusion.
// Grid is a representation supplied transitively by Hybrid's Paving dependency;
// it does not introduce a dependency on evolution or analysis.
#include "system/differential_inclusion.hpp"
#include "paving/grid.hpp"

namespace Ariadne {

Grid DifferentialInclusion::grid() const {
    return Grid(this->dimension());
}

} // namespace Ariadne
