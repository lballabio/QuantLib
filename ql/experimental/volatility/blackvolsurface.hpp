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

/*! \file blackvolsurface.hpp
    \brief Black volatility (smile) surface
*/

#ifndef quantlib_black_vol_surface_hpp
#define quantlib_black_vol_surface_hpp

#include <ql/experimental/volatility/blackatmvolcurve.hpp>

namespace QuantLib {

    class SmileSection;

    //! Black volatility (smile) surface
    /*! This abstract class defines the interface of concrete
        Black volatility (smile) surface which will
        be derived from this one.

        Volatilities are assumed to be expressed on an annual basis.

    */
    class BlackVolSurface : public BlackAtmVolCurve {
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
        BlackVolSurface(BusinessDayConvention bdc = Following,
                        const DayCounter& dc = DayCounter());
        //! initialize with a fixed reference date
        BlackVolSurface(const Date& referenceDate,
                        const Calendar& cal = Calendar(),
                        BusinessDayConvention bdc = Following,
                        const DayCounter& dc = DayCounter());
        //! calculate the reference date based on the global evaluation date
        BlackVolSurface(Natural settlementDays,
                        const Calendar&,
                        BusinessDayConvention bdc = Following,
                        const DayCounter& dc = DayCounter());
        //@}
        //! \name Black spot volatility
        //@{
        //! returns the smile for a given option tenor
        boost::shared_ptr<SmileSection> smileSection(const Period&,
                                                     bool extrapolate) const;
        //! returns the smile for a given option date
        boost::shared_ptr<SmileSection> smileSection(const Date&,
                                                     bool extrapolate) const;
        //! returns the smile for a given option time
        boost::shared_ptr<SmileSection> smileSection(Time,
                                                     bool extrapolate) const;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&);
        //@}
      protected:
        //! \name BlackAtmVolCurve interface
        //@{
        //! spot at-the-money variance calculation
        Real atmVarianceImpl(Time t) const;
        //! spot at-the-money volatility calculation
        Volatility atmVolImpl(Time t) const;
        //@}
        /*! \name Calculations

            This method must be implemented in derived classes to perform
            the actual volatility calculations. When it is called,
            time check has already been performed; therefore, it must
            assume that time-extrapolation is allowed.
        */
        //@{
        virtual boost::shared_ptr<SmileSection> smileSectionImpl(Time) const=0;
        //@}
    };

    // inline definitions

    inline boost::shared_ptr<SmileSection>
    BlackVolSurface::smileSection(const Period& p,
                                  bool extrapolate) const {
        return smileSection(optionDateFromTenor(p), extrapolate);
    }

    inline boost::shared_ptr<SmileSection>
    BlackVolSurface::smileSection(const Date& d,
                                  bool extrapolate) const {
        return smileSection(timeFromReference(d), extrapolate);
    }

    inline boost::shared_ptr<SmileSection>
    BlackVolSurface::smileSection(Time t,
                                  bool extrapolate) const {
        checkRange(t, extrapolate);
        return smileSectionImpl(t);
    }

}

#endif
