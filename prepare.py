#!/usr/bin/env python
# encoding: utf-8

'''
@author:     Damian Abalo Miron <damian.abalo@cern.ch>
@copyright:  2015 CERN
@license:
Copyright (c) 2015, CERN, Universidad de Oviedo.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@contact:    damian.abalo@cern.ch
'''

import os
import stat
import sys
import shutil
import platform
import subprocess
	
baseDir = os.getcwd()
open6Dir = os.path.join(baseDir, 'open62541')
open6BuildDir = os.path.join(open6Dir, 'build')
CMD_generateProjForVisStu12 = 'cmake -DUA_ENABLE_AMALGAMATION=ON {0} -G "Visual Studio 12 Win64"'.format(open6Dir)
CMD_loadVisStu12Env = '"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat" amd64'
CMD_buildProjWithVisStu12 = 'msbuild {0} /clp:ErrorsOnly /property:Platform=x64;Configuration=Release'.format(os.path.join(open6Dir, 'ALL_BUILD.vcxproj'))

def remove_readonly(func, path, _):
	print('clearing read only flag for directory [{0}]'.format(path))
	os.chmod(path, stat.S_IWRITE)
	func(path)

def main():
	if(os.path.exists(open6Dir)):
		print('deleting existing open6 directory [{0}]'.format(open6Dir))
		shutil.rmtree(open6Dir, onerror=remove_readonly)
		print('does directory exist now? [{0}]'.format(os.path.exists(open6Dir)))

	print('git clone https://github.com/open62541/open62541.git')
	subprocess.check_call(['git', 'clone', 'https://github.com/open62541/open62541.git'])
	
	os.chdir(open6Dir)
	
	print('git checkout 7b273f046a0affc61e83837f8df3b6377a835512')
	subprocess.check_call(['git', 'checkout', '7b273f046a0affc61e83837f8df3b6377a835512'])

	shutil.rmtree(open6BuildDir, ignore_errors=True)
	os.mkdir(open6BuildDir)
	os.chdir(open6BuildDir)
	
	if platform.system() == "Windows":
		print('generating Visual Studio 12 project, command: {0}', format(CMD_generateProjForVisStu12))
		subprocess.check_call(CMD_generateProjForVisStu12, shell=True)

		print('loading Visual Studio 12 environment and building project (in same shell to retain environment vars), command: {0} && {1}'.format(CMD_loadVisStu12Env, CMD_buildProjWithVisStu12))
		subprocess.check_call('{0} && {1}'.format(CMD_loadVisStu12Env, CMD_buildProjWithVisStu12), shell=True)
	elif platform.system() == "Linux":
		print('cmake -DUA_ENABLE_AMALGAMATION=ON ../')
		subprocess.check_call(['cmake', '-DUA_ENABLE_AMALGAMATION=ON', '../'])
		print('make')
		subprocess.check_call(['make'], stdout=out)
		
	shutil.copyfile(os.path.join(open6BuildDir, 'open62541.h'), os.path.join(baseDir, 'include', 'open62541.h'))
	shutil.copyfile(os.path.join(open6BuildDir, 'open62541.c'), os.path.join(baseDir, 'src', 'open62541.c'))
	
main()
