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
/*! \file asset.hpp
    \brief Asset slice class

    \fullpath
    ql/%asset.hpp
*/

// $Id$

#ifndef quantlib_asset_h
#define quantlib_asset_h

#include <ql/array.hpp>

namespace QuantLib {
  
    namespace Lattices {
        class Tree;
    }
    class Asset {
      public:
        virtual ~Asset() {}

        Time time() const { return time_; }
        const Array& values() { return values_; }

      protected:
        friend class Lattices::Tree;
        virtual void reset(Size size) = 0;
        void setTime(Time t) { time_ = t; }
        void setValues(const Array& values) { values_ = values; }

        virtual void applyCondition() {}


        Array values_;
        Time time_;
    };

}

#endif
