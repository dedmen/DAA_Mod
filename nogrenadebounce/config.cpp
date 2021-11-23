class CfgPatches
{
    class DAA_NoGrenadeBounce
    {
        units[] = {};
        weapons[] = {};
        requiredVersion = 2.06;
        requiredAddons[] = {
            "A3_Data_F_AoW_Loadorder",
            "rhs_main_loadorder",
            "rhsusf_main_loadorder"
        };
        version = 1.0;
        versionStr = 1.0;
        versionAr[] = {1,0};
        author = "Gesponsort von Willard - Gruppe Adler";
    };
};

class CfgAmmo {
    // vanilla
    class FlareBase;
    class F_40mm_White: FlareBase {
        intensity = 1500000;
    };
    class F_40mm_Yellow: F_40mm_White {};
    class F_40mm_Red: F_40mm_White {};
    class F_40mm_Green: F_40mm_White {};

    class FlareCore;
    class Flare_82mm_AMOS_White: FlareCore {
        intensity = 4000000;
    };

    class SmokeShell;
    class G_40mm_Smoke: SmokeShell
    {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };


    // RHS USAF/AFRF

    class rhs_40mm_smoke_white: SmokeShell {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };
    class rhs_40mm_m714_white: SmokeShell {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };

    class SmokeShellRed;
    class rhs_40mm_smoke_red: SmokeShellRed {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };
    class rhs_40mm_m713_red: SmokeShellRed {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };

    class SmokeShellGreen;
    class rhs_40mm_smoke_green: SmokeShellGreen {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };
    class rhs_40mm_m715_green: SmokeShellGreen {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };

    class SmokeShellYellow;
    class rhs_40mm_smoke_yellow: SmokeShellYellow {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };
    class rhs_40mm_m716_yellow: SmokeShellYellow {
        simulation = "shotSmoke";
        deflectionSlowDown = 0.4;
    };

    class rhs_g_vog25;
    class rhs_g_vg40md_white: rhs_g_vog25 {
        simulation = "shotSmoke";
        deflecting = 30;
        deflectionSlowDown = 0.4;
    };
};
