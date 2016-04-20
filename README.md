![Project Carrot logo](https://raw.githubusercontent.com/soulweaver91/project-carrot/master/Data/PCLogo-300px.png)

# Project Carrot
Project Carrot is a slowly progressing Jazz Jackrabbit 2 engine clone. You can discuss it
[here](http://www.jazz2online.com/jcf/showthread.php?t=19535), though at the moment both
the project and the thread have been inactive for months.

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

For other environments, I'm afraid to say you're on your own, though.

##Running the game
To be able to run Project Carrot, you are required to extract the Jazz Jackrabbit 2 assets from
the `Anims.j2a` file in its installation folder. A tool specifically targeting Project Carrot,
[PCAE](https://github.com/soulweaver91/project-carrot-pcae), will automate this task for you,
not only dumping the graphics and sounds but also assigning them an uniform name independent of
their location inside the file itself. Those files should be moved to `Data\Assets` relative
to the project root or the compiled executable. It is safe to merge that folder to the one in
this repository, as the filetypes output by PCAE are excluded automatically.

To be able to do anything useful in-game, you do also need some levels and tilesets. Tools for
this have been written, but they aren't available yet. I'm working on this later! In the meantime,
you can contact me on J2Online (follow the link above) to obtain Windows binaries for them.
As for music, you should convert the J2B files manually for now to a more compatible format,
such as .it, for example with ModPlug Tracker on Windows.

## License
This software is licensed under the [MIT License](https://opensource.org/licenses/MIT).
See the included `LICENSE` file for the licenses of the third-party libraries used.
