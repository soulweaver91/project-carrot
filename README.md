# Project Carrot
Project Carrot is a slowly progressing Jazz Jackrabbit 2 engine clone. You can discuss it
[here](http://www.jazz2online.com/jcf/showthread.php?t=19535), though at the moment both
the project and the thread have been inactive for months.

##Building
The suggested development environment and required libraries are as follows:

* [Microsoft Visual Studio 2015](http://www.visualstudio.com/)
* [Qt](http://qt-project.org/downloads) 5.6
* [SFML](http://www.sfml-dev.org/download.php) 2.3
* [BASS](http://www.un4seen.com/bass.html) 2.4

If using Visual Studio, configure the Qt paths with the MSVS plugin and set the appropriate
values for the `SFML_DIR` and `BASS_DIR` user macros. For other environments, I'm afraid to
say you're on your own, though.

##Running the game
To be able to run Project Carrot, you are required to extract the Jazz Jackrabbit 2 assets from
the `Anims2.j2a` file in its installation folder. A tool expressly targeting Project Carrot,
[PCAE](https://github.com/soulweaver91/project-carrot-pcae), will automate this task for you,
not only dumping the graphics and sounds but also assigning them an uniform name independent of
their location inside the file itself. Those files should be moved to `Data\Assets` relative
to the project root or the compiled executable. (It is safe to merge the folder to the one
in the repository.)