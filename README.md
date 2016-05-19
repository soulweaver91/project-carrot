![Project Carrot logo](https://raw.githubusercontent.com/soulweaver91/project-carrot/master/Data/PCLogo-300px.png)

# Project Carrot
[![AppVeyor build status](https://ci.appveyor.com/api/projects/status/2f9f6k3x3mytjifj?svg=true)](https://ci.appveyor.com/project/soulweaver91/project-carrot)
[![Travis CI build Status](https://travis-ci.org/soulweaver91/project-carrot.svg)](https://travis-ci.org/soulweaver91/project-carrot)

Project Carrot is a slowly progressing spiritual clone of the engine for Jazz Jackrabbit 2.
You can discuss it [here](http://www.jazz2online.com/jcf/showthread.php?t=19535).

##Building
### Common dependencies

* [Qt](http://qt-project.org/downloads) 5.6
* [SFML](http://www.sfml-dev.org/download.php) 2.3
* [BASS](http://www.un4seen.com/bass.html) 2.4 and a compatible version of BASS FX

If you are planning to use a compiler different than one listed below, make sure it is
sufficiently C++14 compliant as the codebase utilizes many of the more modern C++ features.

### Windows
The suggested development environment, as well as the primary environment the code is written on,
is [Microsoft Visual Studio 2015](http://www.visualstudio.com/). MSVC Windows build status is
automatically evaluated at [AppVeyor](https://travis-ci.org/soulweaver91/project-carrot).

If using Visual Studio, configure the Qt paths with the MSVS plugin and set the appropriate
values for the `SFML_DIR`, `BASS_DIR`, and `BASS_FX_DIR` user macros in the user property
sheets (`Microsoft.Cpp.Win32.user` and `Microsoft.Cpp.x64.user`). These are global to
all projects you compile with Visual Studio, so if you are going to compile the related
Project Carrot projects, you don't have to add these for them separately. Alternatively,
setting these as environment variables should work as well.

### Linux
[GCC 5.3](https://gcc.gnu.org/) or above and [Clang](http://clang.llvm.org/) or above are
provisionally supported and builds on the listed versions are automatically evaluated at
[Travis CI](https://ci.appveyor.com/project/soulweaver91/project-carrot).

Download and extract the BASS and BASS FX Linux archives from their homepage and set
Set the `BASS_DIR` and `BASS_FX_DIR` to point into that directory, and also add them
to your `CPATH`. Additionally, install the other dependencies if you haven't done so
already, for example in Ubuntu:

```
sudo apt-get install qt5-default libsfml-dev
```

Then, configure and build the project, using the `spec` value corresponding to the compiler in use:
```
qmake -spec=linux-g++-64
make release
```

or

```
qmake -spec=linux-clang
make release
```

If in doubt, take a look at the [Travis configuration file](https://github.com/soulweaver91/project-carrot/blob/travis-test/.travis.yml)
and see if it can help you.

### Mac OS X
Not currently officially supported due to various reasons, including the complete lack of an
Apple device, but may work in a similar fashion as Linux does.

##Running the game
To be able to run Project Carrot, you are required to extract the Jazz Jackrabbit 2 assets from
the `Anims.j2a` file in its installation folder. A tool specifically targeting Project Carrot,
[PCAE](https://github.com/soulweaver91/project-carrot-pcae), will automate this task for you,
not only dumping the graphics and sounds but also assigning them an uniform name independent of
their location inside the file itself. Those files should be moved to `Data\Assets` relative
to the project root or the compiled executable. It is safe to merge that folder to the one in
this repository, as the filetypes output by PCAE are excluded automatically and so should not
end up in any potential commits later on.

You should also convert some tilesets and levels for yourself.
[PCTC](https://github.com/soulweaver91/project-carrot-pctc) and
[PCLC](https://github.com/soulweaver91/project-carrot-pclc) do this for you; make sure you
always convert the levels with the latest version of PCLC for highest level of compatibility
with the latest Project Carrot builds. As for music, you should convert the J2B files manually
for now to a more compatible format, such as .it, for example with OpenMPT on Windows. After
converting the levels, make sure the filename in the `config.ini` file for each level
corresponds exactly to a file in the music folder.

Put the level folders into a folder called `Levels` right under the Project Carrot root folder,
creating it if it likely doesn't exist. Same applies to tileset folders, which go to `Tilesets`,
and music files, which go to `Music`.

On Windows, you must put the relevant DLL files for Qt, SFML and BASS into a location the
application could expect to find them from, for example right into the same folder as the
executable itself. On Linux, as long as both Qt 5 and SFML libraries have been installed,
you only need to copy `libbass.so` and `libbass_fx.so` to the same folder as the
executable.

## License
This software is licensed under the [MIT License](https://opensource.org/licenses/MIT).
See the included `LICENSE` file for the licenses of the third-party libraries used.
