import os
import shutil

install = "/usr/include/Arkas/"

if not os.path.isdir(install):
    os.mkdir(install)

for subDir, dirs, files in os.walk("source"):
    for d in dirs:
        os.mkdir(os.path.join(subDir, d).replace("source/", install))

    for file in files:
        src  = os.path.join(subDir, file)
        dest = src.replace("source/", install)

        if not src.endswith(".h"):
            continue
        shutil.copyfile(src, dest)
