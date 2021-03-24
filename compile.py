import sys
import os
import shutil

def copyAndOverride(from_path, to_path):
    if os.path.exists(to_path):
        shutil.rmtree(to_path)
    shutil.copytree(from_path, to_path)


os.system("meson build")

os.chdir("app/shaders")
os.system("make")
os.chdir("../../")
copyAndOverride("app/shaders", "build/app/shaders")
copyAndOverride("app/res", "build/app/res")
copyAndOverride("app/data", "build/app/data")
copyAndOverride("app/cfg", "build/app/cfg")

os.chdir("build")
os.system("meson compile")
os.chdir("..")
