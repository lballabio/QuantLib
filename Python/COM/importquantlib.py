"""
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

"""
"""
    \file importquantlib.py
    \brief Export modules as COM objects
    
    $Source$
    $Log$
    Revision 1.1  2001/05/15 13:33:58  marmar
    Filename importlibrary.py changed to importquantlib.py

"""

import QuantLib
import pywintypes, string, types
from win32com.server.dispatcher import DefaultDebugDispatcher
from win32com.server.policy import BasicWrapPolicy
from win32com.server.policy import DynamicPolicy
from pythoncom import *

def FixArguments(args):
    """Unicode objects are converted to strings, PyIDispatch objects and
    collections are unwrapped.
    """
    newArgs = []
    for arg in args:
        if type(arg) is types.TupleType:
            arg = FixArguments(arg)
        elif type(arg) is pywintypes.UnicodeType:
            arg = str(arg)
        newArgs.append(arg)
    return tuple(newArgs)

def FixVisualBasicName(name):
    """Unfortunately, there are a number of names that are difficult
    to type from VB.  E.g. if you type 'value' it will be converted into
    'Value', 'next' into 'Next' and so on.  This function tries to fix 
    this behaviour.
    """
    return string.lower(name[0]) + name[1:]

def PrepareForReturn(object):
    objectType = type(object)
    print "----> Returning", object, objectType
    if(objectType is types.InstanceType):
        print "Returning wrapped object"
        return WrapObject(DefaultDebugDispatcher(QuitePermissivePolicy, object))
    else:
        print "Returning simple object"
        return object


class QuitePermissivePolicy(DynamicPolicy):
    """A policy specialized to work with python objects.

    Note: here and there VB calls a COM object with flag DISPATCH_METHOD &
    DISPATCH_PROPERTYGET. in order to speed up the call to the python side
    this program interpreters this as a call with flag DISPATCH_METHOD. 
    Therefore, sometimes calls using DISPATCH_PROPERTYGET end up raising an
    unexpected python error.  The advise is anyway to wrap any attribute call
    in a getAttribute method.
    """
    def _wrap_(self, object):
        BasicWrapPolicy._wrap_(self, object)
        self._next_dynamic_ = self._min_dynamic_ = 1000
        self._dyn_dispid_to_name_ = {DISPID_VALUE:'_value_',
                                     DISPID_NEWENUM:'_NewEnum' }
     
    def _invokeex_(self, dispid, lcid, wFlags, args, kwargs, serviceProvider):
        name = str(self._dyn_dispid_to_name_[dispid])
                 
        args = FixArguments(args)
        if not hasattr(self._obj_, name):
            name = FixVisualBasicName(name)
        # Printing debug info
        print "self._obj_:",self._obj_
        print "name:",name
        print "args:",args
        if wFlags & DISPATCH_METHOD:
            #Next we evaluate calls of the type "PyCOM_object.name(args)"
            qlMethod = getattr(self._obj_, name)
            initializedObject = apply(qlMethod, args)
            return PrepareForReturn(initializedObject)
         
        if wFlags & DISPATCH_PROPERTYGET:
            # Here we handle the calls of the type "VB_value = PyCOM_object"
            # in the rare case in which VB uses DISPATCH_PROPERTYGET
            # sometimes these are dispatched as DISPATCH_METHOD in which
            # case an error message will be raised
            initializedObject = self._obj_.__dict__[name]
            return PrepareForReturn(initializedObject)
            
        if wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF):
            # Calls of the type "PyCOM_object = VB_value" are 
            setattr(self._obj_, name, args)

class ComImportLibrary:
    """
    Example of usage from Visual
        Set QuantLib = CreateObject("QuantLib.ImportQuantLib").QuantLib
        Set rng = QuantLib.UniformRandomGenerator
        MsgBox rng.Next
    """
    _reg_clsid_ = "{4B932900-493E-11D5-803B-00D059087C41}"
    _reg_progid_ = "QuantLib.ImportQuantLib"
    _public_methods_ = ['QuantLib']
    def QuantLib(self):
        print "QuantLib called"
        return WrapObject(DefaultDebugDispatcher(QuitePermissivePolicy, QuantLib))

if __name__ == '__main__':
    import win32com.server.register
    win32com.server.register.UseCommandLine(ComImportLibrary)
    
