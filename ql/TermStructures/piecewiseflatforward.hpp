
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

            Rates are assumed to be annual continuos compounding.

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
                         const Date& settlementDate,
                         const std::vector<Handle<RateHelper> >& instruments,
                         const DayCounter& dayCounter,
                         double accuracy=1.0e-12);
           PiecewiseFlatForward(
                         const std::vector<Date>& dates,
                         const std::vector<Rate>& forwards,
                         const DayCounter& dayCounter);
            //! \name TermStructure interface
            //@{
            DayCounter dayCounter() const;
            Date settlementDate() const;
            const std::vector<Date>& dates() const;
            Date maxDate() const;
            const std::vector<Time>& times() const;
            Time maxTime() const;
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
            DayCounter dayCounter_;
            Date settlementDate_;
            std::vector<Handle<RateHelper> > instruments_;
            mutable bool needsBootstrap_;
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

        inline Date PiecewiseFlatForward::settlementDate() const {
            return settlementDate_;
        }

        inline const std::vector<Date>& PiecewiseFlatForward::dates() const {
            if (needsBootstrap_) bootstrap();
            return dates_;
        }

        inline Date PiecewiseFlatForward::maxDate() const {
            if (needsBootstrap_) bootstrap();
            return dates_.back();
        }

        inline const std::vector<Time>& PiecewiseFlatForward::times() const {
            if (needsBootstrap_) bootstrap();
            return times_;
        }

        inline Time PiecewiseFlatForward::maxTime() const {
            if (needsBootstrap_) bootstrap();
            return times_.back();
        }

        inline void PiecewiseFlatForward::update() {
            needsBootstrap_ = true;
            notifyObservers();
        }

    }

}

#endif
