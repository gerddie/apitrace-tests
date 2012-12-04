#!/usr/bin/env python
##########################################################################
#
# Copyright 2011 Jose Fonseca
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/

'''Application test driver.'''


import os.path
import platform
import re
import subprocess
import sys
import time
import json
import base64

try:
    from cStringIO import StringIO
except ImportError:
    from StringIO import StringIO


import tracematch
from base_driver import *


class SrcTraceParser(tracematch.SrcTraceParser):

    def __init__(self, stream):
        tracematch.SrcTraceParser.__init__(self, stream)
        self.swapbuffers = 0

    def handleCall(self, callNo, functionName, args, ret):
        tracematch.SrcTraceParser.handleCall(self, callNo, functionName, args, ret)

        if functionName.find('SwapBuffers') != -1 or \
           repr(args).find('kCGLPFADoubleBuffer') != -1:
            self.swapbuffers += 1


class AppDriver(Driver):

    cmd = None
    cwd = None

    api = 'gl'
    max_frames = None
    trace_file = None

    ref_dump = None

    doubleBuffer = True

    verbose = False

    threshold_precision = 12.0

    def __init__(self):
        Driver.__init__(self)
        self.stateCache = {}
    
    def runApp(self):
        '''Run the application standalone, skipping this test if it fails by
        some reason.'''

        if not self.cmd:
            return

        sys.stderr.write('Run application...\n')

        p = popen(self.cmd, cwd=self.cwd)
        p.wait()
        if p.returncode != 0:
            skip('application returned code %i' % p.returncode)

        sys.stdout.flush()
        sys.stderr.write('\n')

    api_trace_map = {
        'gl': 'gl',
        'egl_gl': 'egl',
        'egl_gles1': 'egl',
        'egl_gles2': 'egl',
        'd3d7': 'd3d7',
        'd3d8': 'd3d8',
        'd3d9': 'd3d9',
        'd3d10': 'd3d10',
        'd3d10_1': 'd3d10_1',
        'd3d11': 'd3d11',
        'd3d11_1': 'd3d11',
    }

    api_retrace_map = {
        'gl': 'glretrace',
        'egl_gl': 'eglretrace',
        'egl_gles1': 'eglretrace',
        'egl_gles2': 'eglretrace',
        'd3d8': 'd3dretrace',
        'd3d9': 'd3dretrace',
        'd3d10': 'd3dretrace',
        'd3d10_1': 'd3dretrace',
        'd3d11': 'd3dretrace',
        'd3d11_1': 'd3dretrace',
    }

    def traceApp(self):
        if not self.cmd:
            return

        sys.stderr.write('Capturing trace...\n')

        if self.trace_file is None:
            if self.ref_dump is not None:
                name = self.ref_dump
            else:
                name = self.cmd[0]
            name, ext = os.path.splitext(os.path.basename(name))
            while ext:
                name, ext = os.path.splitext(os.path.basename(name))
            self.trace_file = os.path.abspath(os.path.join(self.results, name + '.trace'))
        if os.path.exists(self.trace_file):
            os.remove(self.trace_file)
        else:
            trace_dir = os.path.dirname(self.trace_file)
            if not os.path.exists(trace_dir):
                os.makedirs(trace_dir)

        cmd = self.cmd
        env = os.environ.copy()
        
        cmd = [
            options.apitrace, 'trace', 
            '-v',
            '-a', self.api_trace_map[self.api],
            '-o', self.trace_file,
            '--'
        ] + cmd
        if self.max_frames is not None:
            env['TRACE_FRAMES'] = str(self.max_frames)

        p = popen(cmd, env=env, cwd=self.cwd)
        p.wait()
        if p.returncode != 0:
            fail('`apitrace trace` returned code %i' % p.returncode)

        if not os.path.exists(self.trace_file):
            fail('no trace file generated\n')

        sys.stdout.flush()
        sys.stderr.write('\n')
    
    def checkTrace(self):
        sys.stderr.write('Comparing trace %s against %s...\n' % (self.trace_file, self.ref_dump))

        cmd = [options.apitrace, 'dump', '--verbose', '--color=never', self.trace_file]
        p = popen(cmd, stdout=subprocess.PIPE, universal_newlines=True)

        srcParser = SrcTraceParser(p.stdout)
        srcTrace = srcParser.parse()
        self.doubleBuffer = srcParser.swapbuffers > 0

        images = []
        states = []

        if self.ref_dump:
            refParser = tracematch.RefTraceParser(self.ref_dump)
            refTrace = refParser.parse()

            try:
                mo = refTrace.match(srcTrace)
            except tracematch.TraceMismatch, ex:
                fail(str(ex))

            dirName, baseName = os.path.split(os.path.abspath(self.ref_dump))
            prefix, _ = os.path.splitext(baseName)
            prefix += '.'
            fileNames = os.listdir(dirName)
            for fileName in fileNames:
                if fileName.startswith(prefix) and fileName != self.ref_dump:
                    rest = fileName[len(prefix):]
                    paramName, ext = os.path.splitext(rest)
                    if ext in ('.json', '.png'):
                        if paramName.isdigit():
                            callNo = int(paramName)
                        else:
                            try:
                                callNo = mo.params[paramName]
                            except KeyError:
                                fail('could not find parameter %s for %s' % (paramName, fileName))
                        filePath = os.path.join(dirName, fileName)
                        if ext == '.png':
                            images.append((callNo, filePath))
                        if ext == '.json':
                            states.append((callNo, filePath))
        p.wait()
        if p.returncode != 0:
            fail('`apitrace dump` returned code %i' % p.returncode)

        sys.stdout.flush()
        sys.stderr.write('\n')

        if self.api not in self.api_retrace_map:
            return

        for callNo, refImageFileName in images:
            self.checkImage(callNo, refImageFileName)
        for callNo, refStateFileName in states:
            self.checkState(callNo, refStateFileName)

    def checkImage(self, callNo, refImageFileName):
        sys.stderr.write('Comparing snapshot from call %u against %s...\n' % (callNo, refImageFileName))
        try:
            from PIL import Image
        except ImportError:
            sys.stderr.write('warning: PIL not found, skipping image comparison\n');
            return

        srcImage = self.getImage(callNo)
        refImage = Image.open(refImageFileName)

        from snapdiff import Comparer
        comparer = Comparer(refImage, srcImage)
        precision = comparer.precision(filter=True)
        sys.stdout.write('precision of %f bits against %s\n' % (precision, refImageFileName))
        if precision < self.threshold_precision:
            prefix = '%s.%u' % (self.getNamePrefix(), callNo)
            srcImageFileName = prefix + '.src.png'
            srcImage.save(srcImageFileName)
            diffImageFileName = prefix + '.diff.png'
            comparer.write_diff(diffImageFileName)
            fail('snapshot from call %u does not match %s' % (callNo, refImageFileName))

        sys.stdout.flush()
        sys.stderr.write('\n')

    def checkState(self, callNo, refStateFileName):
        sys.stderr.write('Comparing state dump from call %u against %s...\n' % (callNo, refStateFileName))

        srcState = self.getState(callNo)
        refState = self.getRefState(refStateFileName)

        from jsondiff import Comparer, Differ
        comparer = Comparer(ignore_added = True)
        match = comparer.visit(refState, srcState)
        if not match:
            prefix = '%s.%u' % (self.getNamePrefix(), callNo)
            srcStateFileName = prefix + '.src.json'
            diffStateFileName = prefix + '.diff.json'
            self.saveState(srcState, srcStateFileName)
            #diffStateFile = open(diffStateFileName, 'wt')
            diffStateFile = sys.stdout
            differ = Differ(diffStateFile, ignore_added = True)
            differ.visit(refState, srcState)
            fail('state from call %u does not match %s' % (callNo, refStateFileName))

        sys.stdout.flush()
        sys.stderr.write('\n')

    def getRefState(self, refStateFileName):
        stream = open(refStateFileName, 'rt')
        from jsondiff import load
        state = load(stream)
        self.adjustRefState(state)
        return state

    def getNamePrefix(self):
        name = os.path.basename(self.ref_dump)
        try:
            index = name.index('.')
        except ValueError:
            pass
        else:
            name = name[:index]
        return name

    def saveState(self, state, filename):
        s = json.dumps(state, sort_keys=True, indent=2)
        open(filename, 'wt').write(s)

    def retrace(self):
        if self.api not in self.api_retrace_map:
            return

        sys.stderr.write('Retracing %s...\n' % (self.trace_file,))

        p = self._retrace()
        p.wait()
        if p.returncode != 0:
            fail('retrace failed with code %i' % (p.returncode))

        sys.stdout.flush()
        sys.stderr.write('\n')

    def getImage(self, callNo):
        from PIL import Image
        state = self.getState(callNo)
        if self.doubleBuffer:
            attachments = ['GL_BACK', 'GL_BACK_LEFT', 'GL_BACK_RIGHT', 'GL_COLOR_ATTACHMENT0']
        else:
            attachments = ['GL_FRONT', 'GL_FRONT_LEFT', 'GL_FRONT_RIGHT', 'GL_COLOR_ATTACHMENT0']
        imageObj = self.getFramebufferAttachment(state, attachments)
        data = imageObj['__data__']
        stream = StringIO(base64.b64decode(data))
        im = Image.open(stream)
        return im

    def getFramebufferAttachment(self, state, attachments):
        framebufferObj = state['framebuffer']
        for attachment in attachments:
            try:
                attachmentObj = framebufferObj[attachment]
            except KeyError:
                pass
            else:
                return attachmentObj
        raise Exception("no attachment found")

    def getState(self, callNo):
        try:
            state = self.stateCache[callNo]
        except KeyError:
            pass
        else:
            return state

        p = self._retrace(['-D', str(callNo)])
        state = json.load(p.stdout, strict=False)
        p.wait()
        if p.returncode != 0:
            fail('retrace returned code %i' % (p.returncode))

        self.adjustSrcState(state)

        self.stateCache[callNo] = state

        return state

    def adjustSrcState(self, state):
        # Do some adjustments on the obtained state to eliminate failures from
        # bugs/issues outside of apitrace

        try:
            parameters = state['parameters']
        except KeyError:
            return

        # On NVIDIA drivers glGetIntegerv(GL_INDEX_WRITEMASK) returns 255
        self.replaceState(parameters, 'GL_INDEX_WRITEMASK', 255, -1)

    def adjustRefState(self, state):
        # Do some adjustments on reference state to eliminate failures from
        # bugs/issues outside of apitrace

        try:
            parameters = state['parameters']
        except KeyError:
            return

        if platform.system() == 'Darwin':
            # Mac OS X drivers fail on GL_COLOR_SUM
            # XXX: investigate this
            self.removeState(parameters, 'GL_COLOR_SUM')

    def replaceState(self, obj, key, srcValue, dstValue):
        try:
            value = obj[key]
        except KeyError:
            pass
        else:
            if value == srcValue:
                obj[key] = dstValue

    def removeState(self, obj, key):
        try:
            del obj[key]
        except KeyError:
            pass

    def _retrace(self, args = None, stdout=subprocess.PIPE):
        retrace = self.api_retrace_map[self.api]
        #cmd = [get_build_program(retrace)]
        cmd = [options.apitrace, 'retrace']
        if self.doubleBuffer:
            cmd += ['-db']
        else:
            cmd += ['-sb']
        if args:
            cmd += args
        cmd += [self.trace_file]
        return popen(cmd, stdout=stdout)

    def createOptParser(self):
        optparser = Driver.createOptParser(self)

        optparser.add_option(
            '-a', '--api', metavar='API',
            type='string', dest='api', default='gl',
            help='api to trace')
        optparser.add_option(
            '-R', '--results', metavar='PATH',
            type='string', dest='results', default='.',
            help='results directory [default=%default]')
        optparser.add_option(
            '--ref-dump', metavar='PATH',
            type='string', dest='ref_dump', default=None,
            help='reference dump')

        return optparser

    def run(self):
        global options

        (options, args) = self.parseOptions()

        if not os.path.exists(options.results):
            os.makedirs(options.results)

        self.verbose = options.verbose

        self.cmd = args
        self.cwd = options.cwd
        self.api = options.api
        self.ref_dump = options.ref_dump
        self.results = options.results

        self.runApp()
        self.traceApp()
        self.checkTrace()
        self.retrace()

        pass_()

if __name__ == '__main__':
    AppDriver().run()
