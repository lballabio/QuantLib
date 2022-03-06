/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file pagodaoption.hpp
    \brief Roofed Asian option on a number of assets
*/

#ifndef quantlib_pagoda_option_hpp
#define quantlib_pagoda_option_hpp

#include <ql/instruments/multiassetoption.hpp>
#include <ql/time/date.hpp>
#include <vector>

namespace QuantLib {

    //! Roofed Asian option on a number of assets
    /*! The payoff is a given fraction multiplied by the minimum
        between a given roof and the positive portfolio performance.
        If the performance of the portfolio is below then the payoff
        is null.

        \warning This implementation still does not manage seasoned
                 options.

        \ingroup instruments
    */
    class PagodaOption : public MultiAssetOption {
      public:
        class engine;
        class arguments;
        PagodaOption(const std::vector<Date>& fixingDates,
                     Real roof,
                     Real fraction);
        void setupArguments(PricingEngine::arguments*) const override;

      protected:
        // arguments
        std::vector<Date> fixingDates_;
        Real roof_;
        Real fraction_;
    };


    class PagodaOption::arguments : public MultiAssetOption::arguments {
      public:
        arguments();
        void validate() const override;
        std::vector<Date> fixingDates;
        Real roof; Real fraction;
    };


    //! %Pagoda-option %engine base class
    class PagodaOption::engine
        : public GenericEngine<PagodaOption::arguments,
                               PagodaOption::results> {};

}

#endif


#ifndef id_4ad04928db097c0c081d8ea3e7f5d116
#define id_4ad04928db097c0c081d8ea3e7f5d116
inline bool test_4ad04928db097c0c081d8ea3e7f5d116(const int* i) {
    return i != nullptr;
}
#endif
