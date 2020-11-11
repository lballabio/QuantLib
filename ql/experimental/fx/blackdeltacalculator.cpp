/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Dimitri Reiswich

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

#include <ql/experimental/fx/blackdeltacalculator.hpp>

namespace QuantLib {

    BlackDeltaCalculator::BlackDeltaCalculator(
                        Option::Type ot,
                        DeltaVolQuote::DeltaType dt,
                        Real spot,
                        DiscountFactor dDiscount,   // domestic discount
                        DiscountFactor fDiscount,   // foreign  discount
                        Real stdDev):
    dt_(dt), ot_(ot),
    dDiscount_(dDiscount), fDiscount_(fDiscount),
    stdDev_(stdDev), spot_(spot),
    forward_(spot*fDiscount/dDiscount), phi_(Integer(ot)) {

        QL_REQUIRE(spot_>0.0,
                   "positive spot value required: " <<
                   spot_ << " not allowed");
        QL_REQUIRE(dDiscount_>0.0,
                   "positive domestic discount factor required: " <<
                   dDiscount_ << " not allowed");
        QL_REQUIRE(fDiscount_>0.0,
                   "positive foreign discount factor required: " <<
                   fDiscount_ << " not allowed");
        QL_REQUIRE(stdDev_>=0.0,
                   "non-negative standard deviation required: "
                   << stdDev_ << " not allowed");

        fExpPos_    =forward_*std::exp(0.5*stdDev_*stdDev_);
        fExpNeg_    =forward_*std::exp(-0.5*stdDev_*stdDev_);
    }


    Real BlackDeltaCalculator::deltaFromStrike(Real strike) const {

        QL_REQUIRE(strike >=0.0,
                   "positive strike value required: " <<
                   strike << " not allowed");

        Real res=0.0;

        switch(dt_){
          case DeltaVolQuote::Spot:
            res=phi_*fDiscount_*cumD1(strike);
            break;

          case DeltaVolQuote::Fwd:
            res=phi_*cumD1(strike);
            break;

          case DeltaVolQuote::PaSpot:
            res=phi_*fDiscount_*cumD2(strike)*strike/forward_;
            break;

          case DeltaVolQuote::PaFwd:
            res=phi_*cumD2(strike)*strike/forward_;
            break;

          default:
            QL_FAIL("invalid delta type");
        }
        return res;
    }

    Real BlackDeltaCalculator::strikeFromDelta(Real delta) const {
        return(strikeFromDelta(delta, dt_));
    }

    Real BlackDeltaCalculator::strikeFromDelta(Real delta,
                                               DeltaVolQuote::DeltaType dt)
                                                                        const{
        Real res=0.0;
        Real arg=0.0;
        InverseCumulativeNormal f;

        QL_REQUIRE(delta*phi_>=0.0, "Option type and delta are incoherent.");

        switch (dt) {
          case DeltaVolQuote::Spot:
            QL_REQUIRE(std::fabs(delta)<=fDiscount_,
                       "Spot delta out of range.");

            arg=-phi_*f(phi_*delta/fDiscount_)*stdDev_+0.5*stdDev_*stdDev_;
            res=forward_*std::exp(arg);
            break;

          case DeltaVolQuote::Fwd:
            QL_REQUIRE(std::fabs(delta)<=1.0,
                       "Forward delta out of range.");

            arg=-phi_*f(phi_*delta)*stdDev_+0.5*stdDev_*stdDev_;
            res=forward_*std::exp(arg);
            break;

          case DeltaVolQuote::PaSpot:
          case DeltaVolQuote::PaFwd: {
              // This has to be solved numerically. One of the
              // problems is that the premium adjusted call delta is
              // not monotonic in strike, such that two solutions
              // might occur. The one right to the max of the delta is
              // considered to be the correct strike.  Some proper
              // interval bounds for the strike need to be chosen, the
              // numerics can otherwise be very unreliable and
              // unstable.  I've chosen Brent over Newton, since the
              // interval can be specified explicitly and we can not
              // run into the area on the left of the maximum.  The
              // put delta doesn't have this property and can be
              // solved without any problems, but also numerically.

              BlackDeltaPremiumAdjustedSolverClass f(
                       ot_, dt , spot_,dDiscount_, fDiscount_, stdDev_,delta);

              Brent solver;
              solver.setMaxEvaluations(1000);
              Real accuracy = 1.0e-10;

              Real rightLimit=0.0;
              Real leftLimit=0.0;

              // Strike of not premium adjusted is always to the right of premium adjusted
              if (dt==DeltaVolQuote::PaSpot) {
                  rightLimit=strikeFromDelta(delta, DeltaVolQuote::Spot);
              } else {
                  rightLimit=strikeFromDelta(delta, DeltaVolQuote::Fwd);
              }

              if (phi_<0) { // if put
                  res=solver.solve(f, accuracy, rightLimit, 0.0, spot_*100.0);
                  break;
              } else {

                  // find out the left limit which is the strike
                  // corresponding to the value where premium adjusted
                  // deltas have their maximum.

                  BlackDeltaPremiumAdjustedMaxStrikeClass g(
                               ot_,dt, spot_,dDiscount_, fDiscount_, stdDev_);

                  leftLimit=solver.solve(g, accuracy, rightLimit*0.5,
                                         0.0, rightLimit);

                  Real guess=leftLimit+(rightLimit-leftLimit)*0.5;

                  res=solver.solve(f, accuracy, guess, leftLimit, rightLimit);
              } // end if phi<0 else

              break;
          }

          default:
            QL_FAIL("invalid delta type");
        }

        return res;
    }

    Real BlackDeltaCalculator::atmStrike(DeltaVolQuote::AtmType atmT) const {

        Real res=0.0;

        switch(atmT) {
          case DeltaVolQuote::AtmSpot:
            res=spot_;
            break;

          case DeltaVolQuote::AtmDeltaNeutral:
            if(dt_==DeltaVolQuote::Spot || dt_==DeltaVolQuote::Fwd){
                res=fExpPos_;
            } else {
                res=fExpNeg_;
            }
            break;

          case DeltaVolQuote::AtmFwd:
            res=forward_;
            break;

          case DeltaVolQuote::AtmGammaMax: case DeltaVolQuote::AtmVegaMax:
            res=fExpPos_;
            break;

          case DeltaVolQuote::AtmPutCall50:
            QL_REQUIRE(dt_==DeltaVolQuote::Fwd,
                       "|PutDelta|=CallDelta=0.50 only possible for forward delta.");
            res=fExpPos_;
            break;

          default:
            QL_FAIL("invalid atm type");
        }

        return res;
    }


    Real BlackDeltaCalculator::cumD1(Real strike) const {

        Real d1_=0.0;
        Real cum_d1_pos_ = 1.0; // N(d1)
        Real cum_d1_neg_ = 0.0; // N(-d1)

        CumulativeNormalDistribution f;

        if (stdDev_>=QL_EPSILON) {
            if(strike>0) {
                d1_ = std::log(forward_/strike)/stdDev_ + 0.5*stdDev_;
                return f(phi_*d1_);
            }
        } else {
            if (forward_<strike) {
                cum_d1_pos_ = 0.0;
                cum_d1_neg_ = 1.0;
            } else if(forward_==strike){
                d1_ = 0.5*stdDev_;
                return f(phi_*d1_);
            }
        }

        if (phi_>0) { // if Call
            return cum_d1_pos_;
        } else {
            return cum_d1_neg_;
        }
    }


    Real BlackDeltaCalculator::nD1(Real strike) const {

        Real d1_=0.0;
        Real n_d1_ = 0.0; // n(d1)

        if (stdDev_>=QL_EPSILON){
            if(strike>0){
                d1_ = std::log(forward_/strike)/stdDev_ + 0.5*stdDev_;
                CumulativeNormalDistribution f;
                n_d1_ = f.derivative(d1_);
            }
        }

        return n_d1_;
    }


    Real BlackDeltaCalculator::cumD2(Real strike) const {

        Real d2_=0.0;
        Real cum_d2_pos_= 1.0;  // N(d2)
        Real cum_d2_neg_= 0.0;  // N(-d2)

        CumulativeNormalDistribution f;

        if (stdDev_>=QL_EPSILON){

            if(strike>0){
                d2_ = std::log(forward_/strike)/stdDev_ - 0.5*stdDev_;
                return f(phi_*d2_);
            }

        } else {

            if (forward_<strike) {
                cum_d2_pos_= 0.0;
                cum_d2_neg_= 1.0;
            } else if (forward_==strike) {
                d2_ = -0.5*stdDev_;
                return(f(phi_*d2_));
            }

        }

        if (phi_>0) { // if Call
            return cum_d2_pos_;
        } else {
            return cum_d2_neg_;
        }
    }


    Real BlackDeltaCalculator::nD2(Real strike) const {

        Real d2_=0.0;
        Real n_d2_= 0.0; // n(d2)

        if (stdDev_>=QL_EPSILON){
            if(strike>0){
                d2_ = std::log(forward_/strike)/stdDev_ - 0.5*stdDev_;
                CumulativeNormalDistribution f;
                n_d2_ = f.derivative(d2_);
            }
        }

        return n_d2_;
    }


    void BlackDeltaCalculator::setDeltaType(DeltaVolQuote::DeltaType dt){
        dt_=dt;
    }

    void BlackDeltaCalculator::setOptionType(Option::Type ot){
        ot_=ot;
        phi_=Integer(ot_);
    }


    // helper classes

    BlackDeltaPremiumAdjustedSolverClass::BlackDeltaPremiumAdjustedSolverClass(
                        Option::Type ot,
                        DeltaVolQuote::DeltaType dt,
                        Real spot,
                        DiscountFactor dDiscount,   // domestic discount
                        DiscountFactor fDiscount,   // foreign  discount
                        Real stdDev,
                        Real delta):
    bdc_(ot,dt,spot,dDiscount,fDiscount,stdDev), delta_(delta) {}


    Real BlackDeltaPremiumAdjustedSolverClass::operator()(Real strike) const {
        return bdc_.deltaFromStrike(strike)-delta_;
    }


    BlackDeltaPremiumAdjustedMaxStrikeClass::BlackDeltaPremiumAdjustedMaxStrikeClass(
                        Option::Type ot,
                        DeltaVolQuote::DeltaType dt,
                        Real spot,
                        DiscountFactor dDiscount,   // domestic discount
                        DiscountFactor fDiscount,   // foreign  discount
                        Real stdDev):
    bdc_(ot,dt,spot,dDiscount,fDiscount,stdDev), stdDev_(stdDev) {}

    Real BlackDeltaPremiumAdjustedMaxStrikeClass::operator()(Real strike) const {
        return bdc_.cumD2(strike)*stdDev_ - bdc_.nD2(strike);
    }

}
