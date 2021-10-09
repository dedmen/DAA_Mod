#if __A3_DEBUG__
diag_log ["DAA MPSETUP", serverName];
#endif

private _mods = (("true" configClasses (configFile >> "CfgPatches")) apply {configName _x}) select {(_x select [0,2] != "A3") && (_x select [0,11] != "CuratorOnly")};

DAA_ChatEH = addMissionEventHandler ["HandleChatMessage", {
	params ["_channel", "_owner", "_from", "_text", "_person", "_name", "_strID", "_forcedDisplay", "_isPlayerMessage", "_sentenceType", "_chatMessageType"];

#if __A3_DEBUG__
    diag_log ["DAA Chat", _this];
#endif

    // Admin and Zeus see all join/leave messages
    if (serverCommandAvailable "#logout" /* adminrights */ || !isNull getAssignedCuratorLogic player) exitWith {false};

    // Allow admin messages
    if ("Adminmessage " in _from || "Adminnachricht " in _from) exitWith {false};

    true
}];
