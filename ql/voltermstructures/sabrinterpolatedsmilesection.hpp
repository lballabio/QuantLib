/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file SabrInterpolatedSmileSection.hpp
    \brief Interpolated smile section class
*/

#ifndef quantlib_sabr_interpolated_smile_section_hpp
#define quantlib_sabr_interpolated_smile_section_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/voltermstructures/smilesection.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>
#include <boost/scoped_ptr.hpp>
namespace QuantLib {
    class Quote;
    class SabrInterpolatedSmileSection : public SmileSection,
                                         public LazyObject {
      public:
        SabrInterpolatedSmileSection(
                           const Date& optionDate,
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
                           const boost::shared_ptr<EndCriteria>& endCriteria
                            = boost::shared_ptr<EndCriteria>(),
                           const boost::shared_ptr<OptimizationMethod>& method
                            = boost::shared_ptr<OptimizationMethod>(),
                           const DayCounter& dc = Actual365Fixed()
                           );
        void performCalculations() const;
        virtual void update();
        Real varianceImpl(Rate strike) const;
        Volatility volatilityImpl(Rate strike) const;
        Real alpha() const;
        Real beta() const;
        Real nu() const;
        Real rho() const;
        Real interpolationError() const;
        Real interpolationMaxError() const;
        EndCriteria::Type endCriteria() const;
        Real minStrike () const;
        Real maxStrike () const;
        Real atmLevel() const;
      private:
        void createInterpolation() const;
        const boost::shared_ptr<EndCriteria> endCriteria_;
        const boost::shared_ptr<OptimizationMethod> method_;
        std::vector<Rate> strikes_;
        std::vector<Handle<Quote> > stdDevHandles_;
        const Handle<Quote> forward_;
        mutable Real forwardValue_;
        bool isRhoFixed_, vegaWeighted_;
        mutable std::vector<Volatility> vols_;
        mutable boost::scoped_ptr<SABRInterpolation> sabrInterpolation_;
        Real alpha_, beta_, nu_, rho_;
        bool isAlphaFixed_, isBetaFixed_, isNuFixed_;
        mutable Date evaluationDate_;
        mutable std::vector<Rate> actualStrikes_;
    };

    inline void SabrInterpolatedSmileSection::update() {
        LazyObject::update();
        SmileSection::update();
    }

    inline Real SabrInterpolatedSmileSection::volatilityImpl(Rate strike) const {
        calculate();
        return (*sabrInterpolation_)(strike, true);
    }

    inline Real SabrInterpolatedSmileSection::alpha() const {
        calculate();
        return sabrInterpolation_->alpha();
    }

    inline Real SabrInterpolatedSmileSection::beta() const {
        calculate();
        return sabrInterpolation_->beta();
    }

    inline Real SabrInterpolatedSmileSection::nu() const {
        calculate();
        return sabrInterpolation_->nu();
    }

    inline Real SabrInterpolatedSmileSection::rho() const {
        calculate();
        return sabrInterpolation_->rho();
    }

    inline Real SabrInterpolatedSmileSection::interpolationError() const {
        calculate();
        return sabrInterpolation_->interpolationError();
    }

    inline Real SabrInterpolatedSmileSection::interpolationMaxError() const {
        calculate();
        return sabrInterpolation_->interpolationMaxError();
    }

    inline EndCriteria::Type SabrInterpolatedSmileSection::endCriteria() const {
        calculate();
        return sabrInterpolation_->endCriteria();
    }

    inline Real SabrInterpolatedSmileSection::minStrike() const {
        calculate();
        return strikes_.front();
    }

    inline Real SabrInterpolatedSmileSection::maxStrike() const {
        calculate();
        return strikes_.back();
    }

    inline Real SabrInterpolatedSmileSection::atmLevel() const {
        calculate();
        return forwardValue_;
    }
}

#endif
