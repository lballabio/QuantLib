/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
        Constraint(unsigned int size)
        : minParams_(size, QL_MIN_DOUBLE), maxParams_(size, QL_MAX_DOUBLE) {}
        virtual ~Constraint() {}

        void setLowerBound(unsigned int i, double boundary) {
            minParams_[i] = boundary;
        }

        void setUpperBound(unsigned int i, double boundary) {
            maxParams_[i] = boundary;
        }

        virtual bool operator()(const std::vector<double>& params) const {
            Size size(params.size());
            QL_REQUIRE(size == minParams_.size(),
              "parameter vector is not of appropriate size");
            for (unsigned i=0; i<size; i++) {
                if ((minParams_[i]>=params[i]) || (maxParams_[i]<=params[i]))
                    return false;
            }
            return true;
        }
        double minParam(unsigned i) const { return minParams_[i]; }
        double maxParam(unsigned i) const { return maxParams_[i]; }

      private:
        std::vector<double> minParams_;
        std::vector<double> maxParams_;

    };
}

#endif
