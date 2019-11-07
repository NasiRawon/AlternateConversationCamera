#pragma once

struct Settings
{
	static float	fCameraSpeed;
	static float	fAddOverShoulderPosX;
	static float	fAddOverShoulderPozZ;
	static float	fAddOverShoulderPosY;
	static float	f1stZoom;
	static float	f3rdZoom;
	static float	fDragonZoom;
	static float	fMessagePosX;
	static float	fMessagePosY;
	static float	fHumanCamOffsetX;
	static float	fHumanCamOffsetY;
	static float	fHumanCamOffsetZ;
	static float	fCreatureCamOffsetX;
	static float	fCreatureCamOffsetY;
	static float	fCreatureCamOffsetZ;
	static int		uLetterBoxThickness;
	static int		uLetterBoxSpeed;
	static bool		bLockOn;
	static bool		bForceFirstPerson;
	static bool		bSmoothTransition;
	static bool		bSwitchTarget;
	static bool		bForceThirdPerson;
	static bool		bHeadTracking;
	static bool		bConversationHT;
	static bool		bLetterBox;
	static bool		bHideDialogueMenu;

	static void Load();
};
