/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file gaussian1dnonstandardswaptionengine.hpp
    \brief
*/

#ifndef quantlib_pricers_gaussian1d_nonstandardswaption_hpp
#define quantlib_pricers_gaussian1d_nonstandardswaption_hpp

#include <ql/instruments/nonstandardswaption.hpp>
#include <ql/models/shortrate/onefactormodels/gsr.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>

namespace QuantLib {

    //! One factor model non standard swaption engine
    /*! \ingroup swaptionengines

       All fixed coupons with start date greater or equal to the
       respective option expiry are considered to be part of the
       exercise into right.

       All float coupons with start date greater or equal to the
       respective option expiry are consideres to be part of the
       exercise into right.

       For redemption flows an associated start date is considered
       in the criterion, which is the start date of the regular
       xcoupon period with same payment date as the redemption flow.

       \warning Cash settled swaptions are not supported

    */

    class Gaussian1dNonstandardSwaptionEngine
        : public BasketGeneratingEngine,
          public GenericModelEngine<Gaussian1dModel,
                                    NonstandardSwaption::arguments,
                                    NonstandardSwaption::results> {
      public:
        enum Probabilities {
            None,
            Naive,
            Digital
        };

        Gaussian1dNonstandardSwaptionEngine(
            const ext::shared_ptr<Gaussian1dModel> &model,
            const int integrationPoints = 64, const Real stddevs = 7.0,
            const bool extrapolatePayoff = true,
            const bool flatPayoffExtrapolation = false,
            const Handle<Quote> &oas = Handle<Quote>(), // continuously
                                                        // compounded w.r.t. yts
                                                        // daycounter
            const Handle<YieldTermStructure> &discountCurve =
                Handle<YieldTermStructure>(),
            const Probabilities probabilities = None)
            : BasketGeneratingEngine(model, oas, discountCurve),
              GenericModelEngine<Gaussian1dModel,
                                 NonstandardSwaption::arguments,
                                 NonstandardSwaption::results>(model),
              integrationPoints_(integrationPoints), stddevs_(stddevs),
              extrapolatePayoff_(extrapolatePayoff),
              flatPayoffExtrapolation_(flatPayoffExtrapolation),
              discountCurve_(discountCurve), oas_(oas),
              probabilities_(probabilities) {

            if (!oas_.empty())
                registerWith(oas_);

            if (!discountCurve_.empty())
                registerWith(discountCurve_);
        }

        Gaussian1dNonstandardSwaptionEngine(
            const Handle<Gaussian1dModel> &model,
            const int integrationPoints = 64, const Real stddevs = 7.0,
            const bool extrapolatePayoff = true,
            const bool flatPayoffExtrapolation = false,
            const Handle<Quote> &oas = Handle<Quote>(), // continuously
                                                        // compounded w.r.t. yts
                                                        // daycounter
            const Handle<YieldTermStructure> &discountCurve =
                Handle<YieldTermStructure>(),
            const Probabilities probabilities = None)
            : BasketGeneratingEngine(model, oas, discountCurve),
              GenericModelEngine<Gaussian1dModel,
                                 NonstandardSwaption::arguments,
                                 NonstandardSwaption::results>(model),
              integrationPoints_(integrationPoints), stddevs_(stddevs),
              extrapolatePayoff_(extrapolatePayoff),
              flatPayoffExtrapolation_(flatPayoffExtrapolation),
              discountCurve_(discountCurve), oas_(oas),
              probabilities_(probabilities) {

            if (!oas_.empty())
                registerWith(oas_);

            if (!discountCurve_.empty())
                registerWith(discountCurve_);
        }

        void calculate() const override;

      protected:
        Real underlyingNpv(const Date& expiry, Real y) const override;
        Swap::Type underlyingType() const override;
        const Date underlyingLastDate() const override;
        const Disposable<Array> initialGuess(const Date& expiry) const override;

      private:
        const int integrationPoints_;
        const Real stddevs_;
        const bool extrapolatePayoff_, flatPayoffExtrapolation_;
        const Handle<YieldTermStructure> discountCurve_;
        const Handle<Quote> oas_;
        const Probabilities probabilities_;
    };
}

#endif


#ifndef id_9e3c3ed525826f3e326ef77761d869bd
#define id_9e3c3ed525826f3e326ef77761d869bd
inline bool test_9e3c3ed525826f3e326ef77761d869bd(const int* i) {
    return i != nullptr;
}
#endif
