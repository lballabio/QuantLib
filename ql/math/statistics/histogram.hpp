/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Gang Liang

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

/*! \file histogram.hpp
    \brief statistics tool for generating histogram of given data
*/

#ifndef quantlib_histogram_hpp
#define quantlib_histogram_hpp

#include <ql/utilities/null.hpp>
#include <vector>

namespace QuantLib {

    //! Histogram class
    /*! This class computes the histogram of a given data set.  The
        caller can specify the number of bins, the breaks, or the
        algorithm for determining these quantities in computing the
        histogram.
    */
    class Histogram {
      public:
        enum Algorithm { None, Sturges, FD, Scott };

        //! \name constructors
        //@{
        Histogram() : algorithm_(Algorithm(-1)) {}

        template <class T>
        Histogram(T data_begin, T data_end, Size breaks)
        : data_(data_begin,data_end), bins_(breaks+1),
          algorithm_(None) {
            calculate();
        }

        template <class T>
        Histogram(T data_begin, T data_end, Algorithm algorithm)
        : data_(data_begin,data_end), bins_(Null<Size>()),
          algorithm_(algorithm) {
            calculate();
        }

        template <class T, class U>
        Histogram(T data_begin, T data_end,
                  U breaks_begin, U breaks_end)
        : data_(data_begin,data_end), bins_(Null<Size>()),
          algorithm_(None), breaks_(breaks_begin,breaks_end) {
            bins_ = breaks_.size()+1;
            calculate();
        }
        //@}

        //! \name inspectors
        //@{
        Size bins() const;
        const std::vector<Real>& breaks() const;
        Algorithm algorithm() const;
        bool empty() const;
        //@}

        //! \name results
        //@{
        Size counts(Size i) const;
        Real frequency(Size i) const;
        //@}
      private:
        std::vector<Real> data_;
        Size bins_ = 0;
        Algorithm algorithm_;
        std::vector<Real> breaks_;
        std::vector<Size> counts_;
        std::vector<Real> frequency_;
        // update counts and frequencies
        void calculate();
    };

}

#endif


#ifndef id_ed8e81cbc4ff265a9d675cce594a722d
#define id_ed8e81cbc4ff265a9d675cce594a722d
inline bool test_ed8e81cbc4ff265a9d675cce594a722d(int* i) { return i != 0; }
#endif
