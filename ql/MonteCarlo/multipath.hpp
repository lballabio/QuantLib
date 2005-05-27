/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000-2005 StatPro Italia srl

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

/*! \file multipath.hpp
    \brief Correlated multiple asset paths
*/

#ifndef quantlib_montecarlo_multi_path_hpp
#define quantlib_montecarlo_multi_path_hpp

#include <ql/MonteCarlo/path.hpp>

namespace QuantLib {

    //! Correlated multiple asset paths
    /*! MultiPath contains the list of paths for each asset, i.e.,
        multipath[j] is the path followed by the j-th asset.

        \ingroup mcarlo
    */
    class MultiPath {
      public:
        MultiPath(Size nAsset,
                  const TimeGrid& timeGrid);
        MultiPath(const std::vector<Path>& multiPath);
        //! \name inspectors
        //@{
        Size assetNumber() const { return multiPath_.size(); }
        Size pathSize() const { return multiPath_[0].length(); }
        //@}
        //! \name read/write access to components
        //@{
        const Path& operator[](Size j) const { return multiPath_[j]; }
        Path& operator[](Size j) { return multiPath_[j]; }
        //@}
      private:
        std::vector<Path> multiPath_;
    };


    // inline definitions

    inline MultiPath::MultiPath(Size nAsset, const TimeGrid& timeGrid)
    : multiPath_(nAsset,Path(timeGrid)) {
        QL_REQUIRE(nAsset > 0, "number of asset must be positive");
    }

    inline MultiPath::MultiPath(const std::vector<Path>& multiPath)
    : multiPath_(multiPath) {}

}


#endif
