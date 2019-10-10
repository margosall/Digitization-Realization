# Digitization-Realization

## Setting up dev env:
1. Clone/Pull
2. git submodule update --init --recursive 
3. ./add_profile_paths.sh ; adds dev env to .bashrc is under helper_scripts

## Building @ project folder
1. make clean
2. make flash
3. make app ; for compiling just cpp
4. make app-flash ; for flashing just .elf 

## Debugging project
1. Install native-debug extension (webfreak.debug)
2. Hit F5
3. Add breakpoints and enjoy

## Todo:

- [x] ESP-IDF/ADF + Arduino 28.09
- [x] ESP-Mic / Line-in Datastream reading 29.09
- [ ] Reading SD card files
- [ ] ESP-DSP <- FIR / FFT  
- TBD...

