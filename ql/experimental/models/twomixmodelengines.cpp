/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

#include <ql/experimental/models/twomixmodelengines.hpp>
#include <ql/experimental/inflation/yoyoptionletvolatilitystructures.hpp>
#include <ql/experimental/inflation/capflooredinflationcoupon.hpp>
#include <ql/experimental/inflation/inflationcouponpricer.hpp>
#include <ql/termstructures/volatility/optionlet/constantoptionletvol.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/time/daycounters/actualactual.hpp>

namespace QuantLib {

    MixtureBachelierYoYInflationCapFloorEngine::
    MixtureBachelierYoYInflationCapFloorEngine(
                              const Handle<YieldTermStructure>& termStructure,
                              const std::vector<Time> &times,
                              const std::vector<Real> &lambda,
                              const std::vector<Real> &mu,
                              const std::vector<Volatility> &sig1,
                              const std::vector<Volatility> &sig2)
    : YoYInflationCapFloorEngine(termStructure),
      lambda_(times), mu_(times), sig1_(times), sig2_(times) {
        // validate parameters while you set them
        QL_REQUIRE(!times.empty(), "zero parameter sets given");
        for (Size i = 0; i < times.size(); i++) {
            QL_REQUIRE(times[i]>0, "times[] must be >0 was "<<times[i]
                       <<" for ith where i = " << i);
            if (i>0) {
                QL_REQUIRE(times[i]>times[i-1],
                           "times[] must be increasing, found "
                           <<times[i-1] << " then " << times[i]
                           <<" for ith where i = " << i);
            }
            QL_REQUIRE(lambda[i]>0 && lambda[i]<1,
                       "lambda[] must be INSIDE (0,1) was "<<lambda[i]
                       <<" for ith where i = " << i);
            lambda_.setParam(i, lambda[i]);
            Rate drift = (1.0 - lambda[i]*mu[i]*times[i]) / (1.0 - lambda[i]);
            QL_REQUIRE(mu[i]*times[i] > -1.0 && drift > -1.0,
                       "effective drifts must be >-1 at max maturity "
                       "for the drift: lambda "<<lambda[i]
                       <<", mu "<< mu[i]<<", t "<<times[i]
                       <<", for ith where i = " << i
                       <<", giving " << (mu[i]*times[i]) << " and " << drift);
            mu_.setParam(i, mu[i]);
            QL_REQUIRE(sig1[i]>0,
                       "sig1[] must be >0 was "<<sig1[i]
                       <<" for ith where i = " << i);
            sig1_.setParam(i, sig1[i]);
            QL_REQUIRE(sig1[i]>0,
                       "sig2[] must be >0 was "<<sig2[i]
                       <<" for ith where i = " << i);
            sig2_.setParam(i, sig2[i]);
        }
    }


    void MixtureBachelierYoYInflationCapFloorEngine::calculate() const {

        Real value = 0.0;
        std::vector<Real> optionletsPrice;
        YoYInflationCapFloor::Type type = arguments_.type;

        // now build a capfloored yoy inflation coupon from the
        // inflation coupon you've been given using the strikes in the
        // arguments
        for (Size i=0; i<arguments_.floatingLeg.size(); i++) {
            Rate c = arguments_.capRates[i];
            Rate f = arguments_.floorRates[i];
            // time is as calendar, i.e. relative to reference date
            Period coupon_lag = arguments_.floatingLeg[i]->lag();
            Date eval = Settings::instance().evaluationDate();
            Date obs = arguments_.floatingLeg[i]->fixingDate();
            ActualActual dc;
            Time t = dc.yearFraction(eval, obs);
            // now get the correct volatility values
            Volatility vu = sig1_(t);
            Volatility vd = sig2_(t);
            // and put them in constant surfaces
            boost::shared_ptr<YoYOptionletVolatilitySurface> vuP(
                           new ConstantYoYOptionletVolatility(vu,coupon_lag));
            Handle<YoYOptionletVolatilitySurface> vuH(vuP);
            boost::shared_ptr<YoYOptionletVolatilitySurface> vdP(
                           new ConstantYoYOptionletVolatility(vd,coupon_lag));
            Handle<YoYOptionletVolatilitySurface> vdH(vdP);

            // make very sure that the strikes are correct: adjust or nullify
            Rate cu,cd,fu,fd;
            Real d1 = - mu_(t) * t;
            Real d2 = lambda_(t)*mu_(t)*t / (1.0 - lambda_(t));
            Real expectedDrift = lambda_(t)*d1 + (1-lambda_(t))*d2;
            QL_REQUIRE(std::fabs(expectedDrift)<1e-20,
                       "expected drift in mixture !=0: "<<expectedDrift);
            if ( type == YoYInflationCapFloor::Cap ) {
                fu = Null<Rate>();
                fd = Null<Rate>();
            } else {
                fu = f + d1;
                fd = f + d2;
            }
            if ( type == YoYInflationCapFloor::Floor ) {
                cu = Null<Rate>();
                cd = Null<Rate>();
            } else {
                cu = c + d1;
                cd = c + d2;
            }

            // make very sure that the gearing = 1 and spread = 0
            // otherwise the effective drift term in the mixture needs changing
            // in the coupon and we don't have access from here
            QL_REQUIRE(arguments_.floatingLeg[i]->gearing()==1.0,
                       "Gearing must equal 1 in capfloor "
                       "when using mixture pricer, was: "
                       << arguments_.floatingLeg[i]->gearing()
                       << " for coupon " << i);
            QL_REQUIRE(arguments_.floatingLeg[i]->spread()==0.0,
                       "Spread must equal 0 in capfloor "
                       "when using mixture pricer, was: "
                       << arguments_.floatingLeg[i]->spread()
                       << " for coupon " << i);

            Date d = arguments_.floatingLeg[i]->date();

            // get new capfloored coupons
            // drift UP
            CappedFlooredYoYInflationCoupon temp( arguments_.floatingLeg[i],
                                                  cu, fu, false);
            boost::shared_ptr<BachelierYoYInflationCouponPricer>
                yoyPricerU(new BachelierYoYInflationCouponPricer(vuH));
            temp.setPricer(yoyPricerU);
            Real priceU = temp.amount();
            // drift DOWN, new price but same underlying coupon
            // underlying the cappedfloored
            boost::shared_ptr<BachelierYoYInflationCouponPricer>
                yoyPricerD(new BachelierYoYInflationCouponPricer(vdH));
            temp.setPricer(yoyPricerD);
            Real priceD = temp.amount();

            // value is the weighted sum of the two
            optionletsPrice.push_back( termStructure_->discount(d)
                                       * (lambda_(t)*priceU
                                          + (1.0 - lambda_(t))*priceD) );
            value += optionletsPrice.back();
        }

        results_.value = value;
        results_.additionalResults["optionletsPrice"] = optionletsPrice;
    }


    namespace { // for our replacement for bachelierBlackFormula

        Real bachelierFormulaX(Option::Type optionType,
                               Real strike,
                               Real forward,
                               Real sigma) {

            QL_REQUIRE(sigma>=0.0,
                       "stdDev (" << sigma << ") must be non-negative");
            Real d = (forward-strike)*optionType;
            Real h = d/sigma;
            if (sigma==0.0)
                return std::max(d, 0.0);

            CumulativeNormalDistribution Phi;
            // different formula from bachelierBlackFormula
            // because this is for Normally distributed forwards
            // in their terminal measures that are
            // not interest rates
            // N.B. can have -h or +h in Phi.derivative
            // because Standard Normal is symmetric about mean=0.
            Real result = d*Phi(h) + sigma*Phi.derivative(-h);

            QL_ENSURE(result>=0.0,
                      "[bachelierFormula] negative value (" <<
                      result << ") for " <<
                      sigma << " sigma, " <<
                      optionType << " option, " <<
                      strike << " strike , " <<
                      forward << " forward");

            return result;
        }

    }



    // Prices only the cap-/floor-let effective at date d_
    void MNDMYoYInflationCapFloorModelEngine::calculate() const {

        Real value = 0.0;
        std::vector<Real> optionletsPrice;
        YoYInflationCapFloor::Type type = arguments_.type;
        Size hits = 0;

        // now build a capfloored yoy inflation coupon
        // from the inflation coupon you've been given
        // using the strikes in the arguments
        for (Size i=0; i<arguments_.floatingLeg.size(); i++) {
            // only do the caplet effective at time t_
            // boost::shared_ptr<Coupon> c = arguments_.floatingLeg[i];
            if (arguments_.floatingLeg[i]->accrualStartDate() < d_ &&
                d_ < arguments_.floatingLeg[i]->accrualEndDate()) {
                hits++;
                Rate c = arguments_.capRates[i];
                Rate f = arguments_.floorRates[i];
                // time is as calendar, i.e. relative to reference date
                Period coupon_lag = arguments_.floatingLeg[i]->lag();
                Date eval = Settings::instance().evaluationDate();
                Date obs = arguments_.floatingLeg[i]->fixingDate();
                ActualActual dc;
                Time t = dc.yearFraction(eval, obs);
                // now get the correct volatility values, now from the model
                Volatility vu = model_->sig1(t);
                Volatility vd = model_->sig2(t);
                // and put them in constant surfaces
                boost::shared_ptr<YoYOptionletVolatilitySurface> vuP(
                           new ConstantYoYOptionletVolatility(vu,coupon_lag));
                Handle<YoYOptionletVolatilitySurface> vuH(vuP);
                boost::shared_ptr<YoYOptionletVolatilitySurface> vdP(
                           new ConstantYoYOptionletVolatility(vd,coupon_lag));
                Handle<YoYOptionletVolatilitySurface> vdH(vdP);

                // make very sure that the strikes are correct: adjust
                // or nullify
                Rate cu,cd,fu,fd;
                Real d1 = model_->mu(t) * t;
                Real d2 = - model_->lambda(t)*model_->mu(t)*t /
                            (1.0 - model_->lambda(t));
                Real expectedDrift = model_->lambda(t)*d1 +
                                     (1-model_->lambda(t))*d2;
                QL_REQUIRE(std::fabs(expectedDrift)<1e-12,
                           "expected drift in mixture !=0: "<<expectedDrift);

                if ( type == YoYInflationCapFloor::Cap ) {
                    fu = Null<Rate>();
                    fd = Null<Rate>();
                } else {
                    fu = f - d1;
                    fd = f - d2;
                }
                if ( type == YoYInflationCapFloor::Floor ) {
                    cu = Null<Rate>();
                    cd = Null<Rate>();
                } else {
                    cu = c - d1;
                    cd = c - d2;
                }

                // make very sure that the gearing = 1 and spread = 0
                // otherwise the effective drift term in the mixture
                // needs changing in the coupon and we don't have
                // access from here
                QL_REQUIRE(arguments_.floatingLeg[i]->gearing()==1.0,
                           "Gearing must equal 1 in capfloor "
                           "when using mixture pricer, was: "
                           << arguments_.floatingLeg[i]->gearing()
                           << " for coupon " << i);
                QL_REQUIRE(arguments_.floatingLeg[i]->spread()==0.0,
                           "Spread must equal 0 in capfloor "
                           "when using mixture pricer, was: "
                           << arguments_.floatingLeg[i]->spread()
                           << " for coupon " << i);

                Date d = arguments_.floatingLeg[i]->date();

                // get new capfloored coupons
                // drift UP
                CappedFlooredYoYInflationCoupon tempU(
                                    arguments_.floatingLeg[i], cu, fu, false);
                boost::shared_ptr<BachelierYoYInflationCouponPricer>
                    yoyPricerU(new BachelierYoYInflationCouponPricer(vuH));
                tempU.setPricer(yoyPricerU);
                Real priceU = tempU.amount();
                // drift DOWN, new price but same underlying coupon
                // underlying the cappedfloored
                boost::shared_ptr<BachelierYoYInflationCouponPricer>
                    yoyPricerD(new BachelierYoYInflationCouponPricer(vdH));
                //where is tempD ... not just vol changes!!!!! also
                //strike because of mu
                CappedFlooredYoYInflationCoupon tempD(
                                    arguments_.floatingLeg[i], cd, fd, false);
                tempD.setPricer(yoyPricerD);
                Real priceD = tempD.amount();

                // value is the weighted sum of the two
                optionletsPrice.push_back( model_->termStructure()->discount(d)
                                           * (model_->lambda(t)*priceU +
                                              (1.0-model_->lambda(t))*priceD) );
                value += optionletsPrice.back();

            }
        }

        QL_REQUIRE(hits==1,
                   "Not _exactly_ one coupon priced "
                   "in MNDMYoYInflationCapFloorModelEngine: "
                   << hits << "; either date (" << d_
                   << ") outside all coupons, or overlap.");

        results_.value = value;
        results_.additionalResults["optionletsPrice"] = optionletsPrice;
    }




    // Largely a copy of (non-model) BlackCapFloorEngine:: with double
    // calculation of optionlet of interest.  Note that, unlike the
    // MNDM model, arbitrary gearings are allowed.
    void SLMUP2CapFloorModelEngine::calculate() const {

        QL_REQUIRE(model_,
                   "no model given in SLMUP2CapFloorModelEngine::calculate()");

        Real value = 0.0;
        std::vector<Real> optionletsPrice;
        CapFloor::Type type = arguments_.type;
        // get today from a fake volatility structure
        Volatility fakeVol=0.2;
        DayCounter dc = model_->termStructure()->dayCounter();
        boost::shared_ptr<OptionletVolatilityStructure> fake(
                     new ConstantOptionletVolatility(0, NullCalendar(),
                                                     Following, fakeVol, dc));
        Date today = fake->referenceDate();
        Date settlementDate = model_->termStructure()->referenceDate();

        for (Size i = 0; i < arguments_.startDates.size(); i++) {

            Date paymentDate = arguments_.endDates[i];
            Time accrualTime = arguments_.accrualTimes[i];
            Date startDate = arguments_.startDates[i];
            if ( paymentDate > settlementDate ) {

                Real nominal = arguments_.nominals[i];
                Real gearing = arguments_.gearings[i];
                DiscountFactor q =
                    model_->termStructure()->discount(paymentDate);
                Rate forward = arguments_.forwards[i];

                // include caplets with past fixing date ...
                // arrange so that std dev=0 for past fixings
                Real dt = dc.yearFraction(today, arguments_.fixingDates[i]);
                Real temp = q * accrualTime * nominal * gearing;
                Real caplet = temp;
                if (dt < 0) {
                    dt = 0.0;
                }
                Real volMultiplier = std::sqrt(dt);
                if (type == CapFloor::Cap || type == CapFloor::Collar) {
                    Rate strike = arguments_.capRates[i];
                    caplet *= ( model_->prob(dt)*blackFormula(
                                              Option::Call,
                                              strike + model_->displ1(dt),
                                              forward + model_->displ1(dt),
                                              volMultiplier*model_->sig1(dt) )
                                + (1.0-model_->prob(dt))*blackFormula(
                                              Option::Call,
                                              strike + model_->displ2(dt),
                                              forward + model_->displ2(dt),
                                              volMultiplier*model_->sig2(dt) ));
                    optionletsPrice.push_back(caplet);
                    value += caplet;
                }
                if (type == CapFloor::Floor || type == CapFloor::Collar) {
                    Rate strike = arguments_.floorRates[i];
                    temp *= ( model_->prob(dt)*blackFormula(
                                              Option::Put,
                                              strike + model_->displ1(dt),
                                              forward + model_->displ1(dt),
                                              volMultiplier*model_->sig1(dt) )
                              + (1.0-model_->prob(dt))*blackFormula(
                                              Option::Put,
                                              strike + model_->displ2(dt),
                                              forward + model_->displ2(dt),
                                              volMultiplier*model_->sig2(dt) ));
                    if (type == CapFloor::Floor) {
                        value += temp;
                        optionletsPrice.push_back(temp);
                    } else {
                        // collar is long a cap and short a floor
                        value -= temp;
                        // N.B. missing in blackcapfloorengine.cpp!
                        optionletsPrice.back() -= temp;
                    }
                }
            }
        }

        results_.value = value;
        results_.additionalResults["optionletsPrice"] = optionletsPrice;
    }

}

