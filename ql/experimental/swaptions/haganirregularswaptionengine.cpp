/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011, 2012, 2013, 2023 Andre Miemiec
 Copyright (C) 2012 Samuel Tebege
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

#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/exercise.hpp>
#include <ql/experimental/swaptions/haganirregularswaptionengine.hpp>
#include <ql/instruments/swaption.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/pricingengines/swaption/blackswaptionengine.hpp>
#include <utility>

namespace QuantLib {

    //////////////////////////////////////////////////////////////////////////
    // Implementation of helper class HaganIrregularSwaptionEngine::Basket  //
    //////////////////////////////////////////////////////////////////////////

    HaganIrregularSwaptionEngine::Basket::Basket(
        ext::shared_ptr<IrregularSwap> swap,
        Handle<YieldTermStructure> termStructure,
        Handle<SwaptionVolatilityStructure> volatilityStructure)
    : swap_(std::move(swap)), termStructure_(std::move(termStructure)),
      volatilityStructure_(std::move(volatilityStructure)) {

        engine_ = ext::shared_ptr<PricingEngine>(new DiscountingSwapEngine(termStructure_));

        // store swap npv
        swap_->setPricingEngine(engine_);
        targetNPV_ = swap_->NPV();

        // build standard swaps

        const Leg& fixedLeg = swap_->fixedLeg();
        const Leg& floatLeg = swap_->floatingLeg();

        Leg fixedCFS, floatCFS;

        for (Size i = 0; i < fixedLeg.size(); ++i) {
            // retrieve fixed rate coupon from fixed leg
            ext::shared_ptr<FixedRateCoupon> coupon =
                ext::dynamic_pointer_cast<FixedRateCoupon>(fixedLeg[i]);
            QL_REQUIRE(coupon, "dynamic cast of fixed leg coupon failed.");

            expiries_.push_back(coupon->date());

            ext::shared_ptr<FixedRateCoupon> newCpn = ext::make_shared<FixedRateCoupon>(
                coupon->date(), 1.0, coupon->rate(), coupon->dayCounter(),
                coupon->accrualStartDate(), coupon->accrualEndDate(),
                coupon->referencePeriodStart(), coupon->referencePeriodEnd());

            fixedCFS.push_back(newCpn);

            annuities_.push_back(10000 * CashFlows::bps(fixedCFS, **termStructure_, true));

            floatCFS.clear();

            for (const auto& j : floatLeg) {
                // retrieve ibor coupon from floating leg
                ext::shared_ptr<IborCoupon> coupon = ext::dynamic_pointer_cast<IborCoupon>(j);
                QL_REQUIRE(coupon, "dynamic cast of float leg coupon failed.");

                if (coupon->date() <= expiries_[i]) {
                    ext::shared_ptr<IborCoupon> newCpn = ext::make_shared<IborCoupon>(
                        coupon->date(), 1.0, coupon->accrualStartDate(), coupon->accrualEndDate(),
                        coupon->fixingDays(), coupon->iborIndex(), 1.0, coupon->spread(),
                        coupon->referencePeriodStart(), coupon->referencePeriodEnd(),
                        coupon->dayCounter(), coupon->isInArrears());


                    if (!newCpn->isInArrears())
                        newCpn->setPricer(
                            ext::shared_ptr<FloatingRateCouponPricer>(new BlackIborCouponPricer()));

                    floatCFS.push_back(newCpn);
                }
            }

            Real floatLegNPV = CashFlows::npv(floatCFS, **termStructure_, true);

            fairRates_.push_back(floatLegNPV / annuities_[i]);
        }
    }


    // computes a replication of the swap in terms of a basket of vanilla swaps
    // by solving a linear system of equation
    Array HaganIrregularSwaptionEngine::Basket::compute(Rate lambda) const {

        // update members
        lambda_ = lambda;

        Size n = swap_->fixedLeg().size();

        // build linear system of equations
        Matrix arr(n, n, 0.0);
        Array rhs(n);


        // fill the matrix describing the linear system of equations by looping over rows
        for (Size r = 0; r < n; ++r) {

            ext::shared_ptr<FixedRateCoupon> cpn_r =
                ext::dynamic_pointer_cast<FixedRateCoupon>(swap_->fixedLeg()[r]);
            QL_REQUIRE(cpn_r, "Cast to fixed rate coupon failed.");

            // looping over columns
            for (Size c = r; c < n; ++c) {

                // set homogenous part of lse
                arr[r][c] = (fairRates_[c] + lambda_) * cpn_r->accrualPeriod();
            }

            // add nominal repayment for i-th swap
            arr[r][r] += 1;
        }


        for (Size r = 0; r < n; ++r) {
            ext::shared_ptr<FixedRateCoupon> cpn_r =
                ext::dynamic_pointer_cast<FixedRateCoupon>(swap_->fixedLeg()[r]);

            // set inhomogenity of lse
            Real N_r = cpn_r->nominal();

            if (r < n - 1) {

                ext::shared_ptr<FixedRateCoupon> cpn_rp1 =
                    ext::dynamic_pointer_cast<FixedRateCoupon>(swap_->fixedLeg()[r + 1]);

                Real N_rp1 = cpn_rp1->nominal();

                rhs[r] = N_r * (cpn_r->rate()) * cpn_r->accrualPeriod() + (N_r - N_rp1);

            } else {

                rhs[r] = N_r * (cpn_r->rate()) * cpn_r->accrualPeriod() + N_r;
            }
        }


        SVD svd(arr);

        return svd.solveFor(rhs);
    }


    Real HaganIrregularSwaptionEngine::Basket::operator()(Rate lambda) const {

        Array weights = compute(lambda);

        Real defect = -targetNPV_;

        for (Size i = 0; i < weights.size(); ++i)
            defect -= Integer(swap_->type()) * lambda * weights[i] * annuities_[i];

        return defect;
    }


    // creates a standard swap by deducing its conventions from market data objects
    ext::shared_ptr<VanillaSwap> HaganIrregularSwaptionEngine::Basket::component(Size i) const {

        ext::shared_ptr<IborCoupon> iborCpn =
            ext::dynamic_pointer_cast<IborCoupon>(swap_->floatingLeg()[0]);
        QL_REQUIRE(iborCpn, "dynamic cast of float leg coupon failed. Can't find index.");
        ext::shared_ptr<IborIndex> iborIndex = iborCpn->iborIndex();


        Period dummySwapLength = Period(1, Years);

        ext::shared_ptr<VanillaSwap> memberSwap_ =
            MakeVanillaSwap(dummySwapLength, iborIndex)
                .withType(swap_->type())
                .withEffectiveDate(swap_->startDate())
                .withTerminationDate(expiries_[i])
                .withRule(DateGeneration::Backward)
                .withDiscountingTermStructure(termStructure_);

        Real stdAnnuity = 10000 * CashFlows::bps(memberSwap_->fixedLeg(), **termStructure_, true);

        // compute annuity transformed rate
        Rate transformedRate = (fairRates_[i] + lambda_) * annuities_[i] / stdAnnuity;

        memberSwap_ = MakeVanillaSwap(dummySwapLength, iborIndex, transformedRate)
                          .withType(swap_->type())
                          .withEffectiveDate(swap_->startDate())
                          .withTerminationDate(expiries_[i])
                          .withRule(DateGeneration::Backward)
                          .withDiscountingTermStructure(termStructure_);


        return memberSwap_;
    }


    ///////////////////////////////////////////////////////////
    // Implementation of class HaganIrregularSwaptionEngine  //
    ///////////////////////////////////////////////////////////


    HaganIrregularSwaptionEngine::HaganIrregularSwaptionEngine(
        Handle<SwaptionVolatilityStructure> volatilityStructure,
        Handle<YieldTermStructure> termStructure)
    : termStructure_(std::move(termStructure)),
      volatilityStructure_(std::move(volatilityStructure)) {
        registerWith(termStructure_);
        registerWith(volatilityStructure_);
    }


    void HaganIrregularSwaptionEngine::calculate() const {

        // check exercise type
        ext::shared_ptr<Exercise> exercise_ = this->arguments_.exercise;
        QL_REQUIRE(exercise_->type() == QuantLib::Exercise::European, "swaption must be european");

        // extract the underlying irregular swap
        ext::shared_ptr<IrregularSwap> swap_ = this->arguments_.swap;


        // Reshuffle spread from float to fixed (, i.e. remove spread from float side by finding the
        // adjusted fixed coupon such that the NPV of the swap stays constant).
        Leg fixedLeg = swap_->fixedLeg();
        Real fxdLgBPS = CashFlows::bps(fixedLeg, **termStructure_, true);

        Leg floatLeg = swap_->floatingLeg();
        Real fltLgNPV = CashFlows::npv(floatLeg, **termStructure_, true);
        Real fltLgBPS = CashFlows::bps(floatLeg, **termStructure_, true);


        Leg floatCFS, fixedCFS;

        floatCFS.clear();

        for (auto& j : floatLeg) {
            // retrieve ibor coupon from floating leg
            ext::shared_ptr<IborCoupon> coupon = ext::dynamic_pointer_cast<IborCoupon>(j);
            QL_REQUIRE(coupon, "dynamic cast of float leg coupon failed.");

            ext::shared_ptr<IborCoupon> newCpn = ext::make_shared<IborCoupon>(
                coupon->date(), coupon->nominal(), coupon->accrualStartDate(),
                coupon->accrualEndDate(), coupon->fixingDays(), coupon->iborIndex(),
                coupon->gearing(), 0.0, coupon->referencePeriodStart(),
                coupon->referencePeriodEnd(), coupon->dayCounter(), coupon->isInArrears());


            if (!newCpn->isInArrears())
                newCpn->setPricer(
                    ext::shared_ptr<FloatingRateCouponPricer>(new BlackIborCouponPricer()));

            floatCFS.push_back(newCpn);
        }


        Real sprdLgNPV = fltLgNPV - CashFlows::npv(floatCFS, **termStructure_, true);
        Rate avgSpread = sprdLgNPV / fltLgBPS / 10000;

        Rate cpn_adjustment = avgSpread * fltLgBPS / fxdLgBPS;

        fixedCFS.clear();

        for (auto& i : fixedLeg) {
            // retrieve fixed rate coupon from fixed leg
            ext::shared_ptr<FixedRateCoupon> coupon = ext::dynamic_pointer_cast<FixedRateCoupon>(i);
            QL_REQUIRE(coupon, "dynamic cast of fixed leg coupon failed.");

            ext::shared_ptr<FixedRateCoupon> newCpn = ext::make_shared<FixedRateCoupon>(
                coupon->date(), coupon->nominal(), coupon->rate() - cpn_adjustment,
                coupon->dayCounter(), coupon->accrualStartDate(), coupon->accrualEndDate(),
                coupon->referencePeriodStart(), coupon->referencePeriodEnd());

            fixedCFS.push_back(newCpn);
        }


        // this is the irregular swap with spread removed
        swap_ = ext::make_shared<IrregularSwap>(arguments_.swap->type(), fixedCFS, floatCFS);


        // Sets up the basket by implementing the methodology described in
        // P.S.Hagan "Callable Swaps and Bermudan 'Exercise into Swaptions'"
        Basket basket(swap_, termStructure_, volatilityStructure_);


        ///////////////////////////////////////////////////////////////////////////////////////////////////
        // find lambda //
        ///////////////////////////////////////////////////////////////////////////////////////////////////

        Bisection s1d;

        Rate minLambda = -0.5;
        Rate maxLambda = 0.5;
        s1d.setMaxEvaluations(10000);
        s1d.setLowerBound(minLambda);
        s1d.setUpperBound(maxLambda);
        s1d.solve(basket, 1.0e-8, 0.01, minLambda, maxLambda);


        /////////////////////////////////////////////////////////////////////////////////////////////////
        //  compute the price of the irreg swaption as the sum of the prices of the regular
        //  swaptions  //
        /////////////////////////////////////////////////////////////////////////////////////////////////


        results_.value = HKPrice(basket, exercise_);
    }


    /////////////////////////////////////////////////////////////////////////////////////////
    // Computes irregular swaption price according to P.J. Hunt, J.E. Kennedy:             //
    // "Implied interest rate pricing models", Finance Stochast. 2, 275-293 (1998)         //
    /////////////////////////////////////////////////////////////////////////////////////////

    Real HaganIrregularSwaptionEngine::HKPrice(Basket& basket,
                                               ext::shared_ptr<Exercise>& exercise) const {

        // Swaption Engine: assumes that the swaptions exercise date equals the swap start date
        QL_REQUIRE((volatilityStructure_->volatilityType() == Normal),
                   "swaptionEngine: only normal volatility implemented.");


        ext::shared_ptr<PricingEngine> swaptionEngine = ext::shared_ptr<PricingEngine>(
            new BachelierSwaptionEngine(termStructure_, volatilityStructure_));


        // retrieve weights of underlying swaps
        Array weights = basket.weights();

        Real npv = 0.0;

        for (Size i = 0; i < weights.size(); ++i) {
            ext::shared_ptr<VanillaSwap> pvSwap_ = basket.component(i);
            Swaption swaption = Swaption(pvSwap_, exercise);
            swaption.setPricingEngine(swaptionEngine);
            npv += weights[i] * swaption.NPV();
        }

        return npv;
    }


}
