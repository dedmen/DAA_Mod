// MP Slotting screen


#if __A3_DEBUG__
diag_log ["DAA MPSETUP", serverName];
#endif

DAA_ChatEH = addMissionEventHandler ["HandleChatMessage", {
	params ["_channel", "_owner", "_from", "_text", "_person", "_name", "_strID", "_forcedDisplay", "_isPlayerMessage", "_sentenceType", "_chatMessageType"];

#if __A3_DEBUG__
    diag_log ["DAA Chat", _this];
#endif

    // Admin see all messages
    if (serverCommandAvailable "#logout" /* adminrights */) exitWith {false};

    // Allow admin messages
    if ("Adminmessage " in _from || "Adminnachricht " in _from) exitWith {false};

    // In slotting screen block group channel
    if (_channel == 3) exitWith {true};

    // leave everything else through?
    false
}];
