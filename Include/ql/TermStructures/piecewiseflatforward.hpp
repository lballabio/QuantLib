
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file piecewiseflatforward.hpp
    \brief piecewise flat forward term structure

    $Id$
*/

// $Source$
// $Log$
// Revision 1.8  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/06/13 16:18:23  lballabio
// Polished rate helper interfaces
//
// Revision 1.6  2001/06/12 13:43:04  lballabio
// Today's date is back into term structures
// Instruments are now constructed with settlement days instead of settlement date
//
// Revision 1.5  2001/05/25 09:29:40  nando
// smoothing #include xx.hpp and cutting old Log messages
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_piecewise_flat_forward_curve_h
#define quantlib_piecewise_flat_forward_curve_h

#include "ql/TermStructures/ratehelpers.hpp"
#include "ql/solver1d.hpp"

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
            maturity marks the end of such segment is correctly repriced on the 
            curve.
            
            \warning The bootstrapping algorithm will raise an exception if any 
            two instruments have the same maturity date.
        */
        /* This class is derived directly from term structure since we are 
           rewriting all of forward, discount and zeroYield to take advantage 
           of its own internal structure. */
        class PiecewiseFlatForward : public TermStructure {
          public:
            // constructor
            PiecewiseFlatForward(Currency currency,
                const Handle<DayCounter>& dayCounter, const Date& todaysDate, 
                const Handle<Calendar>& calendar, int settlementDays,
                const std::vector<Handle<RateHelper> >& instruments);
            // inspectors
            Currency currency() const;
            Handle<DayCounter> dayCounter() const;
            Date todaysDate() const;
            int settlementDays() const;
            Handle<Calendar> calendar() const;
            Date settlementDate() const;
            Date maxDate() const;
            Date minDate() const;
            // zero yield
            Rate zeroYield(const Date&, bool extrapolate = false) const;
            // discount
            DiscountFactor discount(const Date&,
                bool extrapolate = false) const;
            // forward (instantaneous)
            Rate forward(const Date&, bool extrapolate = false) const;
          private:
            // inner classes
            // objective function for solver
            class FFObjFunction;
            friend class FFObjFunction;
            class FFObjFunction : public ObjectiveFunction {
              public:
                FFObjFunction(PiecewiseFlatForward*,
                    const Handle<RateHelper>&, int segment);
                double operator()(double discountGuess) const;
              private:
                PiecewiseFlatForward* curve_;
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
            int referenceNode(const Date& d, bool extrapolate) const;
            // data members
            Currency currency_;
            Handle<DayCounter> dayCounter_;
            Date todaysDate_;
            Handle<Calendar> calendar_;
            int settlementDays_;
            Date settlementDate_;
            std::vector<Date> nodes_;
            std::vector<Time> times_;
            std::vector<DiscountFactor> discounts_;
            std::vector<Rate> forwards_, zeroYields_;
            static const double accuracy_;
        };

        // inline definitions

        inline Currency PiecewiseFlatForward::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> PiecewiseFlatForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date PiecewiseFlatForward::todaysDate() const {
            return todaysDate_;
        }

        inline Handle<Calendar> PiecewiseFlatForward::calendar() const {
            return calendar_;
        }

        inline int PiecewiseFlatForward::settlementDays() const {
            return settlementDays_;
        }

        inline Date PiecewiseFlatForward::settlementDate() const {
            return settlementDate_;
        }

        inline Date PiecewiseFlatForward::maxDate() const {
            return nodes_.back();
        }

        inline Date PiecewiseFlatForward::minDate() const {
            return settlementDate_;
        }

    }

}


#endif
