



// Enforce latest TFAR plugin version
["TFAR_ConfigRefresh", {
    // TFAR just updated its plugin config, shove in a new min version after the fact.
    // Next frame because there is a TFAR bug where it can process plugin messages in reverse order.
    // Meaning the version that the TFAR script sent first, could arrive after our message and overwrite ours
    [{
        ["minimumPluginVersion", 328] call TFAR_fnc_setPluginSetting;
    ] call CBA_fnc_execNextFrame; 
}] call CBA_fnc_addEventHandler;



DAA_ChatEH = addMissionEventHandler ["HandleChatMessage", {
	params ["_channel", "_owner", "_from", "_text", "_person", "_name", "_strID", "_forcedDisplay", "_isPlayerMessage", "_sentenceType", "_chatMessageType"];

    diag_log ["DAA Chat", _this];
}];




