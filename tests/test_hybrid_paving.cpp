/***************************************************************************
 *            test_hybrid_paving.cpp
 *
 *  Copyright  2026  Luca Geretti
 *
 ****************************************************************************/

/*
 *  This file is part of Threading.
 *
 *  Threading is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Threading is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Threading.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "macro/testing.hpp"

#include "hybrid/hybrid_space.hpp"
#include "hybrid/hybrid_paving.hpp"
#include "hybrid/hybrid_bounded_constraint_set.hpp"

#include "system/hybrid_automaton.hpp"
#include "system/hybrid_paving.hpp"

using namespace Ariadne;

Void test_extend_auxiliary()
{
    RealVariable x("x"), a("a");
    StringVariable q("q");
    DiscreteLocation loc({q|"1"});

    // A mode with an auxiliary (algebraic) variable a = x + 1.
    HybridAutomaton automaton;
    ARIADNE_TEST_EXECUTE(automaton.new_mode(loc,
        List<RealAssignment>{let(a)=x+1},
        List<DottedRealAssignment>{dot(x)=-x}));

    // A one-cell state paving in location loc.
    RealSpace space({x});
    MonolithicHybridSpace hspace;
    hspace.new_location(loc, space);
    HybridGrid grid(hspace);
    HybridGridTreePaving paving(grid);
    HybridExactBoxes boxes;
    boxes.insert(loc, space, ExactBoxType({{0,1}}));
    paving.adjoin_outer_approximation(boxes, 3u);
    ARIADNE_TEST_ASSERT(paving.nontrivial_location(loc));

    // Extending with the auxiliary variable must produce a paving over the
    // combined state+auxiliary space, with the same location and cells.
    HybridGridTreePaving extended = extend_auxiliary(paving, automaton);
    ARIADNE_TEST_ASSERT(extended.nontrivial_location(loc));
    ARIADNE_TEST_ASSERT(extended.size() > 0u);
    ARIADNE_TEST_ASSERT(extended.grid().has_location(loc));
    ARIADNE_TEST_EQUALS(extended.grid().space()[loc].size(), 2u);
    ARIADNE_TEST_PRINT(extended);
}

Int main()
{
    ARIADNE_TEST_CALL(test_extend_auxiliary());
    return ARIADNE_TEST_FAILURES;
}
