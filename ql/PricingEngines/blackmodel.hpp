
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
        BlackModel(const RelinkableHandle<Quote>& volatility,
                   const RelinkableHandle<TermStructure>& termStructure);

        void update() { notifyObservers(); }

        //Returns the Black volatility
        double volatility() const;

        const RelinkableHandle<TermStructure>& termStructure() const;

        //! General Black formula
        static double formula(double f, double k, double v, double w);
        //! In-the-money cash probability
        static double itmProbability(double f, double k, double v, double w);
      private:
        RelinkableHandle<Quote> volatility_;
        RelinkableHandle<TermStructure> termStructure_;
    };

    // inline definitions

    inline BlackModel::BlackModel(
        const RelinkableHandle<Quote>& volatility,
        const RelinkableHandle<TermStructure>& termStructure)
    : volatility_(volatility), termStructure_(termStructure) {
        registerWith(volatility_);
        registerWith(termStructure_);
    }

    inline double BlackModel::volatility() const {
        return volatility_->value();
    }

    inline 
    const RelinkableHandle<TermStructure>& BlackModel::termStructure() const {
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
    inline double BlackModel::formula(double f, double k, double v, double w) {
        if (QL_FABS(v) < QL_EPSILON)
            return QL_MAX(f*w - k*w, 0.0);
        double d1 = QL_LOG(f/k)/v + 0.5*v;
        double d2 = d1 - v;
        CumulativeNormalDistribution phi;
        double result = w*(f*phi(w*d1) - k*phi(w*d2));
        // numerical inaccuracies can yield a negative answer
        return QL_MAX(0.0, result);
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
    inline double BlackModel::itmProbability(double f, double k, 
                                             double v, double w) {
        if (QL_FABS(v) < QL_EPSILON)
            return (f*w > k*w ? 1.0 : 0.0);
        double d1 = QL_LOG(f/k)/v + 0.5*v;
        double d2 = d1 - v;
        CumulativeNormalDistribution phi;
        return phi(w*d2);
    }

}


#endif
