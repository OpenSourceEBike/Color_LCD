#!/bin/sh

echo
echo This script will release a new version of the 850C, 860C and SW102 firmwares
echo 
echo Please enter the new version number, e.g. 0.5.1

read VERSION

RELEASE_FOLDER=${PWD}/releases/$VERSION

if [ ! -d "$RELEASE_FOLDER" ]; then
	
  # create folder
	mkdir -p $RELEASE_FOLDER

	cd 860C_850C/src/
	# version 860C bootloader
        make -f Makefile clean
	rm -R common/src
        make -f Makefile VERSION=$VERSION DISPLAY_VERSION="860C_BOOTLOADER"
	cp main.bin $RELEASE_FOLDER/860C_v$VERSION.bin
	cd ../..

	echo 
	echo Done! If the build went correctly, find the files on: $RELEASE_FOLDER
else
	echo 
	echo Release $VERSION already exists!
fi

