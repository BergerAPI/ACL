"""
Copying the std library into "$HOME/.acl/std"
"""

import os

# Creating the directory
if not os.path.exists(os.path.expanduser("~/.acl/std")):
    os.makedirs(os.path.expanduser("~/.acl/std"))

# Copying the entire lib directory
for root, dirs, files in os.walk("./lib"):
    for file in files:
        src = os.path.join(root, file)
        dst = os.path.expanduser("~/.acl/std/" + file)
        print("Copying {} to {}".format(src, dst))
        with open(src, "r") as f:
            with open(dst, "w") as g:
                g.write(f.read())

print("Copying the std library into \"$HOME/.acl/std\"")
