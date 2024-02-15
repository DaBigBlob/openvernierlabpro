# Compilation

## On Linux

On Linux, just install libusb, cmake, and libedit with the package manager, then run the
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

You need CMake 3.9 or later to build liblabpro. Get it from [cmake.org](https://cmake.org).

You also need Visual Studio. The free [Visual Studio Community](https://www.visualstudio.com/thank-you-downloading-visual-studio/?sku=Community) edition will
work fine; however, Visual Studio Code will *not*. Install Visual Studio
and make sure you've also chosen to install the SDKs for "Desktop development
with C++." If you already have Visual Studio installed, but aren't sure whether
you have the necessary SDKs, open the Start Menu, search for "Visual Studio
Installer," click "Modify" under "Visual Studio Community 2017," and make sure
"Desktop development with C++" is checked.

Create the directory `dependencies-win` in the same folder
as `CMakeScripts` and `src`. Download the latest libusb binary package
for Windows from http://libusb.info and extract the it into the
`dependencies-win` folder so that your directory structure looks like
the diagram below.

Sadly, the WinEditLine binaries seem to have some incompatibility with
VC 2017, so you'll have to compile it yourself.

1. Get WinEditline with SVN: `svn checkout svn://svn.code.sf.net/p/mingweditline/code/mingweditline/trunk wineditline`
2. Open CMake-GUI. Select the `wineditline` folder you checked out under "Where is the
source code". Choose the same folder under "Where to build the binaries", but add "/build"
to the end of the path.
3. Hit **Configure**, allow CMake to create the build directory, then choose
"Visual Studio 15 2017 Win64" as the generator. Then hit Generate.
4. Close CMake-GUI.
5. Open `WinEditLine.sln` in the `build` directory in Visual Studio. Go to
"Build" > "Build Solution".
6. From the wineditline source directory, copy the following files to the
locations indicated in the diagram below:
~~~
build/src/Debug/edit.dll
build/src/Debug/edit.lib
build/src/Debug/edit.pdb
src/editline/readline.h
~~~

~~~
liblabpro-code
  |- branches
  |- tags
  \- trunk
       |- CMakeScripts
       |- dependencies-win
       |    |- libusb
       |    |    |- examples
       |    |    |- include
       |    |    |- MinGW32
       |    |    |- MinGW64
       |    |    |- MS32
       |    |    |- MS64
       |    |    |- libusb-1.0.def
       |    |    \- README.txt
       |    \- wineditline
       |         |- bin64
       |         |    |- edit.dll
       |         |    |- edit.lib
       |         |    \- edit.pdb (Optional)
       |         \- include
       |              \- editline
       |                   \- readline.h
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
