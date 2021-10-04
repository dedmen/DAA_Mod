class CfgPatches
{
    class DAA_Main
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 2.06;
        requiredAddons[] = {
            "A3_Data_F_Orange_Loadorder", // really only need one of these, the latest but lazy AF
            "A3_Data_F_Enoch_Loadorder",
            "A3_Data_F_Tacops_Loadorder",
            "A3_Data_F_Oldman_Loadorder",
            "A3_Data_F_AoW_Loadorder",
            "cba_main" // XEH
			
        };
        version = 1.0;
        versionStr = 1.0;
        versionAr[] = {1,0};
        author = "dedmen";
    };
};

class Extended_PreInit_EventHandlers {
    class ADDON {
        init = "call compileScript ['z\daa\addons\main\XEH_preInit.sqf']";
    };
};



class CfgMainMenuSpotlight
{
    // delete/disable all basegame ones
    class Orange_Campaign { condition = "false";};
    class Tacops_Campaign_01 { condition = "false";};
    class Contact_Campaign { condition = "false";};
    class OldMan { condition = "false";};
    delete ApexProtocol;
    delete BootCamp;
    delete EastWind;
    delete Orange_CampaignGerman;
    delete Orange_Showcase_IDAP;
    delete Orange_Showcase_LoW;
    delete Showcase_TankDestroyers;
    delete Tacops_Campaign_02;
    delete Tacops_Campaign_03;
    delete Tanks_Campaign_01;
    delete gm_campaign_01;
    delete SP_FD14;
    delete AoW_Showcase_AoW;
    delete AoW_Showcase_Future;

    class DAA_JoinButton
    {
        text = ""; // empty text is better as logo is centered
        textIsQuote = 0;
        picture = "\z\daa\addons\main\DAA-full.paa";
        video = "\z\daa\addons\main\DAA-full.ogv";
        action = "0 = [_this] execVM '\z\daa\addons\main\joinServer.sqf'"; // lazy
        actionText = "Deutsche Arma Allianz";
        condition = "true";
    }
};




class CfgDifficultyPresets {
        defaultPreset = "DAA";
		
		class DAA {
			displayName = "Deutsche Arma Allianz Difficulty";
			description = "Deutsche Arma Allianz Difficulty";
			optionDescription = "Hallo!";
			optionPicture = "\A3\Ui_f\data\Logos\arma3_white_ca.paa";
			levelAI = "DAAAILevel";
			class Options {
					/* Simulation */

					reducedDamage = 0;              // Reduced damage

					/* Situational awareness */

					groupIndicators = 0;            // Group indicators (0 = never, 1 = limited distance, 2 = always)
					friendlyTags = 0;               // Friendly name tags (0 = never, 1 = limited distance, 2 = always)
					enemyTags = 0;                  // Enemy name tags (0 = never, 1 = limited distance, 2 = always)
					detectedMines = 0;              // Detected mines (0 = never, 1 = limited distance, 2 = always)
					commands = 0;                   // Commands (0 = never, 1 = fade out, 2 = always)
					waypoints = 0;                  // Waypoints (0 = never, 1 = fade out, 2 = always)
					tacticalPing = 0;               // Tactical ping (0 = disable, 1 = enable)

					/* Personal awareness */

					weaponInfo = 1;                 // Weapon info (0 = never, 1 = fade out, 2 = always)
					stanceIndicator = 0;            // Stance indicator (0 = never, 1 = fade out, 2 = always)
					staminaBar = 0;                 // Stamina bar
					weaponCrosshair = 0;            // Weapon crosshair
					visionAid = 0;                  // Vision aid

					/* View */

					thirdPersonView = 0;            // 3rd person view
					cameraShake = 1;                // Camera shake

					/* Multiplayer */

					scoreTable = 0;                 // Score table
					deathMessages = 0;              // Killed by
					vonID = 0;                      // VoN ID

					/* Misc */

					mapContent = 0;                 // Extended map content
					autoReport = 0;                 // (former autoSpot) Automatic reporting of spotted enemied by players only. This doesn't have any effect on AIs.
					multipleSaves = 0;              // Multiple saves
			};

			// aiLevelPreset defines AI skill level and is counted from 0 and can have following values: 0 (Low), 1 (Normal), 2 (High), 3 (Custom).
			// when 3 (Custom) is chosen, values of skill and precision are taken from the class CustomAILevel.
			aiLevelPreset = 3;
        };

		class DAAAILevel {
				skillAI = 0.6;
				precisionAI = 0.4;
		};
};

class CfgAILevelPresets
{
	class DAAAILevel {
		displayName="DAA AI Level";
		skillAI = 0.6;
		precisionAI = 0.4;
	};
};