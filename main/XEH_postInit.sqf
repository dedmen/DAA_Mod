DAA_EXEH = addMissionEventHandler ["ExtensionCallback", DAA_fnc_extensionCallback];

if (!hasInterface || !isMultiplayer) exitWith {};

// Send modlist
private _mods = (("true" configClasses (configFile >> "CfgPatches")) apply {configName _x}) select {(_x select [0,2] != "A3") && (_x select [0,11] != "CuratorOnly")};

"daa" callExtension ["put", 
    [
        "", // don't care when its done, no handle
        format ["https://daa.webalf.de/api/v1/users/%1", getPlayerUID player], 
        _mods joinString ",",
        "Content-Type:text/plain"
    ]
];