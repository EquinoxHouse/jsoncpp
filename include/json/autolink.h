// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#ifndef JSONCPP_AUTOLINK_H_INCLUDED
#define JSONCPP_AUTOLINK_H_INCLUDED

#include "config.h"

#ifdef JSONCPP_IN_CPPTL
#include <cpptl/cpptl_autolink.h>
#endif

#if !defined(JSONCPP_NO_AUTOLINK) && !defined(JSONCPP_DLL_BUILD) && !defined(JSONCPP_IN_CPPTL)
#define CPPTL_AUTOLINK_NAME "json"
#undef CPPTL_AUTOLINK_DLL
#ifdef JSONCPP_DLL
#define CPPTL_AUTOLINK_DLL
#endif
#include "autolink.h"
#endif

#endif // JSONCPP_AUTOLINK_H_INCLUDED
