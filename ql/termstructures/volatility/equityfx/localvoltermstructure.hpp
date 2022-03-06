/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl

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

/*! \file localvoltermstructure.hpp
    \brief Local volatility term structure base class
*/

#ifndef quantlib_local_vol_term_structures_hpp
#define quantlib_local_vol_term_structures_hpp

#include <ql/termstructures/voltermstructure.hpp>
#include <ql/patterns/visitor.hpp>

namespace QuantLib {

    /*! This abstract class defines the interface of concrete
        local-volatility term structures which will be derived from this one.

        Volatilities are assumed to be expressed on an annual basis.
    */
    class LocalVolTermStructure : public VolatilityTermStructure {
      public:
        /*! \name Constructors
            See the TermStructure documentation for issues regarding
            constructors.
        */
        //@{
        //! default constructor
        /*! \warning term structures initialized by means of this
                     constructor must manage their own reference date
                     by overriding the referenceDate() method.
        */
        LocalVolTermStructure(BusinessDayConvention bdc = Following,
                              const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        LocalVolTermStructure(const Date& referenceDate,
                              const Calendar& cal = Calendar(),
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        LocalVolTermStructure(Natural settlementDays,
                              const Calendar&,
                              BusinessDayConvention bdc = Following,
                              const DayCounter& dc = DayCounter());
        //@}
        ~LocalVolTermStructure() override = default;
        //! \name Local Volatility
        //@{
        Volatility localVol(const Date& d,
                            Real underlyingLevel,
                            bool extrapolate = false) const;
        Volatility localVol(Time t,
                            Real underlyingLevel,
                            bool extrapolate = false) const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        /*! \name Calculations

            These methods must be implemented in derived classes to perform
            the actual volatility calculations. When they are called,
            range check has already been performed; therefore, they must
            assume that extrapolation is required.
        */
        //@{
        //! local vol calculation
        virtual Volatility localVolImpl(Time t, Real strike) const = 0;
        //@}
    };

}

#endif


#ifndef id_804441590ccd78682f87880a87e462f4
#define id_804441590ccd78682f87880a87e462f4
inline bool test_804441590ccd78682f87880a87e462f4(int* i) { return i != 0; }
#endif
