
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file xibormanager.hpp
    \brief global repository for %Xibor histories

    \fullpath
    ql/Indexes/%xibormanager.hpp
*/

// $Id$

#ifndef quantlib_xibor_manager_hpp
#define quantlib_xibor_manager_hpp

#include <ql/history.hpp>
#include <ql/termstructure.hpp>
#include <map>

namespace QuantLib {

    namespace Indexes {

        //! global repository for libor histories
        class XiborManager {
          public:
            // historical fixings
            static void setHistory(const std::string& name,
                const History&);
            static const History& getHistory(const std::string& name);
            static bool hasHistory(const std::string& name);
          private:
            typedef std::map<std::string,History> HistoryMap;
            static HistoryMap historyMap_;
        };

    }

}


#endif
