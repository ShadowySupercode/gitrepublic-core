#! /bin/bash

Help()
{
   # Display Help
   echo "Build script for ease of use."
   echo
   echo "Syntax: ./build.sh [-f|-t|-j|-h]"
   echo "Example ./build.sh -t <executable-target-name> -j <number-of-cpu-cores>"
   echo "Options:"
   echo "-f     Path to the Makefile."
   echo "-t     Target to be built. Default to all."
   echo "-j     Number of cores to be used during building."
   echo "-h     Prints this help"
}

# Get the options

MAKEFILE="./Makefile"
TARGETNAMES="all" # All targets in Makefile
NBCORES=""

while getopts ":f:t:j:h" option; do
   case $option in
      f)
         MAKEFILE=$OPTARG;;
      t)
         TARGETNAMES=$OPTARG;;
      j)
         NBCORES=$OPTARG;;
      h)
         Help
         exit;;
      \?)
         echo "Error: Invalid option"
         exit;;
   esac
done

if  [[ ! -d "build" ]]; then
   mkdir build;
fi

if [[ ! -d "out" ]]; then
   mkdir out
   if [[ ! -d "out/bin" ]]; then
      mkdir "out/bin"
   elif [[ ! -d "out/lib" ]]; then
      mkdir "out/lib"
   fi
fi

cd build
cmake ..

make -f $MAKEFILE -j $NBCORES $TARGETNAMES