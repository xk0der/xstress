#!/bin/sh

export CVS_RSH=ssh
cvs -z3 -d:ext:xk0der@xstress.cvs.sourceforge.net:/cvsroot/xstress $*
