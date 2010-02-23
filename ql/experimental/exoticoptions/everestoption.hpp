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

/*! \file everestoption.hpp
    \brief Everest option on a number of assets
*/

#ifndef quantlib_everest_option_hpp
#define quantlib_everest_option_hpp

#include <ql/instruments/multiassetoption.hpp>

namespace QuantLib {

    class EverestOption : public MultiAssetOption {
      public:
        class engine;
        class arguments;
        class results;
        EverestOption(Real notional,
                      Rate guarantee,
                      const boost::shared_ptr<Exercise>&);
        Rate yield() const;

        void setupArguments(PricingEngine::arguments*) const;
        void fetchResults(const PricingEngine::results*) const;
      private :
        Real notional_;
        Rate guarantee_;
        mutable Rate yield_;
    };

    class EverestOption::arguments : public MultiAssetOption::arguments {
      public:
        arguments();
        void validate() const;

        Real notional;
        Rate guarantee;
    };

    class EverestOption::results : public MultiAssetOption::results {
      public:
        void reset();

        Rate yield;
    };

    class EverestOption::engine
        : public GenericEngine<EverestOption::arguments,
                               EverestOption::results> {};

}

#endif
