
// Cannot do this in preStart because riiiight after preStart the handlers are removed

DAA_EXEH = addMissionEventHandler ["ExtensionCallback", DAA_fnc_extensionCallback];
#if __A3_DEBUG__
diag_log ["DAA EXEH MainMenu", DAA_EXEH];
#endif
"daa" callExtension "callbackReady"; // Ready to listen


