"""
/*
 * Copyright (C) 2001 QuantLib Group
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
 *
 * QuantLib license is also available at:
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/
"""
"""
    \file importlibrary.py
    \brief Export modules as COM objects
    
    $Source$
    $Log$
    Revision 1.2  2001/03/09 12:51:09  marmar
    Now python can handle COM call to _value_

    Revision 1.1  2001/03/06 13:52:35  marmar
    Export the QuantLib functionalities to the COM world

"""    

"""
Example:
    ' The following is an example of VB code that creates the
    ' QuantLib module and uses it

    ' The object ImportLibrary must be initialized before any python 
    ' module can be created. Tip: define ImportLibrary as Public
    Public ImportLibrary As Object
    ImportLibrary = CreateObject("Python.ImportLibrary")
    
    ' Import the python module QuantLib and registers it
    Dim QuantLib As Object
    Set QuantLib = ImportLibrary.QuantLib
    
    ' Create a QuantLib object
    Dim uniformDeviate As Object
    Set uniformDeviate = QuantLib.UniformRandomGenerator()
    MsgBox uniformDeviate.next()

COM Exceptions raised:
    #5, ERROR_ACCESS_DENIED if import is called with
                    DISPATCH_PROPERTYGET or DISPATCH_METHOD
    #2, ERROR_FILE_NOT_FOUND if import of required python module failed
    #12, ERROR_INVALID_ACCESS if problems arise in applying method to object
All the other errors are not transformed into COM exception but presented as 
Unknown python errors.
"""

import pywintypes, string, types, winerror, pythoncom
import win32com.client, win32com.server.util, win32com.server.policy
from win32com.server.dispatcher import DefaultDebugDispatcher
from win32com.server.exception import COMException
from win32com.server.policy import BasicWrapPolicy
from win32com.server.policy import DynamicPolicy

PyIDispatchType = pythoncom.TypeIIDs[pythoncom.IID_IDispatch]
useDebug = 0 # Note that this will remain hard-coded in the object

def FixArguments(args):
    """Unicode objects are converted to strings, and PyIDispatch objects 
    are unwrapped. Does this recursively, to ensure sub-lists (ie, arrays) 
    are also converted
    """
    newArgs = []
    for arg in args:
        if type(arg) is types.TupleType:
            arg = FixArguments(arg)
        elif type(arg) is pywintypes.UnicodeType:
            arg = str(arg)
        elif type(arg) is PyIDispatchType:
            arg = win32com.server.util.unwrap(arg)
        newArgs.append(arg)
    return tuple(newArgs)

def FixVisualBasicName(name):
    """Unfortunately, there are a number of names that are difficult
    to type from VB.  E.g. if you type 'value' it will be converted into
    'Value', 'next' into 'Next' and so on.  This function tries to fix 
    this behaviour.
    """
    firstLetter = string.lower(name[0])
    lastPartOfName = name[1:]
    return firstLetter + lastPartOfName

def PrepareForReturn(object):
    objectType = type(object)
    if (objectType is types.InstanceType or
        objectType is types.ModuleType):
        if useDebug == 1:
            ob = DefaultDebugDispatcher(QuitePermissivePolicy, object)
        else:
            ob = QuitePermissivePolicy(object)
        wrapped = pythoncom.WrapObject(ob)
        return wrapped
    else:
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
    _reg_clsctx_ = pythoncom.CLSCTX_INPROC_SERVER
     
    def _wrap_(self, object):
        BasicWrapPolicy._wrap_(self, object)
        self._next_dynamic_ = self._min_dynamic_ = 1000
        self._dyn_dispid_to_name_ = {pythoncom.DISPID_VALUE:'_value_',
                                     pythoncom.DISPID_NEWENUM:'_NewEnum' }
     
    def _invokeex_(self, dispid, lcid, wFlags, args, kwargs, serviceProvider):
        try:
          name = str(self._dyn_dispid_to_name_[dispid])
        except KeyError:
          raise COMException(scode = winerror.DISP_E_MEMBERNOTFOUND,
                             desc="Member not found")
         
        args = FixArguments(args)
        if name not in dir(self._obj_):
            # VB sometimes screws up names of methods
            name = FixVisualBasicName(name)

#        print '---> _invokeex_ ---> self._obj_', self._obj_
#        print '---> _invokeex_ ---> name', name
#        print '---> _invokeex_ ---> args', args
            
        if wFlags & pythoncom.DISPATCH_METHOD:
            if name == '_value_':
                if hasattr(self._obj_, '__call__'):
                    return PrepareForReturn(apply(self._obj_,args))                
                elif len(args) == 1: 
                    return PrepareForReturn(self._obj_[args[0]])
                elif len(args) == 2:
                    return PrepareForReturn(self._obj_[args[0]][args[1]])
                elif len(args) == 3:
                    return PrepareForReturn(self._obj_[args[0]][args[1]][args[2]])
               
            try:
                qlMethod = getattr(self._obj_, name)
            except AttributeError:
                raise COMException(
                    "Attribute '%s' not found for object '%s'" % (
                    name, self._obj_), winerror.ERROR_INVALID_ACCESS)
            initializedObject = apply(qlMethod, args)
            return PrepareForReturn(initializedObject)
                    
        if wFlags & pythoncom.DISPATCH_PROPERTYGET:
            initializedObject = self._obj_.__dict__[name]
            return PrepareForReturn(initializedObject)
            
        if wFlags & (pythoncom.DISPATCH_PROPERTYPUT |
                     pythoncom.DISPATCH_PROPERTYPUTREF):
            if len(args) == 1:
                setattr(self._obj_, name, args)
            elif len(args) == 2:
                self._obj_[args[0]] = args[1]
            elif len(args) == 3:
                self._obj_[args[0]][args[1]] = args[2]
            elif len(args) == 4:
                self._obj_[args[0]][args[1]][args[2]] = args[3]
            else:
                raise COMException(
                   "DISPATCH_PROPERTYPUT called with too many arguments",
                   winerror.E_INVALIDARG)
        return
            
        raise COMException(scode=winerror.E_INVALIDARG, desc="invalid wFlags")


class ComImportLibrary:
    """ Creates a COM wrapper for any python module.
    """
    _reg_clsid_ = "{13BD06C1-123F-11D5-83CD-0050DA367EDA}"
    _reg_progid_ = "Python.ImportLibrary"
    _reg_policy_spec_ = "DynamicPolicy"
    _reg_clsctx_ = pythoncom.CLSCTX_INPROC_SERVER
    def _dynamic_(self, name, lcid, wFlags, args):
        name = str(name)
        args = FixArguments(args)
        if (not (wFlags & (pythoncom.DISPATCH_PROPERTYGET |
                           pythoncom.DISPATCH_METHOD)) ):
            raise COMException("Operation not valid on a python module",
                               winerror.ERROR_ACCESS_DENIED)
        try :
            exec ('import ' + name)
            exec ('factory = PrepareForReturn(%s)' % (name,))
#            print "Exporting python module %s, as %s" % (name, factory)
            return factory 
        except:
            raise COMException("Python module '%s' not found" % (name,),
                               winerror.ERROR_FILE_NOT_FOUND)

if __name__ == '__main__':
    import win32com.server.register
    win32com.server.register.UseCommandLine(ComImportLibrary)
