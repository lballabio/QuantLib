/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco

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


#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/math/optimization/constraint.hpp>


namespace QuantLib {

    inline void validateAbcdParameters(Real a, Real, Real c, Real d) {
        QL_REQUIRE(a+d>=0,
                   "a+d (" << a << ", " << d <<") must be non negative");
        QL_REQUIRE(c>=0,
                   "c (" << c << ") must be non negative");
        QL_REQUIRE(d>=0,
                   "d (" << d << ") must be non negative");
    }

    //! %Abcd functional form for instantaneous volatility
    /*! \f[ f(T-t) = [ a + b(T-t) ] e^{-c(T-t)} + d \f]
        following Rebonato's notation.
    */
    struct AbcdFunction : public std::unary_function<Real, Real> {
        Real a_, b_, c_, d_;

        AbcdFunction(Real a=-0.06, Real b=0.17, Real c=0.54, Real d=0.17);

        //! volatility function value at time u: \f[ f(u) \f]
        Real operator()(Time u) const;

        //! time at which the volatility function reaches maximum (if any)
        Real maximumLocation() const;

        //! maximum value of the volatility function
        Real maximumValue() const;

        //! volatility function value at time 0: \f[ f(0) \f]
        Real shortTermValue() const { return a_+d_; }

        //! volatility function value at time +inf: \f[ f(\inf) \f]
        Real longTermValue() const { return d_; }

        /*! instantaneous covariance function at time t between T-fixing and
            S-fixing rates \f[ f(T-t)f(S-t) \f]
        */
        Real covariance(Time t, Time T, Time S) const;

        /*! integral of the instantaneous covariance function between
            time t1 and t2 for T-fixing and S-fixing rates
            \f[ \int_{t1}^{t2} f(T-t)f(S-t)dt \f]
        */

        Real covariance(Time t1, Time t2, Time T, Time S) const;

        /*! indefinite integral of the instantaneous covariance function at
            time t between T-fixing and S-fixing rates
            \f[ \int f(T-t)f(S-t)dt \f]
        */
        Real primitive(Time t, Time T, Time S) const;

         /*! volatility in [tMin,tMax] of T-fixing rate:
            \f[ \sqrt{ \int_{tMin}^{tMax} f^2(T-u)du }\f]
        */
        Real volatility(Time T, Time tMax, Time tMin) const;

        /*! variance in [tMin,tMax] of T-fixing rate:
            \f[ \int_{tMin}^{tMax} f^2(T-u)du \f]
        */
        Real variance(Time T, Time tMax, Time tMin) const;
    };

    inline AbcdFunction::AbcdFunction(Real a, Real b, Real c, Real d)
    : a_(a), b_(b), c_(c), d_(d) {
        validateAbcdParameters(a, b, c, d);
    }

    inline Real AbcdFunction::operator()(Time u) const {
        return u<0 ? 0.0 : (a_ + b_*u)*std::exp(-c_*u) + d_;
    }

    inline Real AbcdFunction::maximumLocation() const {
        if (b_<=0) {
            return 0.0;
        } else {
            if((b_-c_*a_)/(c_*b_)>0) {
                return (b_-c_*a_)/(c_*b_);
            } else
                return 0.0;
        }
    }

    inline Real AbcdFunction::maximumValue() const {
        if (b_<=0) {
            return shortTermValue();
        } else {
            if ((b_-c_*a_)/(c_*b_) > 0.0) {
                return b_/c_*std::exp(-1.0 +c_*a_/b_) + d_;
            } else
                return shortTermValue();
        }
    }

    inline Real AbcdFunction::covariance(Time t, Time T, Time S) const {
        return (*this)(T-t) * (*this)(S-t);
    }

    inline Real AbcdFunction::primitive(Time t, Time T, Time S) const {
        if (T<t || S<t) return 0.0;

        Real k1=std::exp(c_*t), k2=std::exp(c_*S), k3=std::exp(c_*T);
        return (b_*b_*(-1 - 2*c_*c_*S*T - c_*(S + T)
                     + k1*k1*(1 + c_*(S + T - 2*t) + 2*c_*c_*(S - t)*(T - t)))
                + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                         +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*t)
                + 2*b_*c_*(a_*(-1 - c_*(S + T) + k1*k1*(1 + c_*(S + T - 2*t)))
                         -2*d_*(k3*(1 + c_*S) + k2*(1 + c_*T)
                               - k1*k3*(1 + c_*(S - t))
                               - k1*k2*(1 + c_*(T - t)))
                         )
                ) / (4*c_*c_*c_*k2*k3);
    }

    inline Real AbcdFunction::covariance(Time t1, Time t2, Time T, Time S)
        const {
        QL_REQUIRE(t1<=t2,
                   "integrations bounds (" << t1 <<
                   "," << t2 << ") are in reverse order");
        Time cutOff = std::min(S,T);
        if (t1>=cutOff) {
            return 0.0;
        } else {
            cutOff = std::min(t2, cutOff);
            return primitive(cutOff, T, S) - primitive(t1, T, S);
        }
    }

    inline Real AbcdFunction::volatility(Time T, Time tMax, Time tMin) const {
        if (tMax==tMin)
            return std::sqrt(covariance(tMax, T, T));
        QL_REQUIRE(tMax>tMin, "tMax must be > tMin");
        return std::sqrt(variance(tMin, tMax, T)/(tMax-tMin));
    }

    inline Real AbcdFunction::variance(Time T, Time tMax, Time tMin) const {
        return covariance(tMin, tMax, T, T);
    }


    // Helper class used by unit tests
    struct AbcdSquared : public std::unary_function<Real,Real> {
        boost::shared_ptr<AbcdFunction> abcdFunction_;
        Time T_, S_;
        AbcdSquared(Real a, Real b, Real c, Real d, Time T, Time S)
        : abcdFunction_(new AbcdFunction(a,b,c,d)), T_(T), S_(S) {}
        Real operator()(Time t) const {
            return abcdFunction_->covariance(t, T_, S_);
        }
    };

    class OptimizationMethod;

    //! %Abcd functional form for instantaneous volatility
    /*!
        \f[
        f(T-t) = [ a + b(T-t) ] e^{-c(T-t)} + d
        \f]

        following Rebonato notation.

    */
    class Abcd : public std::unary_function<Real,Real> {
      public:
        Abcd(Real a = -0.06,
             Real b =  0.17,
             Real c =  0.54,
             Real d =  0.17,
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

        /*! instantaneous volatility at time t of the T-fixing rate:
            \f[ f(T-t) \f]
        */
        Real instantaneousVolatility(Time t, Time T) const;

        /*! instantaneous variance at time t of T-fixing rate:
            \f[ f(T-t)f(T-t) \f]
        */
        Real instantaneousVariance(Time t, Time T) const;

        /*! instantaneous covariance at time t between T and S fixing rates:
            \f[ f(T-u)f(S-u) \f]
        */
        Real instantaneousCovariance(Time u, Time T, Time S) const;

        /*! volatility in [tMin,tMax] of T-fixing rate:
            \f[ \sqrt{ \int_{tMin}^{tMax} f^2(T-u)du }\f]
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
        //! volatility error
        Real error(const std::vector<Real>& blackVols,
                   const std::vector<Real>::const_iterator& t) const;
        //! volatility errors
        Disposable<Array> errors(const std::vector<Real>& blackVols,
                                 const std::vector<Real>::const_iterator& t) const;
        //! volatility max error
        Real maxError(const std::vector<Real>& blackVols,
                      const std::vector<Real>::const_iterator& t) const;

        //! calibration
        EndCriteria::Type calibration(
            const std::vector<Real>& blackVols,
            const std::vector<Real>::const_iterator& t,
            const boost::shared_ptr<EndCriteria>& endCriteria
                                = boost::shared_ptr<EndCriteria>(),
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
            Disposable<Array> values(const Array& x) const {
                if (!abcd_->aIsFixed_) abcd_->a_ = x[0];
                if (!abcd_->bIsFixed_) abcd_->b_ = x[1];
                if (!abcd_->cIsFixed_) abcd_->c_ = x[2];
                if (!abcd_->dIsFixed_) abcd_->d_ = x[3];
                return abcd_->errors(blackVols_, t_);
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

    // inline

    inline Real Abcd::shortTermVolatility() const {
        return a_+d_;
    }

    inline Real Abcd::longTermVolatility() const {
        return d_;
    }

    inline Real Abcd::maximumLocation() const {
        if (b_<=0) {
            return 0.0;
        } else {
            if((b_-c_*a_)/(c_*b_)>0) {
                return (b_-c_*a_)/(c_*b_);
            } else
                return 0.0;
        }
        //return (b_>0.0 ? (b_-c_*a_)/(c_*b_) : 0.0);
    }

    inline Real Abcd::maximumVolatility() const {
        if (b_<=0) {
            return shortTermVolatility();
        } else {
            if ((b_-c_*a_)/(c_*b_) > 0.0) {
                return b_/c_*std::exp(-1.0 +c_*a_/b_) + d_;
            } else
                return shortTermVolatility();
        }        
        //return (b_>0.0 ?
        //        b_/c_*std::exp(-1.0 +c_*a_/b_)+d_ :
        //        shortTermVolatility());
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
        QL_REQUIRE(tMax>tMin, "tMax must be > tMin");
        return std::sqrt(variance(tMin, tMax, T)/(tMax-tMin));
    }

    inline Real Abcd::variance(Time tMin, Time tMax, Time T) const {
        return covariance(tMin, tMax, T, T);
    }

}

#endif
