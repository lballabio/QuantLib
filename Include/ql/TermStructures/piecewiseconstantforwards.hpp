
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

/*! \file piecewiseconstantforwards.hpp
    \brief piecewise constant forward rate term structure

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_piecewise_constant_forward_curve_h
#define quantlib_piecewise_constant_forward_curve_h

#include "ql/termstructure.hpp"
#include "ql/depositrate.hpp"
#include <vector>

namespace QuantLib {

    namespace TermStructures {

        class PiecewiseConstantForwards : public TermStructure {
          public:
            // constructor
            PiecewiseConstantForwards(Currency currency,
                                      Handle<DayCounter> dayCounter,
                                      const Date& settlementDate,
                                      const std::vector<DepositRate>& deposits);
            // inspectors
            Currency currency() const;
            Handle<DayCounter> dayCounter() const;
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
            // methods
            int nextNode_(const Date& d, bool extrapolate) const;
            // data members
            Currency currency_;
            Handle<DayCounter> dayCounter_;
            Date settlementDate_;
            int nodesNumber_;
            std::vector<Date> nodes_;
            std::vector<Time> times_;
            std::vector<DiscountFactor> discounts_;
            std::vector<Rate> forwards_, zeroYields_;
            // here to be used in the clone method
            // to be replaced by an observable pattern
            std::vector<DepositRate> deposits_;
        };

        // inline definitions

        inline Currency PiecewiseConstantForwards::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> PiecewiseConstantForwards::dayCounter() const{
            return dayCounter_;
        }

        inline Date PiecewiseConstantForwards::settlementDate() const {
            return settlementDate_;
        }

        inline Date PiecewiseConstantForwards::maxDate() const {
            return nodes_.back();
        }

        inline Date PiecewiseConstantForwards::minDate() const {
            return settlementDate();
        }

    }

}


#endif
