@ECHO OFF
REM You need svn and wget in your system path.
REM You also need 7-Zip installed.

IF EXIST "dependencies-win" del /S /Q dependencies-win
MKDIR dependencies-win

IF NOT EXIST "dependencies-build" MKDIR dependencies-build
CD dependencies-build
IF NOT EXIST "wineditline-src" (
    ECHO "Checking out WinEditline (MinGWEditline) with SVN."
    svn checkout svn://svn.code.sf.net/p/mingweditline/code/mingweditline/trunk wineditline-src
)
CD wineditline-src
ECHO Updating WinEditLine to latest version.
svn update

ECHO Cleaning build directories...
del /S /Q build32
del /S /Q build64

MKDIR build32
ECHO Generating solution files for 32-bit build.
CD build32
cmake -G"Visual Studio 15 2017" ..
ECHO Building 32-bit binaries.
REM We have to use RelWithDebInfo instead of Debug to avoid linking to non-redist DLLs like vcruntime140d.dll.
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" /property:Configuration=RelWithDebInfo wineditline.sln

ECHO Generating solution files for 64-bit build.
CD ..
MKDIR build64
CD build64
cmake -G"Visual Studio 15 2017 Win64" ..
ECHO Building 64-bit binaries.
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\15.0\Bin\MSBuild.exe" /property:Configuration=RelWithDebInfo wineditline.sln

CD ..\..
IF NOT EXIST "libusb" (
    ECHO Downloading and extracting libusb.
    wget --output-document=libusb.7z https://github.com/libusb/libusb/releases/download/v1.0.22/libusb-1.0.22.7z

    IF EXIST "C:\Program Files\7-Zip\7z.exe" (
        "C:\Program Files\7-Zip\7z.exe" x  -olibusb libusb.7z
    ) ELSE (
        "C:\Program Files (x86)\7-Zip\7z.exe" x -olibusb libusb.7z
    )
)

CD ..\dependencies-win
MKDIR i386
MKDIR i386\libusb-1.0
MKDIR i386\editline
MKDIR amd64
MKDIR amd64\libusb-1.0
MKDIR amd64\editline
MKDIR include
MKDIR include\libusb-1.0
MKDIR include\editline

COPY ..\dependencies-build\libusb\MS32\dll\libusb-1.0.dll .\i386\libusb-1.0\libusb-1.0.dll
COPY ..\dependencies-build\libusb\MS32\dll\libusb-1.0.lib .\i386\libusb-1.0\libusb-1.0.lib
COPY ..\dependencies-build\libusb\MS32\dll\libusb-1.0.pdb .\i386\libusb-1.0\libusb-1.0.pdb

COPY ..\dependencies-build\libusb\MS64\dll\libusb-1.0.dll .\amd64\libusb-1.0\libusb-1.0.dll
COPY ..\dependencies-build\libusb\MS64\dll\libusb-1.0.lib .\amd64\libusb-1.0\libusb-1.0.lib
COPY ..\dependencies-build\libusb\MS64\dll\libusb-1.0.pdb .\amd64\libusb-1.0\libusb-1.0.pdb

COPY ..\dependencies-build\libusb\include\libusb-1.0\libusb.h .\include\libusb-1.0\libusb.h

COPY ..\dependencies-build\wineditline-src\build32\src\RelWithDebInfo\edit.dll .\i386\editline\edit.dll
COPY ..\dependencies-build\wineditline-src\build32\src\RelWithDebInfo\edit.lib .\i386\editline\edit.lib
COPY ..\dependencies-build\wineditline-src\build32\src\RelWithDebInfo\edit.pdb .\i386\editline\edit.pdb

COPY ..\dependencies-build\wineditline-src\build64\src\RelWithDebInfo\edit.dll .\amd64\editline\edit.dll
COPY ..\dependencies-build\wineditline-src\build64\src\RelWithDebInfo\edit.lib .\amd64\editline\edit.lib
COPY ..\dependencies-build\wineditline-src\build64\src\RelWithDebInfo\edit.pdb .\amd64\editline\edit.pdb

COPY ..\dependencies-build\wineditline-src\src\editline\readline.h .\include\editline\readline.h

cd ..
