/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006, 2007 StatPro Italia srl

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

#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/yieldtermstructure.hpp>
#include <ql/quotes/simplequote.hpp>

namespace {
    using namespace QuantLib;
    class ImpliedVolHelper {
          public:
            ImpliedVolHelper(const CapFloor&,
                             const Handle<YieldTermStructure>&,
                             Real targetValue);
            Real operator()(Volatility x) const;
            Real derivative(Volatility x) const;
          private:
            boost::shared_ptr<PricingEngine> engine_;
            Handle<YieldTermStructure> discountCurve_;
            Real targetValue_;
            boost::shared_ptr<SimpleQuote> vol_;
            const Instrument::results* results_;
        };

     ImpliedVolHelper::ImpliedVolHelper(
                              const CapFloor& cap,
                              const Handle<YieldTermStructure>& discountCurve,
                              Real targetValue)
    : discountCurve_(discountCurve), targetValue_(targetValue) {

        vol_ = boost::shared_ptr<SimpleQuote>(new SimpleQuote(0.0));
        Handle<Quote> h(vol_);
        engine_ = boost::shared_ptr<PricingEngine>(new BlackCapFloorEngine(h));
        cap.setupArguments(engine_->getArguments());

        results_ =
            dynamic_cast<const Instrument::results*>(engine_->getResults());
    }

    Real ImpliedVolHelper::operator()(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate();
        return results_->value-targetValue_;
    }

    Real ImpliedVolHelper::derivative(Volatility x) const {
        vol_->setValue(x);
        engine_->calculate();
        return 0.0;
        //return results_->vega;
    }
}

namespace QuantLib {

    CapFloor::CapFloor(
                 CapFloor::Type type,
                 const Leg& floatingLeg,
                 const std::vector<Rate>& capRates,
                 const std::vector<Rate>& floorRates,
                 const Handle<YieldTermStructure>& discountCurve,
                 const boost::shared_ptr<PricingEngine>& engine)
    : type_(type), floatingLeg_(floatingLeg),
      capRates_(capRates), floorRates_(floorRates),
      discountCurve_(discountCurve) {
        setPricingEngine(engine);
        if (type_ == Cap || type_ == Collar) {
            QL_REQUIRE(!capRates_.empty(), "no cap rates given");
            capRates_.reserve(floatingLeg_.size());
            while (capRates_.size() < floatingLeg_.size())
                capRates_.push_back(capRates_.back());
        }
        if (type_ == Floor || type_ == Collar) {
            QL_REQUIRE(!floorRates_.empty(), "no floor rates given");
            floorRates_.reserve(floatingLeg_.size());
            while (floorRates_.size() < floatingLeg_.size())
                floorRates_.push_back(floorRates_.back());
        }
        Leg::const_iterator i;
        for (i = floatingLeg_.begin(); i != floatingLeg_.end(); ++i)
            registerWith(*i);

        registerWith(discountCurve);
        registerWith(Settings::instance().evaluationDate());
    }

    CapFloor::CapFloor(
                 CapFloor::Type type,
                 const Leg& floatingLeg,
                 const std::vector<Rate>& strikes,
                 const Handle<YieldTermStructure>& discountCurve,
                 const boost::shared_ptr<PricingEngine>& engine)
    : type_(type), floatingLeg_(floatingLeg),
      discountCurve_(discountCurve) {
        setPricingEngine(engine);
        QL_REQUIRE(!strikes.empty(), "no strikes given");
        if (type_ == Cap) {
            capRates_ = strikes;
            capRates_.reserve(floatingLeg_.size());
            while (capRates_.size() < floatingLeg_.size())
                capRates_.push_back(capRates_.back());
        } else if (type_ == Floor) {
            floorRates_ = strikes;
            floorRates_.reserve(floatingLeg_.size());
            while (floorRates_.size() < floatingLeg_.size())
                floorRates_.push_back(floorRates_.back());
        } else
            QL_FAIL("only Cap/Floor types allowed in this constructor");

        Leg::const_iterator i;
        for (i = floatingLeg_.begin(); i != floatingLeg_.end(); ++i)
            registerWith(*i);

        registerWith(discountCurve);
        registerWith(Settings::instance().evaluationDate());
    }

    Rate CapFloor::atmRate() const {
        return CashFlows::atmRate(floatingLeg_, **discountCurve_);
    }

    bool CapFloor::isExpired() const {
        Date lastPaymentDate = Date::minDate();
        for (Size i=0; i<floatingLeg_.size(); i++)
            lastPaymentDate = std::max(lastPaymentDate,
                                       floatingLeg_[i]->date());
        return lastPaymentDate < discountCurve_->referenceDate();
    }

    Date CapFloor::startDate() const {
        return CashFlows::startDate(floatingLeg_);
    }

    Date CapFloor::maturityDate() const {
        return CashFlows::maturityDate(floatingLeg_);
    }

    Date CapFloor::lastFixingDate() const {
        boost::shared_ptr<CashFlow> lastCoupon(floatingLeg_.back());
        boost::shared_ptr<FloatingRateCoupon> lastFloatingCoupon =
            boost::dynamic_pointer_cast<FloatingRateCoupon>(lastCoupon);
        return lastFloatingCoupon->fixingDate();
    }

    Real CapFloor::lastAccrualPeriod() const {
        boost::shared_ptr<CashFlow> lastCF(floatingLeg_.back());
        boost::shared_ptr<Coupon> lastCoupon =
            boost::dynamic_pointer_cast<Coupon>(lastCF);
        return lastCoupon->accrualPeriod();
    }

    void CapFloor::setupArguments(PricingEngine::arguments* args) const {
        CapFloor::arguments* arguments =
            dynamic_cast<CapFloor::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        Size n = floatingLeg_.size();

        arguments->startTimes.clear();
        arguments->startTimes.reserve(n);

        arguments->fixingDates.clear();
        arguments->fixingDates.reserve(n);

        arguments->fixingTimes.clear();
        arguments->fixingTimes.reserve(n);

        arguments->endTimes.clear();
        arguments->endTimes.reserve(n);

        arguments->accrualTimes.clear();
        arguments->accrualTimes.reserve(n);

        arguments->forwards.clear();

        arguments->discounts.clear();

        arguments->nominals.clear();
        arguments->nominals.reserve(n);

        arguments->gearings.clear();
        arguments->gearings.reserve(n);

        arguments->capRates.clear();
        arguments->capRates.reserve(n);

        arguments->floorRates.clear();
        arguments->floorRates.reserve(n);

        arguments->spreads.clear();
        arguments->spreads.reserve(n);

        arguments->type = type_;

        Date today = Settings::instance().evaluationDate();
        Date settlement = discountCurve_->referenceDate();
        DayCounter counter = discountCurve_->dayCounter();

        for (Size i=0; i<n; i++) {
            boost::shared_ptr<FloatingRateCoupon> coupon =
                boost::dynamic_pointer_cast<FloatingRateCoupon>(floatingLeg_[i]);
            QL_REQUIRE(coupon, "non-iborCoupon given");
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
                                  discountCurve_->discount(coupon->date()));
            } else {
                arguments->forwards.push_back(Null<Rate>());
                arguments->discounts.push_back(Null<DiscountFactor>());
            }
            arguments->nominals.push_back(coupon->nominal());
            Spread spread = coupon->spread();
            Real gearing = coupon->gearing();
            QL_REQUIRE(gearing > 0.0, "positive gearing required");
            arguments->gearings.push_back(gearing);
            arguments->spreads.push_back(spread);
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

        ImpliedVolHelper f(*this, discountCurve_, targetValue);
        Brent solver;
        //NewtonSafe solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
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
