# TODO: Fix the license header and authors

import os
import shutil

import prepare

prepare.main()

build_directory = 'build'

if os.path.isdir(build_directory):
    shutil.rmtree(build_directory)
os.mkdir(build_directory)
os.chdir(build_directory)
# FIXME: windows compatibility?
os.system('cmake -DSTANDALONE_BUILD=ON ../')
os.system('make')
