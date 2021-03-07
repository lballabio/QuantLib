/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2010 Kakhkhor Abdijalilov
 
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

/*! \file lsmbasissystem.cpp
    \brief utility classes for longstaff schwartz early exercise Monte Carlo
*/

#include <ql/math/integrals/gaussianquadratures.hpp>
#include <ql/methods/montecarlo/lsmbasissystem.hpp>
#include <numeric>
#include <set>
#include <utility>

namespace QuantLib {
    namespace {

        // makes typing a little easier
        typedef std::vector<ext::function<Real(Real)> > VF_R;
        typedef std::vector<ext::function<Real(Array)> > VF_A;
        typedef std::vector<std::vector<Size> > VV;

        // pow(x, order)
        class MonomialFct {
          public:
            explicit MonomialFct(Size order): order_(order) {}
            inline Real operator()(const Real x) const {
                Real ret = 1.0;
                for(Size i=0; i<order_; ++i)
                    ret *= x;
                return ret;
            }
          private:
            const Size order_;
        };

        /* multiplies [Real -> Real] functors
           to create [Array -> Real] functor */
        class MultiDimFct {
          public:
            explicit MultiDimFct(VF_R b) : b_(std::move(b)) {
                QL_REQUIRE(!b_.empty(), "zero size basis");
            }
            inline Real operator()(const Array& a) const {
                #if defined(QL_EXTRA_SAFETY_CHECKS)
                QL_REQUIRE(b_.size()==a.size(), "wrong argument size");
                #endif
                Real ret = b_[0](a[0]);
                for(Size i=1; i<b_.size(); ++i)
                    ret *= b_[i](a[i]);
                return ret;
            }
          private:
            const VF_R b_;
        };

        // check size and order of tuples
        void check_tuples(const VV& v, Size dim, Size order) {
            for (const auto& i : v) {
                QL_REQUIRE(dim == i.size(), "wrong tuple size");
                QL_REQUIRE(order == std::accumulate(i.begin(), i.end(), 0UL), "wrong tuple order");
            }
        }

        // build order N+1 tuples from order N tuples
        VV next_order_tuples(const VV& v) {
            const Size order = std::accumulate(v[0].begin(), v[0].end(), 0UL);
            const Size dim = v[0].size();

            check_tuples(v, dim, order);

            // the set of unique tuples
            std::set<std::vector<Size> > tuples;
            std::vector<Size> x;
            for(Size i=0; i<dim; ++i) {
                // increase i-th value in every tuple by 1
                for (const auto& j : v) {
                    x = j;
                    x[i] += 1;
                    tuples.insert(x);
                }
            }

            VV ret(tuples.begin(), tuples.end());
            return ret;
        }
    } 

    // LsmBasisSystem static methods

    VF_R LsmBasisSystem::pathBasisSystem(Size order, PolynomType polyType) {
        VF_R ret(order+1);
        for (Size i=0; i<=order; ++i) {
            switch (polyType) {
              case Monomial:
                ret[i] = MonomialFct(i);
                break;
              case Laguerre:
                {
                  GaussLaguerrePolynomial p;
                  ret[i] = [=](Real x){ return p.weightedValue(i, x); };
                }
                break;
              case Hermite:
                {
                  GaussHermitePolynomial p;
                  ret[i] = [=](Real x){ return p.weightedValue(i, x); };
                }
                break;
              case Hyperbolic:
                {
                  GaussHyperbolicPolynomial p;
                  ret[i] = [=](Real x){ return p.weightedValue(i, x); };
                }
                break;
              case Legendre:
                {
                  GaussLegendrePolynomial p;
                  ret[i] = [=](Real x){ return p.weightedValue(i, x); };
                }
                break;
              case Chebyshev:
                {
                  GaussChebyshevPolynomial p;
                  ret[i] = [=](Real x){ return p.weightedValue(i, x); };
                }
                break;
              case Chebyshev2nd:
                {
                  GaussChebyshev2ndPolynomial p;
                  ret[i] = [=](Real x){ return p.weightedValue(i, x); };
                }
                break;
              default:
                QL_FAIL("unknown regression type");
            }
        }
        return ret;
    }

    VF_A LsmBasisSystem::multiPathBasisSystem(Size dim, Size order,
                                              PolynomType polyType) {
        QL_REQUIRE(dim>0, "zero dimension");
        // get single factor basis
        VF_R pathBasis = pathBasisSystem(order, polyType);
        VF_A ret;
        // 0-th order term
        VF_R term(dim, pathBasis[0]);
        ret.push_back(MultiDimFct(term));
        // start with all 0 tuple
        VV tuples(1, std::vector<Size>(dim));
        // add multi-factor terms
        for(Size i=1; i<=order; ++i) {
            tuples = next_order_tuples(tuples);
            // now we have all tuples of order i
            // for each tuple add the corresponding term
            for (auto& tuple : tuples) {
                for(Size k=0; k<dim; ++k)
                    term[k] = pathBasis[tuple[k]];
                ret.push_back(MultiDimFct(term));
            }
        }
        return ret;
    }
}
