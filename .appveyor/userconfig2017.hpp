/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2011 Ferdinando Ametrano
 Copyright (C) 2004, 2005 StatPro Italia srl

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

#ifndef quantlib_config_hpp
#define quantlib_config_hpp

/***************************************************************
   User configuration section:
   modify the following definitions to suit your preferences.

   Do not modify this file if you are using a Linux/Unix system:
   it will not be read by the compiler. The definitions below
   will be provided by running ./configure instead.
****************************************************************/

/* Define this if error messages should include current function
   information. */
#ifndef QL_ERROR_FUNCTIONS
#   define QL_ERROR_FUNCTIONS
#endif

/* Define this if error messages should include file and line information. */
#ifndef QL_ERROR_LINES
#   define QL_ERROR_LINES
#endif

/* Define this if tracing messages should be allowed (whether they are
   actually emitted will depend on run-time settings.) */
#ifndef QL_ENABLE_TRACING
#   define QL_ENABLE_TRACING
#endif

/* Define this if extra safety checks should be performed. This can degrade
   performance. */
#ifndef QL_EXTRA_SAFETY_CHECKS
#   define QL_EXTRA_SAFETY_CHECKS
#endif

/* Define this to use indexed coupons instead of par coupons in floating
   legs as the default in 'static bool IborCoupon::usingAtParCoupons();'. */
#ifndef QL_USE_INDEXED_COUPON
#   define QL_USE_INDEXED_COUPON
#endif

/* Define this to have singletons return different instances for
   different sessions. You will have to provide and link with the
   library a sessionId() function in namespace QuantLib, returning a
   different session id for each session.*/
#ifndef QL_ENABLE_SESSIONS
#   define QL_ENABLE_SESSIONS
#endif

/* Define this to enable the thread-safe observer pattern. You should
   enable it if you want to use QuantLib via the SWIG layer within
   the JVM or .NET eco system or any environment with an
   async garbage collector */
#ifndef QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
#    define QL_ENABLE_THREAD_SAFE_OBSERVER_PATTERN
#endif

/* Define this to enable a date resolution down to microseconds and
   allow for accurate intraday pricing.*/
#ifndef QL_HIGH_RESOLUTION_DATE
#    define QL_HIGH_RESOLUTION_DATE
#endif

/* Define this to use standard smart pointers instead of Boost ones.
   Note that std::shared_ptr does not check access and can
   cause segmentation faults. */
#ifndef QL_USE_STD_SHARED_PTR
#    define QL_USE_STD_SHARED_PTR
#endif

/* Undefine this to use std::auto_ptr instead of std::unique_ptr. */
#ifndef QL_USE_STD_UNIQUE_PTR
#    define QL_USE_STD_UNIQUE_PTR
#endif

/* Define this to use std::function and std::bind instead of
   boost::function and boost::bind. */
#ifndef QL_USE_STD_FUNCTION
#    define QL_USE_STD_FUNCTION
#endif

/* Define this to use std::tuple instead of boost::tuple. */
#ifndef QL_USE_STD_TUPLE
#    define QL_USE_STD_TUPLE
#endif

/* Define this if you want to use the Disposable class template.
   This should be no longer necessary in C++11
   and might interfere with compiler optimizations. */
#ifndef QL_USE_DISPOSABLE
#    define QL_USE_DISPOSABLE
#endif

/* Define this to enable the parallel unit test runner */
#ifndef QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
//#    define QL_ENABLE_PARALLEL_UNIT_TEST_RUNNER
#endif

/* Define this to make Singleton initialization thread-safe.
   Note: There is no support for thread safety and multiple sessions.
*/
#ifndef QL_ENABLE_SINGLETON_THREAD_SAFE_INIT
//#   define QL_ENABLE_SINGLETON_THREAD_SAFE_INIT
#endif

#endif
