
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file piecewiseflatforward.cpp
    \brief piecewise flat forward term structure

    $Source$
    $Log$
    Revision 1.2  2001/05/24 11:15:57  lballabio
    Stripped conventions from Currencies

    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#include "ql/TermStructures/piecewiseflatforward.hpp"
#include "ql/Solvers1D/brent.hpp"
#include <algorithm>

namespace QuantLib {

    using Solvers1D::Brent;

    namespace TermStructures {

        const double PiecewiseFlatForward::accuracy_ = 1.0e-12;
        
        PiecewiseFlatForward::PiecewiseFlatForward(Currency currency, 
            const Handle<DayCounter>& dayCounter, const Date& settlementDate,
            const std::vector<Handle<RateHelper> >& instruments)
        : currency_(currency), dayCounter_(dayCounter), 
          settlementDate_(settlementDate) {
            QL_REQUIRE(instruments.size()>0, "No instrument given");
            // values at settlement date
            discounts_.push_back(1.0);
            nodes_.push_back(settlementDate_);
            times_.push_back(0.0);

            Brent solver;
            // sort risk helpers
            std::vector<Handle<RateHelper> > sortedInstruments = instruments;
			int i;
            for (i=0; i<sortedInstruments.size(); i++)
                sortedInstruments[i]->setTermStructure(this);
            std::sort(sortedInstruments.begin(),sortedInstruments.end(),
                RateHelperSorter());
            // bootstrapping loop
            for (i=1; i<sortedInstruments.size()+1; i++) {
                Handle<RateHelper> instrument = sortedInstruments[i-1];
                double guess = instrument->discountGuess();
                if (guess == Null<double>())
                    guess = discounts_[i-1]*0.9;
                // bracket
                double min = accuracy_, max = discounts_[i-1];
                solver.solve(FFObjFunction(this,instrument,i),
                    accuracy_,guess,min,max);
            }
        }

        Rate PiecewiseFlatForward::zeroYield(
            const Date& d, bool extrapolate) const {
                if (d == settlementDate_) {
                    return zeroYields_[0];
                } else {
                    int n = referenceNode(d, extrapolate);
                    if (d == nodes_[n]) {
                        return zeroYields_[n];
                    } else {
                        Time t = dayCounter_->yearFraction(settlementDate_,d);
                        Time tn = times_[n-1];
                        return (zeroYields_[n-1]*tn+forwards_[n]*(t-tn))/t;
                    }
                }
                QL_DUMMY_RETURN(Rate());
        }

        DiscountFactor PiecewiseFlatForward::discount(
            const Date& d, bool extrapolate) const {
                if (d == settlementDate_) {
                    return discounts_[0];
                } else {
                    int n = referenceNode(d, extrapolate);
                    if (d == nodes_[n]) {
                        return discounts_[n];
                    } else {
                        Time t = dayCounter_->yearFraction(settlementDate_,d);
                        return discounts_[n-1] * 
                            QL_EXP(-forwards_[n] * (t-times_[n-1]));
                    }
                }
                QL_DUMMY_RETURN(DiscountFactor());
        }

        Rate PiecewiseFlatForward::forward(
            const Date& d, bool extrapolate) const {
                if (d == settlementDate_) {
                    return forwards_[0];
                } else {
                    return forwards_[referenceNode(d, extrapolate)];
                }
                QL_DUMMY_RETURN(Rate());
        }

        int PiecewiseFlatForward::referenceNode(
            const Date& d, bool extrapolate) const {
                QL_REQUIRE(d>=minDate() && (d<=maxDate() || extrapolate),
                    "date outside curve definition");
                if (d>=maxDate())
                    return nodes_.size()-1;
                std::vector<Date>::const_iterator i=nodes_.begin(),
                    j=nodes_.end(), k;
                while (j-i > 1) {
                    k = i+(j-i)/2;
                    if (d <= *k)
                        j = k;
                    else
                        i = k;
                }
                return (j-nodes_.begin());
        }

        PiecewiseFlatForward::FFObjFunction::FFObjFunction(
            PiecewiseFlatForward* curve, const Handle<RateHelper>& rateHelper,
            int segment)
        : curve_(curve), rateHelper_(rateHelper), segment_(segment) {
            // extend curve to next point
            curve_->nodes_.push_back(rateHelper_->maturity());
            curve_->times_.push_back(curve_->dayCounter()->yearFraction(
                curve_->settlementDate(),curve_->nodes_[segment_]));
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
            double discountGuess) const {
                curve_->discounts_[segment_] = discountGuess;
                curve_->zeroYields_[segment_] = 
                    -QL_LOG(discountGuess) / curve_->times_[segment_];
                curve_->forwards_[segment_] = 
                    QL_LOG(curve_->discounts_[segment_-1]/discountGuess) /
                    (curve_->times_[segment_]-curve_->times_[segment_-1]);
                if (segment_ == 1) {
                    curve_->forwards_[0] = curve_->zeroYields_[0] = 
                        curve_->forwards_[1];
                }
                return rateHelper_->rateError();
        }

        bool PiecewiseFlatForward::RateHelperSorter::operator()(
            const Handle<RateHelper>& h1, const Handle<RateHelper>& h2) const {
                return (h1->maturity() < h2->maturity());
        }

    }

}
