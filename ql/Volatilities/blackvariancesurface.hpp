
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file blackvariancesurface.hpp
    \brief Black volatility surface modelled as variance surface
*/

#ifndef quantlib_black_variance_surface_hpp
#define quantlib_black_variance_surface_hpp

#include <ql/voltermstructure.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/interpolation2D.hpp>
#include <ql/DayCounters/actual365.hpp>

namespace QuantLib {

    //! Black volatility surface modelled as variance surface
    /*! This class calculates time/strike dependent Black volatilities
        using as input a matrix of Black volatilities observed in the
        market.

        The calculation is performed interpolating on the variance
        surface.  Bilinear interpolation is used as default; this can
        be changed by the setInterpolation() method.

        \todo check time extrapolation

    */
    class BlackVarianceSurface : public BlackVarianceTermStructure,
                                 public Observer {
      public:
        enum Extrapolation { ConstantExtrapolation,
                             InterpolatorDefaultExtrapolation };
        BlackVarianceSurface(const Date& referenceDate,
                             const std::vector<Date>& dates,
                             const std::vector<double>& strikes,
                             const Matrix& blackVolMatrix,
                             Extrapolation lowerExtrapolation =
                             InterpolatorDefaultExtrapolation,
                             Extrapolation upperExtrapolation =
                             InterpolatorDefaultExtrapolation,
                             const DayCounter& dayCounter = Actual365());
        //! \name BlackVolTermStructure interface
        //@{
        Date referenceDate() const { 
            return referenceDate_; 
        }
        DayCounter dayCounter() const { 
            return dayCounter_; 
        }
        Date maxDate() const { 
            return maxDate_; 
        }
        double minStrike() const { 
            return strikes_.front(); 
        }
        double maxStrike() const { 
            return strikes_.back(); 
        }
        //@}
        //! \name Modifiers
        //@{
        template <class Traits>
        #if defined(QL_PATCH_MICROSOFT)
        void setInterpolation(const Traits&) {
        #else
        void setInterpolation() {
        #endif
            varianceSurface_ = 
                Traits::make_interpolation(times_.begin(), times_.end(),
                                           strikes_.begin(), strikes_.end(),
                                           variances_);
            notifyObservers();
        }
        //@}
        //! \name Observer interface
        //@{
        void update() {
            notifyObservers();
        }
        //@}
        //! \name Visitability
        //@{
        virtual void accept(AcyclicVisitor&);
        //@}
      protected:
        virtual double blackVarianceImpl(Time t, double strike) const;
      private:
        Date referenceDate_;
        DayCounter dayCounter_;
        Date maxDate_;
        std::vector<double> strikes_;
        std::vector<Time> times_;
        Matrix variances_;
        Interpolation2D varianceSurface_;
        Extrapolation lowerExtrapolation_, upperExtrapolation_;
    };


    // inline definitions

    inline void BlackVarianceSurface::accept(AcyclicVisitor& v) {
        Visitor<BlackVarianceSurface>* v1 = 
            dynamic_cast<Visitor<BlackVarianceSurface>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            BlackVarianceTermStructure::accept(v);
    }

}


#endif
