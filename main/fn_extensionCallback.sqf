params ["_name", "_function", "_data"];

#if __A3_DEBUG__
diag_log ["DAA Extension", _this];
#endif
if (_name != "DAA") exitWith {};


//_function is the handle of the request


if (_function == "DNSServerJoin") exitWith {
    if (_data select [0,4] == "http") then {
        // DNS Record redirected us to a web API
        "daa" callExtension ["get", ["HTTPServerJoin", _data]]; // Launch new request via HTTP
    } else {
        private _serverData = _data splitString ":"; // "ip","port","password"
        _serverData set [1, parseNumber (_serverData select 1)]; // "ip",port,"password"
        uiNamespace setVariable ["DAA_ServerJoinData", _serverData];
    };
};

if (_function == "HTTPServerJoin") exitWith {
    private _serverData = _data splitString ":"; // "ip","port","password"
    _serverData set [1, parseNumber (_serverData select 1)]; // "ip",port,"password"
    uiNamespace setVariable ["DAA_ServerJoinData", _serverData];
};



if (_function == "DNSAdminList") exitWith {
    if (_data select [0,4] == "http") then {
        // DNS Record redirected us to a web API
        "daa" callExtension ["get", ["HTTPAdminList", _data]]; // Launch new request via HTTP
    } else {
        uiNamespace setVariable ["DAA_AdminList", _data splitString ","];
    };
};

if (_function == "HTTPAdminList") exitWith {
    uiNamespace setVariable ["DAA_AdminList", _data splitString ","];
};
