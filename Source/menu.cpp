#include "menu.hpp"
#define  NOMINMAX
#include <Windows.h>
//#include <chrono>

#include "Resources/Sdk/csgostructs.hpp"
#include "Resources/Helpers/input.hpp"
#include "options.hpp"
#include "config.hpp"

//imgui
#define IMGUI_DEFINE_MATH_OPERATORS
#include "Resources/Imgui/imgui_internal.h"
#include "Resources/Imgui/Impl/imgui_impl_dx9.h"
#include "Resources/Imgui/Impl/imgui_impl_win32.h"

//Menu style
#define WINDOW_WIDTH 820
#define	WINDOW_HEIGHT 650

//Sidebar tabs
static char* sidebar_tabs[] = {
	"LEGIT",
	"RAGE",
	"ANTIAIM",
	"VISUALS",
	"SKINCHANGER",
	"MISC"
};

constexpr static float get_sidebar_item_width() { return 0.0f; } //150.0f
constexpr static float get_sidebar_item_height() { return 0.0f; } //50.0f

enum {
	TAB_LEGIT,
	TAB_RAGE,
	TAB_ANTIAIM,
	TAB_VISUALS,
	TAB_SKINCHANGER,
	TAB_MISC
};

/* IMGUI CMD LINES 
Slider Float 	- Imgui::SliderFloat("TITLE", g_Options.NAME_NAME, 0.0f, 150.0f);
Slider INT      - Imgui::SliderInt("TITLE", g_Options.NAME_NAME, 0, 0);
Checkbox        - ImGui::Checkbox("TITLE", g_Options.NAME_NAME);
Colour select 	- ImGuiEx::ColorEdit3("TITLE", g_Options.NAME_NAME);
*/

//Aimbot general
ImGui::Checkbox("Enable", g_Options.legit_aim_enable); //Add on key
const char* items[] = { "HEAD", "ARMS", "CHEST", "LOWER-CHEST", "STOMACH", "PELVIS", "LEGS", "FEET" }; //Hitbox groups
static const char* current_item = NULL;

if (ImGui::BeginCombo("Hitbox", current_item))
{
    for (int n = 0; n < IM_ARRAYSIZE(items); n++)
    {
        bool is_selected = (current_item == items[n]);
        if (ImGui::Selectable(items[n], is_selected)
            current_item = items[n];
            if (is_selected)
                ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
}
ImGui::Checkbox("Magnet", g_Options.legit_aim_magnet);
ImGui::Checkbox("Aimlock", g_Options.legit_aim_aimlock);
ImGui::SliderFloat("Hitchance", g_Options.legit_aim_hitchance, 0.0f, 100.0f);
ImGui::Checkbox("Silent", g_Options.legit_aim_silent);
ImGui::SliderFloat("FOV", g_Options.legit_aim_fov, 0.0f, 90.0f); //90d aimbot
ImGui::SliderFloat("Distant FOV", g_Options.legit_aim_distant_fov, 0.0f, 90.0f);
ImGui::Checkbox("Rectangle FOV", g_Options.legit_aim_rectangular);
ImGui::SliderFloat("Strength", g_Options.legit_aim_strength, 0.0f, 100.0f);
ImGui::SliderFloat("Speed(X)", g_Options.legit_aim_speed_x, 0.0f, 100.0f);
ImGui::SliderFloat("Speed(Y)", g_Options.legit_aim_speed_y, 0.0f, 100.0f);
ImGui::SliderFloat("Delay(ms)", g_Options.legit_aim_delay, 0.0f, 1000.0f);
ImGui::Checkbox("Auto stop", g_Options.legit_aim_auto_stop);
ImGui::Checkbox("Auto fire", g_Options.legit_aim_auto_fire);
ImGui::Checkbox("Wall penetration", g_Options.legit_aim_wall_penetration);
ImGui::Checkbox("Smoke check", g_Options.legit_aim_smoke_check);
ImGui::Checkbox("Flash check", g_Options.legit_aim_flash_check);
ImGui::Checkbox("Jump check", g_Options.legit_aim_jump_check);
ImGui::Checkbox("Enable backtrack", g_Options.legit_aim_backtrack_enable);
if(Enable backtrack){
ImGui::SliderFloat("Backtrack amount(ms)", g_Options.legit_aim_backtrack_amount, 0.0f, 200.0f););
};
ImGui::Checkbox("Friendly fire", g_Options.legit_aim_friendly_fire);

//Weapon config (GROUP)
ImGui::Checkbox("Overide default", g_Options.)
ImGui::Checkbox("Magnet", g_Options.);
ImGui::Checkbox("Aimlock", g_Options.);
ImGui::SlideFloat("Hitchance", g_Options., 0.0f, 100.0f);
ImGui::Checkbox("Silent", g_Options.);
ImGui::SliderFloat("FOV", g_Options., 0.0f, 90.0f); //90d aimbot
ImGui::SliderFloat("Distant FOV", g_Options., 0.0f, 90.0f);
ImGui::Checkbox("Rectangle FOV", g_Options.);
ImGui::SliderFloat("Strength", g_Options., 0.0f, 100.0f);
ImGui::SlideFloat("Speed(X)", g_Options., 0.0f, 100.0f);
ImGui::SlideFloat("Speed(Y)", g_Options., 0.0f, 100.0f);
ImGui::SlideFloat("Delay(ms)", g_Options., 0.0f, 100.0f);
ImGui::Checkbox("Auto stop", g_Options.);
ImGui::Checkbox("Auto fire", g_Options.);
ImGui::Checkbox("Wall penetration", g_Options.);
ImGui::Checkbox("Smoke check", g_Options.);
ImGui::Checkbox("Flash check", g_Options.);
ImGui::Checkbox("Jump check", g_Options.);
ImGui::Checkbox("Enable backtrack", g_Options.);
if(Enable backtrack){
ImGui::SliderFloat("Backtrack amount(ms)", g_Options., 0.0f, 200.0f););
};
ImGui::Checkbox("Friendly fire", g_Options.);

//Triggerbot
ImGui::Checkbox("Enable", g_Options.legit_trigger_enable);
ImGui::SliderFloat("Hitchance", g_Options.legit_trigger_hitchance, 0.0f, 100.0f);
ImGui::SliderFloat("Delay(ms)", g_Options.legit_trigger_delay, 0.0f, 1000.0f);
ImGui::Checkbox("Auto stop", g_Options.legit_trigger_auto_stop);
ImGui::Checkbox("Wall penetration", g_Options.legit_trigger_wall_penetration);
ImGui::Checkbox("Smoke check", g_Options.legit_trigger_smoke_check);
ImGui::Checkbox("Flash check", g_Options.legit_trigger_flash_check);
ImGui::Checkbox("Jump check", g_Options.legit_trigger_jump_check);


//RCS
ImGui::Checkbox("Recoil control", g_Options.legit_rcs_enable);
ImGui::SliderFloat("Control after", g_Options.legit_rcs_delay, 0.0f, 10.0f); //Shots
ImGui::SliderFloat("X-axis", g_Options.legit_rcs_control_x, 0.0f, 100.0f);
ImGui::SliderFloat("Y-axis", g_Options.legit_rcs_control_y, 0.0f, 100.0f);

//Other

//Hitbox selection dropdown menu
const char* items[] = { "HEAD", "ARMS", "CHEST", "LOWER-CHEST", "STOMACH", "PELVIS", "LEGS", "FEET" }; //Hitbox groups
static const char* current_item = NULL;

if (ImGui::BeginCombo("Hitbox", current_item))
{
	for (int n = 0; n < IM_ARRAYSIZE(items); n++)
	{
		bool is_selected = (current_item == items[n]);
		if (ImGui::Selectable(items[n], is_selected)
			current_item = items[n];
			if (is_selected)
				ImGui::SetItemDefaultFocus();
	}
	ImGui::EndCombo();
}

//Individual weapon dropdown menu
const char* items[] = { "DEFAULT", "P200", "USP-S", "GLOCK-18", "DUEL-BERETTAS", "P250", "CZ75-AUTO", "TEC-9", "FIVE-SEVEN", "DESERT EAGLE", "R8 REVOLVER", //Pistols
						"MAC-10", "MP9", "MP5-SD", "MP7", "UMP-45", "P90", "PP-BIZON",																		//SMGs
						"GALIL-AR", "FAMAS", "AK-47", "M4A4", "M4A1-S", "SSG-08", "SG-553", "AUG", "AWP", "G3SG1", "SCAR-20",								//Rifles
						"NOVA", "XM1014", "SAWED-OFF", "MAG-7", "M249", "NEGEV" };																			//Heavy
static const char* current_item = NULL;

if (ImGui::BeginCombo("Singular weapon", current_item))
{
	for (int n = 0; n < IM_ARRAYSIZE(items); n++)
	{
		bool is_selected = (current_item == items[n]);
		if (ImGui::Selectable(items[n], is_selected)
			current_item = items[n];
			if (is_selected)
				ImGui::SetItemDefaultFocus();
	}
	ImGui::EndCombo();
}

//Group weapon dropdown menu
const char* items[] = { "DEFAULT", "PISTOL", "SMG", "RIFLE", "SHOTGUN", "SNIPER" }; //Weapon groups
static const char* current_item = NULL;

if (ImGui::BeginCombo("Group weapon", current_item))
{
	for (int n = 0; n < IM_ARRAYSIZE(items); n++)
	{
		bool is_selected = (current_item == items[n]);
		if (ImGui::Selectable(items[n], is_selected)
			current_item = items[n];
			if (is_selected)
				ImGui::SetItemDefaultFocus();
	}
	ImGui::EndCombo();
}

//Incomplete
