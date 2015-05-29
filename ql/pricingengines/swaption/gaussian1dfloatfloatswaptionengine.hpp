/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013, 2015 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib liense.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file gaussian1dfloatfloatswaptionengine.hpp
    \brief float float swaption engine for one factor interest rate models
*/

#ifndef quantlib_pricers_gaussian1d_floatfloatswaption_hpp
#define quantlib_pricers_gaussian1d_floatfloatswaption_hpp

#include <ql/instruments/floatfloatswaption.hpp>
#include <ql/models/shortrate/onefactormodels/gaussian1dmodel.hpp>
#include <ql/rebatedexercise.hpp>

#include <ql/pricingengines/genericmodelengine.hpp>

namespace QuantLib {

    //! One factor model float float swaption engine
    /*! \ingroup swaptionengines

         All float coupons with fixing date greater or
         equal the respective option expiry are considered
         part of the exercise into right. Note that this
         is different from the usual accrual start date
         greater or equal exercise date if the fixing lag
         is strictly greater than the exercise lag (which
         should be a rare case). For the redepmtion flows
         the criterion is that the associated start date
         of the redemption flow (i.e. the start date of
         the regular coupon period with same payment date
         as the redemption flow) is greater or equal the
         exercise date.

         The addtional result underlyingValue is the npv
         of the underlying (as seen from "today") including
         all fixings greater (or greater equal depending
         on includeTodaysExercise) today.
   */

    class Gaussian1dFloatFloatSwaptionEngine
        : public BasketGeneratingEngine,
          public GenericModelEngine<Gaussian1dModel,
                                    FloatFloatSwaption::arguments,
                                    FloatFloatSwaption::results> {
      public:
        enum Probabilities {
            None,
            Naive,
            Digital
        };

        Gaussian1dFloatFloatSwaptionEngine(
            const boost::shared_ptr<Gaussian1dModel> &model,
            const int integrationPoints = 64, const Real stddevs = 7.0,
            const bool extrapolatePayoff = true,
            const bool flatPayoffExtrapolation = false,
            const Handle<Quote> &oas =
                Handle<Quote>(), // continously compounded w.r.t. yts daycounter
            const Handle<YieldTermStructure> &discountCurve =
                Handle<YieldTermStructure>(),
            const bool includeTodaysExercise = false,
            const Probabilities probabilities = None)
            : BasketGeneratingEngine(model, oas, discountCurve),
              GenericModelEngine<Gaussian1dModel, FloatFloatSwaption::arguments,
                                 FloatFloatSwaption::results>(model),
              integrationPoints_(integrationPoints), stddevs_(stddevs),
              extrapolatePayoff_(extrapolatePayoff),
              flatPayoffExtrapolation_(flatPayoffExtrapolation), model_(model),
              oas_(oas), discountCurve_(discountCurve),
              includeTodaysExercise_(includeTodaysExercise),
              probabilities_(probabilities) {

            if (!discountCurve_.empty())
                registerWith(discountCurve_);

            if (!oas_.empty())
                registerWith(oas_);
        }

        void calculate() const;

        Handle<YieldTermStructure> discountingCurve() const {
            return discountCurve_.empty() ? model_->termStructure()
                                          : discountCurve_;
        }

      protected:
        const Real underlyingNpv(const Date &expiry, const Real y) const;
        const VanillaSwap::Type underlyingType() const;
        const Date underlyingLastDate() const;
        const Disposable<Array> initialGuess(const Date &expiry) const;

      private:
        const int integrationPoints_;
        const Real stddevs_;
        const bool extrapolatePayoff_, flatPayoffExtrapolation_;
        const boost::shared_ptr<Gaussian1dModel> model_;
        const Handle<Quote> oas_;
        const Handle<YieldTermStructure> discountCurve_;
        const bool includeTodaysExercise_;
        const Probabilities probabilities_;

        const std::pair<Real, Real>
        npvs(const Date &expiry, const Real y,
             const bool includeExerciseOnxpiry,
             const bool considerProbabilities=false) const;

        mutable boost::shared_ptr<RebatedExercise> rebatedExercise_;
    };
}

#endif
