
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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

        PiecewiseFlatForward::PiecewiseFlatForward(Currency currency,
            const DayCounter& dayCounter, const Date& todaysDate,
            const Calendar& calendar, int settlementDays,
            const std::vector<Handle<RateHelper> >& instruments,
            double accuracy)
        : currency_(currency), dayCounter_(dayCounter),
          todaysDate_(todaysDate), calendar_(calendar),
          settlementDays_(settlementDays), instruments_(instruments),
          needsBootstrap_(true), accuracy_(accuracy) {
            QL_REQUIRE(instruments_.size()>0, "No instrument given");
            settlementDate_ = calendar_.advance(
                todaysDate_,settlementDays_,Days);
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
                    "Two instruments have the same maturity (" +
                    DateFormatter::toString(m1) + ")");
            }
            for (i=0; i<instruments_.size(); i++)
                instruments_[i]->registerObserver(this);
        }

        PiecewiseFlatForward::~PiecewiseFlatForward() {
            for (Size i=0; i<instruments_.size(); i++)
                instruments_[i]->unregisterObserver(this);
        }

        void PiecewiseFlatForward::bootstrap() const {
            // prevent recursively calling bootstrap() when the
            // term structure methods are called by the rate helpers
            needsBootstrap_ = false;
            try {
                // values at settlement date
                maxDate_ = settlementDate_;
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
                                DoubleFormatter::toString(forwards_[j1],3) + " ";
                        }

                        std::string discount_string;
                        for(Size j2 = k; j2 < i; j2++){
                            discount_string += " " +
                                DoubleFormatter::toString(discounts_[j2],10) + " ";
                        }

                        std::string zeroYield_string;
                        for(Size j3 = k; j3 < i; j3++){
                            zeroYield_string += " " +
                                DoubleFormatter::toString(zeroYields_[j3],3) + " ";
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
            curve_->maxDate_ = rateHelper_->maturity();
            curve_->times_.push_back(curve_->dayCounter().yearFraction(
                curve_->settlementDate(),curve_->maxDate_));
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
