
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

/*! \file blackvariancecurve.hpp
    \brief Black volatility curve modelled as variance curve
*/

#ifndef quantlib_blackvariancecurve_hpp
#define quantlib_blackvariancecurve_hpp

#include <ql/voltermstructure.hpp>
#include <ql/Math/interpolation.hpp>
#include <ql/DayCounters/actual365.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Black volatility curve modelled as variance curve
        /*! This class calculates time-dependent Black volatilities
            using as input a vector of (ATM) Black volatilities
            observed in the market.

            The calculation is performed interpolating on the variance curve.
            Linear interpolation is used as default; this can be changed
            by the setInterpolation() method.
            
            For strike dependence, see BlackVarianceSurface.
        */
        class BlackVarianceCurve : public BlackVarianceTermStructure,
                                   public Patterns::Observer {
          public:
            BlackVarianceCurve(const Date& referenceDate,
                               const std::vector<Date>& dates,
                               const std::vector<double>& blackVolCurve,
                               const DayCounter& dayCounter = 
                                   DayCounters::Actual365());
            //! \name BlackVolTermStructure interface
            //@{
            Date referenceDate() const;
            DayCounter dayCounter() const;
            Date maxDate() const;
            double strikeDerivative(Time t, 
                                    double strike, 
                                    bool extrapolate = false) const;
            double strikeSecondDerivative(Time t, 
                                          double strike, 
                                          bool extrapolate = false) const;
            //@}
            //! \name Modifiers
            //@{
            template <class Traits>
            #if defined(QL_PATCH_MICROSOFT)
            void setInterpolation(const Traits&) {
            #else
            void setInterpolation() {
            #endif
                varianceCurve_ = 
                    Traits::make_interpolation(times_.begin(), times_.end(),
                                               variances_.begin());
                notifyObservers();
            }
            //@}
            //! \name Observer interface
            //@{
            void update();
            //@}
            //! \name Visitability
            //@{
            virtual void accept(Patterns::AcyclicVisitor&);
            //@}
          protected:
            virtual double blackVarianceImpl(Time t, double,
                                             bool extrapolate = false) const;
          private:
            typedef Math::Interpolation<std::vector<Time>::const_iterator,
                                        std::vector<double>::const_iterator>
                Interpolation;
            Date referenceDate_;
            DayCounter dayCounter_;
            Date maxDate_;
            std::vector<Time> times_;
            std::vector<double> variances_;
            Handle<Interpolation> varianceCurve_;
        };


        // inline definitions

        inline Date BlackVarianceCurve::referenceDate() const { 
            return referenceDate_; 
        }

        inline DayCounter BlackVarianceCurve::dayCounter() const { 
            return dayCounter_; 
        }

        inline Date BlackVarianceCurve::maxDate() const { 
            return maxDate_; 
        }

        inline double 
        BlackVarianceCurve::strikeDerivative(Time t, double strike, 
                                             bool extrapolate) const {
            return 0.0;
        }

        inline double 
        BlackVarianceCurve::strikeSecondDerivative(Time t, double strike, 
                                                   bool extrapolate) const {
            return 0.0;
        }

        inline void BlackVarianceCurve::update() {
            notifyObservers();
        }

        inline 
        void BlackVarianceCurve::accept(Patterns::AcyclicVisitor& v) {
            using namespace Patterns;
            Visitor<BlackVarianceCurve>* v1 = 
                dynamic_cast<Visitor<BlackVarianceCurve>*>(&v);
            if (v1 != 0)
                v1->visit(*this);
            else
                BlackVarianceTermStructure::accept(v);
        }

    }

}


#endif
