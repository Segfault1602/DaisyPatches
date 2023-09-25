# Phaseshapers

## Author

Alex St-Onge

## Description

This is a simple v/oct calibrator for the Daisy Patch. This app will save the calibration data to
persistent storage as defined in ../common/VOctCalibrationSettings.h.

The calibration is done in two steps:
1. Send a 1v signal to all 4 CV inputs.
3. Send a 3v signal to all 4 CV inputs.

The encoder is used to go to the next step.
I used the Mordax Data to send the voltages.

If you only need to calibrate some CV inputs, you can simply leave the other ones unconnected.
The calibration for these will be wrong, so just remember which CV inputs are calibrated.

To read the calibration in an other project, include the VOctCalibrationSettings.h file and create
a PersistentStorage object. After initialization, the PersistentStorage object should contain you
calibration data.

## Controls

CV pots needs to be fully counter-clockwise otherwise the calibration will be off.
Press the encoder to go to next step