/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_abcd_hpp
#define quantlib_abcd_hpp

#include <ql/types.hpp>
#include <ql/Optimization/method.hpp>
#include <boost/shared_ptr.hpp>
#include <functional>
#include <vector>

namespace QuantLib
{
    //! Abcd functional form for instantaneous volatility
    /*!
        \f[
        f(T-t) = [ a + b(T-t) ] e^{-c(T-t)} + d
        \f]

        following Rebonato notation.

    */
    class Abcd : public std::unary_function<Real,Real> {
    public:
        Abcd(Real a = -0.0597,
             Real b = 0.1677,
             Real c = 0.5403,
             Real d = 0.1710,
             bool aIsFixed = false,
             bool bIsFixed = false,
             bool cIsFixed = false,
             bool dIsFixed = false);
        //! instantaneous volatility at time to maturity u: \f[ f(u) \f]
        Real operator()(Time u) const;
        Real a() const { return a_; }
        Real b() const { return b_; }
        Real c() const { return c_; }
        Real d() const { return d_; }

        /*! instantaneous volatility at time u of the T-fixing rate:
            \f[ f(T-u) \f]
        */
        Real instantaneousVolatility(Time u, Time T) const;
        /*! instantaneous variance at time u of T-fixing rate:
            \f[ f(T-u)f(T-u) \f]
        */
        Real instantaneousVariance(Time u, Time T) const;
        /*! instantaneous covariance at time u between T and S fixing rates:
            \f[ f(T-u)f(S-u) \f]
        */
        Real instantaneousCovariance(Time u, Time T, Time S) const;

        /*! volatility in [tMin,tMax] of T-fixing rate:
            \f[ \sqrt_{ \int_{tMin}^{tMax} f^2(T-u)du }\f]
        */
        Real volatility(Time tMin, Time tMax, Time T) const;
        /*! variance in [tMin,tMax] of T-fixing rate:
            \f[ \int_{tMin}^{tMax} f^2(T-u)du \f]
        */
        Real variance(Time tMin, Time tMax, Time T) const;
        /*! covariance in [tMin,tMax] between T and S fixing rates:
            \f[ \int_{tMin}^{tMax} f(T-u)f(S-u)du \f]
        */
        Real covariance(Time tMin, Time tMax, Time T, Time S) const;

        //! instantaneous volatility when time to maturity = 0.0
        Real shortTermVolatility() const;
        //! instantaneous volatility when time to maturity = +inf
        Real longTermVolatility() const;
        //! time to maturity at which the instantaneous volatility reaches maximum (if any)
        Real maximumLocation() const;
        //! maximum of the instantaneous volatility
        Real maximumVolatility() const;

        //! adjustment factors needed to match Black vols
        std::vector<Real> k(const std::vector<Real>& blackVols,
                            const std::vector<Real>::const_iterator& t) const;
        //! vol error
        Real error(const std::vector<Real>& blackVols,
                   const std::vector<Real>::const_iterator& t) const;

        //! calibration
        EndCriteria::Type calibrate(const std::vector<Real>& blackVols,
                       const std::vector<Real>::const_iterator& t,
                       const boost::shared_ptr<OptimizationMethod>& method
                                = boost::shared_ptr<OptimizationMethod>());
    private:
        //! indefinite integral \f[ \int f(T-t)f(S-t)dt \f]
        Real primitive(Time u, Time T, Time S) const;
        //! optimization constraints
        class AbcdConstraint : public Constraint {
            private:
            class Impl : public Constraint::Impl {
                public:
                bool test(const Array& params) const {
                    return params[0] + params[3] > 0.0  // a + d
                        && params[2] > 0.0              // c
                        && params[3] > 0.0;             // d
                }
            };
            public:
            AbcdConstraint()
                : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl)) {}
        };
        //! function to minimize
        class AbcdCostFunction;
        friend class AbcdCostFunction;
        class AbcdCostFunction : public CostFunction {
            public:
            AbcdCostFunction(Abcd* abcd,
                             const std::vector<Real>& blackVols,
                             const std::vector<Real>::const_iterator& t)
            : abcd_(abcd), blackVols_(blackVols), t_(t) {}
            Real value(const Array& x) const {
                if (!abcd_->aIsFixed_) abcd_->a_ = x[0];
                if (!abcd_->bIsFixed_) abcd_->b_ = x[1];
                if (!abcd_->cIsFixed_) abcd_->c_ = x[2];
                if (!abcd_->dIsFixed_) abcd_->d_ = x[3];
                return abcd_->error(blackVols_, t_);
            }
            private:
            Abcd* abcd_;
            std::vector<Real> blackVols_;
            std::vector<Real>::const_iterator t_;
        };
        //! Parameters
        Real a_, b_, c_, d_;
        bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
    };

    //! Abcd Squared functional. Helper class.
    class AbcdSquared : public std::unary_function<Real,Real> {
    public:  
        AbcdSquared(Real a, Real b, Real c, Real d, Time S, Time T)
        : abcd_(new Abcd(a,b,c,d)), S_(S), T_(T) {}
        Real operator()(Time u) const {
            if (u>T_ || u>S_)
                return 0.0;
            else
                return (*abcd_)(T_-u)*(*abcd_)(S_-u);        
        }

    private:
        boost::shared_ptr<Abcd> abcd_;
        Time S_, T_;
    };


    // inline

    inline Real Abcd::shortTermVolatility() const {
        return a_+d_;
    }

    inline Real Abcd::longTermVolatility() const {
        return d_;
    }

    inline Real Abcd::maximumLocation() const {
        return (b_>0.0 ? (b_-c_*a_)/(c_*b_) : 0.0);
    }

    inline Real Abcd::maximumVolatility() const {
        return (b_>0.0 ?
                b_/c_*std::exp(-1.0 +c_*a_/b_)+d_ :
                shortTermVolatility());
    }

    inline Real Abcd::instantaneousVolatility(Time u, Time T) const {
        return std::sqrt(instantaneousVariance(u, T));
    }

    inline Real Abcd::instantaneousVariance(Time u, Time T) const {
        return instantaneousCovariance(u, T, T);
    }

    inline Real Abcd::volatility(Time tMin, Time tMax, Time T) const {
        if (tMax==tMin)
            return instantaneousVolatility(tMax, T);
        else
            return std::sqrt(variance(tMin, tMax, T)/(tMax-tMin));
    }

    inline Real Abcd::variance(Time tMin, Time tMax, Time T) const {
        return covariance(tMin, tMax, T, T);
    }

}

#endif
