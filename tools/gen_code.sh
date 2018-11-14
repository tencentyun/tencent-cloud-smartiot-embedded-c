#!/usr/bin/env bash

cd ../tools

python tc_iot_code_generator.py -c ../examples/advanced_edition/iot-product.json code_templates/*
# python tc_iot_code_generator.py -c ../examples/advanced_edition/iot-b87oxqbs.json code_templates/*
exit 1

python tc_iot_code_generator.py -c ../examples/scenarios/light/iot-product.json code_templates/tc_iot_device_*
python tc_iot_code_generator.py -c ../examples/scenarios/ota/iot-product.json code_templates/tc_iot_device_config.h

python tc_iot_code_generator.py -c ../tests/gtest//iot-product.json code_templates/tc_iot_device_config.h
python tc_iot_code_generator.py -c ../tests/gtest/data_template/iot-product.json code_templates/tc_iot_device_*
mv ../tests/gtest/data_template/tc_iot_device_logic.c ../tests/gtest/data_template/tc_iot_device_logic.cpp

ed ../tests/gtest/data_template/tc_iot_device_logic.cpp << END
1i
extern "C" {
.
5i
}
.
w
q
END

echo "All done."
