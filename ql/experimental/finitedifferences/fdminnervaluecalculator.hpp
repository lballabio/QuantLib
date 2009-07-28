/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008,2009 Ralph Schreyer
 Copyright (C) 2008,2009 Klaus Spanderen

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

/*! \file fdminnervaluecalculator.hpp
    \brief layer of abstraction to calculate the inner value
*/

#ifndef quantlib_fdm_inner_value_calculator_hpp
#define quantlib_fdm_inner_value_calculator_hpp

#include <ql/experimental/finitedifferences/fdmmesher.hpp>
#include <ql/math/integrals/simpsonintegral.hpp>
#include <ql/math/array.hpp>
#include <ql/payoff.hpp>


namespace QuantLib {

    class FdmInnerValueCalculator {

      public:
        virtual ~FdmInnerValueCalculator() {}

        virtual Real innerValue(
			const boost::shared_ptr<FdmMesher>& mesher,
			const FdmLinearOpIterator& iter) = 0;

		virtual Real avgInnerValue(
			const boost::shared_ptr<FdmMesher>& mesher,
			const FdmLinearOpIterator& iter) = 0;
    };


    class FdmLogInnerValue : public FdmInnerValueCalculator {

      public:
        FdmLogInnerValue(const boost::shared_ptr<Payoff>& payoff,
                         Size direction)
        : payoff_(payoff), direction_(direction) {};

		Real innerValue(const boost::shared_ptr<FdmMesher>& mesher,
						const FdmLinearOpIterator& iter) {
            return payoff_->operator()(std::exp(mesher->location(
												iter,direction_)));
        };

		Real avgInnerValue(const boost::shared_ptr<FdmMesher>& mesher,
						   const FdmLinearOpIterator& iter) {
			const Size dim = mesher->layout()->dim()[direction_];
			const Size coord = iter.coordinates()[direction_];
			const Real loc = std::exp(mesher->location(iter,direction_));
			Real a = loc;
			Real b = loc;

			if (coord > 0) {
				a -= std::exp(mesher->dminus(iter, direction_))/2.0;
			}
			if (coord < dim-1) {
				b += std::exp(mesher->dplus(iter, direction_))/2.0;
			}

			boost::function1<Real, Real> f = std::bind1st(
					std::mem_fun(&Payoff::operator()), payoff_.get());
			
			Real retVal;
			try {
			    retVal = SimpsonIntegral(1e-4, 4)(f, a, b)/(b-a); 
			}
			catch (Error&) {
			    // use default value
			    retVal = innerValue(mesher, iter);
			}
			return retVal;
		};

      private:
        const boost::shared_ptr<Payoff> payoff_;
        const Size direction_;
    };
}

#endif
