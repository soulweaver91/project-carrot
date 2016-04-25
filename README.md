![Project Carrot logo](https://raw.githubusercontent.com/soulweaver91/project-carrot/master/Data/PCLogo-300px.png)

# Project Carrot
Project Carrot is a slowly progressing Jazz Jackrabbit 2 engine clone. You can discuss it
[here](http://www.jazz2online.com/jcf/showthread.php?t=19535), though following the long
period of no progress, the thread hasn't been active for months as of right now.

##Building
The suggested development environment and required libraries are as follows:

* [Microsoft Visual Studio 2015](http://www.visualstudio.com/)
* [Qt](http://qt-project.org/downloads) 5.6
* [SFML](http://www.sfml-dev.org/download.php) 2.3
* [BASS](http://www.un4seen.com/bass.html) 2.4 and a compatible version of BASS FX

If using Visual Studio, configure the Qt paths with the MSVS plugin and set the appropriate
values for the `SFML_DIR`, `BASS_DIR`, and `BASS_FX_DIR` user macros in the user property
sheets (`Microsoft.Cpp.Win32.user` and `Microsoft.Cpp.x64.user`). These are global to
all projects you compile with Visual Studio, so if you are going to compile the related
Project Carrot projects, you don't have to add these for them separately.

For other environments, I'm afraid to say you're on your own, though. You should at least make
sure your compiler understands C++11, as many of its features have been used in the codebase.

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

## License
This software is licensed under the [MIT License](https://opensource.org/licenses/MIT).
See the included `LICENSE` file for the licenses of the third-party libraries used.
