
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file piecewiseflatforward.cpp
    \brief piecewise flat forward term structure

    \fullpath
    ql/TermStructures/%piecewiseflatforward.cpp
*/

// $Id$

#include <ql/TermStructures/piecewiseflatforward.hpp>
#include <ql/Solvers1D/brent.hpp>

namespace QuantLib {

    using Solvers1D::Brent;

    namespace TermStructures {

        PiecewiseFlatForward::PiecewiseFlatForward(const Date& todaysDate,
            const Date& settlementDate,
            const std::vector<Handle<RateHelper> >& instruments,
            const DayCounter& dayCounter, double accuracy)
        : dayCounter_(dayCounter), todaysDate_(todaysDate), 
          settlementDate_(settlementDate), instruments_(instruments), 
          needsBootstrap_(true), accuracy_(accuracy) {

            QL_REQUIRE(instruments_.size()>0, "No instrument given");

            // sort risk helpers
            Size i;
            for (i=0; i<instruments_.size(); i++)
                instruments_[i]->setTermStructure(this);
            std::sort(instruments_.begin(),instruments_.end(),
                RateHelperSorter());
            // check that there is no instruments with the same maturity
            for (i=1; i<instruments_.size(); i++) {
                Date m1 = instruments_[i-1]->maturity(),
                     m2 = instruments_[i]->maturity();
                QL_REQUIRE(m1 != m2,
                    "PiecewiseFlatForward::PiecewiseFlatForward : "
                    "Two instruments have the same maturity (" +
                    DateFormatter::toString(m1) + ")");
            }
            for (i=0; i<instruments_.size(); i++)
                registerWith(instruments_[i]);
        }

        PiecewiseFlatForward::PiecewiseFlatForward(
            const Date& todaysDate,
            const std::vector<Date>& dates, const std::vector<Rate>& forwards,
            const DayCounter& dayCounter)
        : dayCounter_(dayCounter), todaysDate_(todaysDate), 
          settlementDate_(dates[0]), needsBootstrap_(false),
          times_(dates.size()), dates_(dates), 
          discounts_(dates.size()), forwards_(forwards), 
          zeroYields_(dates.size()) {

            QL_REQUIRE(dates_.size()>0, "No dates given");
            QL_REQUIRE(dates_.size()==forwards_.size(),
                "mismatch between dates and forwards");
            times_[0]=0.0;
            discounts_[0]=1.0;
            zeroYields_[0]=forwards_[0];
            for (Size i=1; i<dates_.size(); i++) {
                times_[i] = dayCounter_.yearFraction(settlementDate_,
                    dates_[i]);
                discounts_[i] = discounts_[i-1] * QL_EXP(-forwards_[i]*
                    (times_[i]-times_[i-1]));
                zeroYields_[i] = -QL_LOG(discounts_[i])/times_[i];
            }
        }

        void PiecewiseFlatForward::bootstrap() const {
            // prevent recursively calling bootstrap() when the
            // term structure methods are called by the rate helpers
            needsBootstrap_ = false;
            try {
                // values at settlement date
                dates_ = std::vector<Date>(1, settlementDate_);
                times_ = std::vector<Time>(1, 0.0);
                discounts_ = std::vector<DiscountFactor>(1, 1.0);
                forwards_ = zeroYields_ = std::vector<Rate>();

                // the choice of the solver determines whether the
                // accuracy is on the discount or the instrument rate
                Brent solver;

                // bootstrapping loop
                for (Size i=1; i<instruments_.size()+1; i++) {
                    Handle<RateHelper> instrument = instruments_[i-1];
                    // don't try this at home!
                    instrument->setTermStructure(
                        const_cast<PiecewiseFlatForward*>(this));
                    double guess = instrument->discountGuess();
                    if (guess == Null<double>()) {
                        if (i > 1) {    // we can extrapolate
                            guess = this->discount(
                                instrument->maturity(),true);
                        } else {        // any guess will do
                            guess = 0.9;
                        }
                    }
                    // bracket
                    double min = accuracy_*10e-4, max = discounts_[i-1];
                    try{
                        solver.solve(FFObjFunction(this,instrument,i),
                            accuracy_,guess,min,max);
                    } catch (std::exception& e) {

                        Size k = i-1; // only outputs the last results
                        // Size k = 0; // outputs all results
                        std::string forward_string;
                        for(Size j1 = k; j1 < i; j1++){
                            forward_string +=
                                DoubleFormatter::toString(forwards_[j1],3) +
                                " ";
                        }

                        std::string discount_string;
                        for(Size j2 = k; j2 < i; j2++){
                            discount_string += " " +
                                DoubleFormatter::toString(discounts_[j2],10)
                                + " ";
                        }

                        std::string zeroYield_string;
                        for(Size j3 = k; j3 < i; j3++){
                            zeroYield_string += " " +
                                DoubleFormatter::toString(zeroYields_[j3],3) +
                                " ";
                        }


                        throw Error(
                            "Could not bootstrap curve. segment " +
                            IntegerFormatter::toString(i) + " of " +
                            IntegerFormatter::toString(instruments_.size()) +
                            ", last forward = " + forward_string +
                            ", last discount = " + discount_string +
                            ", last zero-yield = " + zeroYield_string  +
                            ", last guess was " +
                            DoubleFormatter::toString(guess,15) +
                            " error generated by " +
                            std::string(e.what())
                            );
                    }
                }
            } catch (...) {
                // signal incomplete state
                needsBootstrap_ = true;
                // rethrow
                throw;
            }
        }

        Rate PiecewiseFlatForward::zeroYieldImpl(Time t,
            bool extrapolate) const {
                if (needsBootstrap_)
                    bootstrap();

                if (t == 0.0) {
                    return zeroYields_[0];
                } else {
                    int n = referenceNode(t, extrapolate);
                    if (t == times_[n]) {
                        return zeroYields_[n];
                    } else {
                        Time tn = times_[n-1];
                        return (zeroYields_[n-1]*tn+forwards_[n]*(t-tn))/t;
                    }
                }
                QL_DUMMY_RETURN(Rate());
        }

        DiscountFactor PiecewiseFlatForward::discountImpl(
            Time t, bool extrapolate) const {
                if (needsBootstrap_)
                    bootstrap();

                if (t == 0.0) {
                    return discounts_[0];
                } else {
                    int n = referenceNode(t, extrapolate);
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

        Rate PiecewiseFlatForward::forwardImpl(Time t,
            bool extrapolate) const {
                if (needsBootstrap_)
                    bootstrap();

                if (t == 0.0) {
                    return forwards_[0];
                } else {
                    return forwards_[referenceNode(t, extrapolate)];
                }
                QL_DUMMY_RETURN(Rate());
        }

        int PiecewiseFlatForward::referenceNode(
            Time t, bool extrapolate) const {
                QL_REQUIRE(t >= 0.0 && (t <= times_.back() || extrapolate),
                    "PiecewiseFlatForward: time (" +
                    DoubleFormatter::toString(t) +
                    ") outside curve definition [" +
                    DoubleFormatter::toString(0.0) + ", " +
                    DoubleFormatter::toString(times_.back()) + "]");
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
            const Handle<RateHelper>& rateHelper,
            int segment)
        : curve_(curve), rateHelper_(rateHelper), segment_(segment) {
            // extend curve to next point
            curve_->dates_.push_back(rateHelper_->maturity());
            curve_->times_.push_back(curve_->dayCounter().yearFraction(
                curve_->settlementDate(),curve_->dates_.back()));
            if (segment_ == 1) {
                // add dummy values at settlement
                curve_->forwards_.push_back(0.0);
                curve_->zeroYields_.push_back(0.0);
            }
            // add dummy values for next point - will be reset by operator()
            curve_->discounts_.push_back(0.0);
            curve_->forwards_.push_back(0.0);
            curve_->zeroYields_.push_back(0.0);
        }

        double PiecewiseFlatForward::FFObjFunction::operator()(
            double discount) const {

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

        bool PiecewiseFlatForward::RateHelperSorter::operator()(
          const Handle<RateHelper>& h1, const Handle<RateHelper>& h2) const {
            return (h1->maturity() < h2->maturity());
        }

    }

}
