/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file pricingengine.hpp
    \brief Base class for pricing engines
*/

#ifndef quantlib_pricingengine_h
#define quantlib_pricingengine_h

#include <ql/argsandresults.hpp>
#include <ql/Patterns/observable.hpp>

namespace QuantLib {

    //! interface for pricing engines
    class PricingEngine : public Observable {
      public:
        virtual ~PricingEngine() {}
        virtual Arguments* arguments() const = 0;
        virtual const Results* results() const = 0;
        virtual void reset() const = 0;
        virtual void calculate() const = 0;
    };


    //! template base class for option pricing engines
    /*! Derived engines only need to implement
        the <tt>calculate()</tt> method.
    */
    template<class ArgumentsType, class ResultsType>
    class GenericEngine : public PricingEngine {
      public:
        Arguments* arguments() const { return &arguments_; }
        const Results* results() const { return &results_; }
        void reset() const { results_.reset(); }
      protected:
        mutable ArgumentsType arguments_;
        mutable ResultsType results_;
    };

}


#endif
