
/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

// $Id$

#ifndef quantlib_constraint_h
#define quantlib_constraint_h

#include "ql/errors.hpp"
#include "ql/types.hpp"
#include <vector>

namespace QuantLib {

    class Constraint {
      public:
        Constraint(Size size)
        : minParams_(size, QL_MIN_DOUBLE), maxParams_(size, QL_MAX_DOUBLE) {}
        virtual ~Constraint() {}

        void setLowerBound(Size i, double boundary) {
            minParams_[i] = boundary;
        }

        void setUpperBound(Size i, double boundary) {
            maxParams_[i] = boundary;
        }

        virtual bool operator()(const std::vector<double>& params) const {
            Size size(params.size());
            QL_REQUIRE(size == minParams_.size(),
              "parameter vector is not of appropriate size");
            for (Size i=0; i<size; i++) {
                if ((minParams_[i]>=params[i]) || (maxParams_[i]<=params[i]))
                    return false;
            }
            return true;
        }
        double minParam(Size i) const { return minParams_[i]; }
        double maxParam(Size i) const { return maxParams_[i]; }

      private:
        std::vector<double> minParams_;
        std::vector<double> maxParams_;

    };
}

#endif
