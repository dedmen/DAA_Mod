params ["_name", "_function", "_data"];

#if __A3_DEBUG__
diag_log ["DAA Extension", _this];
#endif
if (_name != "DAA") exitWith {};


//_function is the handle of the request

private _setServerJoinData = {
    params ["_data"];

    private _serverData = _data splitString ":"; // "ip","port","password"
    _serverData set [1, parseNumber (_serverData select 1)]; // "ip",port,"password"
    _serverData set [2, _serverData param [2, ""]]; // in case password wasn't provided, make sure we have it set

    uiNamespace setVariable ["DAA_ServerJoinData", _serverData];
};


if (_function == "DNSServerJoin") exitWith {
    if (_data select [0,4] == "http") then {
        // DNS Record redirected us to a web API
        "daa" callExtension ["get", ["HTTPServerJoin", _data]]; // Launch new request via HTTP
    } else {
        _data call _setServerJoinData;
    };
};

if (_function == "HTTPServerJoin") exitWith {
    _data call _setServerJoinData;
};



if (_function == "DNSAdminList") exitWith {
    if (_data select [0,4] == "http") then {
        // DNS Record redirected us to a web API
        "daa" callExtension ["get", ["HTTPAdminList", _data]]; // Launch new request via HTTP
    } else {
        uiNamespace setVariable ["DAA_AdminList", _data splitString ","];
        if (!isNil "DAA_loadingAdminList") then {
            // also set it on server
            [_data splitString ",", {uiNamespace setVariable ["DAA_AdminList", _this]}] remoteExec ["call", 2];
            DAA_loadingAdminList = nil; // only once
        }
    };
};

if (_function == "HTTPAdminList") exitWith {
    uiNamespace setVariable ["DAA_AdminList", _data splitString ","];
    if (!isNil "DAA_loadingAdminList") then {
        // also set it on server
        [_data splitString ",", {uiNamespace setVariable ["DAA_AdminList", _this]}] remoteExec ["call", 2];
        DAA_loadingAdminList = nil; // only once
    }
};
