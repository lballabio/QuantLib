/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file pool.hpp
    \brief pool of issuers
*/

#ifndef quantlib_pool_hpp
#define quantlib_pool_hpp

#include <ql/experimental/credit/issuer.hpp>
#include <map>

namespace QuantLib {

    class Pool {
      public:
        Pool();
        Size size() const;
        void clear();
        bool has (const std::string& name) const;
        void add (const std::string& name, const Issuer& issuer, 
            const DefaultProbKey& contractTrigger = NorthAmericaCorpDefaultKey(
                Currency(), SeniorSec, Period(), 1.));
        const Issuer& get (const std::string& name) const;
        const DefaultProbKey& defaultKey(const std::string& name) const;
        void setTime(const std::string& name, Real time);
        Real getTime (const std::string& name) const;
        const std::vector<std::string>& names() const;
        std::vector<DefaultProbKey> defaultKeys() const;
    private:
        // \todo: needs to cehck all defaul TS have the same ref date? here or
        //   where used? e.g. simulations.
        std::map<std::string,Issuer> data_;
        std::map<std::string,Real> time_;
        std::vector<std::string> names_;
        /*! default events seniority and currency this name enters the basket 
        with. Determines to which event/probability this pool referes to. */
        std::map<std::string, DefaultProbKey> defaultKeys_;
    };

}


#endif
