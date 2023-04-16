/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file interestratevolsurface.hpp
    \brief Interest rate volatility (smile) surface
*/

#ifndef quantlib_interest_rate_vol_surface_hpp
#define quantlib_interest_rate_vol_surface_hpp

#include <ql/experimental/volatility/blackvolsurface.hpp>
#include <ql/experimental/volatility/interestratevolsurface.hpp>
#include <ql/indexes/interestrateindex.hpp>

namespace QuantLib {

    //! Interest rate volatility (smile) surface
    /*! This abstract class defines the interface of concrete
        Interest rate volatility (smile) surfaces which will
        be derived from this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class InterestRateVolSurface : public BlackVolSurface {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        explicit InterestRateVolSurface(std::shared_ptr<InterestRateIndex>,
                                        BusinessDayConvention bdc = Following,
                                        const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        InterestRateVolSurface(std::shared_ptr<InterestRateIndex>,
                               const Date& referenceDate,
                               const Calendar& cal = Calendar(),
                               BusinessDayConvention bdc = Following,
                               const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        InterestRateVolSurface(std::shared_ptr<InterestRateIndex>,
                               Natural settlementDays,
                               const Calendar&,
                               BusinessDayConvention bdc = Following,
                               const DayCounter& dc = DayCounter());
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        //! period/date conversion
        Date optionDateFromTenor(const Period&) const;
        //@}
        const std::shared_ptr<InterestRateIndex>& index() const;
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        std::shared_ptr<InterestRateIndex> index_;
    };


    // inline

    inline const std::shared_ptr<InterestRateIndex>&
    InterestRateVolSurface::index() const {
        return index_;
    }

}

#endif
