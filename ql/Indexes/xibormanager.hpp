

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
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
#include <vector>

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
            static std::vector<std::string> histories();
          private:
            typedef std::map<std::string,History> HistoryMap;
            static HistoryMap historyMap_;
        };

    }

}


#endif
