/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file path.hpp
    \brief single factor random walk
*/

#ifndef quantlib_montecarlo_path_hpp
#define quantlib_montecarlo_path_hpp

#include <ql/timegrid.hpp>
#include <ql/Math/array.hpp>

namespace QuantLib {

    namespace Old {

        /*! single-factor random walk

            \ingroup mcarlo

            \deprecated when QL_DISABLE_DEPRECATED is defined,
                        New::Path is used instead.
        */
        class Path {
          public:
            Path(const TimeGrid& timeGrid,
                 const Array& drift = Array(),
                 const Array& diffusion = Array());
            //! \name inspectors
            //@{
            Real operator[](Size i) const;
            Size size() const;
            //@}
            //! \name read/write access to components
            //@{
            const TimeGrid& timeGrid() const;
            TimeGrid& timeGrid();
            const Array& drift() const;
            Array& drift();
            const Array& diffusion() const;
            Array& diffusion();
            //@}
          private:
            TimeGrid timeGrid_;
            Array drift_;
            Array diffusion_;
        };

    }

    namespace New {

        /*! single-factor random walk

            \ingroup mcarlo

            \note the path includes the initial asset value as its first point.
        */
        class Path {
          public:
            Path(const TimeGrid& timeGrid,
                 const Array& values = Array());
            //! \name inspectors
            //@{
            bool empty() const;
            Size length() const;
            //! asset value at the \f$ i \f$-th point
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

    }


    // inline definitions

    namespace Old {

        inline Path::Path(const TimeGrid& timeGrid, const Array& drift,
                          const Array& diffusion)
        : timeGrid_(timeGrid), drift_(drift), diffusion_(diffusion) {
            if (drift_.size()==0) {
                if (timeGrid_.size() > 0)
                    drift_ = Array(timeGrid_.size()-1);
            } else {
                QL_REQUIRE(drift_.size() == timeGrid_.size()-1,
                           "drift and times have different size");
            }
            if (diffusion_.size()==0) {
                if (timeGrid_.size() > 0)
                    diffusion_ = Array(timeGrid_.size()-1);
            } else {
                QL_REQUIRE(diffusion_.size() == timeGrid_.size()-1,
                           "diffusion and times have different size");
            }
        }

        inline Real Path::operator[](Size i) const {
            return drift_[i] + diffusion_[i];
        }

        inline Size Path::size() const {
            return drift_.size();
        }

        inline const TimeGrid& Path::timeGrid() const {
            return timeGrid_;
        }

        inline TimeGrid& Path::timeGrid() {
            return timeGrid_;
        }

        inline const Array& Path::drift() const {
            return drift_;
        }

        inline Array& Path::drift() {
            return drift_;
        }

        inline const Array& Path::diffusion() const {
            return diffusion_;
        }

        inline Array& Path::diffusion() {
            return diffusion_;
        }

    }

    namespace New {

        inline Path::Path(const TimeGrid& timeGrid, const Array& values)
        : timeGrid_(timeGrid), values_(values_) {
            if (values_.size() == 0)
                values_ = Array(timeGrid_.size());
            QL_REQUIRE(values_.size() == timeGrid_.size(),
                       "different number of times and asset values");
        }

        inline bool Path::empty() const {
            return timeGrid_.size() == 0;
        }

        inline Size Path::length() const {
            return timeGrid_.size();
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

}


#endif
