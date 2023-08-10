/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2014 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006, 2007 StatPro Italia srl
 Copyright (C) 2016 Paolo Mazzocchi

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

#include <ql/any.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/instruments/capfloor.hpp>
#include <ql/math/solvers1d/newtonsafe.hpp>
#include <ql/pricingengines/capfloor/bacheliercapfloorengine.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <utility>

namespace QuantLib {

    namespace {

        class ImpliedCapVolHelper {
          public:
            ImpliedCapVolHelper(const CapFloor&,
                                Handle<YieldTermStructure> discountCurve,
                                Real targetValue,
                                Real displacement,
                                VolatilityType type);
            Real operator()(Volatility x) const;
            Real derivative(Volatility x) const;
          private:
            ext::shared_ptr<PricingEngine> engine_;
            Handle<YieldTermStructure> discountCurve_;
            Real targetValue_;
            ext::shared_ptr<SimpleQuote> vol_;
            const Instrument::results* results_;
        };

        ImpliedCapVolHelper::ImpliedCapVolHelper(const CapFloor& cap,
                                                 Handle<YieldTermStructure> discountCurve,
                                                 Real targetValue,
                                                 Real displacement,
                                                 VolatilityType type)
        : discountCurve_(std::move(discountCurve)), targetValue_(targetValue),
          vol_(ext::make_shared<SimpleQuote>(-1.0)) {

            // vol_ is set an implausible value, so that calculation is forced
            // at first ImpliedCapVolHelper::operator()(Volatility x) call
            Handle<Quote> h(vol_);

            switch (type) {
            case ShiftedLognormal:
                engine_ = ext::shared_ptr<PricingEngine>(new
                    BlackCapFloorEngine(discountCurve_, h, Actual365Fixed(),
                                                                displacement));
                break;
            case Normal:
                engine_ = ext::shared_ptr<PricingEngine>(new
                    BachelierCapFloorEngine(discountCurve_, h, 
                                                            Actual365Fixed()));
                break;
            default:
                QL_FAIL("unknown VolatilityType (" << type << ")");
                break;
            }

            cap.setupArguments(engine_->getArguments());

            results_ =
                dynamic_cast<const Instrument::results*>(engine_->getResults());
        }

        Real ImpliedCapVolHelper::operator()(Volatility x) const {
            if (x!=vol_->value()) {
                vol_->setValue(x);
                engine_->calculate();
            }
            return results_->value-targetValue_;
        }

        Real ImpliedCapVolHelper::derivative(Volatility x) const {
            if (x!=vol_->value()) {
                vol_->setValue(x);
                engine_->calculate();
            }
            auto vega_ = results_->additionalResults.find("vega");
            QL_REQUIRE(vega_ != results_->additionalResults.end(),
                       "vega not provided");
            return ext::any_cast<Real>(vega_->second);
        }
    }

    std::ostream& operator<<(std::ostream& out,
                             CapFloor::Type t) {
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

    CapFloor::CapFloor(CapFloor::Type type,
                       Leg floatingLeg,
                       std::vector<Rate> capRates,
                       std::vector<Rate> floorRates)
    : type_(type), floatingLeg_(std::move(floatingLeg)), capRates_(std::move(capRates)),
      floorRates_(std::move(floorRates)) {
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

        registerWith(Settings::instance().evaluationDate());
    }

    CapFloor::CapFloor(CapFloor::Type type, Leg floatingLeg, const std::vector<Rate>& strikes)
    : type_(type), floatingLeg_(std::move(floatingLeg)) {
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

        registerWith(Settings::instance().evaluationDate());
    }

    bool CapFloor::isExpired() const {
        for (Size i=floatingLeg_.size(); i>0; --i)
            if (!floatingLeg_[i-1]->hasOccurred())
                return false;
        return true;
    }

    Date CapFloor::startDate() const {
        return CashFlows::startDate(floatingLeg_);
    }

    Date CapFloor::maturityDate() const {
        return CashFlows::maturityDate(floatingLeg_);
    }

    ext::shared_ptr<FloatingRateCoupon>
    CapFloor::lastFloatingRateCoupon() const {
        ext::shared_ptr<CashFlow> lastCF(floatingLeg_.back());
        ext::shared_ptr<FloatingRateCoupon> lastFloatingCoupon =
            ext::dynamic_pointer_cast<FloatingRateCoupon>(lastCF);
        return lastFloatingCoupon;
    }

    ext::shared_ptr<CapFloor> CapFloor::optionlet(const Size i) const {
        QL_REQUIRE(i < floatingLeg().size(),
                   io::ordinal(i+1) << " optionlet does not exist, only " <<
                   floatingLeg().size());
        Leg cf(1, floatingLeg()[i]);

        std::vector<Rate> cap, floor;
        if (type() == Cap || type() == Collar)
            cap.push_back(capRates()[i]);
        if (type() == Floor || type() == Collar)
            floor.push_back(floorRates()[i]);

        return ext::make_shared<CapFloor>(type(), cf, cap, floor);
    }

    void CapFloor::setupArguments(PricingEngine::arguments* args) const {
        auto* arguments = dynamic_cast<CapFloor::arguments*>(args);
        QL_REQUIRE(arguments != nullptr, "wrong argument type");

        Size n = floatingLeg_.size();

        arguments->startDates.resize(n);
        arguments->fixingDates.resize(n);
        arguments->endDates.resize(n);
        arguments->accrualTimes.resize(n);
        arguments->forwards.resize(n);
        arguments->nominals.resize(n);
        arguments->gearings.resize(n);
        arguments->capRates.resize(n);
        arguments->floorRates.resize(n);
        arguments->spreads.resize(n);
        arguments->indexes.resize(n);

        arguments->type = type_;

        Date today = Settings::instance().evaluationDate();

        for (Size i=0; i<n; ++i) {
            ext::shared_ptr<FloatingRateCoupon> coupon =
                ext::dynamic_pointer_cast<FloatingRateCoupon>(
                                                             floatingLeg_[i]);
            QL_REQUIRE(coupon, "non-FloatingRateCoupon given");
            arguments->startDates[i] = coupon->accrualStartDate();
            arguments->fixingDates[i] = coupon->fixingDate();
            arguments->endDates[i] = coupon->date();

            // this is passed explicitly for precision
            arguments->accrualTimes[i] = coupon->accrualPeriod();

            // this is passed explicitly for precision...
            if (arguments->endDates[i] >= today) { // ...but only if needed
                arguments->forwards[i] = coupon->adjustedFixing();
            } else {
                arguments->forwards[i] = Null<Rate>();
            }

            arguments->nominals[i] = coupon->nominal();
            Spread spread = coupon->spread();
            Real gearing = coupon->gearing();
            arguments->gearings[i] = gearing;
            arguments->spreads[i] = spread;

            if (type_ == Cap || type_ == Collar)
                arguments->capRates[i] = (capRates_[i]-spread)/gearing;
            else
                arguments->capRates[i] = Null<Rate>();

            if (type_ == Floor || type_ == Collar)
                arguments->floorRates[i] = (floorRates_[i]-spread)/gearing;
            else
                arguments->floorRates[i] = Null<Rate>();

            arguments->indexes[i] = coupon->index();
        }
    }

    void CapFloor::deepUpdate() {
        for (auto& i : floatingLeg_) {
            i->deepUpdate();
        }
        update();
    }

    void CapFloor::arguments::validate() const {
        QL_REQUIRE(endDates.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of end dates ("
                   << endDates.size() << ")");
        QL_REQUIRE(accrualTimes.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of accrual times ("
                   << accrualTimes.size() << ")");
        QL_REQUIRE(type == CapFloor::Floor ||
                   capRates.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of cap rates ("
                   << capRates.size() << ")");
        QL_REQUIRE(type == CapFloor::Cap ||
                   floorRates.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of floor rates ("
                   << floorRates.size() << ")");
        QL_REQUIRE(gearings.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of gearings ("
                   << gearings.size() << ")");
        QL_REQUIRE(spreads.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of spreads ("
                   << spreads.size() << ")");
        QL_REQUIRE(nominals.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of nominals ("
                   << nominals.size() << ")");
        QL_REQUIRE(forwards.size() == startDates.size(),
                   "number of start dates (" << startDates.size()
                   << ") different from that of forwards ("
                   << forwards.size() << ")");
    }

    Rate CapFloor::atmRate(const YieldTermStructure& discountCurve) const {
        bool includeSettlementDateFlows = false;
        Date settlementDate = discountCurve.referenceDate();
        return CashFlows::atmRate(floatingLeg_, discountCurve,
                                  includeSettlementDateFlows,
                                  settlementDate);
    }

    Volatility CapFloor::impliedVolatility(Real targetValue,
                                           const Handle<YieldTermStructure>& d,
                                           Volatility guess,
                                           Real accuracy,
                                           Natural maxEvaluations,
                                           Volatility minVol,
                                           Volatility maxVol,
                                           VolatilityType type,
                                           Real displacement) const {
        //calculate();
        QL_REQUIRE(!isExpired(), "instrument expired");

        ImpliedCapVolHelper f(*this, d, targetValue, displacement, type);
        //Brent solver;
        NewtonSafe solver;
        solver.setMaxEvaluations(maxEvaluations);
        return solver.solve(f, accuracy, guess, minVol, maxVol);
    }

}
