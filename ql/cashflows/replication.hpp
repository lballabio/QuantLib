/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2007 Cristina Duminuco

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

/*! \file replication.hpp
    \brief Sub, Central, or Super replication
*/

#ifndef quantlib_replication_hpp
#define quantlib_replication_hpp

#include <ql/types.hpp>
#include <iosfwd>

namespace QuantLib {

    //! Digital option replication strategy
    /*! Specification of replication strategies used to price
        the embedded digital option in a digital coupon.
    */
    struct Replication {
        enum Type { Sub, Central, Super };
    };

    /*! \relates Replication */
    std::ostream& operator<<(std::ostream&,
                             Replication::Type);

    class DigitalReplication {
    public:
        DigitalReplication(Replication::Type t = Replication::Central,
                           Real gap = 1e-4);
        Replication::Type replicationType() const { return replicationType_;};
        Real gap() const { return gap_;};
    private:
        Real gap_;
        Replication::Type replicationType_;
    };

}

#endif


#ifndef id_1d741ff0d652e93bebdf8c4c38c61d3f
#define id_1d741ff0d652e93bebdf8c4c38c61d3f
inline bool test_1d741ff0d652e93bebdf8c4c38c61d3f(const int* i) {
    return i != nullptr;
}
#endif
