# Compiling the source code
import os
import argparse

# Parsing the arguments: available options are --use-doas, --bin
parser = argparse.ArgumentParser()
parser.add_argument("--use-doas", help="Use DOAS instead of SUDO", action="store_true")
parser.add_argument("--bin", help="Compile the binary", action="store_true")
args = parser.parse_args()

print("Compiling the source code...")

# Creating a build directory if it doesn't exist
if not os.path.exists('build'):
    os.mkdir('build')

print("Created build directory")

# Changing the working directory to the build directory
os.chdir('build')

# Checking if cmake is runnable
if not os.system('cmake ..'):
    print("CMake has been executed successfully")

    # Checking if make is runnable
    if not os.system('make'):
        print("Make has been executed successfully")

        # If the user passed the argument "--bin"
        if args.bin:
            # Copying the binary to the bin directory
            if not os.system('sudo cp -rf ./ACL /bin/acl'):
                print("Binary has been copied successfully")

print("Compilation complete")
