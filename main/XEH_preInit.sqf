DAA_EXEH = addMissionEventHandler ["ExtensionCallback", DAA_fnc_extensionCallback];

if (!hasInterface) exitWith {};


#if __A3_DEBUG__
diag_log "DAA PREINIT";
#endif

// Enforce latest TFAR plugin version
["TFAR_ConfigRefresh", {
    // TFAR just updated its plugin config, shove in a new min version after the fact.
    // Next frame because there is a TFAR bug where it can process plugin messages in reverse order.
    // Meaning the version that the TFAR script sent first, could arrive after our message and overwrite ours.
    [{
        ["minimumPluginVersion", 328] call TFAR_fnc_setPluginSetting;
    }] call CBA_fnc_execNextFrame; 
}] call CBA_fnc_addEventHandler;

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


["refreshAdmin", {
    remoteExec ["DAA_reloadAdminList", -2];
    DAA_loadingAdminList = true;
}, "all"] call CBA_fnc_registerChatCommand;