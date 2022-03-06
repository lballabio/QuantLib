/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2007 Giorgio Facchinetti

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

#ifndef quantlib_abcd_hpp
#define quantlib_abcd_hpp

#include <ql/types.hpp>
#include <ql/errors.hpp>
#include <ql/math/abcdmathfunction.hpp>

namespace QuantLib {
    
    //! %Abcd functional form for instantaneous volatility
    /*! \f[ f(T-t) = [ a + b(T-t) ] e^{-c(T-t)} + d \f]
        following Rebonato's notation. */
    class AbcdFunction : public AbcdMathFunction {

      public:
        AbcdFunction(Real a = -0.06,
                     Real b =  0.17,
                     Real c =  0.54,
                     Real d =  0.17);

        //! maximum value of the volatility function
        Real maximumVolatility() const { return maximumValue(); }

        //! volatility function value at time 0: \f[ f(0) \f]
        Real shortTermVolatility() const { return (*this)(0.0); }

        //! volatility function value at time +inf: \f[ f(\inf) \f]
        Real longTermVolatility() const { return longTermValue(); }

        /*! instantaneous covariance function at time t between T-fixing and
            S-fixing rates \f[ f(T-t)f(S-t) \f] */
        Real covariance(Time t, Time T, Time S) const;

        /*! integral of the instantaneous covariance function between
            time t1 and t2 for T-fixing and S-fixing rates
            \f[ \int_{t1}^{t2} f(T-t)f(S-t)dt \f] */
        Real covariance(Time t1, Time t2, Time T, Time S) const;

         /*! average volatility in [tMin,tMax] of T-fixing rate:
            \f[ \sqrt{ \frac{\int_{tMin}^{tMax} f^2(T-u)du}{tMax-tMin} } \f] */
        Real volatility(Time tMin, Time tMax, Time T) const;

        /*! variance between tMin and tMax of T-fixing rate:
            \f[ \frac{\int_{tMin}^{tMax} f^2(T-u)du}{tMax-tMin} \f] */
        Real variance(Time tMin, Time tMax, Time T) const;
        

        
        // INSTANTANEOUS
        /*! instantaneous volatility at time t of the T-fixing rate:
            \f[ f(T-t) \f] */
        Real instantaneousVolatility(Time t, Time T) const;

        /*! instantaneous variance at time t of T-fixing rate:
            \f[ f(T-t)f(T-t) \f] */
        Real instantaneousVariance(Time t, Time T) const;

        /*! instantaneous covariance at time t between T and S fixing rates:
            \f[ f(T-u)f(S-u) \f] */
        Real instantaneousCovariance(Time u, Time T, Time S) const;

        // PRIMITIVE
        /*! indefinite integral of the instantaneous covariance function at
            time t between T-fixing and S-fixing rates
            \f[ \int f(T-t)f(S-t)dt \f] */
        Real primitive(Time t, Time T, Time S) const;
        
    };

    
    // Helper class used by unit tests
    class AbcdSquared {
      
      public:
        typedef Real argument_type;
        typedef Real result_type;

        AbcdSquared(Real a, Real b, Real c, Real d, Time T, Time S);
        Real operator()(Time t) const;
      
      private:
        ext::shared_ptr<AbcdFunction> abcd_;
        Time T_, S_;
    };

    inline Real abcdBlackVolatility(Time u, Real a, Real b, Real c, Real d) {
        AbcdFunction model(a,b,c,d);
        return model.volatility(0.,u,u);
    }
}

#endif


#ifndef id_129c57eb25a66688c51c0b232b79c144
#define id_129c57eb25a66688c51c0b232b79c144
inline bool test_129c57eb25a66688c51c0b232b79c144(int* i) { return i != 0; }
#endif
