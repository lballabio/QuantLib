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

/*! \file basketgeneratingengine.hpp
    \brief base class for pricing engines capable of
           generating a calibration basket
*/

#ifndef quantlib_basketgeneratingengine_hpp
#define quantlib_basketgeneratingengine_hpp

#include <ql/qldefines.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/models/shortrate/onefactormodels/gaussian1dmodel.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/cashflows/fixedratecoupon.hpp>
#include <ql/cashflows/iborcoupon.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    /* \warning the generated calibrating swaptions have a strike floored at
       0.1bp (minus lognormal shift, if applicable), this is not true for atm
       swaptions where the strike is generated in the swaption helper.

       \warning the standardSwapBase index should have associated forward and
       discount curves. These curves are used for setup of the swaption helper.
       This means that the market price of the calibration instrument is calculated
       using these curves. Therefore the model price must be calculated using the
       same curves, otherwise the calibration gets incosistent, i.e. the pricing
       engine used for model calibration has to be capable of using the same curves
       as associated to the index. Also the volatility structure passed to construct
       the calibration helper should use curves that are consistent with the model
       calibration curve setup. Finally the discountCurve given in the constructor
       should be the same curve as the discounting curve of the swapIndex used to
       determine the calibration basket. */

    class BasketGeneratingEngine {

      public:

        typedef enum CalibrationBasketType {
            Naive,
            MaturityStrikeByDeltaGamma
        } CalibrationBasketType;

        Disposable<std::vector<boost::shared_ptr<CalibrationHelper> > >
        calibrationBasket(
            const boost::shared_ptr<Exercise> &exercise,
            boost::shared_ptr<SwapIndex> standardSwapBase,
            boost::shared_ptr<SwaptionVolatilityStructure> swaptionVolatility,
            const CalibrationBasketType basketType =
                MaturityStrikeByDeltaGamma) const;

      protected:

        BasketGeneratingEngine(const boost::shared_ptr<Gaussian1dModel> &model,
                               const Handle<Quote> &oas,
                               const Handle<YieldTermStructure> &discountCurve)
            : onefactormodel_(model), oas_(oas), discountCurve_(discountCurve) {
        }

        virtual ~BasketGeneratingEngine() {}

        virtual const Real underlyingNpv(const Date &expiry,
                                         const Real y) const = 0;

        virtual const VanillaSwap::Type underlyingType() const = 0;

        virtual const Date underlyingLastDate() const = 0;

        virtual const Disposable<Array>
        initialGuess(const Date &expiry) const = 0; // return (nominal,
                                                    // maturity, rate)

      private:

        const boost::shared_ptr<Gaussian1dModel> onefactormodel_;
        const Handle<Quote> oas_;
        const Handle<YieldTermStructure> discountCurve_;

        class MatchHelper;
        friend class MatchHelper;
        class MatchHelper : public CostFunction {
          public:
            MatchHelper(const VanillaSwap::Type type, const Real npv,
                        const Real delta, const Real gamma,
                        const boost::shared_ptr<Gaussian1dModel> &model,
                        const boost::shared_ptr<SwapIndex> &indexBase,
                        const Date &expiry, const Real maxMaturity,
                        const Real h)
                : type_(type), mdl_(model), indexBase_(indexBase),
                  expiry_(expiry), maxMaturity_(maxMaturity), npv_(npv),
                  delta_(delta), gamma_(gamma), h_(h) {}

            Real NPV(boost::shared_ptr<VanillaSwap> swap, Real fixedRate,
                     Real nominal, Real y, int type) const {
                Real npv = 0.0;
                for (Size i = 0; i < swap->fixedLeg().size(); i++) {
                    boost::shared_ptr<FixedRateCoupon> c =
                        boost::dynamic_pointer_cast<FixedRateCoupon>(
                            swap->fixedLeg()[i]);
                    npv -=
                        fixedRate * c->accrualPeriod() * nominal *
                        mdl_->zerobond(c->date(), expiry_, y,
                                       indexBase_->discountingTermStructure());
                }
                for (Size i = 0; i < swap->floatingLeg().size(); i++) {
                    boost::shared_ptr<IborCoupon> c =
                        boost::dynamic_pointer_cast<IborCoupon>(
                            swap->floatingLeg()[i]);
                    npv +=
                        mdl_->forwardRate(c->fixingDate(), expiry_, y,
                                          c->iborIndex()) *
                        c->accrualPeriod() * nominal *
                        mdl_->zerobond(c->date(), expiry_, y,
                                       indexBase_->discountingTermStructure());
                }
                return (Real)type * npv;
            }

            Real value(const Array &v) const {
                Array vals = values(v);
                Real res = 0.0;
                for (Size i = 0; i < vals.size(); i++) {
                    res += vals[i] * vals[i];
                }
                return std::sqrt(res / vals.size());
            }

            Disposable<Array> values(const Array &v) const {
                // transformations
                int type = type_; // start with same type as non standard
                                  // underlying (1 means payer, -1 receiver)
                Real nominal = std::fabs(v[0]);
                if (v[0] < 0.0)
                    type *= -1;
                Real maturity = std::min(std::fabs(v[1]), maxMaturity_);

                Real fixedRate = v[2]; // allow for negative rates explicitly
                // (though it might not be reasonable for calibration depending
                // on the model to calibrate and the market instrument quotation)
                Size years = (Size)std::floor(maturity);
                maturity -= (Real)years;
                maturity *= 12.0;
                Size months = (Size)std::floor(maturity);
                Real alpha = 1.0 - (maturity - (Real)months);
                if (years == 0 && months == 0) {
                    months = 1;  // ensure a maturity of at least one month ...
                    alpha = 1.0; // ... but in this case only look at the lower
                                 // maturity swap
                }
                // maturity -= (Real)months; maturity *= 365.25;
                // Size days = (Size)std::floor(maturity);
                // Real alpha = 1.0-(maturity-(Real)days);
                // generate swap
                Period lowerPeriod =
                    years * Years + months * Months;           //+days*Days;
                Period upperPeriod = lowerPeriod + 1 * Months; // 1*Days;
                boost::shared_ptr<SwapIndex> tmpIndexLower, tmpIndexUpper;
                tmpIndexLower = indexBase_->clone(lowerPeriod);
                tmpIndexUpper = indexBase_->clone(upperPeriod);
                boost::shared_ptr<VanillaSwap> swapLower =
                    tmpIndexLower->underlyingSwap(expiry_);
                boost::shared_ptr<VanillaSwap> swapUpper =
                    tmpIndexUpper->underlyingSwap(expiry_);
                // compute npv, delta, gamma
                Real npvm =
                    alpha * NPV(swapLower, fixedRate, nominal, -h_, type) +
                    (1.0 - alpha) *
                        NPV(swapUpper, fixedRate, nominal, -h_, type);
                Real npv =
                    alpha * NPV(swapLower, fixedRate, nominal, 0.0, type) +
                    (1.0 - alpha) *
                        NPV(swapUpper, fixedRate, nominal, 0.0, type);
                Real npvu =
                    alpha * NPV(swapLower, fixedRate, nominal, h_, type) +
                    (1.0 - alpha) *
                        NPV(swapUpper, fixedRate, nominal, h_, type);
                Real delta = (npvu - npvm) / (2.0 * h_);
                Real gamma = (npvu - 2.0 * npv + npvm) / (h_ * h_);

                // debug output global standard underlying npv
                // Real xtmp = -5.0;
                // std::cout << "globalStandardNpv;";
                // while (xtmp <= 5.0 + QL_EPSILON) {
                //     std::cout << alpha *NPV(swapLower, fixedRate, nominal, xtmp,
                //                             type) +
                //                      (1.0 - alpha) * NPV(swapUpper, fixedRate,
                //                                          nominal, xtmp, type)
                //               << ";";
                //     xtmp += 0.1;
                // }
                // std::cout << std::endl;
                // end debug output

                // return target function values
                Array res(3);
                res[0] = (npv - npv_) / delta_;
                res[1] = (delta - delta_) / delta_;
                res[2] = (gamma - gamma_) / gamma_;
                return res;
            }

            const VanillaSwap::Type type_;
            const boost::shared_ptr<Gaussian1dModel> mdl_;
            const boost::shared_ptr<SwapIndex> indexBase_;
            const Date expiry_;
            const Real maxMaturity_;
            const Real npv_, delta_, gamma_, h_;
        };
    };
}

#endif
