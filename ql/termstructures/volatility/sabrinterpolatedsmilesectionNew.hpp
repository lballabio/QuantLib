/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2007 Cristina Duminuco
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

#ifndef quantlib_sabr_interpolated_smile_section_new_hpp
#define quantlib_sabr_interpolated_smile_section_new_hpp

#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/math/interpolations/sabrinterpolation.hpp>

namespace QuantLib {

    class Quote;
    class SabrInterpolatedSmileSectionNew : public SmileSection,
                                            public LazyObject {
      public:
        //! \name Constructors
        //@{
        //! all market data are quotes
        SabrInterpolatedSmileSectionNew(
                           const Date& optionDate,
                           const Handle<Quote>& forward,
                           const std::vector<Rate>& strikes,
                           bool hasFloatingStrikes,
                           const Handle<Quote>& atmVolatility,
                           const std::vector<Handle<Quote> >& volHandles,
                           Real alpha, Real beta, Real nu, Real rho,
                           bool isAlphaFixed = false, bool isBetaFixed = false,
                           bool isNuFixed = false, bool isRhoFixed = false,
                           bool vegaWeighted = true,
                           const boost::shared_ptr<EndCriteria>& endCriteria
                            = boost::shared_ptr<EndCriteria>(),
                           const boost::shared_ptr<OptimizationMethod>& method
                            = boost::shared_ptr<OptimizationMethod>(),
                           const DayCounter& dc = Actual365Fixed()
                           );
        //! no quotes
        SabrInterpolatedSmileSectionNew(
                           const Date& optionDate,
                           const Rate& forward,
                           const std::vector<Rate>& strikes,
                           bool hasFloatingStrikes,
                           const Volatility& atmVolatility,
                           const std::vector<Volatility>& vols,
                           Real alpha, Real beta, Real nu, Real rho,
                           bool isAlphaFixed = false, bool isBetaFixed = false,
                           bool isNuFixed = false, bool isRhoFixed = false,
                           bool vegaWeighted = true,
                           const boost::shared_ptr<EndCriteria>& endCriteria
                            = boost::shared_ptr<EndCriteria>(),
                           const boost::shared_ptr<OptimizationMethod>& method
                            = boost::shared_ptr<OptimizationMethod>(),
                           const DayCounter& dc = Actual365Fixed()
                           );
        //@}
        //! \name LazyObject interface
        //@{
        virtual void performCalculations() const;
        virtual void update();
        //@}
        //! \name SmileSection interface
        //@{
        Real minStrike () const;
        Real maxStrike () const;
        Real atmLevel() const;
        //@}
        Real varianceImpl(Rate strike) const;
        Volatility volatilityImpl(Rate strike) const;
         //! \name Inspectors
        //@{
        Real alpha() const;
        Real beta() const;
        Real nu() const;
        Real rho() const;
        Real rmsError() const;
        Real maxError() const;
        EndCriteria::Type endCriteria() const;
        //@}

      protected:

        //! Creates the mutable SABRInterpolation
        void createInterpolation() const;
        mutable boost::shared_ptr<SABRInterpolation> sabrInterpolation_;

        //! Market data
        const Handle<Quote> forward_;
        const Handle<Quote> atmVolatility_;
        std::vector<Handle<Quote> > volHandles_;
        mutable std::vector<Rate> strikes_;
        //! Only strikes corresponding to valid market data
        mutable std::vector<Rate> actualStrikes_;
        bool hasFloatingStrikes_;

        mutable Real forwardValue_;
        mutable std::vector<Volatility> vols_;
        //! Sabr parameters
        Real alpha_, beta_, nu_, rho_;
        //! Sabr interpolation settings
        bool isAlphaFixed_, isBetaFixed_, isNuFixed_, isRhoFixed_;
        bool vegaWeighted_;
        const boost::shared_ptr<EndCriteria> endCriteria_;
        const boost::shared_ptr<OptimizationMethod> method_;


        mutable Date evaluationDate_;

    };

    inline void SabrInterpolatedSmileSectionNew::update() {
        LazyObject::update();
        SmileSection::update();
    }

    inline Real SabrInterpolatedSmileSectionNew::volatilityImpl(Rate strike) const {
        calculate();
        return (*sabrInterpolation_)(strike, true);
    }

    inline Real SabrInterpolatedSmileSectionNew::alpha() const {
        calculate();
        return sabrInterpolation_->alpha();
    }

    inline Real SabrInterpolatedSmileSectionNew::beta() const {
        calculate();
        return sabrInterpolation_->beta();
    }

    inline Real SabrInterpolatedSmileSectionNew::nu() const {
        calculate();
        return sabrInterpolation_->nu();
    }

    inline Real SabrInterpolatedSmileSectionNew::rho() const {
        calculate();
        return sabrInterpolation_->rho();
    }

    inline Real SabrInterpolatedSmileSectionNew::rmsError() const {
        calculate();
        return sabrInterpolation_->rmsError();
    }

    inline Real SabrInterpolatedSmileSectionNew::maxError() const {
        calculate();
        return sabrInterpolation_->maxError();
    }

    inline EndCriteria::Type SabrInterpolatedSmileSectionNew::endCriteria() const {
        calculate();
        return sabrInterpolation_->endCriteria();
    }

    inline Real SabrInterpolatedSmileSectionNew::minStrike() const {
        calculate();
        return actualStrikes_.front();

    }

    inline Real SabrInterpolatedSmileSectionNew::maxStrike() const {
        calculate();
        return actualStrikes_.back();
    }

    inline Real SabrInterpolatedSmileSectionNew::atmLevel() const {
        calculate();
        return forwardValue_;
    }


}

#endif
