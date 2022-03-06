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

/*! \file gaussian1dswaptionengine.hpp
    \brief
*/

#ifndef quantlib_pricers_gaussian1d_swaption_hpp
#define quantlib_pricers_gaussian1d_swaption_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/models/shortrate/onefactormodels/gaussian1dmodel.hpp>
#include <ql/pricingengines/genericmodelengine.hpp>
#include <utility>

namespace QuantLib {

    //! One factor model swaption engine
    /*! \ingroup swaptionengines

        All fixed coupons with start date greater or equal to the respective
        option expiry are considered to be
        part of the exercise into right.

        \warning Cash settled swaptions are not supported
    */

    class Gaussian1dSwaptionEngine
        : public GenericModelEngine<Gaussian1dModel, Swaption::arguments,
                                    Swaption::results> {
      public:
        enum Probabilities {
            None,
            Naive,
            Digital
        };

        Gaussian1dSwaptionEngine(
            const ext::shared_ptr<Gaussian1dModel>& model,
            const int integrationPoints = 64,
            const Real stddevs = 7.0,
            const bool extrapolatePayoff = true,
            const bool flatPayoffExtrapolation = false,
            Handle<YieldTermStructure> discountCurve = Handle<YieldTermStructure>(),
            const Probabilities probabilities = None)
        : GenericModelEngine<Gaussian1dModel, Swaption::arguments, Swaption::results>(model),
          integrationPoints_(integrationPoints), stddevs_(stddevs),
          extrapolatePayoff_(extrapolatePayoff), flatPayoffExtrapolation_(flatPayoffExtrapolation),
          discountCurve_(std::move(discountCurve)), probabilities_(probabilities) {

            if (!discountCurve_.empty())
                registerWith(discountCurve_);
        }

        Gaussian1dSwaptionEngine(
            const Handle<Gaussian1dModel>& model,
            const int integrationPoints = 64,
            const Real stddevs = 7.0,
            const bool extrapolatePayoff = true,
            const bool flatPayoffExtrapolation = false,
            Handle<YieldTermStructure> discountCurve = Handle<YieldTermStructure>(),
            const Probabilities probabilities = None)
        : GenericModelEngine<Gaussian1dModel, Swaption::arguments, Swaption::results>(model),
          integrationPoints_(integrationPoints), stddevs_(stddevs),
          extrapolatePayoff_(extrapolatePayoff), flatPayoffExtrapolation_(flatPayoffExtrapolation),
          discountCurve_(std::move(discountCurve)), probabilities_(probabilities) {

            if (!discountCurve_.empty())
                registerWith(discountCurve_);
        }

        void calculate() const override;

      private:
        const int integrationPoints_;
        const Real stddevs_;
        const bool extrapolatePayoff_, flatPayoffExtrapolation_;
        const Handle<YieldTermStructure> discountCurve_;
        const Probabilities probabilities_;
    };
}

#endif


#ifndef id_13ede54b5a47fa15ae248ba1e5cac145
#define id_13ede54b5a47fa15ae248ba1e5cac145
inline bool test_13ede54b5a47fa15ae248ba1e5cac145(const int* i) {
    return i != nullptr;
}
#endif
