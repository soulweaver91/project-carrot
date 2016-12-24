#!/usr/bin/env bash
# Updates qt_preprocess.mak in Xcode project.
# Make sure you're sitting on master pulled from the repo first.
cd "${BASH_SOURCE%/*}" || exit
cd ..
qmake -spec macx-xcode
cp CarrotQt5.xcodeproj/qt_preprocess.mak .
git checkout -- CarrotQt5.xcodeproj/ Info.plist
sed -i '' -E 's#/usr/local/Cellar/qt5/[0-9\.]+/#/usr/local/opt/qt5/#g' qt_preprocess.mak
mv qt_preprocess.mak CarrotQt5.xcodeproj/qt_preprocess.mak
