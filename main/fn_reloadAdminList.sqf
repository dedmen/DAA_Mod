#if __A3_DEBUG__
diag_log ["DAA reloadAdminList", serverName];
#endif

"daa" callExtension ["dns", ["DNSAdminList", "DAAAdmins.arma3.io"]]; // Request DAA adminlist