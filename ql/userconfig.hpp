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
//#   define QL_ERROR_FUNCTIONS
#endif

/* Define this if error messages should include file and line information. */
#ifndef QL_ERROR_LINES
//#   define QL_ERROR_LINES
#endif

/* Define this if tracing messages should be allowed (whether they are
   actually emitted will depend on run-time settings.) */
#ifndef QL_ENABLE_TRACING
//#   define QL_ENABLE_TRACING
#endif

/* Define this if negative rates should be allowed. */
#ifndef QL_NEGATIVE_RATES
#   define QL_NEGATIVE_RATES
#endif

/* Define this if extra safety checks should be performed. This can degrade
   performance. */
#ifndef QL_EXTRA_SAFETY_CHECKS
//#   define QL_EXTRA_SAFETY_CHECKS
#endif

/* Define this to use indexed coupons instead of par coupons in floating
   legs. */
#ifndef QL_USE_INDEXED_COUPON
//#   define QL_USE_INDEXED_COUPON
#endif

/* Define this to have singletons return different instances for
   different sessions. You will have to provide and link with the
   library a sessionId() function in namespace QuantLib, returning a
   different session id for each session.*/
#ifndef QL_ENABLE_SESSIONS
//#   define QL_ENABLE_SESSIONS
#endif

#endif
