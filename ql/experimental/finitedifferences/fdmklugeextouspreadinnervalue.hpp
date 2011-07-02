/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Klaus Spanderen

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

/*! \file fdoujumpmodelinnervalue.hpp
    \brief inner value calculator for the Ornstein Uhlenbeck
           plus exponential jumps model (Kluge Model)
*/

#ifndef quantlib_fdm_kluge_ext_ou_spread_inner_value_hpp
#define quantlib_fdm_kluge_ext_ou_spread_inner_value_hpp

#include <ql/instruments/basketoption.hpp>
#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/experimental/finitedifferences/fdmlinearopiterator.hpp>
#include <ql/experimental/finitedifferences/fdminnervaluecalculator.hpp>

namespace QuantLib {

    class FdmKlugeExtOUSpreadInnerValue : public FdmInnerValueCalculator {
      public:
        typedef std::vector<std::pair<Time, Real> > Shape;

        FdmKlugeExtOUSpreadInnerValue(
                const boost::shared_ptr<BasketPayoff>& payoff,
                const boost::shared_ptr<FdmMesher>& mesher,
                const boost::shared_ptr<Shape>& klugeShape =
                                                    boost::shared_ptr<Shape>(),
                const boost::shared_ptr<Shape>& extOUShape =
                                                    boost::shared_ptr<Shape>())
        : payoff_(payoff),
          mesher_(mesher),
          klugeShape_(klugeShape),
          extOUShape_(extOUShape) { }

        Real innerValue(const FdmLinearOpIterator& iter, Time t) {
            const Real x = mesher_->location(iter, 0);
            const Real y = mesher_->location(iter, 1);
            const Real u = mesher_->location(iter, 2);

            Real f = 0;
            if (klugeShape_) {
                f = std::lower_bound(klugeShape_->begin(), klugeShape_->end(),
                   std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;
            }

            Real g = 0;
            if (extOUShape_) {
                g = std::lower_bound(extOUShape_->begin(), extOUShape_->end(),
                   std::pair<Time, Real>(t-std::sqrt(QL_EPSILON), 0.0))->second;
            }
            Array a(2);
            a[0] = std::exp(f+x+y);
            a[1] = std::exp(g+u);

            return payoff_->operator()(a);
        }
        Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) {
            return innerValue(iter, t);
        }

      private:
        const boost::shared_ptr<BasketPayoff> payoff_;
        const boost::shared_ptr<FdmMesher> mesher_;
        const boost::shared_ptr<Shape> klugeShape_, extOUShape_;
    };
}

#endif
