
/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/grid.hpp>
#include <ql/Math/comparison.hpp>
#include <iomanip>

namespace QuantLib {

    namespace {

        class CloseEnoughTo : public std::unary_function<Real,bool> {
          public:
            CloseEnoughTo(Real target) : target_(target) {}
            bool operator()(Real x) const {
                return close_enough(x,target_);
            }
          private:
            Real target_;
        };

    }


    Size TimeGrid::findIndex(Time t) const {
        const_iterator result = std::find_if(begin(), end(),
                                             CloseEnoughTo(t));
        if (result == end()) {
            Size i;
            for (i=0; i<size(); i++) {
                if ((*this)[i] > t)
                    break;
            }
            if (i == 0) {
                QL_FAIL("using inadequate time grid: all nodes "
                        "are later than the required time t = "
                        << std::setprecision(12) << t
                        << " (earliest node is t1 = "
                        << std::setprecision(12) << (*this)[0] << ")");
            } else if (i == size()) {
                QL_FAIL("using inadequate time grid: all nodes "
                        "are earlier than the required time t = "
                        << std::setprecision(12) << t
                        << " (latest node is t1 = "
                        << std::setprecision(12) << (*this)[size()-1] << ")");
            } else {
                QL_FAIL("using inadequate time grid: the nodes closest "
                        "to the required time t = "
                        << std::setprecision(12) << t
                        << " are t1 = "
                        << std::setprecision(12) << (*this)[i-1]
                        << " and t2 = "
                        << std::setprecision(12) << (*this)[i]);
            }
        }
        return result - begin();
    }

}

