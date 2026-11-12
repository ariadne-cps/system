/***************************************************************************
 *            src/differential_inclusion_grid.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Ariadne System.
 *
 *  Ariadne System is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Ariadne System is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System. If not, see <https://www.gnu.org/licenses/>.
 */

/** @file src/differential_inclusion_grid.cpp
 *  @brief Implement grid operations for differential inclusions.
 */

// Construct the default Euclidean grid associated with a differential inclusion.
// Grid is a representation supplied transitively by Hybrid's Paving dependency;
// it does not introduce a dependency on evolution or analysis.
#include "paving/grid.hpp"
#include "system/differential_inclusion.hpp"

namespace Ariadne {

Grid DifferentialInclusion::grid() const {
    return Grid(this->dimension());
}

} // namespace Ariadne
