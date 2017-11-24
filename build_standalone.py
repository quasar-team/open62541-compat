# TODO: Fix the license header and authors

import os
import shutil

build_directory = 'build'

shutil.rmtree(build_directory)
os.mkdir(build_directory)
os.chdir(build_directory)
# FIXME: windows compatibility?
os.system('cmake -DSTANDALONE_BUILD=ON ../')
os.system('make')
