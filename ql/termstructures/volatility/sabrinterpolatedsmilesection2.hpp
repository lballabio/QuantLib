/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

/*! \file SabrInterpolatedSmileSection2.hpp
    \brief Interpolated smile section class
*/

#ifndef quantlib_sabr_interpolated_smile_section2_hpp
#define quantlib_sabr_interpolated_smile_section2_hpp

#include <ql/termstructures/volatility/sabrinterpolatedsmilesection.hpp>

namespace QuantLib {
    class Quote;
    class SabrInterpolatedSmileSection2 : public SabrInterpolatedSmileSection {
      public:
        SabrInterpolatedSmileSection2(
                           const Date& optionDate,
                           const std::vector<Rate>& strikeSpreads,
                           const Handle<Quote>& atmVolatility,
                           const std::vector<Handle<Quote> >& volSpreads,
                           const Handle<Quote>& forward,
                           Real alpha,
                           Real beta,
                           Real nu,
                           Real rho,
                           bool isAlphaFixed,
                           bool isBetaFixed,
                           bool isNuFixed,
                           bool isRhoFixed,
                           bool vegaWeighted,
                           const boost::shared_ptr<EndCriteria>& endCriteria
                            = boost::shared_ptr<EndCriteria>(),
                           const boost::shared_ptr<OptimizationMethod>& method
                            = boost::shared_ptr<OptimizationMethod>(),
                           const DayCounter& dc = Actual365Fixed()
                           );
        void performCalculations() const;

      private:
        std::vector<Rate> strikeSpreads_;
        std::vector<Handle<Quote> > volSpreads_;
        const Handle<Quote> atmVolatility_;
    };

}

#endif
