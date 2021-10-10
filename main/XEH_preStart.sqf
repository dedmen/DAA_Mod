#if __A3_DEBUG__
diag_log ["DAA prestart", serverName];
#endif

// DAAServer.arma3.io
"daa" callExtension ["get", ["HTTPServerJoin", "https://daa.webalf.de/api/v1/server"]]; // Request current DAA server login data
"daa" callExtension ["dns", ["DNSAdminList", "DAAAdmins.arma3.io"]]; // Request DAA adminlist