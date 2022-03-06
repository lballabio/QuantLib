/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file pricingengine.hpp
    \brief Base class for pricing engines
*/

#ifndef quantlib_pricing_engine_hpp
#define quantlib_pricing_engine_hpp

#include <ql/patterns/observable.hpp>

namespace QuantLib {

    //! interface for pricing engines
    class PricingEngine : public Observable {
      public:
        class arguments;
        class results;
        ~PricingEngine() override = default;
        virtual arguments* getArguments() const = 0;
        virtual const results* getResults() const = 0;
        virtual void reset() = 0;
        virtual void calculate() const = 0;
    };

    class PricingEngine::arguments {
      public:
        virtual ~arguments() = default;
        virtual void validate() const = 0;
    };

    class PricingEngine::results {
      public:
        virtual ~results() = default;
        virtual void reset() = 0;
    };


    //! template base class for option pricing engines
    /*! Derived engines only need to implement
        the <tt>calculate()</tt> method.
    */
    template<class ArgumentsType, class ResultsType>
    class GenericEngine : public PricingEngine,
                          public Observer {
      public:
        PricingEngine::arguments* getArguments() const override { return &arguments_; }
        const PricingEngine::results* getResults() const override { return &results_; }
        void reset() override { results_.reset(); }
        void update() override { notifyObservers(); }

      protected:
        mutable ArgumentsType arguments_;
        mutable ResultsType results_;
    };

}


#endif


#ifndef id_cee37487fb59b8ced6badc48c7eaa44c
#define id_cee37487fb59b8ced6badc48c7eaa44c
inline bool test_cee37487fb59b8ced6badc48c7eaa44c(const int* i) {
    return i != nullptr;
}
#endif
