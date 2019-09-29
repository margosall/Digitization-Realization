#! /bin/bash

SCRIPTPATH="$( cd "$(dirname "$0")" ; pwd -P )"

sed -i '/Espressif toolchain/d' ~/.bashrc
sed -i '/export PATH=/d' ~/.bashrc
sed -i '/export IDF_PATH=/d' ~/.bashrc
sed -i '/export ADF_PATH=/d' ~/.bashrc
sed -i '/export PROJECT_PATH=/d' ~/.bashrc
sed -i '/export DSPLIB_PATH=/d' ~/.bashrc

echo '#Espressif toolchain + IDF/ADF Paths' >> ~/.bashrc
echo export PATH=$SCRIPTPATH/esp/xtensa-esp32-elf/bin:$PATH >> ~/.bashrc
echo export IDF_PATH=$SCRIPTPATH/esp/esp-idf >> ~/.bashrc
echo export ADF_PATH=$SCRIPTPATH/esp/esp-adf >> ~/.bashrc
echo export DSPLIB_PATH=$SCRIPTPATH/sound_project/components/esp-dsp/modules >> ~/.bashrc