
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
    \brief global repository for libor term structures and histories

    $Id$
    $Source$
    $Log$
    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#include "ql/Indexes/libormanager.hpp"

namespace QuantLib {

    namespace Indexes {

        LiborManager::TermStructureMap LiborManager::termStructureMap_;

        LiborManager::HistoryMap LiborManager::historyMap_;

        void LiborManager::setTermStructure(
            const Handle<Currency>& currency,
            const Handle<TermStructure>& termStructure) {
                QL_REQUIRE(termStructure->currency() == currency,
                    "currency mismatch while setting " +
                    currency->name() + " Libor term structure "
                    "(the given term structure has " +
                    termStructure->currency()->name() +
                    " currency");
                termStructureMap_[currency->name()] = termStructure;
        }
        
        Handle<TermStructure> LiborManager::getTermStructure(
            const Handle<Currency>& currency) {
                LiborManager::TermStructureMap::const_iterator i = 
                    termStructureMap_.find(currency->name());
                QL_REQUIRE(i != termStructureMap_.end(),
                    currency->name() + " Libor term structure not loaded");
                return i->second;
        }
        
        bool LiborManager::hasTermStructure(const Handle<Currency>& currency) {
            return (termStructureMap_.find(currency->name()) != 
                    termStructureMap_.end());
        }

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

        void LiborManager::setHistory(const Handle<Currency>& currency, 
            int n, TimeUnit unit, const History& history) {
                historyMap_[currency->name()+tag(n,unit)] = history;
        }
        
        const History& LiborManager::getHistory(
            const Handle<Currency>& currency, int n, TimeUnit unit) {
                LiborManager::HistoryMap::const_iterator i = 
                    historyMap_.find(currency->name()+tag(n,unit));
                QL_REQUIRE(i != historyMap_.end(),
                    currency->name() + " Libor " + 
                    tag(n,unit) + " history not loaded");
                return i->second;
        }
        
        bool LiborManager::hasHistory(const Handle<Currency>& currency, 
            int n, TimeUnit unit) {
                return (historyMap_.find(currency->name()+tag(n,unit)) != 
                        historyMap_.end());
        }
        
    }
    
}

