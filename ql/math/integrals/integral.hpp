/*
 Copyright (C) 2007 François du Vignaud

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

/*! \file integral.hpp
\brief Integrators base class definition
*/

#ifndef quantlib_math_integrator_hpp
#define quantlib_math_integrator_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

namespace QuantLib {

	class Integrator{
		public:
			Integrator(Real absoluteAccuracy, Size maxEvaluations);
			virtual Real operator()(const boost::function<Real (Real)>& f,
				Real a, Real b) const = 0;

			//! \name Modifiers
			//@{
			void setAbsoluteAccuracy(Real);
			void setMaxEvaluations(Size);
			//@}

			//! \name Inspectors
			//@{
			Real absoluteAccuracy() const;
			Size maxEvaluations() const;
			//@}

			Real absoluteError() const ;
			void setAbsoluteError(Real error) const;

			Size numberOfEvalutions() const;
			void setNumberOfEvalutions(Size nbEvalutions) const;

			virtual bool integrationSuccess() const; 

		private:
			Real absoluteAccuracy_;
			mutable Real absoluteError_;
			Size maxEvaluations_;
			mutable Size nbEvalutions_;
	};

	/*!
	\brief This class allows to delegate the choice of the integration method
	*/
	class IntegratorFactory{
		boost::shared_ptr<Integrator> operator()() const;
	};

}


#endif
