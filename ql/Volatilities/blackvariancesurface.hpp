
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2003 StatPro Italia srl

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

/*! \file blackvariancesurface.hpp
    \brief Black volatility surface modelled as variance surface
*/

#ifndef quantlib_blackvariancesurface_hpp
#define quantlib_blackvariancesurface_hpp

#include <ql/voltermstructure.hpp>
#include <ql/Math/matrix.hpp>
#include <ql/Math/interpolation2D.hpp>
#include <ql/DayCounters/actual365.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Black volatility surface modelled as variance surface
        /*! This class calculates time/strike dependent Black volatilities
            using as input a matrix of Black volatilities
            observed in the market.

            The calculation is performed interpolating on the variance surface.
            Bilinear interpolation is used as default; this can be changed
            by the setInterpolation() method.
        */
        class BlackVarianceSurface : public BlackVarianceTermStructure,
                                     public Patterns::Observer {
          public:
            enum Extrapolation { ConstantExtrapolation,
                                 InterpolatorDefaultExtrapolation };
            BlackVarianceSurface(const Date& referenceDate,
                                 const std::vector<Date>& dates,
                                 const std::vector<double>& strikes,
                                 const Math::Matrix& blackVolMatrix,
                                 Extrapolation lowerExtrapolation =
                                     InterpolatorDefaultExtrapolation,
                                 Extrapolation upperExtrapolation =
                                     InterpolatorDefaultExtrapolation,
                                 const DayCounter& dayCounter =
                                     DayCounters::Actual365());
            Date referenceDate() const { 
                return referenceDate_; 
            }
            DayCounter dayCounter() const { 
                return dayCounter_; 
            }
            Date maxDate() const { 
                return maxDate_; 
            }
            // modifiers
            template <class Traits>
            void setInterpolation() {
                varianceSurface_ = 
                    Traits::make_interpolation(times_.begin(), times_.end(),
                                               strikes_.begin(), 
                                               strikes_.end(),
                                               variances_);
                notifyObservers();
            }
            // Observer interface
            void update() {
                notifyObservers();
            }
          protected:
            virtual double blackVarianceImpl(Time t,
                                             double strike,
                                             bool extrapolate = false) const;
          private:
            typedef Math::Interpolation2D<std::vector<Time>::const_iterator,
                                          std::vector<double>::const_iterator,
                                          Math::Matrix> Interpolation;
            Date referenceDate_;
            DayCounter dayCounter_;
            Date maxDate_;
            std::vector<double> strikes_;
            std::vector<Time> times_;
            Math::Matrix variances_;
            Handle<Interpolation> varianceSurface_;
            Extrapolation lowerExtrapolation_, upperExtrapolation_;
        };

    }

}


#endif
