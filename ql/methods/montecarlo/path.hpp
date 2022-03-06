/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006 StatPro Italia srl
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file path.hpp
    \brief single factor random walk
*/

#ifndef quantlib_montecarlo_path_hpp
#define quantlib_montecarlo_path_hpp

#include <ql/math/array.hpp>
#include <ql/timegrid.hpp>
#include <utility>

namespace QuantLib {

    //! single-factor random walk
    /*! \ingroup mcarlo

        \note the path includes the initial asset value as its first point.
    */
    class Path {
      public:
        Path(TimeGrid timeGrid, Array values = Array());
        //! \name inspectors
        //@{
        bool empty() const;
        Size length() const;
        //! asset value at the \f$ i \f$-th point
        Real operator[](Size i) const;
        Real at(Size i) const;
        Real& operator[](Size i);
        Real& at(Size i);
        Real value(Size i) const;
        Real& value(Size i);
        //! time at the \f$ i \f$-th point
        Time time(Size i) const;
        //! initial asset value
        Real front() const;
        Real& front();
        //! final asset value
        Real back() const;
        Real& back();
        //! time grid
        const TimeGrid& timeGrid() const;
        //@}
        //! \name iterators
        //@{
        typedef Array::const_iterator iterator;
        typedef Array::const_reverse_iterator reverse_iterator;
        iterator begin() const;
        iterator end() const;
        reverse_iterator rbegin() const;
        reverse_iterator rend() const;
        //@}
      private:
        TimeGrid timeGrid_;
        Array values_;
    };


    // inline definitions

    inline Path::Path(TimeGrid timeGrid, Array values)
    : timeGrid_(std::move(timeGrid)), values_(std::move(values)) {
        if (values_.empty())
            values_ = Array(timeGrid_.size());
        QL_REQUIRE(values_.size() == timeGrid_.size(),
                   "different number of times and asset values");
    }

    inline bool Path::empty() const {
        return timeGrid_.empty();
    }

    inline Size Path::length() const {
        return timeGrid_.size();
    }

    inline Real Path::operator[](Size i) const {
        return values_[i];
    }

    inline Real Path::at(Size i) const {
        return values_.at(i);
    }

    inline Real& Path::operator[](Size i) {
        return values_[i];
    }

    inline Real& Path::at(Size i) {
        return values_.at(i);
    }

    inline Real Path::value(Size i) const {
        return values_[i];
    }

    inline Real& Path::value(Size i) {
        return values_[i];
    }

    inline Real Path::front() const {
        return values_[0];
    }

    inline Real& Path::front() {
        return values_[0];
    }

    inline Real Path::back() const {
        return values_[values_.size()-1];
    }

    inline Real& Path::back() {
        return values_[values_.size()-1];
    }

    inline Time Path::time(Size i) const {
        return timeGrid_[i];
    }

    inline const TimeGrid& Path::timeGrid() const {
        return timeGrid_;
    }

    inline Path::iterator Path::begin() const {
        return values_.begin();
    }

    inline Path::iterator Path::end() const {
        return values_.end();
    }

    inline Path::reverse_iterator Path::rbegin() const {
        return values_.rbegin();
    }

    inline Path::reverse_iterator Path::rend() const {
        return values_.rend();
    }

}


#endif


#ifndef id_83e1a0c16f113726ee2c2009277b608f
#define id_83e1a0c16f113726ee2c2009277b608f
inline bool test_83e1a0c16f113726ee2c2009277b608f(const int* i) {
    return i != nullptr;
}
#endif
