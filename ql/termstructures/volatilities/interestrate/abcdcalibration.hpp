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

#ifndef quantlib_abcdcalibration_hpp
#define quantlib_abcdcalibration_hpp


#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/array.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>


namespace QuantLib {
    
    class Quote;
    class OptimizationMethod;
    class ParametersTransformation;

    class AbcdCalibration {
      public:
        AbcdCalibration(
             const std::vector<Real>& t,
             const std::vector<Real>& blackVols,
             Real aGuess = -0.06,
             Real bGuess =  0.17,
             Real cGuess =  0.54,
             Real dGuess =  0.17,
             bool aIsFixed = false,
             bool bIsFixed = false,
             bool cIsFixed = false,
             bool dIsFixed = false,
             bool vegaWeighted = false,
             const boost::shared_ptr<EndCriteria>& endCriteria
                      = boost::shared_ptr<EndCriteria>(),
             const boost::shared_ptr<OptimizationMethod>& method
                      = boost::shared_ptr<OptimizationMethod>());

        //! adjustment factors needed to match Black vols
        std::vector<Real> k(const std::vector<Real>& t,
                            const std::vector<Real>& blackVols) const;
        void compute();
        //calibration results
        Real value(Real x) const;
        Real error() const;
        Real maxError() const;
        Disposable<Array> errors() const;
        EndCriteria::Type endCriteria() const;

        Real a() const;
        Real b() const;
        Real c() const;
        Real d() const;

        bool aIsFixed_, bIsFixed_, cIsFixed_, dIsFixed_;
        Real a_, b_, c_, d_;
        boost::shared_ptr<ParametersTransformation> transformation_;

      private:
        // optimization method used for fitting
        mutable EndCriteria::Type abcdEndCriteria_;
        boost::shared_ptr<EndCriteria> endCriteria_;
        boost::shared_ptr<OptimizationMethod> method_;
        mutable std::vector<Real> weights_;
        bool vegaWeighted_;
        //! Parameters
        std::vector<Real> times_, blackVols_;
    };

}

#endif
