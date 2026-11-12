/***************************************************************************
 *            hybrid/discrete_event.hpp
 *
 *  Copyright  2004-20  Alberto Casagrande, Pieter Collins
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
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Ariadne System.  If not, see <https://www.gnu.org/licenses/>.
 */

/** @file system/discrete_event.hpp
 *  @brief Named discrete events of hybrid automata.
 */

#ifndef ARIADNE_DISCRETE_EVENT_HPP
#define ARIADNE_DISCRETE_EVENT_HPP

#include <ostream>

#include "utility/typedefs.hpp"

namespace Ariadne {

//! \ingroup SystemModule
//! \brief Type of a  discrete event of a hybrid system.
class DiscreteEvent {
  public:
    /** @brief Construct the object. */
    DiscreteEvent() : _id("e?") {}
    /** @brief Construct the object. */
    DiscreteEvent(const StringType& s) : _id(s) {}
    /** @brief Return the requested system property. */
    StringType name() const { return this->_id; }
    Bool operator==(const DiscreteEvent& e) const { return this->_id == e._id; }
    Bool operator!=(const DiscreteEvent& e) const { return this->_id != e._id; }
    Bool operator<=(const DiscreteEvent& e) const { return this->_id <= e._id; }
    Bool operator>=(const DiscreteEvent& e) const { return this->_id >= e._id; }
    Bool operator<(const DiscreteEvent& e) const { return this->_id < e._id; }
    Bool operator>(const DiscreteEvent& e) const { return this->_id > e._id; }
    friend OutputStream& operator<<(OutputStream& os, const DiscreteEvent& e) {
        return os << e._id;
    }

  private:
    StringType _id;
};

} // namespace Ariadne

#endif /* ARIADNE_DISCRETE_EVENT_HPP */
