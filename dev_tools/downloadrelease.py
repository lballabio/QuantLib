#! /usr/bin/env python

__doc__ = """download a QuantLib release"""

import urllib
import time

def printstar(a,b,c):
    # reporting download progression
    print '*',

if __name__=='__main__':
    print 'Hi - you are about to download a QuantLib release'
    releaseName = "0.2.1"
    fromUrl = "http://prdownloads.sourceforge.net/quantlib/"
    toUrl = "Y:/backed_up/QuantLibRelease/"
    url_names=[
               "QuantLib-"+releaseName+"-light-inst.exe",
               "QuantLib-"+releaseName+"-src.tar.bz2",
               "QuantLib-"+releaseName+"-src.tar.gz",
               "QuantLib-"+releaseName+"-src.zip",
               "QuantLib-"+releaseName+"-full-inst.exe",
               "QuantLib-"+releaseName+"-debug.zip",
               "QuantLib-Python-"+releaseName+".linux-i686.tar.bz2",
               "QuantLib-Python-"+releaseName+".linux-i686.tar.gz",
               "QuantLib-Python-"+releaseName+".tar.bz2",
               "QuantLib-Python-"+releaseName+".tar.gz",
               "QuantLib-Python-"+releaseName+".win32-py2.1.exe",
               "QuantLib-Python-"+releaseName+".zip",
               "QuantLib-"+releaseName+"-docs-html.tar.bz2",
               "QuantLib-"+releaseName+"-docs-html.tar.gz",
               "QuantLib-"+releaseName+"-docs-html.zip",
               "QuantLib-"+releaseName+"-docs-man.tar.bz2",
               "QuantLib-"+releaseName+"-docs-man.tar.gz",
               "QuantLib-"+releaseName+"-docs-refman.chm",
               "QuantLib-"+releaseName+"-docs-refman.chm.zip",
               "QuantLib-"+releaseName+"-docs-refman.dvi",
               "QuantLib-"+releaseName+"-docs-refman.dvi.bz2",
               "QuantLib-"+releaseName+"-docs-refman.dvi.gz",
               "QuantLib-"+releaseName+"-docs-refman.dvi.zip",
               "QuantLib-"+releaseName+"-docs-refman.pdf",
               "QuantLib-"+releaseName+"-docs-refman.pdf.bz2",
               "QuantLib-"+releaseName+"-docs-refman.pdf.gz",
               "QuantLib-"+releaseName+"-docs-refman.pdf.zip",
               "QuantLib-"+releaseName+"-docs-refman.ps",
               "QuantLib-"+releaseName+"-docs-refman.ps.bz2",
               "QuantLib-"+releaseName+"-docs-refman.ps.gz",
               "QuantLib-"+releaseName+"-docs-refman.ps.zip",
               "QuantLib-Ruby-"+releaseName+".i386-linux.tar.gz",
               "QuantLib-Ruby-"+releaseName+".tar.gz"
              ]
    fromUrl = "http://prdownloads.sourceforge.net/quantlib/"
    toUrl = "Y:/SFdownload/"
    for i in url_names:
        print '\ndownloading', i,
        urllib.urlretrieve(fromUrl+i, toUrl+i, printstar)

    print 'finished'
