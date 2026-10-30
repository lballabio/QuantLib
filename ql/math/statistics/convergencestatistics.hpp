/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Gary Kennedy
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file convergencestatistics.hpp
    \brief statistics tool with risk measures
*/

#ifndef quantlib_convergence_statistics_hpp
#define quantlib_convergence_statistics_hpp

#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    class DoublingConvergenceSteps {
      public:
        Size initialSamples() const { return 1; }
        Size nextSamples(Size current) { return 2 * current + 1; }
    };

    //! statistics class with convergence table
    /*! This class decorates another statistics class adding a
        convergence table calculation. The table tracks the
        convergence of the mean.

        It is possible to specify the number of samples at which the
        mean should be stored by mean of the second template
        parameter; the default is to store \f$ 2^{n-1} \f$ samples at
        the \f$ n \f$-th step. Any passed class must implement the
        following interface:
        \code
        Size initialSamples() const;
        Size nextSamples(Size currentSamples) const;
        \endcode
        as well as a copy constructor.

        \test results are tested against known good values.
    */
    template <class T, class U = DoublingConvergenceSteps>
    class ConvergenceStatistics : public T {
      public:
        typedef typename T::value_type value_type;
        typedef std::vector<std::pair<Size,value_type> > table_type;
        ConvergenceStatistics(const T& stats,
                              const U& rule = U());
        ConvergenceStatistics(const U& rule = U());
        void add(const value_type& value, Real weight = 1.0);
        template <class DataIterator>
        void addSequence(DataIterator begin, DataIterator end) {
            for (; begin != end; ++begin)
                add(*begin);
        }
        template <class DataIterator, class WeightIterator>
        void addSequence(DataIterator begin, DataIterator end,
                         WeightIterator wbegin) {
            for (; begin != end; ++begin, ++wbegin)
                add(*begin,*wbegin);
        }
        void reset();
        const std::vector<std::pair<Size,value_type> >& convergenceTable()
                                                                        const;
      private:
        table_type table_;
        U samplingRule_;
        Size nextSampleSize_;
    };


    // inline definitions

    template <class T, class U>
    ConvergenceStatistics<T,U>::ConvergenceStatistics(const T& stats,
                                                      const U& rule)
    : T(stats), samplingRule_(rule) {
        reset();
    }

    template <class T, class U>
    ConvergenceStatistics<T,U>::ConvergenceStatistics(const U& rule)
    : samplingRule_(rule) {
        reset();
    }

    #ifndef __DOXYGEN__
    template <class T, class U>
    void ConvergenceStatistics<T,U>::add(
                 const typename ConvergenceStatistics<T,U>::value_type& value,
                 Real weight) {
        T::add(value,weight);
        if (this->samples() == nextSampleSize_) {
            table_.push_back(std::make_pair(this->samples(),this->mean()));
            nextSampleSize_ = samplingRule_.nextSamples(nextSampleSize_);
        }
    }
    #endif

    template <class T, class U>
    void ConvergenceStatistics<T,U>::reset() {
        T::reset();
        nextSampleSize_ = samplingRule_.initialSamples();
        table_.clear();
    }

    template <class T, class U>
    const typename ConvergenceStatistics<T,U>::table_type&
    ConvergenceStatistics<T,U>::convergenceTable() const {
        return table_;
    }

}


#endif

