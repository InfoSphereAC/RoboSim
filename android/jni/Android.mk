LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := robosim

LOCAL_CFLAGS := -DANDROID_NDK -Wall -Wextra -Werror -Wno-unused-parameter -fno-exceptions

LOCAL_SRC_FILES := \
	androidmain.cpp \
	asset_manager.cpp \
	android_ifstream.cpp \
	AndroidFileChooser.cpp \
	AndroidSpeaker.cpp \
	AndroidSoundBuffer.cpp \
	AndroidSoundController.cpp \
	../../Client.cpp \
	../../Controller.cpp \
	../../Drawer.cpp \
	../../Environment.cpp \
	../../EnvironmentDrawer.cpp \
	../../EnvironmentEditor.cpp \
	../../ExecutionContext.cpp \
	../../FileChooser.cpp \
	../../Interpreter.cpp \
	../../Interpreter_CompareInstructions.cpp \
	../../Interpreter_ControlFlowInstructions.cpp \
	../../Interpreter_DataManipulationInstructions.cpp \
	../../Interpreter_LogicalInstructions.cpp \
	../../Interpreter_MathInstructions.cpp \
	../../Interpreter_SystemIOInstructions.cpp \
	../../Model.cpp \
	../../Motor.cpp \
	../../NetworkConstants.cpp \
	../../NetworkInterface.cpp \
	../../NetworkPacket.cpp \
	../../RXEFile.cpp \
	../../Robot.cpp \
	../../RobotDrawer.cpp \
	../../RobotTouchHandler.cpp \
	../../SensorConfigurationScreen.cpp \
	../../Server.cpp \
	../../ServerBrowser.cpp \
	../../ShowMessageBoxAndExit.cpp \
	../../Simulation.cpp \
	../../Single.cpp \
	../../System.cpp \
	../../Texture.cpp \
	../../Time.c \
	../../TouchesRecognizer.cpp \
	../../UIButton.cpp \
	../../UIIcon.cpp \
	../../UIKnob.cpp \
	../../UIRadioGroup.cpp \
	../../UIView.cpp \
	../../UserDefaults.cpp \
	../../UserInterface.cpp \
	../../VMMemory.cpp \
	../../Vec4.cpp \

LOCAL_LDLIBS := -lGLESv1_CM -llog -lz

include $(BUILD_SHARED_LIBRARY)
