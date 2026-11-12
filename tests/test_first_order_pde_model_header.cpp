/***************************************************************************
 *            tests/test_first_order_pde_model_header.cpp
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

/** @file tests/test_first_order_pde_model_header.cpp
 *  @brief Test first order pde model header behavior.
 */

// Regression: the system-level PDE model header is independently includable.
// In particular, algebra/matrix.hpp needs a complete Rational definition.
#include "system/first_order_pde.hpp"

int main() {
    return 0;
}
