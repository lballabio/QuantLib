
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file xibormanager.cpp
    \brief global repository for %Xibor histories

    $Id$
*/

// $Source$
// $Log$
// Revision 1.2  2001/06/15 13:52:07  lballabio
// Reworked indexes
//
// Revision 1.1  2001/06/12 15:05:34  lballabio
// Renamed Libor to GBPLibor and LiborManager to XiborManager
//

#include "ql/Indexes/xibormanager.hpp"

namespace QuantLib {

    namespace Indexes {

        XiborManager::HistoryMap XiborManager::historyMap_;

        void XiborManager::setHistory(const std::string& name, 
            const History& history) {
                historyMap_[name] = history;
        }

        const History& XiborManager::getHistory(const std::string& name) {
                XiborManager::HistoryMap::const_iterator i =
                    historyMap_.find(name);
                QL_REQUIRE(i != historyMap_.end(),
                    name + " history not loaded");
                return i->second;
        }

        bool XiborManager::hasHistory(const std::string& name) {
                return (historyMap_.find(name) != historyMap_.end());
        }

    }

}

