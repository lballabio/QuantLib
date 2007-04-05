/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

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

/*! \file beta.hpp
    \brief Integrators base class definition
*/

#ifndef quantlib_math_Integrator_h
#define quantlib_math_Integrator_h

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <ql/utilities/null.hpp>
#include <boost/function.hpp>


namespace QuantLib {

    class Integrator{
    public:
Real operator()(const F& f, Real a, Real b) const {

            if (a == b)
                return 0.0;
            if (a > b)
                return -(*this)(f,b,a);

            functionEvaluations_ = 0;
            Real result = GaussKronrod(f, a, b, tolerance_);
            return result;
        }
    void setIntegrand(const boost::function<Real (Real)>& f);
    void setUpperBoundary(Real );
    void setLowerBoundary(Real );
    void setAccuracy(Real);
    void setMaxEvaluations(Size);

    void setIntegrand(const boost::function<Real (Real)>& f);
    Real upperBoundary() const;
    Real lowerBoundary() const;
    Real accuracy() const;
    Size maxEvaluations() const;

    private:
        boost::function<Real (Real)>& f_;
        Real upperBoundary_;
        Real lowerBoundary_;
        Real accuracy_;
        Size maxEvaluations_;
    };

    /*! 
    \brief This class allow to delegate the choice of the integration method
    */

    class IntegratorFactory{
    
    };

}


#endif
