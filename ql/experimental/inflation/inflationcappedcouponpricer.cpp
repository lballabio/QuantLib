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

#include <ql/experimental/inflation/inflationcappedcouponpricer.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/randomnumbers/knuthuniformrng.hpp>
#include <ql/math/randomnumbers/boxmullergaussianrng.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

namespace QuantLib {

    void InflationCappedCouponPricer::initialize(
                                        const InflationCappedCoupon& coupon) {

        // get coupon details ... like?
        // want to store a pointer not the object
        coupon_ =  dynamic_cast<const InflationCappedCoupon*>(&coupon);
        nominalSpread_ = coupon_->nominalSpread();
        capGearing_ = coupon_->capGearing();
        inflationSpread_ = coupon_->inflationSpread();
        Date paymentDate = coupon_->date();
        inflationIndex_ = coupon_->yoyIndex();
        nominalIndex_ = coupon_->nominalIndex();

        Handle<YieldTermStructure> rateCurve =
            inflationIndex_->yoyInflationTermStructure()
                           ->nominalTermStructure();

        today_ = Settings::instance().evaluationDate();
        if(paymentDate > today_)
            discount_ = rateCurve->discount(paymentDate);
        else
            discount_ = 1.0;
        //  fundingLegValue_ =  FIXME
    }


    Rate InflationCappedCouponPricer::adjustedFixing() const {
        // this is the value if the fixing date has passed
        // e.g. 12mEUR + x% - g*max(0, YOY - y%), floored at zero
        if (coupon_->fixingDate() < today_) {
            Rate nominalRate =
                nominalIndex_->fixing(coupon_->nominalFixingDate());
            Rate inflationRate =
                inflationIndex_->fixing(coupon_->yoyFixingDate());
            return std::max(0.0,
                            nominalRate + nominalSpread_
                            - capGearing_ * std::max(0.0,
                                                     inflationRate
                                                     - inflationSpread_));
        } else {
            // this is actually the no-arbitrage price given the market
            return optionletPriceImp();
        }
    }


    Real InflationCappedCouponPricer::price() const {
        // past or future fixing is managed in fixing
        return adjustedFixing() * coupon_->accrualPeriod() * discount_;
    }


    Real InflationCappedCouponPricer::rate() const {
        return adjustedFixing();
    }




    // function to integrate
    Real AnalyticInflationCappedCouponPricer::payoffContribution(
                                                        const Real sd) const {

        Real tauNextra = 1.0;   // years, because in arrears relative to fwd
        // given SD get the rest ...
        // sd takes the role of w1 in the MC version
        // then we have an analytic value given w1
        Real inf = Nstart_ + Nmu_*(tau_ + tauNextra) + Nsd_ * sd * sqrt(tau_ + tauNextra);
        // N.B. nom is actually a distribution at this point
        // we never wish to include negative values of nominal
        Real gK = 0.0000001;     // strike ZERO for nominal
        Real gmuX = -0.5*DDsd_*DDsd_*tau_;
        Real gsigmaX = DDsd_*sqrt(tau_);
        Real gV = gsigmaX*sqrt(1.0 - correlation_*correlation_);
        Real gM = gmuX + correlation_*gsigmaX*sd;
        Real glogRat = log( (gK+DDs_) / (DDstart_+DDs_) );
        Real nomCE = (DDstart_+DDs_)*exp(gM+0.5*gV*gV)*Phi((gM-glogRat+gV*gV)/gV) - (gK+DDs_)*Phi((gM-glogRat)/gV);
        // gets used in both non-call bits

        Real value = 0.0;
        if (inf < inflationSpread_) {
            value = nomCE + nominalSpread_;
            //value = 0;//FIXME
        } else if (inf > (inflationSpread_ + nominalSpread_ / capGearing_) ) {
            // this is the interesting case, variant of E.5 in [BM06]
            Real K = -(nominalSpread_ - capGearing_*(inf - inflationSpread_));
            Real muX = -0.5*DDsd_*DDsd_*tau_;
            Real sigmaX = DDsd_*sqrt(tau_);
            Real V = sigmaX*sqrt(1.0 - correlation_*correlation_);
            Real M = muX + correlation_*sigmaX*sd;
            Real logRat = log( (K+DDs_) / (DDstart_+DDs_) );
            value = (DDstart_+DDs_)*exp(M+0.5*V*V)*Phi((M-logRat+V*V)/V) - (K+DDs_)*Phi((M-logRat)/V);
        } else {
            value = std::max(0.0, nomCE + nominalSpread_ - capGearing_*std::max(0.0, inf - inflationSpread_));
            //value = 0;//FIXME
        }
        // NOTE that this is not discounted
        return value * Phi.derivative(sd); // scan over a Normal distribution
    }


    Real AnalyticInflationCappedCouponPricer::optionletPriceImp() const {
        // want to integrate a function

        Real nSD = 6; // SD's each way
        Real min = -nSD;
        Real max = nSD;

        Size n = nPoints_;      // integration points
        boost::function<Real (Real)> f =
            boost::bind(&AnalyticInflationCappedCouponPricer::payoffContribution,
                        this, _1);
        SimpsonIntegral I(0.00001,n);
        Real value = I(f, min, max);
        return value;
    }





    Real MCInflationCappedCouponPricer::optionletPriceImp() const {
        KnuthUniformRng U01(seed_);
        BoxMullerGaussianRng<KnuthUniformRng> G(U01);

        Real tauNextra = 1.0; // inflation is in arrears (naturally) which adds a year
        // straight substitution of values, like for past dates
        Real value = 0.0;
        Real temp;
        Real inf, nom;  // inflation and nominal values
        Real w1, w2, wb;
        for (Size i = 0; i < nSamples_; i++) {
            w1 = G.next().value;   // get two N(0,1) random samples
            w2 = G.next().value;
            wb = correlation_ * w1 + sqrt(1.0 - correlation_*correlation_) * w2;
            inf = Nstart_ + Nmu_*(tau_ + tauNextra) + Nsd_ * w1 * sqrt(tau_ + tauNextra);
            nom = (DDstart_ + DDs_) * exp( -0.5*DDsd_*DDsd_*tau_ + DDsd_*wb*sqrt(tau_)) - DDs_;
            temp = std::max(0.0, std::max(0.0, nom) + nominalSpread_
                            - capGearing_ * std::max(0.0, inf - inflationSpread_)
                            );

            // if(inf < inflationSpread_) {temp=0;} //FIXME
            // else if(inf > (inflationSpread_ + nominalSpread_ / capGearing_) ) value += temp;
            // else {temp=0;}//FIXME
            value += temp;
        }
        value /= (Real)nSamples_;
        //cout << value << " <<<< MCwas " << endl;
        return value;
    }

}

