
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file localvolsurface.hpp
    \brief Local volatility surface derived from a Black vol surface
*/

#ifndef quantlib_localvolsurface_hpp
#define quantlib_localvolsurface_hpp

#include <ql/voltermstructure.hpp>
#include <ql/termstructure.hpp>

namespace QuantLib {

    //! Local volatility surface derived from a Black vol surface
    /*! For details about this implementation refers to
        "Stochastic Volatility and Local Volatility," in
        "Case Studies in Financial Modelling Course Notes," by
        Jim Gatheral, Fall Term, 2003

        see www.math.nyu.edu/fellows_fin_math/gatheral/Lecture1_Fall02.pdf

        \bug This class is untested, probably unreliable.

    */
    class LocalVolSurface : public LocalVolTermStructure,
                            public Observer {
      public:
        LocalVolSurface(const RelinkableHandle<BlackVolTermStructure>& blackTS,
                        const RelinkableHandle<TermStructure>& riskFreeTS,
                        const RelinkableHandle<TermStructure>& dividendTS,
                        const RelinkableHandle<Quote>& underlying);
        LocalVolSurface(const RelinkableHandle<BlackVolTermStructure>& blackTS,
                        const RelinkableHandle<TermStructure>& riskFreeTS,
                        const RelinkableHandle<TermStructure>& dividendTS,
                        double underlying);
        //! \name LocalVolTermStructure interface
        //@{
        Date referenceDate() const {
            return blackTS_->referenceDate();
        }
        DayCounter dayCounter() const {
            return blackTS_->dayCounter();
        }
        Date maxDate() const { return blackTS_->maxDate(); }
        double minStrike() const { return blackTS_->minStrike(); }
        double maxStrike() const { return blackTS_->maxStrike(); }
        //@}
        //! \name Observer interface
        //@{
        void update() { notifyObservers(); }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        double localVolImpl(Time, double) const;
      private:
        RelinkableHandle<BlackVolTermStructure> blackTS_;
        RelinkableHandle<TermStructure> riskFreeTS_, dividendTS_;
        RelinkableHandle<Quote> underlying_;
    };

}


#endif
