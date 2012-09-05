/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Stamm
 Copyright (C) 2009 Jose Aparicio

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

/*! \file blackcdsoptionengine.hpp
    \brief Black credit default swap option engine
*/

#ifndef quantlib_black_cds_option_engine_hpp
#define quantlib_black_cds_option_engine_hpp

#include <ql/experimental/credit/cdsoption.hpp>

namespace QuantLib {

    //! Black-formula CDS-option engine
    /*! \warning The engine assumes that the exercise date equals the
                 start date of the passed CDS.
    */
    class BlackCdsOptionEngine : public CdsOption::engine {
      public:
        BlackCdsOptionEngine(const Handle<DefaultProbabilityTermStructure>&,
                             Real recoveryRate,
                             const Handle<YieldTermStructure>& termStructure,
                             const Handle<Quote>& vol);
        void calculate() const;
        Handle<YieldTermStructure> termStructure();
        Handle<Quote> volatility();
      private:
        Handle<DefaultProbabilityTermStructure> probability_;
        Real recoveryRate_;
        Handle<YieldTermStructure> termStructure_;
        Handle<Quote> volatility_;
    };

}

#endif
