/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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

/*! \file singleton.cpp
    \brief basic support for the singleton pattern
*/

#include <ql/patterns/singleton.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

#if defined(QL_ENABLE_SESSIONS)
ThreadKey defaultSessionId() {
    return {};
}

SessionIdFunction session_id_function = defaultSessionId;

ThreadKey sessionId() {
    return (*session_id_function)();
}

void setSessionIdFunction(SessionIdFunction sid_function) {
    QL_ASSERT(sid_function,
        "setSessionIdFunction was called with a null function");
    session_id_function = sid_function;
}
#endif

}
