
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

/*! \file piecewiseflatforward.hpp
    \brief piecewise flat forward term structure
*/

#ifndef quantlib_piecewise_flat_forward_curve_h
#define quantlib_piecewise_flat_forward_curve_h

#include <ql/TermStructures/ratehelpers.hpp>
#include <ql/solver1d.hpp>

namespace QuantLib {

    //! Piecewise flat forward term structure
    /*! This term structure is bootstrapped on a number of interest
        rate instruments which are passed as a vector of handles to
        RateHelper instances. Their maturities mark the boundaries of
        the flat forward segments.

        The values of the forward rates for each segment are determined
        sequentially starting from the earliest period to the latest.

        The value for each segment is chosen so that the instrument whose
        maturity marks the end of such segment is correctly repriced on
        the curve.

        Rates are assumed to be annual continuos compounding.

        \warning The bootstrapping algorithm will raise an exception if
        any two instruments have the same maturity date.
    */
    /* This class is derived directly from term structure since we are
       rewriting all of forward, discount and zeroYield to take advantage
       of its own internal structure. */
    class PiecewiseFlatForward : public TermStructure,
                                 public LazyObject {
      public:
        // constructor
        PiecewiseFlatForward(
               const Date& todaysDate,
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<RateHelper> >& instruments,
               const DayCounter& dayCounter,
               double accuracy=1.0e-12);
        /*! In this constructor, the first date must be the reference
          date of the curve, the other dates are the nodes of the
          term structure. The forward rate at index \f$i\f$ is used
          in the period \f$t_{i-1} < t \le t_i\f$. Therefore,
          forwards[0] is used only to compute the zero yield for 
          \f$t = 0\f$.
        */
        PiecewiseFlatForward(const Date& todaysDate,
                             const std::vector<Date>& dates,
                             const std::vector<Rate>& forwards,
                             const DayCounter& dayCounter);
        //! \name TermStructure interface
        //@{
        DayCounter dayCounter() const;
        Date todaysDate() const { return todaysDate_; }
        Date referenceDate() const;
        const std::vector<Date>& dates() const;
        Date maxDate() const;
        const std::vector<Time>& times() const;
        Time maxTime() const;
        //@}
      protected:
        Rate zeroYieldImpl(Time) const;
        DiscountFactor discountImpl(Time) const;
        Rate forwardImpl(Time) const;
	    Rate compoundForwardImpl(Time t, int compFreq) const;
      private:
        // inner classes
        // objective function for solver
        class FFObjFunction;
        friend class FFObjFunction;
        class FFObjFunction {
          public:
            FFObjFunction(const PiecewiseFlatForward*,
                          const boost::shared_ptr<RateHelper>&, Size segment);
            double operator()(double discountGuess) const;
          private:
            const PiecewiseFlatForward* curve_;
            boost::shared_ptr<RateHelper> rateHelper_;
            Size segment_;
        };
        // instrument sorter
        class RateHelperSorter {
          public:
            bool operator()(const boost::shared_ptr<RateHelper>&,
                            const boost::shared_ptr<RateHelper>&) const;
        };
        // methods
        Size referenceNode(Time t) const;
        void performCalculations() const;
        // data members
        DayCounter dayCounter_;
        Date todaysDate_, referenceDate_;
        std::vector<boost::shared_ptr<RateHelper> > instruments_;
        mutable std::vector<Time> times_;
        mutable std::vector<Date> dates_;
        mutable std::vector<DiscountFactor> discounts_;
        mutable std::vector<Rate> forwards_, zeroYields_;
        double accuracy_;
    };

    // inline definitions

    inline DayCounter PiecewiseFlatForward::dayCounter() const {
        return dayCounter_;
    }

    inline Date PiecewiseFlatForward::referenceDate() const {
        return referenceDate_;
    }

    inline const std::vector<Date>& PiecewiseFlatForward::dates() const {
        calculate();
        return dates_;
    }

    inline Date PiecewiseFlatForward::maxDate() const {
        calculate();
        return dates_.back();
    }

    inline const std::vector<Time>& PiecewiseFlatForward::times() const {
        calculate();
        return times_;
    }

    inline Time PiecewiseFlatForward::maxTime() const {
        calculate();
        return times_.back();
    }

}


#endif
