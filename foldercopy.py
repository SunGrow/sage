import sys
import shutil
import distutils

def copy_and_overwrite(from_path, to_path):
    if os.path.exists(to_path):
        shutil.rmtree(to_path)
    shutil.copytree(from_path, to_path)

copy_and_overwrite(sys.argv[1], sys.argv[2])
