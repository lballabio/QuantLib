
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
/*! \file localvariancecurve.hpp
    \brief Local volatility curve modelled as variance curve. No asset dependence

    \fullpath
    ql/Volatilities/%localvariancecurve.hpp
*/

// $Id$

#ifndef quantlib_localvariancecurve_hpp
#define quantlib_localvariancecurve_hpp

#include <ql/Volatilities/blackvariancecurve.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Local volatility curve modelled as variance curve
        /*! This class implements the LocalVarianceTermStructure
            interface, that is a time dependant term structure
            of local volatilities, using an underlying time dependant
            term structure of Black volatilities.
            Local volatility and Black volatility are the same when
            volatility is at most time dependent, so this class is
            basically a proxy for BlackVarianceTermStructure.

            For time/asset dependance a LocalVarianceSurface class
            will be developed in the future.
        */
        template<class Interpolator1D>
        class LocalVarianceCurve : public LocalVarianceTermStructure,
                                   public Patterns::Observer {
          public:
            // constructor
            LocalVarianceCurve(const Handle<BlackVarianceCurve<
                Interpolator1D> >& blackVarianceCurve);
            // inspectors
            Date referenceDate() const { return blackVarianceCurve_->referenceDate(); }
            DayCounter dayCounter() const { return blackVarianceCurve_->dayCounter(); }
            Date maxDate() const { return blackVarianceCurve_->maxDate(); }
            // Observer interface
            void update();
          protected:
            double localVarianceImpl(Time t1, Time t2, double, bool extrapolate) const;
          private:
            Handle<BlackVarianceCurve<Interpolator1D> > blackVarianceCurve_;
        };


        template<class Interpolator1D>
        LocalVarianceCurve<Interpolator1D>::LocalVarianceCurve(
            const Handle<BlackVarianceCurve<Interpolator1D> >& blackVarianceCurve)
        : blackVarianceCurve_(blackVarianceCurve) {

        }


        template<class Interpolator1D>
        void LocalVarianceCurve<Interpolator1D>::update() {
            notifyObservers();
        }

        template<class Interpolator1D>
        double LocalVarianceCurve<Interpolator1D>::
            localVarianceImpl(Time t1, Time t2, double, bool extrapolate) const {

            return blackVarianceCurve_->blackForwardVariance(t1, t2, 0.0, extrapolate);
        }

    }

}


#endif
