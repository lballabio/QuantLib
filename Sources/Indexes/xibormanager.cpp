
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
// Revision 1.1  2001/06/12 15:05:34  lballabio
// Renamed Libor to GBPLibor and LiborManager to XiborManager
//

#include "ql/Indexes/xibormanager.hpp"

namespace QuantLib {

    namespace Indexes {

        XiborManager::HistoryMap XiborManager::historyMap_;

        std::string XiborManager::tag(int n, TimeUnit unit) {
            switch (unit) {
              case Days:
                return IntegerFormatter::toString(n)+"d";
              case Weeks:
                return IntegerFormatter::toString(n)+"w";
              case Months:
                return IntegerFormatter::toString(n)+"m";
              case Years:
                return IntegerFormatter::toString(n)+"y";
              default:
                throw Error("invalid time unit");
            }
            QL_DUMMY_RETURN(std::string())
        }

        void XiborManager::setHistory(const std::string& name,
            int n, TimeUnit unit, const History& history) {
                historyMap_[name+tag(n,unit)] = history;
        }

        const History& XiborManager::getHistory(
            const std::string& name, int n, TimeUnit unit) {
                XiborManager::HistoryMap::const_iterator i =
                    historyMap_.find(name+tag(n,unit));
                QL_REQUIRE(i != historyMap_.end(),
                    name + " " +
                    tag(n,unit) + " history not loaded");
                return i->second;
        }

        bool XiborManager::hasHistory(const std::string& name,
            int n, TimeUnit unit) {
                return (historyMap_.find(name+tag(n,unit)) !=
                        historyMap_.end());
        }

    }

}

