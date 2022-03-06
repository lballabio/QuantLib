/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2003 RiskMap srl

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

/*! \file generalstatistics.hpp
    \brief statistics tool
*/

#ifndef quantlib_general_statistics_hpp
#define quantlib_general_statistics_hpp

#include <ql/utilities/null.hpp>
#include <ql/errors.hpp>
#include <vector>
#include <algorithm>
#include <utility>

namespace QuantLib {

    //! Statistics tool
    /*! This class accumulates a set of data and returns their
        statistics (e.g: mean, variance, skewness, kurtosis,
        error estimation, percentile, etc.) based on the empirical
        distribution (no gaussian assumption)

        It doesn't suffer the numerical instability problem of
        IncrementalStatistics. The downside is that it stores all
        samples, thus increasing the memory requirements.
    */
    class GeneralStatistics {
      public:
        typedef Real value_type;
        GeneralStatistics();
        //! \name Inspectors
        //@{
        //! number of samples collected
        Size samples() const;

        //! collected data
        const std::vector<std::pair<Real,Real> >& data() const;

        //! sum of data weights
        Real weightSum() const;

        /*! returns the mean, defined as
            \f[ \langle x \rangle = \frac{\sum w_i x_i}{\sum w_i}. \f]
        */
        Real mean() const;

        /*! returns the variance, defined as
            \f[ \sigma^2 = \frac{N}{N-1} \left\langle \left(
                x-\langle x \rangle \right)^2 \right\rangle. \f]
        */
        Real variance() const;

        /*! returns the standard deviation \f$ \sigma \f$, defined as the
            square root of the variance.
        */
        Real standardDeviation() const;

        /*! returns the error estimate on the mean value, defined as
            \f$ \epsilon = \sigma/\sqrt{N}. \f$
        */
        Real errorEstimate() const;

        /*! returns the skewness, defined as
            \f[ \frac{N^2}{(N-1)(N-2)} \frac{\left\langle \left(
                x-\langle x \rangle \right)^3 \right\rangle}{\sigma^3}. \f]
            The above evaluates to 0 for a Gaussian distribution.
        */
        Real skewness() const;

        /*! returns the excess kurtosis, defined as
            \f[ \frac{N^2(N+1)}{(N-1)(N-2)(N-3)}
                \frac{\left\langle \left(x-\langle x \rangle \right)^4
                \right\rangle}{\sigma^4} - \frac{3(N-1)^2}{(N-2)(N-3)}. \f]
            The above evaluates to 0 for a Gaussian distribution.
        */
        Real kurtosis() const;

        /*! returns the minimum sample value */
        Real min() const;

        /*! returns the maximum sample value */
        Real max() const;

        /*! Expectation value of a function \f$ f \f$ on a given
            range \f$ \mathcal{R} \f$, i.e.,
            \f[ \mathrm{E}\left[f \;|\; \mathcal{R}\right] =
                \frac{\sum_{x_i \in \mathcal{R}} f(x_i) w_i}{
                      \sum_{x_i \in \mathcal{R}} w_i}. \f]
            The range is passed as a boolean function returning
            <tt>true</tt> if the argument belongs to the range
            or <tt>false</tt> otherwise.

            The function returns a pair made of the result and
            the number of observations in the given range.
        */
        template <class Func, class Predicate>
        std::pair<Real,Size> expectationValue(const Func& f,
                                              const Predicate& inRange) const {
            Real num = 0.0, den = 0.0;
            Size N = 0;
            std::vector<std::pair<Real,Real> >::const_iterator i;
            for (i=samples_.begin(); i!=samples_.end(); ++i) {
                Real x = i->first, w = i->second;
                if (inRange(x)) {
                    num += f(x)*w;
                    den += w;
                    N += 1;
                }
            }
            if (N == 0)
                return std::make_pair<Real,Size>(Null<Real>(),0);
            else
                return std::make_pair(num/den,N);
        }

        /*! \f$ y \f$-th percentile, defined as the value \f$ \bar{x} \f$
            such that
            \f[ y = \frac{\sum_{x_i < \bar{x}} w_i}{
                          \sum_i w_i} \f]

            \pre \f$ y \f$ must be in the range \f$ (0-1]. \f$
        */
        Real percentile(Real y) const;

        /*! \f$ y \f$-th top percentile, defined as the value
            \f$ \bar{x} \f$ such that
            \f[ y = \frac{\sum_{x_i > \bar{x}} w_i}{
                          \sum_i w_i} \f]

            \pre \f$ y \f$ must be in the range \f$ (0-1]. \f$
        */
        Real topPercentile(Real y) const;
        //@}

        //! \name Modifiers
        //@{
        //! adds a datum to the set, possibly with a weight
        void add(Real value, Real weight = 1.0);
        //! adds a sequence of data to the set, with default weight
        template <class DataIterator>
        void addSequence(DataIterator begin, DataIterator end) {
            for (;begin!=end;++begin)
                add(*begin);
        }
        //! adds a sequence of data to the set, each with its weight
        template <class DataIterator, class WeightIterator>
        void addSequence(DataIterator begin, DataIterator end,
                         WeightIterator wbegin) {
            for (;begin!=end;++begin,++wbegin)
                add(*begin, *wbegin);
        }

        //! resets the data to a null set
        void reset();

        //! informs the internal storage of a planned increase in size
        void reserve(Size n) const;

        //! sort the data set in increasing order
        void sort() const;
        //@}
      private:
        mutable std::vector<std::pair<Real,Real> > samples_;
        mutable bool sorted_;
    };


    // inline definitions

    inline GeneralStatistics::GeneralStatistics() {
        reset();
    }

    inline Size GeneralStatistics::samples() const {
        return samples_.size();
    }

    inline const std::vector<std::pair<Real,Real> >&
    GeneralStatistics::data() const {
        return samples_;
    }

    inline Real GeneralStatistics::standardDeviation() const {
        return std::sqrt(variance());
    }

    inline Real GeneralStatistics::errorEstimate() const {
        return std::sqrt(variance()/samples());
    }

    inline Real GeneralStatistics::min() const {
        QL_REQUIRE(samples() > 0, "empty sample set");
        return std::min_element(samples_.begin(),
                                samples_.end())->first;
    }

    inline Real GeneralStatistics::max() const {
        QL_REQUIRE(samples() > 0, "empty sample set");
        return std::max_element(samples_.begin(),
                                samples_.end())->first;
    }

    /*! \pre weights must be positive or null */
    inline void GeneralStatistics::add(Real value, Real weight) {
        QL_REQUIRE(weight>=0.0, "negative weight not allowed");
        samples_.emplace_back(value, weight);
        sorted_ = false;
    }

    inline void GeneralStatistics::reset() {
        samples_ = std::vector<std::pair<Real,Real> >();
        sorted_ = true;
    }

    inline void GeneralStatistics::reserve(Size n) const {
        samples_.reserve(n);
    }

    inline void GeneralStatistics::sort() const {
        if (!sorted_) {
            std::sort(samples_.begin(), samples_.end());
            sorted_ = true;
        }
    }

}


#endif


#ifndef id_3b8249d1ce7156c659ab6261be8a1959
#define id_3b8249d1ce7156c659ab6261be8a1959
inline bool test_3b8249d1ce7156c659ab6261be8a1959(const int* i) {
    return i != nullptr;
}
#endif
