
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

    \fullpath
    Sources/TermStructures/%piecewiseflatforward.cpp
    \brief piecewise flat forward term structure

*/

// $Id$
// $Log$
// Revision 1.19  2001/08/31 15:23:48  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.18  2001/08/09 14:59:48  sigmud
// header modification
//
// Revision 1.17  2001/08/08 11:07:50  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.16  2001/08/07 11:25:56  sigmud
// copyright header maintenance
//
// Revision 1.15  2001/07/25 15:47:30  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.14  2001/07/13 14:23:11  sigmud
// removed a few gcc compile warnings
//
// Revision 1.13  2001/07/02 12:36:18  sigmud
// pruned redundant header inclusions
//
// Revision 1.12  2001/06/13 16:18:23  lballabio
// Polished rate helper interfaces
//
// Revision 1.11  2001/06/12 13:43:04  lballabio
// Today's date is back into term structures
// Instruments are now constructed with settlement days instead of settlement date
//
// Revision 1.10  2001/06/04 08:32:31  lballabio
// Set extrapolated discount as next guess
//
// Revision 1.9  2001/06/01 16:50:17  lballabio
// Term structure on deposits and swaps
//
// Revision 1.8  2001/05/29 15:12:48  lballabio
// Reintroduced RollingConventions (and redisabled default extrapolation on PFF curve)
//
// Revision 1.7  2001/05/29 12:04:16  lballabio
// Temporarily forced extrapolation
//
// Revision 1.6  2001/05/29 11:57:49  marmar
// Error message is now more clear
//
// Revision 1.5  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.4  2001/05/24 15:40:10  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#include "ql/TermStructures/piecewiseflatforward.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    using Solvers1D::Brent;

    namespace TermStructures {

        const double PiecewiseFlatForward::accuracy_ = 1.0e-12;

        PiecewiseFlatForward::PiecewiseFlatForward(Currency currency,
            const Handle<DayCounter>& dayCounter, const Date& todaysDate, 
            const Handle<Calendar>& calendar, int settlementDays,
            const std::vector<Handle<RateHelper> >& instruments)
        : currency_(currency), dayCounter_(dayCounter),
          todaysDate_(todaysDate), calendar_(calendar), 
          settlementDays_(settlementDays) {
            QL_REQUIRE(instruments.size()>0, "No instrument given");
            settlementDate_ = calendar_->advance(
                todaysDate_,settlementDays_,Days);
            // values at settlement date
            discounts_.push_back(1.0);
            nodes_.push_back(settlementDate_);
            times_.push_back(0.0);

            // the choice of the solver determines whether the accuracy is on 
            // the discount or the instrument rate
            Brent solver;
            // sort risk helpers
            std::vector<Handle<RateHelper> > sortedInstruments = instruments;
			unsigned int i;
            for (i=0; i<sortedInstruments.size(); i++)
                sortedInstruments[i]->setTermStructure(this);
            std::sort(sortedInstruments.begin(),sortedInstruments.end(),
                RateHelperSorter());
            // check that there is no instruments with the same maturity
            for (i=1; i<sortedInstruments.size(); i++) {
                Date m1 = sortedInstruments[i-1]->maturity(),
                     m2 = sortedInstruments[i]->maturity();
                QL_REQUIRE(m1 != m2,
                    "Two instruments have the same maturity (" +
                    DateFormatter::toString(m1) + ")");
            }
            // bootstrapping loop
            for (i=1; i<sortedInstruments.size()+1; i++) {
                Handle<RateHelper> instrument = sortedInstruments[i-1];
                double guess = instrument->discountGuess();
                if (guess == Null<double>()) {
                    if (i > 1)  // we can extrapolate
                        guess = this->discount(instrument->maturity(),true);
                    else        // any guess will do
                        guess = 0.9;
                }
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
                    "date (" +
					DateFormatter::toString(d) +
					") outside curve definition [" +
					DateFormatter::toString(minDate()) + ", " +					
					DateFormatter::toString(maxDate()) + "]");
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
