
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

/*! \file piecewiseflatforward.hpp
    \brief piecewise flat forward term structure

    \fullpath
    ql/TermStructures/%piecewiseflatforward.hpp
*/

// $Id$

#ifndef quantlib_piecewise_flat_forward_curve_h
#define quantlib_piecewise_flat_forward_curve_h

#include <ql/TermStructures/ratehelpers.hpp>
#include <ql/solver1d.hpp>

namespace QuantLib {

    namespace TermStructures {

        //! Piecewise flat forward term structure
        /*! This term structure is bootstrapped on a number of interest rate
            instruments which are passed as a vector of handles to RateHelper
            instances. Their maturities mark the boundaries of the flat
            forward segments.

            The values of the forward rates for each segment are determined
            sequentially starting from the earliest period to the latest.

            The value for each segment is chosen so that the instrument whose
            maturity marks the end of such segment is correctly repriced on
            the curve.

            \warning The bootstrapping algorithm will raise an exception if
            any two instruments have the same maturity date.
        */
        /* This class is derived directly from term structure since we are
           rewriting all of forward, discount and zeroYield to take advantage
           of its own internal structure. */
        class PiecewiseFlatForward : public TermStructure,
                                     public Patterns::Observer {
          public:
            // constructor
            PiecewiseFlatForward(
                Currency currency,
                const DayCounter& dayCounter,
                const Date& todaysDate,
                const Calendar& calendar,
                int settlementDays,
                const std::vector<Handle<RateHelper> >& instruments,
                double accuracy=1.0e-12);
            ~PiecewiseFlatForward();
            //! \name TermStructure interface
            //@{
            Currency currency() const;
            DayCounter dayCounter() const;
            Date todaysDate() const;
            int settlementDays() const;
            Calendar calendar() const;
            Date settlementDate() const;
            Date maxDate() const;
            Date minDate() const;
            Time maxTime() const;
            Time minTime() const;
            //@}
            //! \name Observer interface
            //@{
            void update();
            //@}
          protected:
            Rate zeroYieldImpl(Time, bool extrapolate = false) const;
            DiscountFactor discountImpl(Time,
                bool extrapolate = false) const;
            Rate forwardImpl(Time, bool extrapolate = false) const;
          private:
            // inner classes
            // objective function for solver
            class FFObjFunction;
            friend class FFObjFunction;
            class FFObjFunction : public ObjectiveFunction {
              public:
                FFObjFunction(const PiecewiseFlatForward*,
                    const Handle<RateHelper>&, int segment);
                double operator()(double discountGuess) const;
              private:
                const PiecewiseFlatForward* curve_;
                Handle<RateHelper> rateHelper_;
                int segment_;
            };
            // instrument sorter
            class RateHelperSorter {
              public:
                bool operator()(const Handle<RateHelper>&,
                                const Handle<RateHelper>&) const;
            };
            // methods
            int referenceNode(Time t, bool extrapolate) const;
            void bootstrap() const;
            // data members
            Currency currency_;
            DayCounter dayCounter_;
            Date todaysDate_;
            Calendar calendar_;
            int settlementDays_;
            Date settlementDate_;
            std::vector<Handle<RateHelper> > instruments_;
            mutable bool needsBootstrap_;
            mutable Date maxDate_;
            mutable std::vector<Time> times_;
            mutable std::vector<DiscountFactor> discounts_;
            mutable std::vector<Rate> forwards_, zeroYields_;
            double accuracy_;
        };

        // inline definitions

        inline Currency PiecewiseFlatForward::currency() const {
            return currency_;
        }

        inline DayCounter PiecewiseFlatForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date PiecewiseFlatForward::todaysDate() const {
            return todaysDate_;
        }

        inline Calendar PiecewiseFlatForward::calendar() const {
            return calendar_;
        }

        inline int PiecewiseFlatForward::settlementDays() const {
            return settlementDays_;
        }

        inline Date PiecewiseFlatForward::settlementDate() const {
            return settlementDate_;
        }

        inline Date PiecewiseFlatForward::maxDate() const {
            if (needsBootstrap_) bootstrap();
            return maxDate_;
        }

        inline Date PiecewiseFlatForward::minDate() const {
            return settlementDate_;
        }

        inline Time PiecewiseFlatForward::maxTime() const {
            if (needsBootstrap_) bootstrap();
            return times_.back();
        }

        inline Time PiecewiseFlatForward::minTime() const {
            return 0.0;
        }

        inline void PiecewiseFlatForward::update() {
            needsBootstrap_ = true;
            notifyObservers();
        }

    }

}


#endif
