
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

/*! \file xibormanager.hpp
    \brief global repository for %Xibor histories

    $Id$
*/

// $Source$
// $Log$
// Revision 1.1  2001/06/12 15:05:33  lballabio
// Renamed Libor to GBPLibor and LiborManager to XiborManager
//

#ifndef quantlib_xibor_manager_hpp
#define quantlib_xibor_manager_hpp

#include "ql/history.hpp"
#include "ql/termstructure.hpp"
#include <map>

namespace QuantLib {

    namespace Indexes {

        //! global repository for libor histories
        class XiborManager {
          public:
            // historical fixings
            static void setHistory(const std::string& name, 
                int n, TimeUnit unit, const History&);
            static const History& getHistory(const std::string& name, 
                int n, TimeUnit unit);
            static bool hasHistory(const std::string& name, 
                int n, TimeUnit unit);
          private:
            static std::string tag(int n, TimeUnit unit);
            typedef std::map<std::string,History> HistoryMap;
            static HistoryMap historyMap_;
        };

    }

}


#endif
