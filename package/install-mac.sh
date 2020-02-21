#!/bin/sh
echo "Checking for brew.."
if ! [ -x "$(command -v brew)" ]; then
	echo 'Error: brew is not installed!'
	echo 'Install brew from https://brew.sh'
	exit 1
fi

echo "Checking for fftw..."
if brew ls --versions fftw > /dev/null; then
	echo "fftw is already installed"
else
	echo "Installing fftw"
	brew install fftw
fi

echo "Uninstalling old version"
rm -rf "/Users/$USER/Library/Application Support/obs-studio/plugins/spectralizer"
echo "Creating plugin folder"
mkdir -p "/Users/$USER/Library/Application Support/obs-studio/plugins/spectralizer"
echo "Moving plugin over"
mv plugin/* "/Users/$USER/Library/Application Support/obs-studio/plugins/spectralizer"
echo "Done!"
