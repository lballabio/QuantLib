
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

/*! \file libormanager.cpp
    \brief global repository for %Libor histories

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/06/05 09:35:14  lballabio
// Updated docs to use Doxygen 1.2.8
//
// Revision 1.7  2001/05/29 09:24:06  lballabio
// Using relinkable handle to term structure
//
// Revision 1.6  2001/05/28 16:12:21  lballabio
// *** empty log message ***
//
// Revision 1.4  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.3  2001/05/24 15:40:09  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/Indexes/libormanager.hpp"

namespace QuantLib {

    namespace Indexes {

        LiborManager::HistoryMap LiborManager::historyMap_;

        std::string LiborManager::tag(int n, TimeUnit unit) {
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

        void LiborManager::setHistory(const std::string& name,
            int n, TimeUnit unit, const History& history) {
                historyMap_[name+tag(n,unit)] = history;
        }

        const History& LiborManager::getHistory(
            const std::string& name, int n, TimeUnit unit) {
                LiborManager::HistoryMap::const_iterator i =
                    historyMap_.find(name+tag(n,unit));
                QL_REQUIRE(i != historyMap_.end(),
                    name + " " +
                    tag(n,unit) + " history not loaded");
                return i->second;
        }

        bool LiborManager::hasHistory(const std::string& name,
            int n, TimeUnit unit) {
                return (historyMap_.find(name+tag(n,unit)) !=
                        historyMap_.end());
        }

    }

}

