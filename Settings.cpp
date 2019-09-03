#include "Settings.h"
#include <Shlwapi.h>
#include <string>

#pragma comment(lib, "shlwapi.lib")

bool	Settings::bLockOn = true;
bool	Settings::bForceFirstPerson = true;
bool	Settings::bStopNPCMove = false;
bool	Settings::bSmoothTransition = true;
bool	Settings::bSwitchTarget = false;
bool	Settings::bForceThirdPerson = false;
bool	Settings::bHeadTracking = true;
bool	Settings::bConversationHT = true;
bool	Settings::bLetterBox = true;
int		Settings::uLetterBoxThickness = 175;
int		Settings::uLetterBoxSpeed = 5;
float	Settings::fCameraSpeed = 500.0f;
float	Settings::fAddOverShoulderPosX = 50.0f;
float	Settings::fAddOverShoulderPozZ = -5.0f;
float	Settings::fAddOverShoulderPosY = 0.0f;
float	Settings::f1stZoom = 50.0f;
float	Settings::f3rdZoom = 75.0f;
float	Settings::fDragonZoom = 125.0f;
float	Settings::fMessagePosX = -601.f;
float	Settings::fMessagePosY = -400.f;

std::wstring thisPath = L"";

void Settings::Load()
{
	TCHAR path[MAX_PATH];
	HMODULE hm;
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		L"AlternateConversationCamera.dll", &hm))
	{
		GetModuleFileName(hm, path, MAX_PATH);
		PathRemoveFileSpec(path);
		thisPath = std::wstring(path);
		if (!thisPath.empty() && thisPath.at(thisPath.length() - 1) != '\\'){
			thisPath += L"\\AlternateConversationCamera.ini";
		}
	}

	TCHAR posX[8];
	TCHAR posZ[8];
	TCHAR posY[8];
	TCHAR mesPosX[8];
	TCHAR mesPosY[8];

	Settings::bLockOn = GetPrivateProfileInt(L"Settings", L"bLockOn", 1, thisPath.c_str());
	Settings::bForceFirstPerson = GetPrivateProfileInt(L"Settings", L"bForceFirstPerson", 1, thisPath.c_str());
	Settings::bStopNPCMove = GetPrivateProfileInt(L"Settings", L"bStopNPCMovement", 0, thisPath.c_str());
	Settings::bSmoothTransition = GetPrivateProfileInt(L"Settings", L"bSmoothTransition", 1, thisPath.c_str());
	Settings::bSwitchTarget = GetPrivateProfileInt(L"Settings", L"bSwitchTarget", 0, thisPath.c_str());
	Settings::bForceThirdPerson = GetPrivateProfileInt(L"Settings", L"bForceThirdPerson", 0, thisPath.c_str());
	Settings::bHeadTracking = GetPrivateProfileInt(L"Settings", L"bHeadTracking", 1, thisPath.c_str());
	Settings::bConversationHT = GetPrivateProfileInt(L"Settings", L"bConversationHT", 1, thisPath.c_str());
	Settings::bLetterBox = GetPrivateProfileInt(L"Settings", L"bLetterBox", 1, thisPath.c_str());
	Settings::uLetterBoxThickness = GetPrivateProfileInt(L"Settings", L"uLetterBoxThickness", 175, thisPath.c_str());
	Settings::uLetterBoxSpeed = GetPrivateProfileInt(L"Settings", L"uLetterBoxSpeed", 5, thisPath.c_str());
	Settings::fCameraSpeed = GetPrivateProfileInt(L"Settings", L"fCameraSpeed", 500, thisPath.c_str());
	Settings::f1stZoom = GetPrivateProfileInt(L"Settings", L"u1stZoom", 50, thisPath.c_str());
	Settings::f3rdZoom = GetPrivateProfileInt(L"Settings", L"u3rdZoom", 75, thisPath.c_str());
	Settings::fDragonZoom = GetPrivateProfileInt(L"Settings", L"uDragonZoom", 125, thisPath.c_str());
	GetPrivateProfileString(L"Settings", L"iAddOverShoulderPosX", L"50", posX, 8, thisPath.c_str());
	GetPrivateProfileString(L"Settings", L"iAddOverShoulderPosZ", L"-5", posZ, 8, thisPath.c_str());
	GetPrivateProfileString(L"Settings", L"iAddOverShoulderPosY", L"0", posY, 8, thisPath.c_str());

	GetPrivateProfileString(L"Settings", L"iMessagePosX", L"-601", mesPosX, 8, thisPath.c_str());
	GetPrivateProfileString(L"Settings", L"iMessagePosY", L"-400", mesPosY, 8, thisPath.c_str());

	Settings::fAddOverShoulderPosX = (float)_wtoi(posX);
	Settings::fAddOverShoulderPozZ = (float)_wtoi(posZ);
	Settings::fAddOverShoulderPosY = (float)_wtoi(posY);
	Settings::fMessagePosX = (float)_wtoi(mesPosX);
	Settings::fMessagePosY = (float)_wtoi(mesPosY);
}
