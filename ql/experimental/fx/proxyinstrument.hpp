/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file proxyengine.hpp
    \brief base class for instruments allowing for proxy engines
*/

#ifndef quantlib_pricingengines_proxy_hpp
#define quantlib_pricingengines_proxy_hpp

namespace QuantLib {

class ProxyInstrument {
  public:
    //! Base class proxy descriptions for approximate pricing engines
    struct ProxyDescription {
        // check if proxy description is valid
        virtual void validate() const = 0;
    };

    virtual boost::shared_ptr<ProxyDescription> proxy() const = 0;
};

} // namespace QuantLib

#endif
