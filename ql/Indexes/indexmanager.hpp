
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file indexmanager.hpp
    \brief global repository for past index fixings
*/

#ifndef quantlib_index_manager_hpp
#define quantlib_index_manager_hpp

#include <ql/history.hpp>
#include <ql/Patterns/singleton.hpp>
#include <map>

namespace QuantLib {

    //! global repository for past index fixings
    class IndexManager : public Singleton<IndexManager> {
        friend class Singleton<IndexManager>;
      private:
        IndexManager() {}
      public:
        void setHistory(const std::string& name, const History&);
        const History& getHistory(const std::string& name) const;
        bool hasHistory(const std::string& name) const;
        std::vector<std::string> histories() const;
      private:
        std::map<std::string,History> data_;
    };

}


#endif
