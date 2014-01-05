/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Yue, Tian

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

#include <ql/experimental/barrieroption/vannavolgadoublebarrierengine.hpp>
#include <ql/experimental/barrieroption/vannavolgainterpolation.hpp>
#include <ql/experimental/barrieroption/analyticdoublebarrierengine.hpp>
#include <ql/experimental/barrieroption/doublebarrieroption.hpp>
#include <ql/experimental/fx/blackdeltacalculator.hpp>
#include <ql/pricingengines/barrier/analyticbarrierengine.hpp>
#include <ql/math/matrix.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/blackconstantvol.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/time/calendars/all.hpp>

namespace QuantLib {

    VannaVolgaDoubleBarrierEngine::VannaVolgaDoubleBarrierEngine(
            const Handle<DeltaVolQuote> atmVol,
            const Handle<DeltaVolQuote> vol25Put,
            const Handle<DeltaVolQuote> vol25Call,
            const Handle<Quote> spotFX,
            const Handle<YieldTermStructure> domesticTS,
            const Handle<YieldTermStructure> foreignTS,
            const bool adaptVanDelta,
            const Real bsPriceWithSmile,
            const int series)
    : GenericEngine<DoubleBarrierOption::arguments,
                    DoubleBarrierOption::results>(),
      atmVol_(atmVol), vol25Put_(vol25Put), vol25Call_(vol25Call), T_(atmVol_->maturity()),
      spotFX_(spotFX), domesticTS_(domesticTS), foreignTS_(foreignTS),
      adaptVanDelta_(adaptVanDelta), bsPriceWithSmile_(bsPriceWithSmile),
      series_(series){

          QL_REQUIRE(vol25Put_->delta() == -0.25, "25 delta put is required by vanna volga method");
          QL_REQUIRE(vol25Call_->delta() == 0.25, "25 delta call is required by vanna volga method");

          QL_REQUIRE(vol25Put_->maturity() == vol25Call_->maturity() && vol25Put_->maturity() == atmVol_->maturity(),
              "Maturity of 3 vols are not the same");

          QL_REQUIRE(!domesticTS_.empty(), "domestic yield curve is not defined");
          QL_REQUIRE(!foreignTS_.empty(), "foreign yield curve is not defined");

          registerWith(atmVol_);
          registerWith(vol25Put_);
          registerWith(vol25Call_);
          registerWith(spotFX_);
          registerWith(domesticTS_);
          registerWith(foreignTS_);

    }

     void VannaVolgaDoubleBarrierEngine::calculate() const {

         const Real sigmaShift_vega = 0.001;
         const Real sigmaShift_volga = 0.0001;
         const Real spotShift_delta = 0.0001 * spotFX_->value();
         const Real sigmaShift_vanna = 0.0001;

         Handle<Quote> x0Quote(  //used for shift
             boost::make_shared<SimpleQuote>(spotFX_->value()));
         Handle<Quote> atmVolQuote( //used for shift
             boost::make_shared<SimpleQuote>(atmVol_->value()));

         boost::shared_ptr<BlackVolTermStructure> blackVolTS =
             boost::make_shared<BlackConstantVol>(
                               Settings::instance().evaluationDate(),
                               NullCalendar(), atmVolQuote, Actual365Fixed());
         boost::shared_ptr<BlackScholesMertonProcess> stochProcess =
             boost::make_shared<BlackScholesMertonProcess>(
                                 x0Quote,
                                 foreignTS_,
                                 domesticTS_,
                                 Handle<BlackVolTermStructure>(blackVolTS));

         boost::shared_ptr<PricingEngine> engineBS =
             boost::make_shared<AnalyticDoubleBarrierEngine>(stochProcess,
                                                             series_);

         BlackDeltaCalculator blackDeltaCalculatorAtm(
                        Option::Call, atmVol_->deltaType(), x0Quote->value(),
                        domesticTS_->discount(T_), foreignTS_->discount(T_),
                        atmVol_->value() * sqrt(T_));
         Real atmStrike = blackDeltaCalculatorAtm.atmStrike(atmVol_->atmType());

         Real call25Vol = vol25Call_->value();
         Real put25Vol = vol25Put_->value();
         BlackDeltaCalculator blackDeltaCalculatorPut25(
                       Option::Put, vol25Put_->deltaType(), x0Quote->value(),
                       domesticTS_->discount(T_), foreignTS_->discount(T_),
                       put25Vol * sqrt(T_));
         Real put25Strike = blackDeltaCalculatorPut25.strikeFromDelta(-0.25);
         BlackDeltaCalculator blackDeltaCalculatorCall25(
                     Option::Call, vol25Call_->deltaType(), x0Quote->value(),
                     domesticTS_->discount(T_), foreignTS_->discount(T_),
                     call25Vol * sqrt(T_));
         Real call25Strike = blackDeltaCalculatorCall25.strikeFromDelta(0.25);

         //here use vanna volga interpolated smile to price vanilla
         std::vector<Real> strikes;
         std::vector<Real> vols;
         strikes.push_back(put25Strike);
         vols.push_back(put25Vol);
         strikes.push_back(atmStrike);
         vols.push_back(atmVol_->value());
         strikes.push_back(call25Strike);
         vols.push_back(call25Vol);
         VannaVolga vannaVolga(x0Quote->value(), foreignTS_->discount(T_), foreignTS_->discount(T_), T_);
         Interpolation interpolation = vannaVolga.interpolate(strikes.begin(), strikes.end(), vols.begin());
         interpolation.enableExtrapolation();
         const boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(arguments_.payoff);
         Real strikeVol = interpolation(payoff->strike());
         //vannila option price
         Real vanillaOption = blackFormula(payoff->optionType(), payoff->strike(),
                                      x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_),
                                      strikeVol * sqrt(T_),
                                      domesticTS_->discount(T_));

         //already out
         if((x0Quote->value() > arguments_.barrier[1] || x0Quote->value() < arguments_.barrier[0])
             && arguments_.barrierType[0] == Barrier::DownOut){
                 results_.value = 0.0;
                 results_.additionalResults["VanillaPrice"] = adaptVanDelta_? bsPriceWithSmile_ : vanillaOption;
                 results_.additionalResults["BarrierInPrice"] = adaptVanDelta_? bsPriceWithSmile_ : vanillaOption;
                 results_.additionalResults["BarrierOutPrice"] = 0.0;
         }
         //already in
         else if((x0Quote->value() > arguments_.barrier[1] || x0Quote->value() < arguments_.barrier[0])
                  && arguments_.barrierType[0] == Barrier::DownIn){
                 results_.value = adaptVanDelta_? bsPriceWithSmile_ : vanillaOption;
                 results_.additionalResults["VanillaPrice"] = adaptVanDelta_? bsPriceWithSmile_ : vanillaOption;
                 results_.additionalResults["BarrierInPrice"] = adaptVanDelta_? bsPriceWithSmile_ : vanillaOption;
                 results_.additionalResults["BarrierOutPrice"] = 0.0;
         }
         else{

                 //set up BS barrier option pricing
                 //only calculate out barrier option price
                 // in barrier price = vanilla - out barrier
                 boost::shared_ptr<StrikedTypePayoff> payoff
                     = boost::static_pointer_cast<StrikedTypePayoff> (arguments_.payoff);
                 DoubleBarrierOption doubleBarrierOption(arguments_.barrierType,
                                             arguments_.barrier,
                                             arguments_.rebate,
                                             payoff,
                                             arguments_.exercise);

                 doubleBarrierOption.setPricingEngine(engineBS);

                 //BS price
                 Real priceBS = doubleBarrierOption.NPV();

                 Real priceAtmCallBS = blackFormula(Option::Call,atmStrike,
                                              x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_), 
                                              atmVol_->value() * sqrt(T_),
                                              domesticTS_->discount(T_));
                 Real price25CallBS = blackFormula(Option::Call,call25Strike,
                                              x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_), 
                                              atmVol_->value() * sqrt(T_),
                                              domesticTS_->discount(T_));
                 Real price25PutBS = blackFormula(Option::Put,put25Strike,
                                              x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_),
                                              atmVol_->value() * sqrt(T_),
                                              domesticTS_->discount(T_));

                 //market price
                 Real priceAtmCallMkt = blackFormula(Option::Call,atmStrike,
                                              x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_), 
                                              atmVol_->value() * sqrt(T_),
                                              domesticTS_->discount(T_));
                 Real price25CallMkt = blackFormula(Option::Call,call25Strike,
                                              x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_), 
                                              call25Vol * sqrt(T_),
                                              domesticTS_->discount(T_));
                 Real price25PutMkt = blackFormula(Option::Put,put25Strike,
                                              x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_),
                                              put25Vol * sqrt(T_),
                                              domesticTS_->discount(T_));

                  //Analytical Black Scholes formula
                 NormalDistribution norm;
                 Real d1atm = (std::log(x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_)/atmStrike) 
                           + 0.5*std::pow(atmVolQuote->value(),2.0) * T_)/(atmVolQuote->value() * sqrt(T_));
                 Real vegaAtm_Analytical = x0Quote->value() * norm(d1atm) * sqrt(T_) * foreignTS_->discount(T_);
                 Real vannaAtm_Analytical = vegaAtm_Analytical/x0Quote->value() *(1.0 - d1atm/(atmVolQuote->value()*sqrt(T_)));
                 Real volgaAtm_Analytical = vegaAtm_Analytical * d1atm * (d1atm - atmVolQuote->value() * sqrt(T_))/atmVolQuote->value();

                 Real d125call = (std::log(x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_)/call25Strike) 
                           + 0.5*std::pow(atmVolQuote->value(),2.0) * T_)/(atmVolQuote->value() * sqrt(T_));
                 Real vega25Call_Analytical = x0Quote->value() * norm(d125call) * sqrt(T_) * foreignTS_->discount(T_);
                 Real vanna25Call_Analytical = vega25Call_Analytical/x0Quote->value() *(1.0 - d125call/(atmVolQuote->value()*sqrt(T_)));
                 Real volga25Call_Analytical = vega25Call_Analytical * d125call * (d125call - atmVolQuote->value() * sqrt(T_))/atmVolQuote->value();

                 Real d125Put = (std::log(x0Quote->value()* foreignTS_->discount(T_)/ domesticTS_->discount(T_)/put25Strike) 
                           + 0.5*std::pow(atmVolQuote->value(),2.0) * T_)/(atmVolQuote->value() * sqrt(T_));
                 Real vega25Put_Analytical = x0Quote->value() * norm(d125Put) * sqrt(T_) * foreignTS_->discount(T_);
                 Real vanna25Put_Analytical = vega25Put_Analytical/x0Quote->value() *(1.0 - d125Put/(atmVolQuote->value()*sqrt(T_)));
                 Real volga25Put_Analytical = vega25Put_Analytical * d125Put * (d125Put - atmVolQuote->value() * sqrt(T_))/atmVolQuote->value();


                 //BS vega
                 boost::static_pointer_cast<SimpleQuote> (atmVolQuote.currentLink())->setValue(atmVolQuote->value() + sigmaShift_vega);
                 doubleBarrierOption.recalculate();
                 Real vegaBarBS = (doubleBarrierOption.NPV() - priceBS)/sigmaShift_vega;
                 boost::static_pointer_cast<SimpleQuote> (atmVolQuote.currentLink())->setValue(atmVolQuote->value() - sigmaShift_vega);//setback

                 //BS volga

                 //vegaBar2
                 //base NPV
                 boost::static_pointer_cast<SimpleQuote> (atmVolQuote.currentLink())->setValue(atmVolQuote->value() + sigmaShift_volga);
                 doubleBarrierOption.recalculate();
                 Real priceBS2 = doubleBarrierOption.NPV();

                 //shifted npv
                 boost::static_pointer_cast<SimpleQuote> (atmVolQuote.currentLink())->setValue(atmVolQuote->value() + sigmaShift_vega);
                 doubleBarrierOption.recalculate();
                 Real vegaBarBS2 = (doubleBarrierOption.NPV() - priceBS2)/sigmaShift_vega;
                 Real volgaBarBS = (vegaBarBS2 - vegaBarBS)/sigmaShift_volga;
                 boost::static_pointer_cast<SimpleQuote> (atmVolQuote.currentLink())->setValue(atmVolQuote->value()
                                                                                               - sigmaShift_volga 
                                                                                               - sigmaShift_vega);//setback

                 //BS Delta
                 //base delta
                 boost::static_pointer_cast<SimpleQuote> (x0Quote.currentLink())->setValue(x0Quote->value() + spotShift_delta);//shift forth
                 doubleBarrierOption.recalculate();
                 Real priceBS_delta1 = doubleBarrierOption.NPV();

                 boost::static_pointer_cast<SimpleQuote> (x0Quote.currentLink())->setValue(x0Quote->value() - 2 * spotShift_delta);//shift back
                 doubleBarrierOption.recalculate();
                 Real priceBS_delta2 = doubleBarrierOption.NPV();

                 boost::static_pointer_cast<SimpleQuote> (x0Quote.currentLink())->setValue(x0Quote->value() +  spotShift_delta);//set back
                 Real deltaBar1 = (priceBS_delta1 - priceBS_delta2)/(2.0*spotShift_delta);

                 //shifted vanna
                 boost::static_pointer_cast<SimpleQuote> (atmVolQuote.currentLink())->setValue(atmVolQuote->value() + sigmaShift_vanna);//shift sigma
                 //shifted delta
                 boost::static_pointer_cast<SimpleQuote> (x0Quote.currentLink())->setValue(x0Quote->value() + spotShift_delta);//shift forth
                 doubleBarrierOption.recalculate();
                 priceBS_delta1 = doubleBarrierOption.NPV();

                 boost::static_pointer_cast<SimpleQuote> (x0Quote.currentLink())->setValue(x0Quote->value() - 2 * spotShift_delta);//shift back
                 doubleBarrierOption.recalculate();
                 priceBS_delta2 = doubleBarrierOption.NPV();

                 boost::static_pointer_cast<SimpleQuote> (x0Quote.currentLink())->setValue(x0Quote->value() +  spotShift_delta);//set back
                 Real deltaBar2 = (priceBS_delta1 - priceBS_delta2)/(2.0*spotShift_delta);

                 Real vannaBarBS = (deltaBar2 - deltaBar1)/sigmaShift_vanna;

                 boost::static_pointer_cast<SimpleQuote> (atmVolQuote.currentLink())->setValue(atmVolQuote->value() - sigmaShift_vanna);//set back

                 //Matrix
                 Matrix A(3,3,0.0);

                 //analytical
                 A[0][0] = vegaAtm_Analytical;
                 A[0][1] = vega25Call_Analytical;
                 A[0][2] = vega25Put_Analytical;
                 A[1][0] = vannaAtm_Analytical;
                 A[1][1] = vanna25Call_Analytical;
                 A[1][2] = vanna25Put_Analytical;
                 A[2][0] = volgaAtm_Analytical;
                 A[2][1] = volga25Call_Analytical;
                 A[2][2] = volga25Put_Analytical;

                 Array b(3,0.0);
                 b[0] = vegaBarBS;
                 b[1] = vannaBarBS;
                 b[2] = volgaBarBS;
                 Array q = inverse(A) * b;

                 Real H = arguments_.barrier[1];
                 Real L = arguments_.barrier[0];
                 Real theta_tilt_minus = ((domesticTS_->zeroRate(T_, Continuous) - foreignTS_->zeroRate(T_, Continuous))/atmVol_->value() - atmVol_->value()/2.0)*std::sqrt(T_);
                 Real h = 1.0/atmVol_->value() * std::log(H/x0Quote->value())/std::sqrt(T_);
                 Real l = 1.0/atmVol_->value() * std::log(L/x0Quote->value())/std::sqrt(T_);
                 CumulativeNormalDistribution cnd;

                 Real doubleNoTouch = 0.0;
                 for(int j = -series_; j< series_;j++ ){
                     Real e_minus = 2*j*(h-l) - theta_tilt_minus;
                     doubleNoTouch += std::exp(-2.0*j*theta_tilt_minus*(h-l))*(cnd(h+e_minus) - cnd(l+e_minus))
                                      - std::exp(-2.0*j*theta_tilt_minus*(h-l)+2.0*theta_tilt_minus*h)*(cnd(h-2.0*h+e_minus) - cnd(l-2.0*h+e_minus));
                 }

                 Real p_survival = doubleNoTouch;

                 Real lambda = p_survival ;
                 Real adjust = q[0]*(priceAtmCallMkt - priceAtmCallBS)
                                          + q[1]*(price25CallMkt - price25CallBS)
                                          + q[2]*(price25PutMkt - price25PutBS);
                 Real outPrice = priceBS + lambda*adjust;//
                 Real inPrice;

                 //adapt Vanilla delta
                 if(adaptVanDelta_ == true){
                     outPrice += lambda*(bsPriceWithSmile_ - vanillaOption);
                     //capfloored by (0, vanilla)
                     outPrice = std::max(0.0, std::min(bsPriceWithSmile_, outPrice));
                     inPrice = bsPriceWithSmile_ - outPrice;
                 }
                 else{
                     //capfloored by (0, vanilla)
                     outPrice = std::max(0.0, std::min(vanillaOption , outPrice));
                     inPrice = vanillaOption - outPrice;
                 }

                 if(arguments_.barrierType[0] == Barrier::DownOut)
                    results_.value = outPrice;
                 else
                    results_.value = inPrice;
                 results_.additionalResults["VanillaPrice"] = vanillaOption;
                 results_.additionalResults["BarrierInPrice"] = inPrice;
                 results_.additionalResults["BarrierOutPrice"] = outPrice;
                 results_.additionalResults["lambda"] = lambda;
         }
    }
}
