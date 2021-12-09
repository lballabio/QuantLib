/*
 Copyright (C) 2007 François du Vignaud

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

/*! \file integral.hpp
\brief Integrators base class definition
*/

#ifndef quantlib_math_integrator_hpp
#define quantlib_math_integrator_hpp

#include <ql/types.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    class Integrator{
      public:
        Integrator(Real absoluteAccuracy,
                   Size maxEvaluations);
        virtual ~Integrator() = default;

        Real operator()(const ext::function<Real (Real)>& f,
                        Real a,
                        Real b) const;

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

        Size numberOfEvaluations() const;

        virtual bool integrationSuccess() const;

      protected:
        virtual Real integrate(const ext::function<Real (Real)>& f,
                               Real a,
                               Real b) const = 0;
        void setAbsoluteError(Real error) const;
        void setNumberOfEvaluations(Size evaluations) const;
        void increaseNumberOfEvaluations(Size increase) const;
      private:
        Real absoluteAccuracy_;
        mutable Real absoluteError_;
        Size maxEvaluations_;
        mutable Size evaluations_;
    };

}


#endif
