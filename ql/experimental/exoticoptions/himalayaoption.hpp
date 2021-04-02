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

/*! \file himalayaoption.hpp
    \brief Himalaya option on a number of assets
*/

#ifndef quantlib_himalaya_option_hpp
#define quantlib_himalaya_option_hpp

#include <ql/instruments/multiassetoption.hpp>
#include <ql/time/date.hpp>
#include <vector>

namespace QuantLib {

    //! Himalaya option
    /*! The payoff of a Himalaya option is computed in the following
        way: Given a basket of N assets, and N time periods, at the
        end of each period the option who performed the best is added
        to the average and then discarded from the basket. At the end
        of the N, periods the option pays the max between the strike
        and the average of the best performers.

        \warning This implementation still does not manage seasoned
                 options.
    */
    class HimalayaOption : public MultiAssetOption {
      public:
        class engine;
        class arguments;
        class results;
        HimalayaOption(const std::vector<Date>& fixingDates,
                       Real strike);

        void setupArguments(PricingEngine::arguments*) const override;

      private:
        std::vector<Date> fixingDates_;
    };

    class HimalayaOption::arguments : public MultiAssetOption::arguments {
      public:
        void validate() const override;
        std::vector<Date> fixingDates;
    };

    class HimalayaOption::results : public MultiAssetOption::results {};

    class HimalayaOption::engine
        : public GenericEngine<HimalayaOption::arguments,
                               HimalayaOption::results> {};

}

#endif
