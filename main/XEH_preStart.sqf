#if __A3_DEBUG__
diag_log ["DAA prestart", serverName];
#endif

"daa" callExtension ["dns", ["DNSServerJoin", "DAAServer.arma3.io"]]; // Request current DAA server login data
"daa" callExtension ["dns", ["DNSAdminList", "DAAAdmins.arma3.io"]]; // Request current DAA server login data