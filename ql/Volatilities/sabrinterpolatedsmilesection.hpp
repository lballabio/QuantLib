/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file SabrInterpolatedSmileSection.hpp
    \brief Interpolated smile section class
*/

#ifndef quantlib_sabr_interpolated_smile_section_hpp
#define quantlib_sabr_interpolated_smile_section_hpp

#include <ql/termstructure.hpp>
#include <ql/quote.hpp>
#include <ql/Patterns/lazyobject.hpp>
#include <ql/Volatilities/smilesection.hpp>
#include <ql/Math/sabrinterpolation.hpp>

namespace QuantLib {

    class SabrInterpolatedSmileSection : public SmileSection,
                                         public LazyObject {
      public:
        SabrInterpolatedSmileSection(
                           Time timeToExpiry,
                           const std::vector<Rate>& strikes,
                           const std::vector<Handle<Quote> >& stdDevHandles,
                           const Handle<Quote>& forward,
                           Real alpha,
                           Real beta,
                           Real nu,
                           Real rho,
                           bool isAlphaFixed,
                           bool isBetaFixed,
                           bool isNuFixed,
                           bool isRhoFixed,
                           bool vegaWeighted = false,
                           const DayCounter& dc = Actual365Fixed()
                           );

        //SabrInterpolatedSmileSection(
        //                   const Date& d,
        //                   const std::vector<Rate>& strikes,
        //                   const std::vector<Handle<Quote> >& stdDevHandles,
        //                   const Handle<Quote>& forward,
        //                   const DayCounter& dc = Actual365Fixed(),
        //                   const Interpolator& interpolator = Interpolator(),
        //                   const Date& referenceDate = Date());
        
        void performCalculations() const;
        Real variance(Rate strike) const;
        Volatility volatility(Rate strike) const;
      private:
        Real exerciseTimeSquareRoot_;
        std::vector<Rate> strikes_;
        std::vector<Handle<Quote> > stdDevHandles_;
        const Handle<Quote> forward_;
        mutable Real forwardValue_;
        mutable std::vector<Volatility> vols_;
        mutable SABRInterpolation sabrInterpolation_;
    };

    inline SabrInterpolatedSmileSection::SabrInterpolatedSmileSection(
                       Time timeToExpiry,
                       const std::vector<Rate>& strikes,
                       const std::vector<Handle<Quote> >& stdDevHandles,
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
                       const DayCounter& dc)
    : SmileSection(timeToExpiry, dc),
      exerciseTimeSquareRoot_(std::sqrt(exerciseTime())), strikes_(strikes),
      stdDevHandles_(stdDevHandles), forward_(forward), 
      vols_(stdDevHandles.size()), 
      sabrInterpolation_(strikes_.begin(), strikes_.end(), vols_.begin(),
                        timeToExpiry, forwardValue_, alpha, beta, nu, rho,
                        isAlphaFixed, isBetaFixed,
                        isNuFixed, isRhoFixed, vegaWeighted,
                        boost::shared_ptr<OptimizationMethod>(), false)
    {
        registerWith(forward_);
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            registerWith(stdDevHandles_[i]);
    }

    
    inline void SabrInterpolatedSmileSection::performCalculations()
                                                                      const {
        forwardValue_ = 1-forward_->value()/100;
        for (Size i=0; i<stdDevHandles_.size(); ++i)
            vols_[i] = stdDevHandles_[i]->value()/exerciseTimeSquareRoot_;
        sabrInterpolation_.update();
    }

    inline Real SabrInterpolatedSmileSection::variance(Real strike) const {
        calculate();
        Real v = sabrInterpolation_(strike, true);
        return v*v*exerciseTime();
    }

    inline Real SabrInterpolatedSmileSection::volatility(Real strike) const {
        calculate();
        return sabrInterpolation_(strike, true);
    }
}

#endif
