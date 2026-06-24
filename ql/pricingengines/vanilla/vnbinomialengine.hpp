/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file vnbinomialengine.hpp
    \brief Vellekoop-Nieuwenhuis binomial engine for discrete dividends
*/

#ifndef quantlib_vn_binomial_engine_hpp
#define quantlib_vn_binomial_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/instruments/dividendschedule.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Leisen-Reimer tree with Vellekoop-Nieuwenhuis dividend interpolation
    /*! Prices European and American vanilla options with discrete cash
        dividends using a recombining Leisen-Reimer binomial tree.
        At each ex-dividend date, option values are adjusted via linear
        interpolation in the stock-price grid following the method of
        Vellekoop & Nieuwenhuis (2006).

        Continuous dividend yield is carried by the BSM process.
        Discrete dividends are fixed dollar amounts passed via
        DividendSchedule (each entry's amount() is the cash drop).

        When no dividends are present, this reduces to a standard
        Leisen-Reimer tree.

        \ingroup vanillaengines
    */
    class VNBinomialVanillaEngine : public VanillaOption::engine {
      public:
        VNBinomialVanillaEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            DividendSchedule dividends,
            Size timeSteps);
        void calculate() const override;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        DividendSchedule dividends_;
        Size timeSteps_;
    };

}

#endif
