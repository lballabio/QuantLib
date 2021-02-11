/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

/*! \file localvolcurve.hpp
    \brief Local volatility curve derived from a Black curve
*/

#ifndef quantlib_localvolcurve_hpp
#define quantlib_localvolcurve_hpp

#include <ql/termstructures/volatility/equityfx/blackvariancecurve.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>

namespace QuantLib {

    //! Local volatility curve derived from a Black curve
    class LocalVolCurve : public LocalVolTermStructure {
      public:
        LocalVolCurve(const Handle<BlackVarianceCurve>& curve)
        : LocalVolTermStructure(curve->businessDayConvention(),
                                curve->dayCounter()),
          blackVarianceCurve_(curve) {
            registerWith(blackVarianceCurve_);
        }
        //! \name TermStructure interface
        //@{
        const Date& referenceDate() const override { return blackVarianceCurve_->referenceDate(); }
        Calendar calendar() const override { return blackVarianceCurve_->calendar(); }
        DayCounter dayCounter() const override { return blackVarianceCurve_->dayCounter(); }
        Date maxDate() const override { return blackVarianceCurve_->maxDate(); }
        //@}
        //! \name VolatilityTermStructure interface
        //@{
        Real minStrike() const override { return QL_MIN_REAL; }
        Real maxStrike() const override { return QL_MAX_REAL; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Volatility localVolImpl(Time, Real) const override;

      private:
        Handle<BlackVarianceCurve> blackVarianceCurve_;
    };



    // inline definitions

    inline void LocalVolCurve::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<LocalVolCurve>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            LocalVolTermStructure::accept(v);
    }

    /*! The relation
        \f[
            \int_0^T \sigma_L^2(t)dt = \sigma_B^2 T
        \f]
        holds, where \f$ \sigma_L(t) \f$ is the local volatility at
        time \f$ t \f$ and \f$ \sigma_B(T) \f$ is the Black
        volatility for maturity \f$ T \f$. From the above, the formula
        \f[
            \sigma_L(t) = \sqrt{\frac{\mathrm{d}}{\mathrm{d}t}\sigma_B^2(t)t}
        \f]
        can be deduced which is here implemented.
    */
    inline Volatility LocalVolCurve::localVolImpl(Time t, Real dummy) const {

        Time dt = (1.0/365.0);
        Real var1 = blackVarianceCurve_->blackVariance(t, dummy, true);
        Real var2 = blackVarianceCurve_->blackVariance(t+dt, dummy, true);
        Real derivative = (var2-var1)/dt;
        return std::sqrt(derivative);
    }

}


#endif
