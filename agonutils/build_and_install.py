import os
import shutil
import subprocess
import sys
import site

def clean_build():
    """Remove the build directory if it exists."""
    build_dir = 'build'
    if os.path.exists(build_dir):
        print(f"Cleaning {build_dir} directory...")
        shutil.rmtree(build_dir)
    else:
        print(f"{build_dir} directory not found, nothing to clean.")

def build_project():
    """Run the setup.py build command."""
    print("Building the project...")
    result = subprocess.run([sys.executable, 'setup.py', 'build'], check=True)
    if result.returncode == 0:
        print("Build successful!")
    else:
        print("Build failed!")
        sys.exit(result.returncode)

def local_install():
    """Run the setup.py install command in the virtual environment."""
    print("Installing the project in the virtual environment...")
    result = subprocess.run([sys.executable, 'setup.py', 'install'], check=True)
    if result.returncode == 0:
        print("Install successful!")
    else:
        print("Install failed!")
        sys.exit(result.returncode)

def set_pythonpath():
    """Set the PYTHONPATH to the user's local site-packages directory."""
    user_site = site.getusersitepackages()
    current_pythonpath = os.environ.get('PYTHONPATH', '')

    if user_site not in current_pythonpath:
        os.environ['PYTHONPATH'] = f"${current_pythonpath}:${user_site}" if current_pythonpath else user_site
        print(f"PYTHONPATH set to: ${os.environ['PYTHONPATH']}")
    else:
        print(f"PYTHONPATH already set to: ${os.environ['PYTHONPATH']}")

def test_install():
    """Test the installed package by importing it and calling a function."""
    print("Testing the installed package...")
    result = subprocess.run([sys.executable, '-c', 'import agonutils; agonutils.hello()'], check=True)
    if result.returncode == 0:
        print("Package works!")
    else:
        print("Package failed to run!")
        sys.exit(result.returncode)

if __name__ == '__main__':
    clean_build()
    build_project()
    local_install()
    set_pythonpath()
    test_install()
