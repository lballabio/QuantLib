/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen

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

#include <ql/termstructures/volatilities/abcd.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/simplex.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    Abcd::Abcd(Real a, Real b, Real c, Real d,
               bool aIsFixed, bool bIsFixed, bool cIsFixed, bool dIsFixed)
    : a_(a), b_(b), c_(c), d_(d), aIsFixed_(aIsFixed), bIsFixed_(bIsFixed),
      cIsFixed_(cIsFixed), dIsFixed_(dIsFixed) {
          validateAbcdParameters(a, b, c, d);
    }

    Real Abcd::operator()(Time u) const {
        if (u<0)
            return 0.0;
        else
            return (a_ + b_*u)*std::exp(-c_*u) + d_;
    }

    Real Abcd::instantaneousCovariance(Time u, Time T, Time S) const {
        return (*this)(T-u)*(*this)(S-u);
    }

    Real Abcd::covariance(Time t1, Time t2, Time T, Time S) const {
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

    std::vector<Real> Abcd::k(
                    const std::vector<Real>& blackVols,
                    const std::vector<Real>& t) const {
        QL_REQUIRE(blackVols.size()==t.size(),
            "mismatch between t size and number of blackVols");
        Size n = blackVols.size();
        std::vector<Real> k(n);
        for (Size i=0; i<n ; i++) {
            k[i]=blackVols[i]/volatility(0.0, t[i], t[i]);
        }
        return k;
    }

    Real Abcd::error(const std::vector<Real>& blackVols,
                     const std::vector<Real>& t) const {
        QL_REQUIRE(blackVols.size()==t.size(),
            "mismatch between t size and number of blackVols");
        Real error = 0.0;
        Size n = blackVols.size();
        for (Size i=0; i<n ; i++) {
            Real temp = blackVols[i]-volatility(0.0, t[i], t[i]);
            error += temp*temp;
        }
        return std::sqrt(error/n);
    }

    Disposable<Array> Abcd::errors(const std::vector<Real>& blackVols,
                                   const std::vector<Real>& t) const {
        QL_REQUIRE(blackVols.size()==t.size(),
            "mismatch between t size and number of blackVols");
        Size n = blackVols.size();
        Array errors(n,0.0);
        for (Size i=0; i<n ; i++) {
            errors[i]= blackVols[i]-volatility(0.0, t[i], t[i]);
        }
        return errors;
    }

    Real Abcd::maxError(const std::vector<Real>& blackVols,
                        const std::vector<Real>& t) const {
        QL_REQUIRE(blackVols.size()==t.size(),
            "mismatch between t size and number of blackVols");        
        Real maxError = QL_MIN_REAL;
        Size n = blackVols.size();
        for (Size i=0; i<n ; i++) {
            Real temp = blackVols[i]-volatility(0.0, t[i], t[i]);
            maxError = std::max(maxError, std::fabs(temp));
        }
        return maxError;
    }

    EndCriteria::Type Abcd::calibration(
                const std::vector<Real>& blackVols,
                const std::vector<Real>& t,
                const boost::shared_ptr<EndCriteria>& endCr,
                const boost::shared_ptr<OptimizationMethod>& meth) {
        
        QL_REQUIRE(blackVols.size()==t.size(),
            "mismatch between t size and number of blackVols");
        boost::shared_ptr<OptimizationMethod> method = meth;
        if (!method) {
            method = boost::shared_ptr<OptimizationMethod>(new
                LevenbergMarquardt(1e-8, 1e-8, 1e-8));
            //method = boost::shared_ptr<OptimizationMethod>(new
            //    Simplex(0.01));
        }
        boost::shared_ptr<EndCriteria> endCriteria = endCr;
        if (!endCriteria) {
            endCriteria = boost::shared_ptr<EndCriteria>(new
                EndCriteria(1000, 100, 1.0e-8, 0.3e-4, 0.3e-4));   // Why 0.3e-4 ?
        }

        Array guess(4);
        guess[0] = a_;
        guess[1] = b_;
        guess[2] = c_;
        guess[3] = d_;

        AbcdConstraint constraint;
        AbcdCostFunction costFunction(this, blackVols, t);
        Problem problem(costFunction, constraint, guess);
        EndCriteria::Type ec = method->minimize(problem, *endCriteria);

		Array result = problem.currentValue();
        if (!aIsFixed_) a_ = result[0];
        if (!bIsFixed_) b_ = result[1];
        if (!cIsFixed_) c_ = result[2];
        if (!dIsFixed_) d_ = result[3];

        validateAbcdParameters(a_, b_, c_, d_);

        return ec;
    }


    Real Abcd::primitive(Time u, Time T, Time S) const {

        if (u>T) return 0.0;
        if (u>S) return 0.0;

        const Real k1=std::exp(c_*u);
        const Real k2=std::exp(c_*S);
        const Real k3=std::exp(c_*T);

        return (b_*b_*(-1 - 2*c_*c_*S*T - c_*(S + T)
                     + k1*k1*(1 + c_*(S + T - 2*u) + 2*c_*c_*(S - u)*(T - u)))
                + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                         +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*u)
                + 2*b_*c_*(a_*(-1 - c_*(S + T) + k1*k1*(1 + c_*(S + T - 2*u)))
                         -2*d_*(k3*(1 + c_*S) + k2*(1 + c_*T)
                               - k1*k3*(1 + c_*(S - u))
                               - k1*k2*(1 + c_*(T - u)))
                         )
                ) / (4*c_*c_*c_*k2*k3);
    }

}
