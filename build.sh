#! /bin/bash

Help()
{
   # Display Help
   echo "Build script for ease of use."
   echo
   echo "Syntax: ./build.sh [-f|-t|-j|-m|-c|-h]"
   echo "Example ./build.sh -t <executable-target-name> -j <number-of-threads>"
   echo "Options:"
   echo "-f     Path to the Makefile."
   echo "-t     Target to be built. Default to all."
   echo "-j     Number of cores to be used during building."
   echo "-m     Mode of build. 'Release' or 'Debug'."
   echo "-c     Clear build. Specify which build to clear: 'Release' or 'Debug' or both."
   echo "-h     Prints this help."
}

# Default values

MAKEFILE="./Makefile"
TARGETNAMES="all" # All targets in Makefile
NBTHREADS=""
BUILD_MODE="Debug"

# Get the options

while getopts ":f:t:j:m:c:h" option; do
   case $option in
      f)
         MAKEFILE=$OPTARG;;
      t)
         TARGETNAMES=$OPTARG;;
      j)
         NBTHREADS=$OPTARG;;
      m)
         BUILD_MODE=$OPTARG;;
      c)
         CLEAR=$OPTARG;;
      h)
         Help
         exit;;
      \?)
         echo "Error: Invalid option"
         exit;;
   esac
done

if [[ $CLEAR != Release ]] && [[ $CLEAR != Debug ]] && [[ $CLEAR != All ]]; then
   echo "Invalid option for -c flag. Either 'Release', 'Debug' or 'All'".
elif [[ $CLEAR == All ]]; then
   echo "Clearing all builds."
   rm -rf build
   rm -rf out
else
   echo "Clearing $CLEAR build."
   rm -rf build/$CLEAR
   rm -rf out/bin/$CLEAR
   rm -rf out/lib/$CLEAR
fi

if [[ $BUILD_MODE != Debug ]] && [[ $BUILD_MODE != Release ]]; then
   echo "'$BUILD_MODE' is an invalid value of build mode option. Valid options are 'Release' and 'Debug'."
   exit
fi

if  [[ ! -d "build/$BUILD_MODE" ]]; then
   mkdir -p build/$BUILD_MODE;
fi

if [[ ! -d "out" ]]; then
   mkdir out
   if [[ ! -d "out/bin" ]]; then
      mkdir -p "out/bin"
   elif [[ ! -d "out/lib" ]]; then
      mkdir -p "out/lib"
   fi

   if [[ ! -d "out/bin/$BUILD_MODE" ]]; then
      mkdir -p "out/bin/$BUILD_MODE"
   elif [[ ! -d "out/lib/$BUILD_MODE" ]]; then
      mkdir -p "out/lib/$BUILD_MODE"
   fi
fi

cd "build/$BUILD_MODE"
cmake -D CMAKE_BUILD_TYPE=$BUILD_MODE ../..

make -f $MAKEFILE -j $NBTHREADS $TARGETNAMES