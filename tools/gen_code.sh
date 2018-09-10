#!/usr/bin/env bash

python tc_iot_code_generator.py -c ../examples/advanced_edition/iot-product.json code_templates/*
python tc_iot_code_generator.py -c ../examples/scenarios/light/iot-product.json code_templates/tc_iot_device_*
python tc_iot_code_generator.py -c ../examples/scenarios/ota/iot-product.json code_templates/tc_iot_device_config.h

