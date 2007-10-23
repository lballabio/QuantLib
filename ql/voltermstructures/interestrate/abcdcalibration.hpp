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
#include <ql/math/optimization/projectedcostfunction.hpp>
#include <ql/math/array.hpp>
#include <boost/shared_ptr.hpp>
#include <vector>


namespace QuantLib {
    
    class Quote;
    class OptimizationMethod;
    class ParametersTransformation;

    class AbcdCalibration {
    
    private:

        class AbcdError : public CostFunction {
          public:
            AbcdError(AbcdCalibration* abcd) : abcd_(abcd) {}

            Real value(const Array& x) const {
                const Array y = abcd_->transformation_->direct(x);
                abcd_->a_ = y[0];
                abcd_->b_ = y[1];
                abcd_->c_ = y[2];
                abcd_->d_ = y[3];
                return abcd_->error();
            }
            Disposable<Array> values(const Array& x) const {
                const Array y = abcd_->transformation_->direct(x);
                abcd_->a_ = y[0];
                abcd_->b_ = y[1];
                abcd_->c_ = y[2];
                abcd_->d_ = y[3];
                return abcd_->errors();
            }
          private:
            AbcdCalibration* abcd_;
        };

        class AbcdParametersTransformation :
              public ParametersTransformation {
                 mutable Array y_;
                 const Real eps1_;
         public:

            AbcdParametersTransformation() : y_(Array(4)),
                eps1_(.000000001){ }

            Array direct(const Array& x) const {
                y_[0] = x[0]*x[0] - x[3]*x[3] + eps1_;  // a + d > 0
                y_[1] = x[1];
                y_[2] = x[2]*x[2]+ eps1_;               // c > 0
                y_[3] = x[3]*x[3]+ eps1_;               // d > 0
                return y_;
            }

            Array inverse(const Array& x) const {
                y_[0] = std::sqrt(x[0] + x[3]- eps1_);
                y_[1] = x[1];
                y_[2] = std::sqrt(x[2]- eps1_);
                y_[3] = std::sqrt(x[3]- eps1_);
                return y_;
            }
        };

      public:


        AbcdCalibration() {};
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
        boost::shared_ptr<OptimizationMethod> optMethod_;
        mutable std::vector<Real> weights_;
        bool vegaWeighted_;
        //! Parameters
        std::vector<Real> times_, blackVols_;
    };

}

#endif
