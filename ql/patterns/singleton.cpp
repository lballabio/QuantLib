/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2005, 2007 StatPro Italia srl
 Copyright (C) 2013 Simon Shakeshaft

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/patterns/singleton.hpp>
#include <ql/settings.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/indexes/indexmanager.hpp>
#include <ql/experimental/commodities/unitofmeasureconversionmanager.hpp>
#include <ql/math/randomnumbers/seedgenerator.hpp>
#include <ql/utilities/tracing.hpp>

namespace QuantLib {

    // template definitions

    template <typename T>
    Singleton<T>::Singleton() { }

    template <typename T>
    T& Singleton<T>::instance() {
        static std::map<Integer, boost::shared_ptr<T> > instances_;
        #if defined(QL_ENABLE_SESSIONS)
        Integer id = sessionId();
        #else
        Integer id = 0;
        #endif
        boost::shared_ptr<T>& instance = instances_[id];
        if (!instance)
            instance = boost::shared_ptr<T>(new T);
        return *instance;
    }

    // explicit instantiations
    
    template class Singleton<Settings>;
    template class Singleton<ExchangeRateManager>;
    template class Singleton<IndexManager>;
    template class Singleton<UnitOfMeasureConversionManager>;
    template class Singleton<SeedGenerator>;

    namespace detail {

        template class Singleton<Tracing>;

    }

}

