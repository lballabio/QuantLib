
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy_ of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file timefunction.hpp
    \brief Time function used for term structure fitting

    \fullpath
    ql/InterestRateModelling/%timefunction.hpp
*/

// $Id$

#ifndef quantlib_time_function_h
#define quantlib_time_function_h

#include <ql/qldefines.hpp>
#include <ql/Math/cubicspline.hpp>
#include <ql/InterestRateModelling/onefactormodel.hpp>

#include <iostream>

namespace QuantLib {

    namespace InterestRateModelling {

        class TimeFunction {
          public:
            typedef Math::CubicSpline<std::vector<Time>::iterator,
                                      std::vector<double>::iterator> Spline;
            TimeFunction() 
            : updated_(false), times_(0), values_(0) {}
            ~TimeFunction() {}
            void set(Time t, double x) {
                updated_ = false;
                times_.push_back(t);
                values_.push_back(x);
            }
            void update() {
                interpolation_ = Handle<Spline>(new Spline(
                    times_.begin(), times_.end(), values_.begin()));
                updated_ = true;
            }
            void reset() {
                times_.clear();
                values_.clear();
                updated_ = false;
            }
            double operator()(Time t) {
                if (!updated_) {
                    for (size_t i=0; i<times_.size(); i++)
                        if (times_[i] == t)
                            return values_[i];
                    std::cout << "interpolating at time " << t << std::endl;
                    update();
                }
                return (*interpolation_)(t);
            }
            void fitToTermStructure(const OneFactorModel& model, 
                                    size_t timeSteps);
          private:
            class FitFunction;

            bool updated_;
            Handle<Spline> interpolation_;
            std::vector<Time> times_;
            std::vector<double> values_;
        };

    }

}


#endif
