/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file localvolsurface.hpp
    \brief Local volatility surface derived from a Black vol surface
*/

#ifndef quantlib_localvolsurface_hpp
#define quantlib_localvolsurface_hpp

#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    class BlackVolTermStructure;
    class YieldTermStructure;
    class Quote;

    //! Local volatility surface derived from a Black vol surface
    /*! For details about this implementation refer to
        "Stochastic Volatility and Local Volatility," in
        "Case Studies and Financial Modelling Course Notes," by
        Jim Gatheral, Fall Term, 2003

        see www.math.nyu.edu/fellows_fin_math/gatheral/Lecture1_Fall02.pdf

        \bug this class is untested, probably unreliable.
    */
    class LocalVolSurface : public LocalVolTermStructure {
      public:
        LocalVolSurface(const Handle<BlackVolTermStructure>& blackTS,
                        const Handle<YieldTermStructure>& riskFreeTS,
                        const Handle<YieldTermStructure>& dividendTS,
                        const Handle<Quote>& underlying);
        LocalVolSurface(const Handle<BlackVolTermStructure>& blackTS,
                        const Handle<YieldTermStructure>& riskFreeTS,
                        const Handle<YieldTermStructure>& dividendTS,
                        Real underlying);
        //! \name TermStructure interface
        //@{
        const Date& referenceDate() const;
        DayCounter dayCounter() const;
        Date maxDate() const;
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const;
        Real maxStrike() const;
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        Volatility localVolImpl(Time, Real) const;
      private:
        Handle<BlackVolTermStructure> blackTS_;
        Handle<YieldTermStructure> riskFreeTS_, dividendTS_;
        Handle<Quote> underlying_;
    };

}

#endif
