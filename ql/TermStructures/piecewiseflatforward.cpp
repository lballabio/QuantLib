
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/TermStructures/piecewiseflatforward.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    namespace {

        class RateHelperSorter {
          public:
            bool operator()(const boost::shared_ptr<RateHelper>& h1,
                            const boost::shared_ptr<RateHelper>& h2) const {
                return (h1->latestDate() < h2->latestDate());
            }
        };

    }

    // objective function for solver
    class PiecewiseFlatForward::FFObjFunction {
      public:
        FFObjFunction(const PiecewiseFlatForward*,
                      const boost::shared_ptr<RateHelper>&, Size segment);
        Real operator()(DiscountFactor discountGuess) const;
      private:
        const PiecewiseFlatForward* curve_;
        boost::shared_ptr<RateHelper> rateHelper_;
        Size segment_;
    };

    #ifndef QL_DISABLE_DEPRECATED
    PiecewiseFlatForward::PiecewiseFlatForward(
               const Date& todaysDate,
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter, Real accuracy)
    : YieldTermStructure(todaysDate,referenceDate), dayCounter_(dayCounter),
      instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    PiecewiseFlatForward::PiecewiseFlatForward(
                                           const Date& todaysDate,
                                           const std::vector<Date>& dates,
                                           const std::vector<Rate>& forwards,
                                           const DayCounter& dayCounter)
    : YieldTermStructure(todaysDate, dates[0]), dayCounter_(dayCounter),
      times_(dates.size()), dates_(dates), discounts_(dates.size()),
      forwards_(forwards), zeroYields_(dates.size()) {

        QL_REQUIRE(!dates_.empty(), "no dates given");
        QL_REQUIRE(dates_.size()==forwards_.size(),
                   "mismatch between dates and forwards");
        times_[0]=0.0;
        discounts_[0]=1.0;
        zeroYields_[0]=forwards_[0];
        for (Size i=1; i<dates_.size(); i++) {
            times_[i] = dayCounter.yearFraction(referenceDate(),
                                                 dates_[i]);
            zeroYields_[i] = (forwards_[i]*(times_[i]-times_[i-1])+
                              zeroYields_[i-1]*times_[i-1])/times_[i];
            discounts_[i] = QL_EXP(-zeroYields_[i]*times_[i]);
        }
        // we don't want to launch the boostrapping process
        freeze();
    }
    #endif

    PiecewiseFlatForward::PiecewiseFlatForward(
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy)
    : YieldTermStructure(referenceDate),
      dayCounter_(dayCounter), instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    PiecewiseFlatForward::PiecewiseFlatForward(
               Integer settlementDays, const Calendar& calendar,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               Real accuracy)
    : YieldTermStructure(settlementDays, calendar),
      dayCounter_(dayCounter), instruments_(instruments), accuracy_(accuracy) {
        checkInstruments();
    }

    PiecewiseFlatForward::PiecewiseFlatForward(
                                           const std::vector<Date>& dates,
                                           const std::vector<Rate>& forwards,
                                           const DayCounter& dayCounter)
    : YieldTermStructure(dates[0]),
      dayCounter_(dayCounter), times_(dates.size()), dates_(dates),
      discounts_(dates.size()), forwards_(forwards),
      zeroYields_(dates.size()) {

        QL_REQUIRE(dates_.size()>0,
                   "no dates given");
        QL_REQUIRE(dates_.size()==forwards_.size(),
                   "mismatch between dates and forwards");
        times_[0]=0.0;
        discounts_[0]=1.0;
        zeroYields_[0]=forwards_[0];
        for (Size i=1; i<dates_.size(); i++) {
            times_[i] = dayCounter.yearFraction(referenceDate(),
                                                dates_[i]);
            zeroYields_[i] = (forwards_[i]*(times_[i]-times_[i-1])+
                              zeroYields_[i-1]*times_[i-1])/times_[i];
            discounts_[i] = QL_EXP(-zeroYields_[i]*times_[i]);
        }
        // we don't want to launch the boostrapping process
        freeze();
    }

    void PiecewiseFlatForward::checkInstruments() {

        QL_REQUIRE(!instruments_.empty(), "no instrument given");

        // sort rate helpers
        Size i;
        for (i=0; i<instruments_.size(); i++)
            instruments_[i]->setTermStructure(this);
        std::sort(instruments_.begin(),instruments_.end(),
                  RateHelperSorter());
        // check that there is no instruments with the same maturity
        for (i=1; i<instruments_.size(); i++) {
            Date m1 = instruments_[i-1]->latestDate(),
                 m2 = instruments_[i]->latestDate();
            QL_REQUIRE(m1 != m2,
                       "two instruments have the same maturity (" +
                       DateFormatter::toString(m1) + ")");
        }
        for (i=0; i<instruments_.size(); i++)
            registerWith(instruments_[i]);
    }

    void PiecewiseFlatForward::performCalculations() const {
        // values at reference date
        dates_ = std::vector<Date>(1, referenceDate());
        times_ = std::vector<Time>(1, 0.0);
        discounts_ = std::vector<DiscountFactor>(1, 1.0);
        forwards_ = zeroYields_ = std::vector<Rate>();

        // the choice of the solver determines whether the
        // accuracy is on the discount or the instrument rate
        Brent solver;

        // bootstrapping loop
        for (Size i=1; i<instruments_.size()+1; i++) {
            boost::shared_ptr<RateHelper> instrument = instruments_[i-1];
            // don't try this at home!
            instrument->setTermStructure(
                                     const_cast<PiecewiseFlatForward*>(this));
            DiscountFactor guess = instrument->discountGuess();
            if (guess == Null<DiscountFactor>()) {
                if (i > 1) {    // we can extrapolate
                    guess = this->discount(instrument->latestDate(),true);
                } else {        // any guess will do
                    guess = 0.9;
                }
            }
            // bracket
            DiscountFactor min = accuracy_*1.0e-3;
            #if defined(QL_NEGATIVE_RATES)
            // discount are not required to be decreasing--all bets are off.
            // We choose as max a value very unlikely to be exceeded.
            DiscountFactor max = 3.0;
            #else
            DiscountFactor max = discounts_[i-1];
            #endif
            solver.solve(FFObjFunction(this,instrument,i),
                         accuracy_,guess,min,max);
        }
    }

    Rate PiecewiseFlatForward::zeroYieldImpl(Time t) const {
        calculate();
        if (t == 0.0) {
            return zeroYields_[0];
        } else {
            Size n = referenceNode(t);
            if (t == times_[n]) {
                return zeroYields_[n];
            } else {
                Time tn = times_[n-1];
                return (zeroYields_[n-1]*tn+forwards_[n]*(t-tn))/t;
            }
        }
        QL_DUMMY_RETURN(Rate());
    }

    DiscountFactor PiecewiseFlatForward::discountImpl(Time t) const {
        calculate();
        if (t == 0.0) {
            return discounts_[0];
        } else {
            Size n = referenceNode(t);
            if (t == times_[n]) {
                return discounts_[n];
            } else {
                if (n == 0)
                    return 1.0;
                return discounts_[n-1] *
                    QL_EXP(-forwards_[n] * (t-times_[n-1]));
            }
        }
        QL_DUMMY_RETURN(DiscountFactor());
    }

    Rate PiecewiseFlatForward::forwardImpl(Time t) const {
        calculate();
        if (t == 0.0) {
            return forwards_[0];
        } else {
            return forwards_[referenceNode(t)];
        }
        QL_DUMMY_RETURN(Rate());
    }

    #ifndef QL_DISABLE_DEPRECATED
    Rate PiecewiseFlatForward::compoundForwardImpl(Time t, Integer compFreq)
                                                                      const {
		Rate zy = zeroYieldImpl(t);
		if (compFreq == 0)
            return zy;
		if (t <= 1.0/compFreq)
            return (QL_EXP(zy*t)-1.0)/t;
		return (QL_EXP(zy*(1.0/compFreq))-1.0)*compFreq;
	}
    #endif

    Size PiecewiseFlatForward::referenceNode(Time t) const {
        if (t>=times_.back())
            return times_.size()-1;
        std::vector<Time>::const_iterator i=times_.begin(),
            j=times_.end(), k;
        while (j-i > 1) {
            k = i+(j-i)/2;
            if (t <= *k)
                j = k;
            else
                i = k;
        }
        return (j-times_.begin());
    }

    PiecewiseFlatForward::FFObjFunction::FFObjFunction(
                              const PiecewiseFlatForward* curve,
                              const boost::shared_ptr<RateHelper>& rateHelper,
                              Size segment)
    : curve_(curve), rateHelper_(rateHelper), segment_(segment) {
        // extend curve to next point
        curve_->dates_.push_back(rateHelper_->latestDate());
        curve_->times_.push_back(
                            curve_->timeFromReference(curve_->dates_.back()));
        if (segment_ == 1) {
            // add dummy values at reference
            curve_->forwards_.push_back(0.0);
            curve_->zeroYields_.push_back(0.0);
        }
        // add dummy values for next point - will be reset by operator()
        curve_->discounts_.push_back(0.0);
        curve_->forwards_.push_back(0.0);
        curve_->zeroYields_.push_back(0.0);
    }

    Real PiecewiseFlatForward::FFObjFunction::operator()(
                                              DiscountFactor discount) const {
        curve_->discounts_[segment_] = discount;
        curve_->zeroYields_[segment_] =
            -QL_LOG(discount) / curve_->times_[segment_];
        curve_->forwards_[segment_] =
            QL_LOG(curve_->discounts_[segment_-1]/discount) /
            (curve_->times_[segment_]-curve_->times_[segment_-1]);
        if (segment_ == 1) {
            curve_->forwards_[0] = curve_->zeroYields_[0] =
                curve_->forwards_[1];
        }
        return rateHelper_->quoteError();
    }

}
