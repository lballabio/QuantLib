
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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

/*! \file blackmodel.hpp
    \brief Abstract class for Black-type models (market models)
*/

#ifndef quantlib_black_model_h
#define quantlib_black_model_h

#include <ql/termstructure.hpp>
#include <ql/Math/normaldistribution.hpp>

namespace QuantLib {

    //! Black-model for vanilla interest-rate derivatives
    class BlackModel : public Observable, public Observer {
      public:
        BlackModel(const Handle<Quote>& volatility,
                   const Handle<TermStructure>& termStructure);

        void update() { notifyObservers(); }

        //Returns the Black volatility
        Volatility volatility() const;

        const Handle<TermStructure>& termStructure() const;

        //! General Black formula
        static Real formula(Real f, Real k, Real v, Real w);
        //! In-the-money cash probability
        static Real itmProbability(Real f, Real k, Real v, Real w);
      private:
        Handle<Quote> volatility_;
        Handle<TermStructure> termStructure_;
    };

    // inline definitions

    inline BlackModel::BlackModel(const Handle<Quote>& volatility,
                                  const Handle<TermStructure>& termStructure)
    : volatility_(volatility), termStructure_(termStructure) {
        registerWith(volatility_);
        registerWith(termStructure_);
    }

    inline Volatility BlackModel::volatility() const {
        return volatility_->value();
    }

    inline 
    const Handle<TermStructure>& BlackModel::termStructure() const {
        return termStructure_;
    }

    /*! Returns
        \f[
            Black(f,k,v,w) = fw\Phi(wd_1(f,k,v)) - kw\Phi(wd_2(f,k,v)),
        \f]
        where
        \f[
            d_1(f,k,v) = \frac{\ln(f/k)+v^2/2}{v}
        \f]
        and 
        \f[
            d_2(f,k,v) = d_1(f,k,v) - v.
        \f]
    */
    inline Real BlackModel::formula(Real f, Real k, Real v, Real w) {
        if (QL_FABS(v) < QL_EPSILON)
            return QL_MAX(f*w - k*w, Real(0.0));
        Real d1 = QL_LOG(f/k)/v + 0.5*v;
        Real d2 = d1 - v;
        CumulativeNormalDistribution phi;
        Real result = w*(f*phi(w*d1) - k*phi(w*d2));
        // numerical inaccuracies can yield a negative answer
        return QL_MAX(Real(0.0), result);
    }

    /*! Returns
        \f[
            P(f,k,v,w) = \Phi(wd_2(f,k,v)),
        \f]
        where
        \f[
            d_1(f,k,v) = \frac{\ln(f/k)+v^2/2}{v}
        \f]
        and 
        \f[
            d_2(f,k,v) = d_1(f,k,v) - v.
        \f]
    */
    inline Real BlackModel::itmProbability(Real f, Real k, Real v, Real w) {
        if (QL_FABS(v) < QL_EPSILON)
            return (f*w > k*w ? 1.0 : 0.0);
        Real d1 = QL_LOG(f/k)/v + 0.5*v;
        Real d2 = d1 - v;
        CumulativeNormalDistribution phi;
        return phi(w*d2);
    }

}


#endif
