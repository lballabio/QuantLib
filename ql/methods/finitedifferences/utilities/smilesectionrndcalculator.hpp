/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Yassine Idyiahia

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

/*! \file smilesectionrndcalculator.hpp
    \brief risk-neutral terminal density from a \c SmileSection via
           the Breeden-Litzenberger identity
*/

#ifndef quantlib_smilesection_rnd_calculator_hpp
#define quantlib_smilesection_rnd_calculator_hpp

#include <ql/math/interpolations/cubicinterpolation.hpp>
#include <ql/methods/finitedifferences/utilities/riskneutraldensitycalculator.hpp>
#include <ql/shared_ptr.hpp>
#include <vector>

namespace QuantLib {

    class SmileSection;

    class SmileSectionRNDCalculator : public RiskNeutralDensityCalculator {
      public:
        //! takes the forward from \c smile->atmLevel()
        explicit SmileSectionRNDCalculator(
            ext::shared_ptr<SmileSection> smile,
            Size nStrikes = 200,
            Real nStd = 5.0);

        //! explicit forward; overrides \c smile->atmLevel()
        SmileSectionRNDCalculator(
            ext::shared_ptr<SmileSection> smile,
            Real forward,
            Size nStrikes = 200,
            Real nStd = 5.0);

        // x = ln(S)
        Real pdf(Real x, Time t) const override;
        Real cdf(Real x, Time t) const override;
        Real invcdf(Real p, Time t) const override;

        // t = smile->exerciseTime()
        Real pdf(Real x) const;
        Real cdf(Real x) const;
        Real invcdf(Real p) const;

      private:
        void initialize() const;
        void checkTime(Time t) const;

        ext::shared_ptr<SmileSection> smile_;
        Size nStrikes_;
        Real nStd_;

        mutable bool initialized_ = false;
        mutable std::vector<Real> strikes_, cdf_;
        mutable ext::shared_ptr<MonotonicCubicNaturalSpline> quantileFn_;
    };

}

#endif
