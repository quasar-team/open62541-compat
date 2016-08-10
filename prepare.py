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
import sys
import shutil
import platform
import subprocess
from shutil import copyfile

def deleteFolderRecursively( topdir, target ):
	for dirpath, dirnames, files in os.walk(topdir):
		for name in dirnames:
			if name == target:
				try:
					shutil.rmtree(os.path.join(dirpath, name))
				except OSError:
					pass
	return;
def deleteFileRecursively( topdir, target ):
	for dirpath, dirnames, files in os.walk(topdir):
		for name in files:
			if name == target:
				try:
					os.remove(os.path.join(dirpath, name))					
				except OSError:
					pass
	return;
	
def main():
	deleteFolderRecursively('.', 'open62541')
	print('git clone https://github.com/open62541/open62541.git')
	subprocess.call(['git', 'clone', 'https://github.com/open62541/open62541.git'])
	
	baseDirectory = os.getcwd()
	os.chdir('open62541')
	
	print('git checkout 7b273f046a0affc61e83837f8df3b6377a835512')
	subprocess.call(['git', 'checkout', '7b273f046a0affc61e83837f8df3b6377a835512'])
	deleteFolderRecursively('.', 'build')
	os.mkdir('build')
	os.chdir('build')
	
	if platform.system() == "Windows":
		print('cmake -DUA_ENABLE_AMALGAMATION=ON ../ -G "Visual Studio 12 2013 Win64"')
		subprocess.call('cmake -DUA_ENABLE_AMALGAMATION=ON ../ -G "Visual Studio 12 2013 Win64"', shell=True)

		# TODO: check to see if cmake can locate and invoke the vcvarsall.bat for the Vis Studio version provided with the -G option
		print('"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat" amd64 && msbuild ALL_BUILD.vcxproj /clp:ErrorsOnly /property:Platform=x64;Configuration=Release')
		subprocess.call('"C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat" amd64 && msbuild ALL_BUILD.vcxproj /clp:ErrorsOnly /property:Platform=x64;Configuration=Release', shell=True)
	elif platform.system() == "Linux":
		print('cmake -DUA_ENABLE_AMALGAMATION=ON ../')
		subprocess.call(['cmake', '-DUA_ENABLE_AMALGAMATION=ON', '../'])
		print('make')
		subprocess.call(['make'], stdout=out)
		
	os.chdir(baseDirectory)
	
	deleteFileRecursively('include', 'open62541.h')
	deleteFileRecursively('src', 'open62541.c')
	copyfile('open62541/build/open62541.h', 'include/open62541.h')
	copyfile('open62541/build/open62541.c', 'src/open62541.c')
	
main()
