#!/usr/bin/env python3
# Copyright (c) 2017 The MagnaChain Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.
'''
提供给GDB的python脚本。
当with_gdb=True时，gdb会运行这个脚本的指令，方便catch crash
'''
import atexit
import gdb
import os
import tempfile
import time

def stop_handler(event):
    print('catch event:',event)
    if isinstance(event, gdb.BreakpointEvent):
        # handle BreakpointEvent
        pass
    else:
	    # handle SignalEvent
	    gdb.execute('bt')
	    gdb.execute('kill')
	    gdb.execute('q')

def exit_handler(event):
    print("handle ExitEvent")
    gdb.execute('kill')

@atexit.register
def on_exit():
    print("gdb exit~~~~~~~~")

gdb.events.stop.connect(stop_handler)
gdb.events.exited.connect(exit_handler)
print("continue",time.time())
gdb.execute('c')
