## MacOS Ventura (Intel)



### Download and install Apple 'XCode' from App Store

(You may need to register with Apple as a developer.)
Install xcode command line utilities:

    xcode-select --install 

### Install 'homebrew'

    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install.sh)"
    brew update

### Install dependencies

    pip3 install --user pytest opentimelineio
    brew install bzip2 freetype zlib 
    brew install qt@5
    brew install cmake 
    brew install nlohmann-json
    brew install pybind11
    brew install fmt
    brew install openexr
    brew install pkg-config
    brew install spdlog
    brew install pulseaudio
    brew install glew
    rehash

    brew tap homebrew/core
   

### Actor framework

    mkdir xstudio-deps
    cd xstudio-deps
    curl -LO https://github.com/actor-framework/actor-framework/archive/refs/tags/0.18.4.tar.gz
    tar -xf 0.18.4.tar.gz
    cd actor-framework-0.18.4
    ./configure
    cd build
    make -j $JOBS
    make install
    cd ../..

### FFMPEG

    brew install automake fdk-aac git lame libass libtool libvorbis libvpx \
        opus sdl shtool texi2html theora wget x264 x265 xvid nasm
    curl -LO https://ffmpeg.org/releases/ffmpeg-5.1.tar.bz2tar
    tar -xf ffmpeg-5.1.tar.bz
    cd ffmpeg-5.1
    ./configure --extra-libs=-lpthread   --extra-libs=-lm    --enable-gpl   --enable-libfdk_aac   --enable-libfreetype   --enable-libmp3lame   --enable-libopus   --enable-libvpx   --enable-libx264   --enable-libx265 --enable-shared --enable-nonfree
    make -j 8
    make install

### QT: You must set the path to the Qt libraries via Qt5_DIR env var

    export Qt5_DIR=/usr/local/opt/qt5/lib/cmake

### I think i had to fiddle with PKG_CONFIG_PATH too

    mkdir __build
    cd __build
    cmake .. -DBUILD_DOCS=No
    make -j8