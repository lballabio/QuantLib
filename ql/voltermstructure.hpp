
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file voltermstructure.hpp
    \brief Volatility term structure

    \fullpath
    ql/%voltermstructure.hpp
*/

// $Id$

#ifndef quantlib_vol_term_structure_hpp
#define quantlib_vol_term_structure_hpp

#include <ql/daycounter.hpp>
#include <ql/Patterns/observable.hpp>

/*! \namespace QuantLib::VolTermStructures
    \brief Concrete implementations of the VolTermStructure interface

*/

namespace QuantLib {

    //! Volatility Term structure
    /*! This abstract class defines the interface of concrete
        volatility term structures which will be derived from this one.

        Volatility are assumed to be expressed on an annual basis

    */
    class VolTermStructure : public Patterns::Observable {
      public:
        virtual ~VolTermStructure() {}
        //! \name Black Volatility
        //@{
        //! Black volatility for an option of fixed strike K and maturity date
        virtual double blackVol(const Date& maturity, double strike, bool extrapolate = false) const = 0;
        //! Black volatility for an option of fixed strike K and maturity time
        virtual double blackVol(Time maturity, double strike, bool extrapolate = false) const = 0;
        //@}

        //! \name Local volatility of the underlying stochastic process
        //@{
        //! Local volatility at a given time for an option of fixed strike K
        virtual double localVol(const Date& evaluationDate, double strike, bool extrapolate = false) const = 0;
        //! Black volatility for an option of fixed strike K and maturity time
        virtual double localVol(Time evaluationTime, double strike, bool extrapolate = false) const = 0;
        //@}

        //! \name Dates
        //@{
        //! returns the reference date for which t=0
        virtual Date referenceDate() const = 0;
        //! returns the day counter
        virtual DayCounter dayCounter() const = 0;
        //! returns the latest date for which the term structure can return vols
        virtual Date maxDate() const = 0;
        //! returns the latest time for which the term structure can return vols
        virtual Time maxTime() const = 0;
        //@}

        //! \name Other inspectors
        //@{
        //! returns the underlying upon which the vol term structure is defined
        virtual std::string underlying() const = 0;
        //@}
    };

}


#endif
