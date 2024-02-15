# Compilation

## On Linux

On Linux, just install pkg-config, libusb, libedit, cmake, make and a C compiler with the package manager, then run the
following commands from a terminal:

~~~
mkdir bld
cd bld
cmake ..
make -j4
sudo make install
~~~

You can then simply use `find_package(liblabpro)` in your CMake project
to import the necessary targets.

## On Windows

The following software is required to build liblabpro:

* [CMake](https://cmake.org) version 3.9 or later.
* [GNU Wget](https://eternallybored.org/misc/wget/). (Copy wget.exe to C:\Windows\System32.)
* [7-Zip](https://www.7-zip.org). Install it to the default location.
* [Subversion](https://sliksvn.com/download/). Make sure to allow the installer to add SVN
  to the path.
* Visual Studio 2017 or later. [Visual Studio Community](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community) edition will
  work fine; however, Visual Studio Code will *not*.
* The VC SDK for C++. You should be given the option to install it as part of the Visual Studio
  installation. Look for the option to install the SDKs for "Desktop development with C++." If
  you already have Visual Studio installed, but aren't sure whether you have the necessary SDKs,
  open the Start Menu, search for "Visual Studio Installer," click "Modify" under "Visual Studio
  Community 2017," and make sure "Desktop development with C++" is checked.

Copy the script `build_dependency_pack.bat` to a directory outside of the `liblabpro-code`
repository and run it. Copy the dependencies-win folder it generates so that it's next to
the other source directories as shown below.

~~~
liblabpro-code
  |- branches
  |- tags
  \- trunk
       |- CMakeScripts
       |- dependencies-win
       |    |- amd64
       |    |    |- libusb-1.0
       |    |    \- editline
       |    |- i386
       |    |    |- libusb-1.0
       |    |    \- editline
       |    \- include
       |         |- libusb-1.0
       |         \- editline
       |- src
       |- CMakeLists.txt
       |- COPYING.md
       |- INSTALL.md
       \- liblabpro.kdev4
       
~~~

Note that if you downloaded a source release rather than an SVN release,
you won't have the `branches`, `tags`, or `trunk` directories.

Once VS, CMake, and the dependencies are in place, open the CMake GUI and choose
the path to the folder containing `CMakeLists.txt` for "Where is the source code."
Choose the same path for "Where to build the binaries," but add "/bld" at the end.
Click "Configure"; CMake will prompt to create the build directory; say yes. When
prompted about the generator, choose your Visual Studio version, but make sure to
choose the Win64 compiler.

Don't be confused by Visual Studio's versioning! Visual Studio 14 is actually
Visual Studio 2015; Visual Studio 15 is actually Visual Studio 2017.

Once CMake is done configuring, click "Generate." When generating is done (usually
it's done right away) you can close CMake. Open Visual Studio and go to **File ->
Open -> Project/Solution** and open `labpro.sln` in the `bld` directory of the
source folder. You can choose an alternative optimization level in the drop-down
menu at the top of Visual Studio (Debug is the default). Then go to **Build ->
Build Solution**. The binaries will be in `bld/Debug` (if using a Debug build
configuration) or `bld/Release` (if using a Release build configuration).

**If you are planning on creating a portable package, build a Release or RelWithDebInfo
solution configuration. Debug will create a dependency on debug versions of VC
DLLs which are non-redistributable.**
