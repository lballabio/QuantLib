
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

/*! \file libormanager.hpp
    \brief global repository for libor term structures and histories

    $Id$
    $Source$
    $Log$
    Revision 1.2  2001/05/24 11:15:57  lballabio
    Stripped conventions from Currencies

    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#ifndef quantlib_libor_manager_hpp
#define quantlib_libor_manager_hpp

#include "ql/qldefines.hpp"
#include "ql/currency.hpp"
#include "ql/date.hpp"
#include "ql/history.hpp"
#include "ql/termstructure.hpp"
#include "ql/handle.hpp"
#include <map>

namespace QuantLib {

    namespace Indexes {
        
        //! global repository for libor term structures and histories
        class LiborManager {
          public:
            // discount curves
            static void setTermStructure(Currency, 
                const Handle<TermStructure>&);
            static Handle<TermStructure> getTermStructure(Currency);
            static bool hasTermStructure(Currency);
            // historical fixings
            static void setHistory(Currency, int n, TimeUnit unit, 
                const History&);
            static const History& getHistory(Currency, int n, TimeUnit unit);
            static bool hasHistory(Currency, int n, TimeUnit unit);
          private:
            static std::string tag(int n, TimeUnit unit);
            typedef std::map<Currency,Handle<TermStructure> >
                TermStructureMap;
            static TermStructureMap termStructureMap_;
            typedef std::map<std::string,History> HistoryMap;
            static HistoryMap historyMap_;
        };
        
    }
    
}


#endif
