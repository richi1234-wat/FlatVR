//============ Copyright (c) Valve Corporation, All rights reserved. ============
//===================== FlatVR Universal Driver =======================

#include <openvr_driver.h>
#include <vector>
#include <thread>
#include <chrono>
#include <algorithm>

#include <atomic> // Required for std::atomic
#include <windows.h>
#include <atlbase.h>
#include "IniReader\IniReader.h"

using namespace vr;
using namespace std::chrono;

#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#define HMD_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define HMD_DLL_IMPORT extern "C" 
#else
#error "Unsupported Platform."
#endif

inline HmdQuaternion_t HmdQuaternion_Init(double w, double x, double y, double z)
{
	HmdQuaternion_t quat;
	quat.w = w; quat.x = x; quat.y = y; quat.z = z;
	return quat;
}

int KeyNameToKeyCode(std::string KeyName) {
	std::transform(KeyName.begin(), KeyName.end(), KeyName.begin(), ::toupper);
	if (KeyName == "NONE") return 0;
	else if (KeyName == "MOUSE-LEFT-BTN") return VK_LBUTTON;
	else if (KeyName == "MOUSE-RIGHT-BTN") return VK_RBUTTON;
	else if (KeyName == "MOUSE-MIDDLE-BTN") return VK_MBUTTON;
	else if (KeyName == "MOUSE-SIDE1-BTN") return VK_XBUTTON1;
	else if (KeyName == "MOUSE-SIDE2-BTN") return VK_XBUTTON2;
	else if (KeyName == "ESCAPE") return VK_ESCAPE;
	else if (KeyName == "F1") return VK_F1; else if (KeyName == "F2") return VK_F2;
	else if (KeyName == "F3") return VK_F3; else if (KeyName == "F4") return VK_F4;
	else if (KeyName == "F5") return VK_F5; else if (KeyName == "F6") return VK_F6;
	else if (KeyName == "F7") return VK_F7; else if (KeyName == "F8") return VK_F8;
	else if (KeyName == "F9") return VK_F9; else if (KeyName == "F10") return VK_F10;
	else if (KeyName == "F11") return VK_F11; else if (KeyName == "F12") return VK_F12;
	else if (KeyName == "~") return 192;
	else if (KeyName == "1") return '1'; else if (KeyName == "2") return '2';
	else if (KeyName == "3") return '3'; else if (KeyName == "4") return '4';
	else if (KeyName == "5") return '5'; else if (KeyName == "6") return '6';
	else if (KeyName == "7") return '7'; else if (KeyName == "8") return '8';
	else if (KeyName == "9") return '9'; else if (KeyName == "0") return '0';
	else if (KeyName == "-") return 189; else if (KeyName == "=") return 187;
	else if (KeyName == "TAB") return VK_TAB; else if (KeyName == "CAPS-LOCK") return VK_CAPITAL;
	else if (KeyName == "SHIFT") return VK_SHIFT; else if (KeyName == "CTRL") return VK_CONTROL;
	else if (KeyName == "RCTRL") return VK_RCONTROL; else if (KeyName == "WIN") return VK_LWIN;
	else if (KeyName == "ALT") return VK_MENU; else if (KeyName == "SPACE") return VK_SPACE;
	else if (KeyName == "ENTER") return VK_RETURN; else if (KeyName == "BACKSPACE") return VK_BACK;
	else if (KeyName == "Q") return 'Q'; else if (KeyName == "W") return 'W';
	else if (KeyName == "E") return 'E'; else if (KeyName == "R") return 'R';
	else if (KeyName == "T") return 'T'; else if (KeyName == "Y") return 'Y';
	else if (KeyName == "U") return 'U'; else if (KeyName == "I") return 'I';
	else if (KeyName == "O") return 'O'; else if (KeyName == "P") return 'P';
	else if (KeyName == "[") return 219; else if (KeyName == "]") return 221;
	else if (KeyName == "A") return 'A'; else if (KeyName == "S") return 'S';
	else if (KeyName == "D") return 'D'; else if (KeyName == "F") return 'F';
	else if (KeyName == "G") return 'G'; else if (KeyName == "H") return 'H';
	else if (KeyName == "J") return 'J'; else if (KeyName == "K") return 'K';
	else if (KeyName == "L") return 'L'; else if (KeyName == ":") return 186;
	else if (KeyName == "APOSTROPHE") return 222; else if (KeyName == "\\") return 220;
	else if (KeyName == "Z") return 'Z'; else if (KeyName == "X") return 'X';
	else if (KeyName == "C") return 'C'; else if (KeyName == "V") return 'V';
	else if (KeyName == "B") return 'B'; else if (KeyName == "N") return 'N';
	else if (KeyName == "M") return 'M'; else if (KeyName == "<") return 188;
	else if (KeyName == ">") return 190; else if (KeyName == "?") return 191;
	else if (KeyName == "PRINTSCREEN") return VK_SNAPSHOT; else if (KeyName == "SCROLL-LOCK") return VK_SCROLL;
	else if (KeyName == "PAUSE") return VK_PAUSE; else if (KeyName == "INSERT") return VK_INSERT;
	else if (KeyName == "HOME") return VK_HOME; else if (KeyName == "DELETE") return VK_DELETE;
	else if (KeyName == "END") return VK_END; else if (KeyName == "PAGE-UP") return VK_PRIOR;
	else if (KeyName == "PAGE-DOWN") return VK_NEXT; else if (KeyName == "UP") return VK_UP;
	else if (KeyName == "DOWN") return VK_DOWN; else if (KeyName == "LEFT") return VK_LEFT;
	else if (KeyName == "RIGHT") return VK_RIGHT; else if (KeyName == "NUM-LOCK") return VK_NUMLOCK;
	else if (KeyName == "NUMPAD0") return VK_NUMPAD0; else if (KeyName == "NUMPAD1") return VK_NUMPAD1;
	else if (KeyName == "NUMPAD2") return VK_NUMPAD2; else if (KeyName == "NUMPAD3") return VK_NUMPAD3;
	else if (KeyName == "NUMPAD4") return VK_NUMPAD4; else if (KeyName == "NUMPAD5") return VK_NUMPAD5;
	else if (KeyName == "NUMPAD6") return VK_NUMPAD6; else if (KeyName == "NUMPAD7") return VK_NUMPAD7;
	else if (KeyName == "NUMPAD8") return VK_NUMPAD8; else if (KeyName == "NUMPAD9") return VK_NUMPAD9;
	else if (KeyName == "NUMPAD-DIVIDE") return VK_DIVIDE; else if (KeyName == "NUMPAD-MULTIPLY") return VK_MULTIPLY;
	else if (KeyName == "NUMPAD-MINUS") return VK_SUBTRACT; else if (KeyName == "NUMPAD-PLUS") return VK_ADD;
	else if (KeyName == "NUMPAD-DEL") return VK_DECIMAL;
	else return 0;
}

static const char* const k_pch_Sample_Section = "FlatVR";
static const char* const k_pch_Sample_SerialNumber_String = "serialNumber";
static const char* const k_pch_Sample_ModelNumber_String = "modelNumber";
static const char* const k_pch_Sample_WindowX_Int32 = "windowX";
static const char* const k_pch_Sample_WindowY_Int32 = "windowY";
static const char* const k_pch_Sample_WindowWidth_Int32 = "windowWidth";
static const char* const k_pch_Sample_WindowHeight_Int32 = "windowHeight";
static const char* const k_pch_Sample_RenderWidth_Int32 = "renderWidth";
static const char* const k_pch_Sample_RenderHeight_Int32 = "renderHeight";
static const char* const k_pch_Sample_SecondsFromVsyncToPhotons_Float = "secondsFromVsyncToPhotons";
static const char* const k_pch_Sample_DisplayFrequency_Float = "displayFrequency";
static const char* const k_pch_Sample_ZoomWidth_Float = "ZoomWidth";
static const char* const k_pch_Sample_ZoomHeight_Float = "ZoomHeight";
static const char* const k_pch_Sample_FOV_Float = "FOV";
static const char* const k_pch_Sample_Stereo_Bool = "Stereo";
static const char* const k_pch_Sample_DebugMode_Bool = "DebugMode";

double FirstCtrlLastPos[3] = { 0, 0, 0 }, SecondCtrlLastPos[3] = { 0, 0, 0 };
milliseconds deltaTime;

typedef struct _HMDData { double X, Y, Z, Yaw, Pitch, Roll; } THMD, * PHMD;

typedef struct _Controller
{
	double	X, Y, Z, Yaw, Pitch, Roll;
	unsigned short	Buttons;
	float	Trigger, AxisX, AxisY;
	float	GripValue;
	bool	ButtonA, ButtonB;
} TController, * PController;

#define GRIP_BTN	0x0001
#define THUMB_BTN	0x0002
#define MENU_BTN	0x0010
#define SYS_BTN		0x0020

THMD MyHMD;
TController MyCtrl, MyCtrl2;
bool HMDConnected = false, ctrlsConnected = false;
std::string ControllerType = "vive_controller";

double DegToRad(double f) { return f * (3.14159265358979323846 / 180); }

int KEY_ID_LEFT_TRIGGER, KEY_ID_LEFT_MENU, KEY_ID_LEFT_GRIP, KEY_ID_LEFT_SYSTEM;
int KEY_ID_RIGHT_TRIGGER, KEY_ID_RIGHT_TRIGGER2, KEY_ID_RIGHT_MENU, KEY_ID_RIGHT_GRIP, KEY_ID_RIGHT_SYSTEM;
int KEY_ID_CHANGE_WEAPON, KEY_ID_TELEPORT, KEY_ID_MOVE_CONTROLLERS_FORWARD, KEY_ID_MOVE_CONTROLLERS_BACK;
int KEY_ID_PLAYER_RISE_HIGHER, KEY_ID_PLAYER_RISE_LOWER, KEY_ID_PLAYER_RISE_RESET, KEY_ID_CROUCH, KEY_ID_LEFT_CTRL_MOTION;
int KEY_ID_ROTATION_CONTROLLERS_RESET;
int KEY_ID_LEFT_HAND_ROTATE, KEY_ID_RIGHT_HAND_ROTATE;
int KEY_ID_HAND_FORWARD, KEY_ID_HAND_BACK, KEY_ID_HAND_LEFT, KEY_ID_HAND_RIGHT;
int KEY_ID_LEFT_TRACKPAD_CLICK, KEY_ID_RIGHT_TRACKPAD_CLICK, KEY_ID_FINE_TUNE;
int KEY_ID_MOTION_MODE_MOVE_UP, KEY_ID_MOTION_MODE_MOVE_DOWN;
int KEY_ID_TURN_LEFT, KEY_ID_TURN_RIGHT;
int KEY_ID_UP, KEY_ID_DOWN, KEY_ID_LEFT, KEY_ID_RIGHT;

// Oculus specific keys
int KEY_ID_LEFT_A, KEY_ID_LEFT_B, KEY_ID_RIGHT_A, KEY_ID_RIGHT_B;
int KEY_ID_STICK_CLICK_LEFT, KEY_ID_STICK_CLICK_RIGHT, KEY_ID_MENU_CLICK;

#define StepPos 0.01;
double HMDPosZ = 0;
double FirstCtrlPos[3] = { 0, 0, 0 }, SecondCtrlPos[3] = { 0, 0, 0 };
double CtrlsRoll = 0, CtrlsPitch = 0;
float CrouchOffsetZ = 0;

THMD mouseHMD;
THMD cameraHMD;
bool firstCP = true;
int m_HalfWidth = 1920 / 2, m_HalfHeight = 1080 / 2;
float mouseSensetivePitch = 0.04, mouseSensetiveYaw = 0.05;
float mouseYaw = 0, mousePitch = 0;
float HandMoveSensitivity = 5.0f;

HWND SteamVRHeadsetWindow = 0;
HWND HalfLifeAlyxWindow = 0;
bool HeadsetWindowFocused = false;
bool HalfLifeAlyxFocused = false;
bool VRMode = false;
float VRAimingAngleOffset;
float oldPitch = 0, deltaPitch = 0, oldRoll = 0, deltaRoll = 0;
double CtrlPosOffset[3] = { 0, 0, 0 };
double SecondCtrlYPROffset[3] = { 0, 0, 0 };
double SecondCtrlLastYPR[3] = { 0, 0, 0 };

float LeftHandYaw = 0, LeftHandPitch = 0, LeftAngleDist = 0, LeftRadDist = 0, LeftYOffset = 0;
float RightAngleDist = 0;
float RightHandYaw = 0, RightHandPitch = 0, RightRadDist = 0;

double LeftHandRotOffset[3] = { 0, 0, 0 };
double RightHandRotOffset[3] = { 0, 0, 0 };
bool LeftHandIsRotating = false, RightHandIsRotating = false;
bool PrevLeftRotateKey = false, PrevRightRotateKey = false;
double RightYOffset = 0.0, LeftRotateYOffset = 0.0;
double LeftXOffset = 0.0, LeftZOffset = 0.0, RightXOffset = 0.0, RightZOffset = 0.0;
float HandRadDist = 0;
float MotionLeftRadDist = 0, MotionLeftAngleDist = 0;
#define OffsetPitchByDefault -45
float ZPosSensetive = 0.009;

// --- NEW: Low-Level Mouse Hook for Scroll Wheel ---
HHOOK g_hMouseHook = NULL;
std::atomic<int> g_MouseWheelDelta(0);
DWORD g_HookThreadId = 0;
std::thread g_HookThread;

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0 && wParam == WM_MOUSEWHEEL) {
		MSLLHOOKSTRUCT* pMouseStruct = (MSLLHOOKSTRUCT*)lParam;
		// Extract high-word delta (correct for Low-Level hook)
		short delta = (short)HIWORD(pMouseStruct->mouseData);
		g_MouseWheelDelta += delta;
	}
	return CallNextHookEx(g_hMouseHook, nCode, wParam, lParam);
}
// --------------------------------------------------

double OffsetYPR(float f, float f2)
{
	f -= f2;
	if (f < -180) f += 360;
	else if (f > 180) f -= 360;
	return f;
}

void MouseToYawPitch()
{
	POINT mousePos;
	if (firstCP) { SetCursorPos(m_HalfWidth, m_HalfHeight); firstCP = false; }
	GetCursorPos(&mousePos);

	mouseYaw += (mousePos.x - m_HalfWidth) * mouseSensetiveYaw * -1;
	if (mouseYaw < -180) mouseYaw += 360;
	else if (mouseYaw > 180) mouseYaw -= 360;
	mouseHMD.Yaw = mouseYaw;

	mousePitch += (mousePos.y - m_HalfHeight) * mouseSensetivePitch * -1;
	
	// REMOVED THE WALL: Only clamp the camera to 90 degrees if we are NOT rotating hands!
	if (!LeftHandIsRotating && !RightHandIsRotating) {
		if (mousePitch > 90) mousePitch = 90;
		else if (mousePitch < -90) mousePitch = -90;
	}
	mouseHMD.Pitch = mousePitch;

	SetCursorPos(m_HalfWidth, m_HalfHeight);
}

void GetHMDData(__out THMD* HMD)
{
	if ((GetAsyncKeyState(KEY_ID_PLAYER_RISE_HIGHER) & 0x8000) != 0) HMDPosZ += StepPos;
	if ((GetAsyncKeyState(KEY_ID_PLAYER_RISE_LOWER) & 0x8000) != 0) HMDPosZ -= StepPos;
	if ((GetAsyncKeyState(KEY_ID_PLAYER_RISE_RESET) & 0x8000) != 0) HMDPosZ = 0;

	HMD->X = 0; HMD->Y = 0; HMD->Z = HMDPosZ - CrouchOffsetZ;
	HMD->Yaw = cameraHMD.Yaw * -1; HMD->Pitch = cameraHMD.Pitch * -1; HMD->Roll = 0;
}

void GetControllersData(__out TController* FirstController, __out TController* SecondController)
{
	FirstController->AxisX = 0; FirstController->AxisY = 0; FirstController->Buttons = 0; FirstController->Trigger = 0;
	SecondController->AxisX = 0; SecondController->AxisY = 0; SecondController->Buttons = 0; SecondController->Trigger = 0;
	SecondCtrlYPROffset[0] = 0; SecondCtrlYPROffset[1] = 0; SecondCtrlYPROffset[2] = 0;
	FirstController->Roll = 0;
	SecondController->Roll = 0;

	if (HeadsetWindowFocused || HalfLifeAlyxFocused)
	{
		if ((GetAsyncKeyState(KEY_ID_ROTATION_CONTROLLERS_RESET) & 0x8000) != 0) {
			CtrlsRoll = 0; CtrlsPitch = 0;
			LeftHandRotOffset[0] = LeftHandRotOffset[1] = LeftHandRotOffset[2] = 0.0;
			RightHandRotOffset[0] = RightHandRotOffset[1] = RightHandRotOffset[2] = 0.0;
			LeftXOffset = 0.0; LeftRotateYOffset = 0.0; LeftZOffset = 0.0;
			RightXOffset = 0.0; RightYOffset = 0.0; RightZOffset = 0.0;
		}
	}

	LeftHandYaw = 0; LeftHandPitch = 0; LeftAngleDist = 0; LeftRadDist = 0;
	RightHandYaw = 0; RightHandPitch = 0; RightAngleDist = 0; RightRadDist = 0;

	if ((GetAsyncKeyState(KEY_ID_CROUCH) & 0x8000) != 0) { HMDPosZ = 0; CrouchOffsetZ = 0.8; }
	else CrouchOffsetZ = 0;

	deltaPitch = mouseHMD.Yaw - oldPitch; oldPitch = mouseHMD.Yaw;
	deltaRoll = mouseHMD.Pitch - oldRoll; oldRoll = mouseHMD.Pitch;
	
	// Increased limits so you can flick the mouse as fast as you want
	if (deltaPitch > 360.0f) deltaPitch = 360.0f; if (deltaPitch < -360.0f) deltaPitch = -360.0f;
	if (deltaRoll > 360.0f) deltaRoll = 360.0f; if (deltaRoll < -360.0f) deltaRoll = -360.0f;

	if (LeftHandIsRotating) { LeftHandRotOffset[0] += deltaPitch; LeftHandRotOffset[1] += deltaRoll; }
	if (RightHandIsRotating) { RightHandRotOffset[0] += deltaPitch; RightHandRotOffset[1] += deltaRoll; }

	float dt = max((int)deltaTime.count(), 1) / 1000.0f;
	float currentSensitivity = HandMoveSensitivity;
	if ((GetAsyncKeyState(KEY_ID_FINE_TUNE) & 0x8000) != 0) currentSensitivity *= 0.1f;

	int wheelDelta = g_MouseWheelDelta.exchange(0);

	if (LeftHandIsRotating) {
		if ((GetAsyncKeyState(KEY_ID_MOTION_MODE_MOVE_UP) & 0x8000) != 0) LeftRotateYOffset += currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_MOTION_MODE_MOVE_DOWN) & 0x8000) != 0) LeftRotateYOffset -= currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_FORWARD) & 0x8000) != 0) LeftZOffset -= currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_BACK) & 0x8000) != 0) LeftZOffset += currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_LEFT) & 0x8000) != 0) LeftXOffset -= currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_RIGHT) & 0x8000) != 0) LeftXOffset += currentSensitivity * dt;

		// Left Hand Twist (Roll) via Mouse Wheel
		if (wheelDelta != 0) {
			LeftHandRotOffset[2] += wheelDelta * 0.05f; // 0.05 sensitivity per notch
		}
	}
	if (RightHandIsRotating) {
		if ((GetAsyncKeyState(KEY_ID_MOTION_MODE_MOVE_UP) & 0x8000) != 0) RightYOffset += currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_MOTION_MODE_MOVE_DOWN) & 0x8000) != 0) RightYOffset -= currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_FORWARD) & 0x8000) != 0) RightZOffset -= currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_BACK) & 0x8000) != 0) RightZOffset += currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_LEFT) & 0x8000) != 0) RightXOffset -= currentSensitivity * dt;
		if ((GetAsyncKeyState(KEY_ID_HAND_RIGHT) & 0x8000) != 0) RightXOffset += currentSensitivity * dt;

		// Right Hand Twist (Roll) via Mouse Wheel
		if (wheelDelta != 0) {
			RightHandRotOffset[2] += wheelDelta * 0.05f;
		}
	}

	// Removed the 0.4 limit so you can stretch your arms out infinitely
	if ((GetAsyncKeyState(KEY_ID_MOVE_CONTROLLERS_FORWARD) & 0x8000) != 0) HandRadDist += 0.005;
	if ((GetAsyncKeyState(KEY_ID_MOVE_CONTROLLERS_BACK) & 0x8000) != 0) HandRadDist -= 0.005;
	if ((GetAsyncKeyState(KEY_ID_MOVE_CONTROLLERS_FORWARD) & 0x8000) == 0 && (GetAsyncKeyState(KEY_ID_MOVE_CONTROLLERS_BACK) & 0x8000) == 0) HandRadDist = 0;

	FirstCtrlPos[0] = (0.4 - LeftRadDist + HandRadDist) * sin(DegToRad(OffsetYPR(OffsetYPR(cameraHMD.Yaw + 180, -17), LeftAngleDist)));
	FirstCtrlPos[1] = (0.4 - LeftRadDist + HandRadDist) * cos(DegToRad(OffsetYPR(OffsetYPR(cameraHMD.Yaw + 180, -17), LeftAngleDist)));
	FirstCtrlPos[2] = cameraHMD.Pitch * ZPosSensetive - CrouchOffsetZ + HMDPosZ;

	double yawRad = DegToRad(cameraHMD.Yaw);
	double moveX_L = LeftXOffset * cos(yawRad) + LeftZOffset * sin(yawRad);
	double moveZ_L = -LeftXOffset * sin(yawRad) + LeftZOffset * cos(yawRad);

	FirstController->X = FirstCtrlPos[0] + moveX_L;
	FirstController->Y = FirstCtrlPos[1] + moveZ_L;
	FirstController->Z = FirstCtrlPos[2] + LeftRotateYOffset - 0.15;

	SecondCtrlPos[0] = (0.4 - RightRadDist + HandRadDist) * sin(DegToRad(OffsetYPR(OffsetYPR(cameraHMD.Yaw + 180, 17), RightAngleDist)));
	SecondCtrlPos[1] = (0.4 - RightRadDist + HandRadDist) * cos(DegToRad(OffsetYPR(OffsetYPR(cameraHMD.Yaw + 180, 17), RightAngleDist)));
	SecondCtrlPos[2] = cameraHMD.Pitch * ZPosSensetive - CrouchOffsetZ + HMDPosZ;

	double moveX_R = RightXOffset * cos(yawRad) + RightZOffset * sin(yawRad);
	double moveZ_R = -RightXOffset * sin(yawRad) + RightZOffset * cos(yawRad);

	SecondController->X = SecondCtrlPos[0] + moveX_R;
	SecondController->Y = SecondCtrlPos[1] + moveZ_R;
	SecondController->Z = SecondCtrlPos[2] + RightYOffset - 0.15;

	// --- OCULUS TOUCH SPECIFIC ADJUSTMENTS ---
	if (ControllerType == "oculus_touch") {
		double handHeight = 0.05f;
		FirstController->Z += handHeight;
		SecondController->Z += handHeight;

		double handSeparation = 0.08f;

		FirstController->X -= handSeparation * cos(yawRad);
		FirstController->Y += handSeparation * sin(yawRad);
		SecondController->X += handSeparation * cos(yawRad);
		SecondController->Y -= handSeparation * sin(yawRad);
	}

	FirstController->Yaw = OffsetYPR(cameraHMD.Yaw, LeftHandYaw);
	FirstController->Pitch = OffsetYPR(OffsetYPR(cameraHMD.Pitch, OffsetPitchByDefault), LeftHandPitch);
	SecondController->Yaw = OffsetYPR(OffsetYPR(cameraHMD.Yaw, SecondCtrlYPROffset[0]), RightHandYaw);
	SecondController->Pitch = OffsetYPR(OffsetYPR(OffsetYPR(cameraHMD.Pitch, OffsetPitchByDefault), SecondCtrlYPROffset[1]), RightHandPitch);

	if ((GetAsyncKeyState(KEY_ID_LEFT_GRIP) & 0x8000) != 0) FirstController->Buttons |= GRIP_BTN;
	if ((GetAsyncKeyState(KEY_ID_LEFT_MENU) & 0x8000) != 0) FirstController->Buttons |= MENU_BTN;
	if ((GetAsyncKeyState(KEY_ID_LEFT_TRIGGER) & 0x8000) != 0) FirstController->Trigger = 1;
	if ((GetAsyncKeyState(KEY_ID_LEFT_SYSTEM) & 0x8000) != 0) FirstController->Buttons |= SYS_BTN;

	// --- LEFT TRACKPAD LOGIC (Arrows + F Click) ---
	bool leftPadActive = false;
	if ((GetAsyncKeyState(KEY_ID_UP) & 0x8000) != 0) { FirstController->AxisY = 1; leftPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_DOWN) & 0x8000) != 0) { FirstController->AxisY = -1; leftPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_LEFT) & 0x8000) != 0) { FirstController->AxisX = -1; leftPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_RIGHT) & 0x8000) != 0) { FirstController->AxisX = 1; leftPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_LEFT_TRACKPAD_CLICK) & 0x8000) != 0) leftPadActive = true;
	
	if (leftPadActive) FirstController->Buttons |= THUMB_BTN; // Force physical click

	// --- RIGHT TRACKPAD LOGIC (B/N + Space + Middle Mouse + V Click) ---
	bool rightPadActive = false;
	if ((GetAsyncKeyState(KEY_ID_TURN_LEFT) & 0x8000) != 0) { SecondController->AxisX = -1; rightPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_TURN_RIGHT) & 0x8000) != 0) { SecondController->AxisX = 1; rightPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_TELEPORT) & 0x8000) != 0) { SecondController->AxisY = -1; rightPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_CHANGE_WEAPON) & 0x8000) != 0) { SecondController->AxisY = 1; rightPadActive = true; }
	if ((GetAsyncKeyState(KEY_ID_RIGHT_TRACKPAD_CLICK) & 0x8000) != 0) rightPadActive = true;

	if (rightPadActive) SecondController->Buttons |= THUMB_BTN; // Force physical click

	// Other buttons
	if ((GetAsyncKeyState(KEY_ID_RIGHT_MENU) & 0x8000) != 0) SecondController->Buttons |= MENU_BTN;
	if ((GetAsyncKeyState(KEY_ID_RIGHT_GRIP) & 0x8000) != 0) SecondController->Buttons |= GRIP_BTN;
	if ((GetAsyncKeyState(KEY_ID_RIGHT_SYSTEM) & 0x8000) != 0) SecondController->Buttons |= SYS_BTN;
	if ((GetAsyncKeyState(KEY_ID_RIGHT_TRIGGER2) & 0x8000) != 0) SecondController->Trigger = 1;
	if ((GetAsyncKeyState(KEY_ID_RIGHT_TRIGGER) & 0x8000) != 0) SecondController->Trigger = 1;

	// Oculus Touch exclusive inputs
	if (ControllerType == "oculus_touch") {
		FirstController->ButtonA = (GetAsyncKeyState(KEY_ID_LEFT_A) & 0x8000) != 0;
		FirstController->ButtonB = (GetAsyncKeyState(KEY_ID_LEFT_B) & 0x8000) != 0;
		SecondController->ButtonA = (GetAsyncKeyState(KEY_ID_RIGHT_A) & 0x8000) != 0;
		SecondController->ButtonB = (GetAsyncKeyState(KEY_ID_RIGHT_B) & 0x8000) != 0;

		FirstController->GripValue = (GetAsyncKeyState(KEY_ID_LEFT_GRIP) & 0x8000) != 0 ? 1.0f : 0.0f;
		SecondController->GripValue = (GetAsyncKeyState(KEY_ID_RIGHT_GRIP) & 0x8000) != 0 ? 1.0f : 0.0f;

		if ((GetAsyncKeyState(KEY_ID_STICK_CLICK_LEFT) & 0x8000) != 0) FirstController->Buttons |= THUMB_BTN;
		if ((GetAsyncKeyState(KEY_ID_STICK_CLICK_RIGHT) & 0x8000) != 0) SecondController->Buttons |= THUMB_BTN;
		if ((GetAsyncKeyState(KEY_ID_MENU_CLICK) & 0x8000) != 0) FirstController->Buttons |= MENU_BTN;
	}

	if (HeadsetWindowFocused == false && HalfLifeAlyxFocused == false)
	{
		FirstController->Buttons = 0; FirstController->Trigger = 0;
		FirstController->AxisX = 0; FirstController->AxisY = 0;
		FirstController->ButtonA = FirstController->ButtonB = false;
		FirstController->GripValue = 0.0f;

		SecondController->Buttons = 0; SecondController->Trigger = 0;
		SecondController->AxisX = 0; SecondController->AxisY = 0;
		SecondController->ButtonA = SecondController->ButtonB = false;
		SecondController->GripValue = 0.0f;
	}

	FirstController->Yaw = OffsetYPR(FirstController->Yaw, LeftHandRotOffset[0]);
	FirstController->Pitch = OffsetYPR(FirstController->Pitch, LeftHandRotOffset[1]);
	FirstController->Roll = OffsetYPR(FirstController->Roll, LeftHandRotOffset[2]);
	SecondController->Yaw = OffsetYPR(SecondController->Yaw, RightHandRotOffset[0]);
	SecondController->Pitch = OffsetYPR(SecondController->Pitch, RightHandRotOffset[1]);
	SecondController->Roll = OffsetYPR(SecondController->Roll, RightHandRotOffset[2]);

	FirstController->Yaw = FirstController->Yaw * -1; FirstController->Pitch = FirstController->Pitch * -1;
	SecondController->Yaw = SecondController->Yaw * -1; SecondController->Pitch = SecondController->Pitch * -1;
}

inline vr::HmdQuaternion_t EulerAngleToQuaternion(double Yaw, double Pitch, double Roll)
{
	vr::HmdQuaternion_t q;
	double cy = cos(Yaw * 0.5); double sy = sin(Yaw * 0.5); double cp = cos(Pitch * 0.5);
	double sp = sin(Pitch * 0.5); double cr = cos(Roll * 0.5); double sr = sin(Roll * 0.5);
	q.w = cr * cp * cy + sr * sp * sy; q.x = sr * cp * cy - cr * sp * sy;
	q.y = cr * sp * cy + sr * cp * sy; q.z = cr * cp * sy - sr * sp * cy;
	return q;
}

class CSampleDeviceDriver : public vr::ITrackedDeviceServerDriver, public vr::IVRDisplayComponent
{
public:
	CSampleDeviceDriver() {
		HMDIndex_t = vr::k_unTrackedDeviceIndexInvalid;
		m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;
		m_flIPD = vr::VRSettings()->GetFloat(k_pch_SteamVR_Section, k_pch_SteamVR_IPD_Float);
		char buf[1024];
		vr::VRSettings()->GetString(k_pch_Sample_Section, k_pch_Sample_SerialNumber_String, buf, sizeof(buf)); m_sSerialNumber = buf;
		vr::VRSettings()->GetString(k_pch_Sample_Section, k_pch_Sample_ModelNumber_String, buf, sizeof(buf)); m_sModelNumber = buf;
		m_nWindowX = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowX_Int32); m_nWindowY = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowY_Int32);
		m_nWindowWidth = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowWidth_Int32); m_nWindowHeight = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_WindowHeight_Int32);
		if (m_nWindowWidth == 0) m_nWindowWidth = GetSystemMetrics(SM_CXSCREEN); if (m_nWindowHeight == 0) m_nWindowHeight = GetSystemMetrics(SM_CYSCREEN);
		m_nRenderWidth = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_RenderWidth_Int32); m_nRenderHeight = vr::VRSettings()->GetInt32(k_pch_Sample_Section, k_pch_Sample_RenderHeight_Int32);
		m_flSecondsFromVsyncToPhotons = vr::VRSettings()->GetFloat(k_pch_Sample_Section, k_pch_Sample_SecondsFromVsyncToPhotons_Float); m_flDisplayFrequency = vr::VRSettings()->GetFloat(k_pch_Sample_Section, k_pch_Sample_DisplayFrequency_Float);
		m_fZoomWidth = vr::VRSettings()->GetFloat(k_pch_Sample_Section, k_pch_Sample_ZoomWidth_Float); m_fZoomHeight = vr::VRSettings()->GetFloat(k_pch_Sample_Section, k_pch_Sample_ZoomHeight_Float);
		m_fFOV = (vr::VRSettings()->GetFloat(k_pch_Sample_Section, k_pch_Sample_FOV_Float) * 3.14159265358979323846 / 180);
		m_bStereoMode = vr::VRSettings()->GetBool(k_pch_Sample_Section, k_pch_Sample_Stereo_Bool); m_bDebugMode = vr::VRSettings()->GetBool(k_pch_Sample_Section, k_pch_Sample_DebugMode_Bool);
	}
	virtual ~CSampleDeviceDriver() {}
	virtual EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId) {
		HMDIndex_t = unObjectId; m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(HMDIndex_t);
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, m_sModelNumber.c_str());
		vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserIpdMeters_Float, 0.0f);
		vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserHeadToEyeDepthMeters_Float, 0.f);
		vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DisplayFrequency_Float, m_flDisplayFrequency);
		vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_SecondsFromVsyncToPhotons_Float, m_flSecondsFromVsyncToPhotons);
		vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DisplayDebugMode_Bool, m_bDebugMode);
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, m_sSerialNumber.c_str());
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_WillDriftInYaw_Bool, false);
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_TrackingFirmwareVersion_String, "1.0");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_HardwareRevision_String, "1.0");
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DeviceIsWireless_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DeviceIsCharging_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_CanUnifyCoordinateSystemWithHmd_Bool, true);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_ContainsProximitySensor_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DeviceCanPowerOff_Bool, false);
		vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_DeviceClass_Int32, vr::TrackedDeviceClass_HMD);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, vr::Prop_HasCamera_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, vr::Prop_Firmware_ForceUpdateRequired_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, vr::Prop_NeverTracked_Bool, false);
		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, vr::Prop_Identifiable_Bool, false);

		// Headset Spoofing Logic
		if (ControllerType == "oculus_touch") {
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ManufacturerName_String, "Oculus");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RegisteredDeviceType_String, "oculus/Facebook");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, "oculus_cv1_headset");
		} else {
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ManufacturerName_String, "HTC");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RegisteredDeviceType_String, "HMD");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());
		}
		return VRInitError_None;
	}
	virtual void Deactivate() { HMDIndex_t = vr::k_unTrackedDeviceIndexInvalid; }
	virtual void EnterStandby() {}
	void* GetComponent(const char* pchComponentNameAndVersion) { if (!_stricmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version)) return (vr::IVRDisplayComponent*)this; return NULL; }
	virtual void PowerOff() {}
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) { if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0; }
	virtual void GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) { *pnX = m_nWindowX; *pnY = m_nWindowY; *pnWidth = m_nWindowWidth; *pnHeight = m_nWindowHeight; }
	virtual bool IsDisplayOnDesktop() { return true; } virtual bool IsDisplayRealDisplay() { return true; }
	virtual void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight) { *pnWidth = m_nRenderWidth; *pnHeight = m_nRenderHeight; }
	
	virtual void GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight) {
		if (m_bStereoMode) {
			*pnY = 0; 
			*pnWidth = m_nWindowWidth / 2; 
			*pnHeight = m_nWindowHeight;
			if (eEye == Eye_Left) *pnX = 0;
			else *pnX = (m_nWindowWidth / 2);
		}
		else {
			*pnY = 0; 
			*pnWidth = m_nWindowWidth; 
			*pnHeight = m_nWindowHeight;
			
			if (eEye == Eye_Left) *pnX = 0;
			else *pnX = m_nWindowWidth; 
		}
	}
	
	virtual void GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom) {
		if (m_bStereoMode) { *pfLeft = -m_fFOV; *pfRight = m_fFOV; *pfTop = -m_fFOV; *pfBottom = m_fFOV; }
		else { *pfLeft = (float)m_nRenderWidth / m_nRenderHeight * -1; *pfRight = (float)m_nRenderWidth / m_nRenderHeight; *pfTop = -1.0; *pfBottom = 1.0; }
	}
	virtual DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV) {
		DistortionCoordinates_t coordinates; float hX, hY; double rr, r2, theta;
		rr = sqrt((fU - 0.5f) * (fU - 0.5f) + (fV - 0.5f) * (fV - 0.5f)); r2 = rr * (1 + 0 * (rr * rr) + 0 * (rr * rr * rr * rr));
		theta = atan2(fU - 0.5f, fV - 0.5f); hX = sin(theta) * r2 * m_fZoomWidth; hY = cos(theta) * r2 * m_fZoomHeight;
		coordinates.rfBlue[0] = hX + 0.5f; coordinates.rfBlue[1] = hY + 0.5f; coordinates.rfGreen[0] = hX + 0.5f; coordinates.rfGreen[1] = hY + 0.5f;
		coordinates.rfRed[0] = hX + 0.5f; coordinates.rfRed[1] = hY + 0.5f; return coordinates;
	}
	virtual DriverPose_t GetPose() {
		DriverPose_t pose = { 0 }; if (HMDConnected) { pose.poseIsValid = true; pose.result = TrackingResult_Running_OK; pose.deviceIsConnected = true; }
		else { pose.poseIsValid = false; pose.result = TrackingResult_Uninitialized; pose.deviceIsConnected = false; }
		pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0); pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);
		if (HMDConnected) { GetHMDData(&MyHMD); pose.qRotation = EulerAngleToQuaternion(DegToRad(MyHMD.Roll), DegToRad(-MyHMD.Yaw), DegToRad(-MyHMD.Pitch)); pose.vecPosition[0] = MyHMD.X; pose.vecPosition[1] = MyHMD.Z; pose.vecPosition[2] = MyHMD.Y; }
		pose.shouldApplyHeadModel = true; pose.poseTimeOffset = 0; pose.willDriftInYaw = false; return pose;
	}
	void RunFrame() { if (HMDIndex_t != vr::k_unTrackedDeviceIndexInvalid) vr::VRServerDriverHost()->TrackedDevicePoseUpdated(HMDIndex_t, GetPose(), sizeof(DriverPose_t)); }
	std::string GetSerialNumber() const { return m_sSerialNumber; }
private:
	vr::TrackedDeviceIndex_t HMDIndex_t; vr::PropertyContainerHandle_t m_ulPropertyContainer;
	std::string m_sSerialNumber, m_sModelNumber;
	int32_t m_nWindowX, m_nWindowY, m_nWindowWidth, m_nWindowHeight, m_nRenderWidth, m_nRenderHeight;
	float m_flSecondsFromVsyncToPhotons, m_flDisplayFrequency, m_flIPD, m_fZoomWidth, m_fZoomHeight, m_fFOV;
	bool m_bStereoMode, m_bDebugMode;
};

class CSampleControllerDriver : public vr::ITrackedDeviceServerDriver
{
	int32_t ControllerIndex;
public:
	CSampleControllerDriver() { Ctrl1Index_t = vr::k_unTrackedDeviceIndexInvalid; Ctrl2Index_t = vr::k_unTrackedDeviceIndexInvalid; m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer; }
	virtual void SetControllerIndex(int32_t CtrlIndex) { ControllerIndex = CtrlIndex; }
	virtual ~CSampleControllerDriver() {}

	virtual EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId)
	{
		switch (ControllerIndex) {
		case 1: Ctrl1Index_t = unObjectId; m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(Ctrl1Index_t); vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, "CTRL1Serial"); vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_LeftHand); break;
		case 2: Ctrl2Index_t = unObjectId; m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(Ctrl2Index_t); vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, "CTRL2Serial"); vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_RightHand); break;
		}

		vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false);

		if (ControllerType == "oculus_touch") {
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "oculus_touch");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Oculus Touch");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "Oculus");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, ControllerIndex == 1 ? "oculus_cv1_controller_left" : "oculus_cv1_controller_right");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_TrackingSystemName_String, "Oculus");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_InputProfilePath_String, "{oculus}/input/touch_profile.json");
			vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, vr::Prop_SupportedButtons_Uint64, 0xFFFFFFFF);

			vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/joystick/x", &HAnalog[0], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
			vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/joystick/y", &HAnalog[1], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
			vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trigger/value", &HAnalog[2], vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedOneSided);
			vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/grip/value", &HAnalog[3], vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedOneSided);

			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/a/click", &HButtons[0]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/b/click", &HButtons[1]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/x/click", &HButtons[2]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/y/click", &HButtons[3]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/joystick/click", &HButtons[4]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/menu/click", &HButtons[5]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/click", &HButtons[6]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trigger/click", &HButtons[7]);
		} else {
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "vive_controller");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "ViveMV");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "HTC");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "vr_controller_vive_1_5");
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_TrackingSystemName_String, "VR Controller");
			vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, vr::Prop_SupportedButtons_Uint64, 0xA4);
			vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_InputProfilePath_String, "{htc}/input/controller_profile.json");

			vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/x", &HAnalog[0], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
			vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/y", &HAnalog[1], vr::VRScalarType_Absolute, vr::VRScalarUnits_NormalizedTwoSided);
			vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trigger/value", &HAnalog[2], vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedOneSided);

			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/application_menu/click", &HButtons[0]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/click", &HButtons[2]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/grip/click", &HButtons[1]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trigger/click", &HButtons[5]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/click", &HButtons[3]);
			vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/touch", &HButtons[4]);
		}

		vr::VRDriverInput()->CreateHapticComponent(m_ulPropertyContainer, "/output/haptic", &m_compHaptic);
		return VRInitError_None;
	}

	virtual void Deactivate() { switch (ControllerIndex) { case 1: Ctrl1Index_t = vr::k_unTrackedDeviceIndexInvalid; break; case 2: Ctrl2Index_t = vr::k_unTrackedDeviceIndexInvalid; break; } }
	virtual void EnterStandby() {}
	void* GetComponent(const char* pchComponentNameAndVersion) { return NULL; }
	virtual void PowerOff() {}
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) { if (unResponseBufferSize >= 1) pchResponseBuffer[0] = 0; }

	virtual DriverPose_t GetPose()
	{
		DriverPose_t pose = { 0 }; pose.poseIsValid = true; pose.result = TrackingResult_Running_OK; pose.deviceIsConnected = true;
		pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0); pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);
		if (ControllerIndex == 1) {
			pose.vecPosition[0] = MyCtrl.X; pose.vecPosition[1] = MyCtrl.Z; pose.vecPosition[2] = MyCtrl.Y;
			pose.vecVelocity[0] = (pose.vecPosition[0] - FirstCtrlLastPos[0]) * 1000 / max((int)deltaTime.count(), 1) / 3; pose.vecVelocity[1] = (pose.vecPosition[1] - FirstCtrlLastPos[1]) * 1000 / max((int)deltaTime.count(), 1) / 3; pose.vecVelocity[2] = (pose.vecPosition[2] - FirstCtrlLastPos[2]) * 1000 / max((int)deltaTime.count(), 1) / 3;
			FirstCtrlLastPos[0] = pose.vecPosition[0]; FirstCtrlLastPos[1] = pose.vecPosition[1]; FirstCtrlLastPos[2] = pose.vecPosition[2];
			pose.qRotation = EulerAngleToQuaternion(DegToRad(MyCtrl.Roll), DegToRad(-MyCtrl.Yaw), DegToRad(-MyCtrl.Pitch));
		}
		else {
			pose.vecPosition[0] = MyCtrl2.X; pose.vecPosition[1] = MyCtrl2.Z; pose.vecPosition[2] = MyCtrl2.Y;
			pose.vecVelocity[0] = (pose.vecPosition[0] - SecondCtrlLastPos[0]) * 1000 / max((int)deltaTime.count(), 1) / 3; pose.vecVelocity[1] = (pose.vecPosition[1] - SecondCtrlLastPos[1]) * 1000 / max((int)deltaTime.count(), 1) / 3; pose.vecVelocity[2] = (pose.vecPosition[2] - SecondCtrlLastPos[2]) * 1000 / max((int)deltaTime.count(), 1) / 3;
			SecondCtrlLastPos[0] = pose.vecPosition[0]; SecondCtrlLastPos[1] = pose.vecPosition[1]; SecondCtrlLastPos[2] = pose.vecPosition[2];
			pose.qRotation = EulerAngleToQuaternion(DegToRad(MyCtrl2.Roll), DegToRad(-MyCtrl2.Yaw), DegToRad(-MyCtrl2.Pitch));
		}
		return pose;
	}

	void RunFrame()
	{
		switch (ControllerIndex) {
		case 1: if (Ctrl1Index_t != vr::k_unTrackedDeviceIndexInvalid) vr::VRServerDriverHost()->TrackedDevicePoseUpdated(Ctrl1Index_t, GetPose(), sizeof(DriverPose_t)); break;
		case 2: if (Ctrl2Index_t != vr::k_unTrackedDeviceIndexInvalid) vr::VRServerDriverHost()->TrackedDevicePoseUpdated(Ctrl2Index_t, GetPose(), sizeof(DriverPose_t)); break;
		}

		if (ControllerIndex == 1) {
			vr::VRDriverInput()->UpdateScalarComponent(HAnalog[0], MyCtrl.AxisX, 0);
			vr::VRDriverInput()->UpdateScalarComponent(HAnalog[1], MyCtrl.AxisY, 0);
			vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], MyCtrl.Trigger, 0);

			if (ControllerType == "oculus_touch") {
				vr::VRDriverInput()->UpdateScalarComponent(HAnalog[3], MyCtrl.GripValue, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[4], (MyCtrl.Buttons & THUMB_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[5], (MyCtrl.Buttons & MENU_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[6], (MyCtrl.Buttons & SYS_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[7], MyCtrl.Trigger > 0.1f, 0);

				// LEFT: X and Y only
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[2], MyCtrl.ButtonA, 0);   // X
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[3], MyCtrl.ButtonB, 0);   // Y
			}
			else {
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[0], (MyCtrl.Buttons & MENU_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[1], (MyCtrl.Buttons & GRIP_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[2], (MyCtrl.Buttons & SYS_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[3], (MyCtrl.Buttons & THUMB_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[4], (((MyCtrl.AxisX != 0 || MyCtrl.AxisY != 0) || (MyCtrl.Buttons & THUMB_BTN) != 0) ? true : false), 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[5], MyCtrl.Trigger > 0.1f, 0);
			}
		}
		else {
			vr::VRDriverInput()->UpdateScalarComponent(HAnalog[0], MyCtrl2.AxisX, 0);
			vr::VRDriverInput()->UpdateScalarComponent(HAnalog[1], MyCtrl2.AxisY, 0);
			vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], MyCtrl2.Trigger, 0);

			if (ControllerType == "oculus_touch") {
				vr::VRDriverInput()->UpdateScalarComponent(HAnalog[3], MyCtrl2.GripValue, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[4], (MyCtrl2.Buttons & THUMB_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[5], (MyCtrl2.Buttons & MENU_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[6], (MyCtrl2.Buttons & SYS_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[7], MyCtrl2.Trigger > 0.1f, 0);

				// RIGHT: A and B only
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[0], MyCtrl2.ButtonA, 0);  // A
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[1], MyCtrl2.ButtonB, 0);  // B
			}
			else {
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[0], (MyCtrl2.Buttons & MENU_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[1], (MyCtrl2.Buttons & GRIP_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[2], (MyCtrl2.Buttons & SYS_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[3], (MyCtrl2.Buttons & THUMB_BTN) != 0, 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[4], (((MyCtrl2.AxisX != 0 || MyCtrl2.AxisY != 0) || (MyCtrl2.Buttons & THUMB_BTN) != 0) ? true : false), 0);
				vr::VRDriverInput()->UpdateBooleanComponent(HButtons[5], MyCtrl2.Trigger > 0.1f, 0);
			}
		}
	}

	void ProcessEvent(const vr::VREvent_t& vrEvent) {}
	std::string GetSerialNumber() const { return (ControllerIndex == 1) ? "CTRL1Serial" : "CTRL2Serial"; }

private:
	vr::TrackedDeviceIndex_t Ctrl1Index_t, Ctrl2Index_t;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;
	vr::VRInputComponentHandle_t m_compHaptic;
	vr::VRInputComponentHandle_t HButtons[10];
	vr::VRInputComponentHandle_t HAnalog[5];
};

class CServerDriver_Sample : public IServerTrackedDeviceProvider
{
public:
	virtual EVRInitError Init(vr::IVRDriverContext* pDriverContext);
	virtual void Cleanup();
	virtual const char* const* GetInterfaceVersions() { return vr::k_InterfaceVersions; }
	virtual void RunFrame();
	virtual bool ShouldBlockStandbyMode() { return false; }
	virtual void EnterStandby() {}
	virtual void LeaveStandby() {}
private:
	CSampleDeviceDriver* m_pNullHmdLatest = nullptr;
	CSampleControllerDriver* m_pController = nullptr;
	CSampleControllerDriver* m_pController2 = nullptr;
};

CServerDriver_Sample g_serverDriverNull;

EVRInitError CServerDriver_Sample::Init(vr::IVRDriverContext* pDriverContext)
{
	VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
	CRegKey key; TCHAR steamPath[MAX_PATH]; LONG status = key.Open(HKEY_CURRENT_USER, _T("Software\\Valve\\Steam"));
	if (status == ERROR_SUCCESS) { ULONG regSize = sizeof(steamPath); status = key.QueryStringValue(_T("SteamPath"), steamPath, &regSize); }
	key.Close();

	TCHAR configPath[MAX_PATH] = { 0 };
	_tcscat_s(configPath, sizeof(configPath), steamPath);
	_tcscat_s(configPath, sizeof(configPath), _T("/steamapps/common/SteamVR/drivers/FlatVR/FlatVR.ini"));

	ctrlsConnected = true;
	CIniReader IniFile((char*)configPath);

	ControllerType = IniFile.ReadString("Main", "ControllerType", "vive_controller");

	KEY_ID_LEFT_TRIGGER = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_TRIGGER", "SHIFT"));
	KEY_ID_LEFT_MENU = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_MENU", "ENTER"));
	KEY_ID_LEFT_GRIP = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_GRIP", "ESCAPE"));
	KEY_ID_LEFT_SYSTEM = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_SYSTEM", "F9"));
	KEY_ID_RIGHT_TRIGGER = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_TRIGGER", "MOUSE-LEFT-BTN"));
	KEY_ID_RIGHT_TRIGGER2 = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_TRIGGER2", "NUMPAD1"));
	KEY_ID_RIGHT_MENU = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_MENU", "NUMPAD-DEL"));
	KEY_ID_RIGHT_GRIP = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_GRIP", "NUMPAD0"));
	KEY_ID_RIGHT_SYSTEM = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_SYSTEM", "4"));
	KEY_ID_CHANGE_WEAPON = KeyNameToKeyCode(IniFile.ReadString("Keys", "CHANGE_WEAPON", "MOUSE-MIDDLE-BTN"));
	KEY_ID_TELEPORT = KeyNameToKeyCode(IniFile.ReadString("Keys", "TELEPORT", "SPACE"));
	KEY_ID_MOVE_CONTROLLERS_FORWARD = KeyNameToKeyCode(IniFile.ReadString("Keys", "MOVE_CONTROLLERS_FORWARD", "INSERT"));
	KEY_ID_MOVE_CONTROLLERS_BACK = KeyNameToKeyCode(IniFile.ReadString("Keys", "MOVE_CONTROLLERS_BACK", "HOME"));
	KEY_ID_PLAYER_RISE_HIGHER = KeyNameToKeyCode(IniFile.ReadString("Keys", "PLAYER_RISE_HIGHER", "PAGE-UP"));
	KEY_ID_PLAYER_RISE_LOWER = KeyNameToKeyCode(IniFile.ReadString("Keys", "PLAYER_RISE_LOWER", "PAGE-DOWN"));
	KEY_ID_PLAYER_RISE_RESET = KeyNameToKeyCode(IniFile.ReadString("Keys", "PLAYER_RISE_RESET", "NUMPAD-MINUS"));
	KEY_ID_CROUCH = KeyNameToKeyCode(IniFile.ReadString("Keys", "CROUCH", "CTRL"));
	KEY_ID_LEFT_CTRL_MOTION = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_CTRL_MOTION", "NONE"));
	KEY_ID_ROTATION_CONTROLLERS_RESET = KeyNameToKeyCode(IniFile.ReadString("Keys", "ROTATION_CONTROLLERS_RESET", "Y"));
	KEY_ID_FINE_TUNE = KeyNameToKeyCode(IniFile.ReadString("Keys", "FINE_TUNE", "RCTRL"));
	KEY_ID_LEFT_HAND_ROTATE = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_HAND_ROTATE", "X"));
	KEY_ID_RIGHT_HAND_ROTATE = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_HAND_ROTATE", "C"));
	KEY_ID_HAND_FORWARD = KeyNameToKeyCode(IniFile.ReadString("Keys", "HAND_FORWARD", "W"));
	KEY_ID_HAND_BACK = KeyNameToKeyCode(IniFile.ReadString("Keys", "HAND_BACK", "Z"));
	KEY_ID_HAND_LEFT = KeyNameToKeyCode(IniFile.ReadString("Keys", "HAND_LEFT", "A"));
	KEY_ID_HAND_RIGHT = KeyNameToKeyCode(IniFile.ReadString("Keys", "HAND_RIGHT", "D"));
	KEY_ID_MOTION_MODE_MOVE_UP = KeyNameToKeyCode(IniFile.ReadString("Keys", "MOTION_MODE_MOVE_UP", "1"));
	KEY_ID_MOTION_MODE_MOVE_DOWN = KeyNameToKeyCode(IniFile.ReadString("Keys", "MOTION_MODE_MOVE_DOWN", "2"));
	KEY_ID_LEFT_TRACKPAD_CLICK = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_TRACKPAD_CLICK", "F"));
	KEY_ID_RIGHT_TRACKPAD_CLICK = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_TRACKPAD_CLICK", "V"));
	KEY_ID_TURN_LEFT = KeyNameToKeyCode(IniFile.ReadString("Keys", "TURN_LEFT", "B"));
	KEY_ID_TURN_RIGHT = KeyNameToKeyCode(IniFile.ReadString("Keys", "TURN_RIGHT", "N"));
	KEY_ID_UP = KeyNameToKeyCode(IniFile.ReadString("Keys", "UP", "UP"));
	KEY_ID_DOWN = KeyNameToKeyCode(IniFile.ReadString("Keys", "DOWN", "DOWN"));
	KEY_ID_LEFT = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT", "LEFT"));
	KEY_ID_RIGHT = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT", "RIGHT"));

	// Oculus specific
	KEY_ID_LEFT_A = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_A", "Z"));
	KEY_ID_LEFT_B = KeyNameToKeyCode(IniFile.ReadString("Keys", "LEFT_B", "X"));
	KEY_ID_RIGHT_A = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_A", "C"));
	KEY_ID_RIGHT_B = KeyNameToKeyCode(IniFile.ReadString("Keys", "RIGHT_B", "V"));
	KEY_ID_STICK_CLICK_LEFT = KeyNameToKeyCode(IniFile.ReadString("Keys", "STICK_CLICK_LEFT", "C"));
	KEY_ID_STICK_CLICK_RIGHT = KeyNameToKeyCode(IniFile.ReadString("Keys", "STICK_CLICK_RIGHT", "V"));
	KEY_ID_MENU_CLICK = KeyNameToKeyCode(IniFile.ReadString("Keys", "MENU_CLICK", "TAB"));

	mouseSensetiveYaw = IniFile.ReadFloat("Main", "AxisX", 0.05);
	mouseSensetivePitch = IniFile.ReadFloat("Main", "AxisY", 0.04);
	ZPosSensetive = IniFile.ReadFloat("Main", "AxisZ", 0.009);
	HandMoveSensitivity = IniFile.ReadFloat("Main", "HandMoveSensitivity", 0.5f);
	VRMode = IniFile.ReadBoolean("Main", "VRMode", false);
	VRAimingAngleOffset = IniFile.ReadFloat("Main", "VRAimingAngleOffset", 0);
	m_HalfWidth = GetSystemMetrics(SM_CXSCREEN) / 2;
	m_HalfHeight = GetSystemMetrics(SM_CYSCREEN) / 2;
	HMDConnected = IniFile.ReadBoolean("Main", "HMD", true);

	// Initialize the low-level mouse hook for scroll wheel on a dedicated message thread
	g_HookThread = std::thread([]() {
		g_HookThreadId = GetCurrentThreadId();
		HMODULE hMod = NULL;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)reinterpret_cast<void*>(MouseHookProc), &hMod);
		g_hMouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, hMod, 0);

		if (g_hMouseHook) {
			MSG msg;
			// Keep the thread alive with a message loop so Windows doesn't kill the hook
			while (GetMessage(&msg, NULL, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			UnhookWindowsHookEx(g_hMouseHook);
			g_hMouseHook = NULL;
		}
	});
	g_HookThread.detach();

	if (HMDConnected) {
		m_pNullHmdLatest = new CSampleDeviceDriver();
		vr::VRServerDriverHost()->TrackedDeviceAdded(m_pNullHmdLatest->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, m_pNullHmdLatest);
	}
	if (ctrlsConnected) {
		m_pController = new CSampleControllerDriver(); m_pController->SetControllerIndex(1); vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController);
		m_pController2 = new CSampleControllerDriver(); m_pController2->SetControllerIndex(2); vr::VRServerDriverHost()->TrackedDeviceAdded(m_pController2->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pController2);
	}
	return VRInitError_None;
}

void CServerDriver_Sample::Cleanup()
{
	if (g_HookThreadId != 0) {
		PostThreadMessage(g_HookThreadId, WM_QUIT, 0, 0);
		g_HookThreadId = 0;
	}
}

void CServerDriver_Sample::RunFrame()
{
	static milliseconds lastMillis = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	deltaTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()) - lastMillis;
	lastMillis = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	SteamVRHeadsetWindow = FindWindow(NULL, "Headset Window");
	HalfLifeAlyxWindow = FindWindow(NULL, "Half-Life: Alyx");

	if ((GetAsyncKeyState(VK_F1) & 0x8000) != 0 || (GetAsyncKeyState(VK_F7) & 0x8000) != 0) SetForegroundWindow(SteamVRHeadsetWindow);

	HeadsetWindowFocused = SteamVRHeadsetWindow != 0 && SteamVRHeadsetWindow == GetForegroundWindow() && IsWindowVisible(SteamVRHeadsetWindow);
	HalfLifeAlyxFocused = HalfLifeAlyxWindow != 0 && HalfLifeAlyxWindow == GetForegroundWindow() && IsWindowVisible(HalfLifeAlyxWindow);

	if (HeadsetWindowFocused || HalfLifeAlyxFocused) {
		bool lr = (GetAsyncKeyState(KEY_ID_LEFT_HAND_ROTATE) & 0x8000) != 0;
		bool rr = (GetAsyncKeyState(KEY_ID_RIGHT_HAND_ROTATE) & 0x8000) != 0;
		
		if (lr && !PrevLeftRotateKey) {
			LeftHandIsRotating = !LeftHandIsRotating;
			if (!LeftHandIsRotating) {
				// STOP THE SNAP: Reset mouse to camera's frozen state on exit
				mouseYaw = cameraHMD.Yaw;
				mousePitch = cameraHMD.Pitch;
				mouseHMD = cameraHMD;
				oldPitch = mouseHMD.Yaw;
				oldRoll = mouseHMD.Pitch;
			}
		}
		if (rr && !PrevRightRotateKey) {
			RightHandIsRotating = !RightHandIsRotating;
			if (!RightHandIsRotating) {
				// STOP THE SNAP: Reset mouse to camera's frozen state on exit
				mouseYaw = cameraHMD.Yaw;
				mousePitch = cameraHMD.Pitch;
				mouseHMD = cameraHMD;
				oldPitch = mouseHMD.Yaw;
				oldRoll = mouseHMD.Pitch;
			}
		}
		PrevLeftRotateKey = lr; PrevRightRotateKey = rr;
		
		MouseToYawPitch();
		if (!LeftHandIsRotating && !RightHandIsRotating) cameraHMD = mouseHMD;
	}

	if (m_pNullHmdLatest) m_pNullHmdLatest->RunFrame();
	if (ctrlsConnected) {
		GetControllersData(&MyCtrl, &MyCtrl2);
		if (m_pController) m_pController->RunFrame();
		if (m_pController2) m_pController2->RunFrame();
		vr::VREvent_t vrEvent;
		while (vr::VRServerDriverHost()->PollNextEvent(&vrEvent, sizeof(vrEvent))) {
			if (m_pController) m_pController->ProcessEvent(vrEvent);
			if (m_pController2) m_pController2->ProcessEvent(vrEvent);
		}
	}
}

HMD_DLL_EXPORT void* HmdDriverFactory(const char* pInterfaceName, int* pReturnCode)
{
	if (0 == strcmp(IServerTrackedDeviceProvider_Version, pInterfaceName)) return &g_serverDriverNull;
	if (pReturnCode) *pReturnCode = VRInitError_Init_InterfaceNotFound;
	return NULL;
}