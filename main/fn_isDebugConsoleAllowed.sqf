if (is3DEN || is3DENMultiplayer) exitWith {true};
if (!isMultiplayer) exitWith {true};

private _userCanAdmin = {
    params ["_playerUnit"];

    if (isNull _playerUnit) exitWith {false};
    // player with Zeus access
    if (!isNull getAssignedCuratorLogic _playerUnit) exitWith {true};

    private _playerUID = getPlayerUID _playerUnit;

    // hard whitelist
    if (_playerUID in (uiNamespace getVariable ["DAA_AdminList", []])) exitWith {true};
    // description.ext whitelist
    if (_playerUID in (getArray (missionConfigFile >> "enableDebugConsole"))) exitWith {true};
    // fallback for when server doesn't have access to DAA_AdminList, clientside still won't show console if they think they aren't whitelisted
    if (isServer && (uiNamespace getVariable ["DAA_AdminList", []]) isEqualTo []) exitWith {true};

    false
};

// when this code runs on clientside, either selfhost, or logged in admin, or whitelisted
if (hasInterface && { isServer /* local selfhost */ || { serverCommandAvailable "#logout" /* adminrights */ } || { player call _userCanAdmin }}) exitWith {true};

// Serverside check after a client tried to exec a script, check logged in admin or whitelisted
if (isServer && isRemoteExecuted &&
    {
        private _ownerID = remoteExecutedOwner;
        admin _ownerID > 1 /* adminrights */
        ||
        { 
            private _playerIndex = allPlayers findIf {owner _x isEqualTo _ownerID};
            private _playerUnit = allPlayers param [_playerIndex, objNull];
            (_playerUnit call _userCanAdmin) && {
                diag_log ["DAA Admin access", _ownerID, _playerIndex, _playerUnit];
                // send admin message to everyone with debug console rights to notify of debug console usage https://github.com/gruppe-adler/grad_adminMessages/blob/main/addons/adminMessages/functions/fnc_receiveMessage.sqf
                [name _playerUnit,getPlayerUID _playerUnit, "Used debug console", {call BIS_fnc_isDebugConsoleAllowed}] remoteExec ["grad_adminMessages_adminmessages_fnc_receiveMessage",0,false];
                true
            }
        } 
    }
) exitWith {true};

false 