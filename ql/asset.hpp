
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
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file asset.hpp
    \brief Asset slice class

    \fullpath
    ql/%asset.hpp
*/

// $Id$

#ifndef quantlib_asset_h
#define quantlib_asset_h

#include <ql/qldefines.hpp>
#include <ql/array.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    class Asset {
      public:
        virtual ~Asset() {}

        Time time() const { return time_; }
        void setTime(Time t) { time_ = t; }

        virtual void reset(size_t size) = 0;

        const Array& values() { return values_; }
        Array& newValues() { return newValues_; }

        virtual void applyCondition() {
            values_ = newValues_;
        }

      protected:
        Array newValues_;
        Array values_;
        Time time_;
    };

}

#endif
