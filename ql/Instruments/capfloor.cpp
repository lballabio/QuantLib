/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
#include <ql/Instruments/swap.hpp>
#include <ql/CashFlows/floatingratecoupon.hpp>
#include <ql/CashFlows/fixedratecoupon.hpp>
#include <ql/PricingEngines/CapFloor/blackcapfloorengine.hpp>
#include <ql/Solvers1D/brent.hpp>
#include <ql/Solvers1D/newtonsafe.hpp>
#include <ql/CashFlows/analysis.hpp>

namespace QuantLib {

    CapFloor::CapFloor(
                 CapFloor::Type type,
                 const std::vector<boost::shared_ptr<CashFlow> >& floatingLeg,
                 const std::vector<Rate>& capRates,
                 const std::vector<Rate>& floorRates,
                 const Handle<YieldTermStructure>& termStructure,
                 const boost::shared_ptr<PricingEngine>& engine)
    : type_(type), floatingLeg_(floatingLeg),
      capRates_(capRates), floorRates_(floorRates),
      termStructure_(termStructure) {
        setPricingEngine(engine);
        if (type_ == Cap || type_ == Collar) {
            QL_REQUIRE(!capRates_.empty(), "no cap rates given");
            while (capRates_.size() < floatingLeg_.size())
                capRates_.push_back(capRates_.back());
        }
        if (type_ == Floor || type_ == Collar) {
            QL_REQUIRE(!floorRates_.empty(), "no floor rates given");
            while (floorRates_.size() < floatingLeg_.size())
                floorRates_.push_back(floorRates_.back());
        }
        std::vector<boost::shared_ptr<CashFlow> >::const_iterator i;
        for (i = floatingLeg_.begin(); i != floatingLeg_.end(); ++i)
            registerWith(*i);

        registerWith(termStructure);
        registerWith(Settings::instance().evaluationDate());
    }

    void CapFloor::fetchResults (const Results* r) const{
        Instrument::fetchResults(r);
        const CapFloor::results* results =
            dynamic_cast<const CapFloor::results*>(r);
        vega_ = results->vega;
        
    };

    Rate CapFloor::atmRate() const {

        Real bps = Cashflows::bps(floatingLeg_, termStructure_);
        Real npv = Cashflows::npv(floatingLeg_, termStructure_);
        return 1.0e-4*npv/bps;

        //std::vector<boost::shared_ptr<CashFlow> > fixedLeg_(floatingLeg_.size());
        //Rate fixedRate = 0.04;

        //for (Size i = 0; i < fixedLeg_.size(); i++){
        //    boost::shared_ptr<FloatingRateCoupon> floatingRateCoupon  
        //        = boost::dynamic_pointer_cast<FloatingRateCoupon>(floatingLeg_[i]);
        //    fixedLeg_[i] = boost::shared_ptr<CashFlow>
        //        (new FixedRateCoupon(floatingRateCoupon->nominal(),
        //                             floatingRateCoupon->date(),
        //                             fixedRate,
        //                             floatingRateCoupon->dayCounter(),
        //                             floatingRateCoupon->accrualStartDate(),
        //                             floatingRateCoupon->accrualEndDate()));
        //}

        //boost::shared_ptr<Swap> swap(
        //    new Swap(termStructure_, floatingLeg_, fixedLeg_));
        //return fixedRate - swap->NPV()/(swap->legBPS(1)/1.0e-4);
    };

    bool CapFloor::isExpired() const {
        Date lastPaymentDate = Date::minDate();
        for (Size i=0; i<floatingLeg_.size(); i++)
            lastPaymentDate = std::max(lastPaymentDate, floatingLeg_[i]->date());
        return lastPaymentDate < termStructure_->referenceDate();
    }

    void CapFloor::setupArguments(Arguments* args) const {
        CapFloor::arguments* arguments =
            dynamic_cast<CapFloor::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->type = type_;
        arguments->capRates.clear();
        arguments->floorRates.clear();
        arguments->startTimes.clear();
        arguments->fixingTimes.clear();
        arguments->fixingDates.clear();
        arguments->endTimes.clear();
        arguments->accrualTimes.clear();
        arguments->forwards.clear();
        arguments->gearings.clear();
        arguments->discounts.clear();
        arguments->nominals.clear();

        Date today = Settings::instance().evaluationDate();
        Date settlement = termStructure_->referenceDate();
        DayCounter counter = termStructure_->dayCounter();

        for (Size i=0; i<floatingLeg_.size(); i++) {
            boost::shared_ptr<FloatingRateCoupon> coupon =
                boost::dynamic_pointer_cast<FloatingRateCoupon>(
                                                             floatingLeg_[i]);
            QL_REQUIRE(coupon, "non-floating coupon given");
            Date beginDate = coupon->accrualStartDate();
            Time time = counter.yearFraction(settlement, beginDate);
            arguments->startTimes.push_back(time);
            Date fixingDate = coupon->fixingDate();
            arguments->fixingDates.push_back(fixingDate);
            time = counter.yearFraction(today, fixingDate);
            arguments->fixingTimes.push_back(time);
            time = counter.yearFraction(settlement, coupon->date());
            arguments->endTimes.push_back(time);
            // this is passed explicitly for precision
            arguments->accrualTimes.push_back(coupon->accrualPeriod());
            // this is passed explicitly for precision
            if (arguments->endTimes.back() >= 0.0) { // but only if needed
                arguments->forwards.push_back(coupon->adjustedFixing());
                arguments->discounts.push_back(
                                  termStructure_->discount(coupon->date()));
            } else {
                arguments->forwards.push_back(Null<Rate>());
                arguments->discounts.push_back(Null<DiscountFactor>());
            }
            arguments->nominals.push_back(coupon->nominal());
            Spread spread = coupon->spread();
            Real gearing = coupon->gearing();
            QL_REQUIRE(gearing > 0.0, "positive gearing required");
            arguments->gearings.push_back(gearing);
            if (type_ == Cap || type_ == Collar)
                arguments->capRates.push_back((capRates_[i]-spread)/gearing);
            if (type_ == Floor || type_ == Collar)
                arguments->floorRates.push_back(
                                             (floorRates_[i]-spread)/gearing);
        }
    }

    void CapFloor::arguments::validate() const {
        QL_REQUIRE(endTimes.size() == startTimes.size(),
                   "number of start times (" << startTimes.size()
                   << ") different from that of end times ("
                   << endTimes.size() << ")");
        QL_REQUIRE(accrualTimes.size() == startTimes.size(),
                   "number of start times (" << startTimes.size()
                   << ") different from that of accrual times ("
                   << accrualTimes.size() << ")");
        QL_REQUIRE(type == CapFloor::Floor ||
                   capRates.size() == startTimes.size(),
                   "number of start times (" << startTimes.size()
                   << ") different from that of cap rates ("
                   << capRates.size() << ")");
        QL_REQUIRE(type == CapFloor::Cap ||
                   floorRates.size() == startTimes.size(),
                   "number of start times (" << startTimes.size()
                   << ") different from that of floor rates ("
                   << floorRates.size() << ")");
        QL_REQUIRE(gearings.size() == startTimes.size(),
                   "number of start times (" << startTimes.size()
                   << ") different from that of gearings ("
                   << floorRates.size() << ")");
        QL_REQUIRE(nominals.size() == startTimes.size(),
                   "number of start times (" << startTimes.size()
                   << ") different from that of nominals ("
                   << nominals.size() << ")");
    }

    Volatility CapFloor::impliedVolatility(Real targetValue,
                                           Real accuracy,
                                           Size maxEvaluations,
                                           Volatility minVol,
                                           Volatility maxVol) const {
        calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");

        Volatility guess = 0.10;   // no way we can get a more accurate one

        ImpliedVolHelper f(*this, termStructure_, targetValue);
        Brent solver;
        //NewtonSafe solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }
    
    Real CapFloor::vega() const {
        calculate();
        return vega_;
    }

    Real CapFloor::vega(const Volatility& volatility) const {
        calculate();
        static const Real shift = 1e-4;
        boost::shared_ptr<SimpleQuote> vol(new SimpleQuote(volatility+shift));
        Handle<Quote> h(vol);
        boost::shared_ptr<PricingEngine> engine = 
            boost::shared_ptr<PricingEngine>(new BlackCapFloorEngine(h));
        this->setupArguments(engine_->arguments());
        engine->calculate();
        const Value* value = dynamic_cast<const Value*>(engine_->results());
        Real vega = (value->value - NPV_)/shift;
        return vega;
    }

    CapFloor::ImpliedVolHelper::ImpliedVolHelper(
                              const CapFloor& cap,
                              const Handle<YieldTermStructure>& termStructure,
                              Real targetValue)
    : termStructure_(termStructure), targetValue_(targetValue) {

        vol_ = boost::shared_ptr<SimpleQuote>(new SimpleQuote(0.0));
        Handle<Quote> h(vol_);
        engine_ = boost::shared_ptr<PricingEngine>(new BlackCapFloorEngine(h));
        cap.setupArguments(engine_->arguments());

        results_ = dynamic_cast<const Value*>(engine_->results());
    }

    Real CapFloor::ImpliedVolHelper::operator()(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate();
        return results_->value-targetValue_;
    }

    Real CapFloor::ImpliedVolHelper::derivative(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate();
        return 0.0;
        //return results_->vega;
    }

    std::ostream& operator<<(std::ostream& out, CapFloor::Type t) {
        switch (t) {
          case CapFloor::Cap:
            return out << "Cap";
          case CapFloor::Floor:
            return out << "Floor";
          case CapFloor::Collar:
            return out << "Collar";
          default:
            QL_FAIL("unknown CapFloor::Type (" << Integer(t) << ")");
        }
    }

}
