
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

/*! \file dividendvanillaoption.hpp
    \brief Vanilla option on a single asset with discrete dividends
*/

#ifndef quantlib_dividend_vanilla_option_hpp
#define quantlib_dividend_vanilla_option_hpp

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    //! Single-asset vanilla option (no barriers) with discrete dividends
    /*! \ingroup instruments */
    class DividendVanillaOption : public VanillaOption {
      public:
        class arguments;
        class engine;
        DividendVanillaOption(
                      const boost::shared_ptr<BlackScholesProcess>&,
                      const boost::shared_ptr<StrikedTypePayoff>& payoff,
                      const boost::shared_ptr<Exercise>& exercise,
                      const std::vector<Date>& dividendDates,
                      const std::vector<Real>& dividends,
                      const boost::shared_ptr<PricingEngine>& engine =
                          boost::shared_ptr<PricingEngine>());
      protected:
        void setupArguments(Arguments*) const;
      private:
        std::vector<Date> dividendDates_;
        std::vector<Real> dividends_;
    };


    //! %Arguments for dividend vanilla option calculation
    class DividendVanillaOption::arguments : public VanillaOption::arguments {
      public:
        arguments() {}
        void validate() const;
        std::vector<Date> dividendDates;
        std::vector<Real> dividends;
    };

    //! Dividend vanilla option engine base class
    class DividendVanillaOption::engine 
        : public GenericEngine<DividendVanillaOption::arguments,
                               DividendVanillaOption::results> {};

}


#endif

