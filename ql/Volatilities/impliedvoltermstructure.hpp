
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
/*! \file impliedvoltermstructure.hpp
    \brief Implied volatility term structure

    \fullpath
    ql/Volatilities/%impliedvoltermstructure.hpp
*/

// $Id$

#ifndef quantlib_implied_vol_term_structure_hpp
#define quantlib_implied_vol_term_structure_hpp

#include <ql/voltermstructure.hpp>

namespace QuantLib {
    namespace VolTermStructures {

        //! Implied vol term structure at a given date in the future
        /*! The given date will be the implied reference date.
            \note This vol term structure will remain linked to the original
                structure, i.e., any changes in the latter will be reflected in
                this structure as well.
        */
        class ImpliedLocalVolTermStructure : public LocalVolTermStructure,
                                             public Patterns::Observer {
          public:
            ImpliedLocalVolTermStructure(const RelinkableHandle<LocalVolTermStructure>&,
                                         const Date& newReferenceDate);
            //! \name LocalVolTermStructure interface
            //@{
            DayCounter dayCounter() const {return originalCurve_->dayCounter();}
            Date referenceDate() const {return newReferenceDate_;}
            Date maxDate() const {return originalCurve_->maxDate();}
            //@}
            //! \name Observer interface
            //@{
            void update();
            //@}
          protected:
            //! returns the variance as seen from the reference date
            double blackVarianceImpl(Time maturity, double strike,
               bool extrapolate = false) const;
          private:
            RelinkableHandle<LocalVolTermStructure> originalCurve_;
            Date newReferenceDate_;
        };



        inline ImpliedVolTermStructure::ImpliedVolTermStructure(
            const RelinkableHandle<BlackVolTermStructure>& h,
            const Date& newReferenceDate)
        : originalCurve_(h), newReferenceDate_(newReferenceDate) {
            registerWith(originalCurve_);
        }

        inline DayCounter ImpliedTermStructure::dayCounter() const {
            return originalCurve_->dayCounter();
        }

        inline Date ImpliedTermStructure::todaysDate() const {
            return newTodaysDate_;
        }

        inline Date ImpliedTermStructure::settlementDate() const {
            return newSettlementDate_;
        }

        inline Date ImpliedTermStructure::maxDate() const {
            return originalCurve_->maxDate();
        }

        inline Time ImpliedTermStructure::maxTime() const {
            return dayCounter().yearFraction(
                newSettlementDate_,originalCurve_->maxDate());
        }

        inline void ImpliedTermStructure::update() {
            notifyObservers();
        }

        inline DiscountFactor ImpliedTermStructure::discountImpl(Time t,
            bool extrapolate) const {
                /* t is relative to the current settlement date
                   and needs to be converted to the time relative
                   to the settlement date of the original curve */
                Time originalTime = t + dayCounter().yearFraction(
                    originalCurve_->settlementDate(),
                    newSettlementDate_);
                // evaluationDate cannot be an extrapolation
                /* discount at evaluation date cannot be cached
                   since the original curve could change between
                   invocations of this method */
                return originalCurve_->discount(originalTime, extrapolate) /
                       originalCurve_->discount(settlementDate(),false);
        }
    }
}

#endif


