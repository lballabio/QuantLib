
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

#include <map>

namespace QuantLib {

    struct timestr {
        bool operator()(Time t1, Time t2) const { 
            return t1<t2;
        }
    };

    class TimeFunction {
      public:
        TimeFunction() : values_() {}
        double  operator()(Time t) {
            std::map<Time, double, timestr>::iterator upper = 
                values_.upper_bound(t);
            Time high = upper->first;
            double highValue = upper->second;
            --upper;
            Time low = upper->first;
            double lowValue = upper->second;
            Time dt = high - low;
            Time weight = 1.0 - (t - low)/dt;
            return lowValue*weight + highValue*(1.0 - weight);
        }
        void set(Time t, double value) {
            values_[t] = value;
        }

        void clear() {
            values_.clear();
        }

      private:
        std::map<Time, double, timestr > values_;
    };

}


#endif
