﻿#include <intrin.h>
#include <array>
#include "hooks.h"
#include "resources/utils/global.h"
#include "resources/utils/variables.h"
#include "resources/utils/inputsystem.h"
#include "resources/utils/logging.h"
#include "resources/utils/ui.h"
#include "resources/utils/utils.h"
#include "menu.h"

//features

static constexpr std::array<const char *, 3U> arrSmokeMaterials =
	{
		//"particle/vistasmokev1/vistasmokev1_fire",  // to look cool fresh fashionable yo :sunglasses: (if u wont be cool just uncomment this)
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
};

#pragma region hooks_get
bool H::Setup()
{
	SEH_START

	if (MH_Initialize() != MH_OK)
		throw std::runtime_error(XorStr("failed initialize minhook"));

	if (!DTR::Reset.Create(MEM::GetVFunc(I::DirectDevice, VTABLE::RESET), &hkReset))
		return false;

	if (!DTR::EndScene.Create(MEM::GetVFunc(I::DirectDevice, VTABLE::ENDSCENE), &hkEndScene))
		return false;

	if (!DTR::AllocKeyValuesMemory.Create(MEM::GetVFunc(I::KeyValuesSystem, VTABLE::ALLOCKEYVALUESMEMORY), &hkAllocKeyValuesMemory))
		return false;

	if (!DTR::CreateMoveProxy.Create(MEM::GetVFunc(I::Client, VTABLE::CREATEMOVE), &hkCreateMoveProxy))
		return false;

	if (!DTR::FrameStageNotify.Create(MEM::GetVFunc(I::Client, VTABLE::FRAMESTAGENOTIFY), &hkFrameStageNotify))
		return false;

	if (!DTR::OverrideView.Create(MEM::GetVFunc(I::ClientMode, VTABLE::OVERRIDEVIEW), &hkOverrideView))
		return false;

	if (!DTR::GetViewModelFOV.Create(MEM::GetVFunc(I::ClientMode, VTABLE::GETVIEWMODELFOV), &hkGetViewModelFOV))
		return false;

	if (!DTR::DoPostScreenEffects.Create(MEM::GetVFunc(I::ClientMode, VTABLE::DOPOSTSCREENEFFECTS), &hkDoPostScreenEffects))
		return false;

	if (!DTR::IsConnected.Create(MEM::GetVFunc(I::Engine, VTABLE::ISCONNECTED), &hkIsConnected))
		return false;

	if (!DTR::ListLeavesInBox.Create(MEM::GetVFunc(I::Engine->GetBSPTreeQuery(), VTABLE::LISTLEAVESINBOX), &hkListLeavesInBox))
		return false;

	if (!DTR::PaintTraverse.Create(MEM::GetVFunc(I::Panel, VTABLE::PAINTTRAVERSE), &hkPaintTraverse))
		return false;

	if (!DTR::DrawModel.Create(MEM::GetVFunc(I::StudioRender, VTABLE::DRAWMODEL), &hkDrawModel))
		return false;

	if (!DTR::RenderSmokeOverlay.Create(MEM::GetVFunc(I::ViewRender, VTABLE::RENDERSMOKEOVERLAY), &hkRenderSmokeOverlay))
		return false;

	if (!DTR::RunCommand.Create(MEM::GetVFunc(I::Prediction, VTABLE::RUNCOMMAND), &hkRunCommand))
		return false;

	if (!DTR::SendMessageGC.Create(MEM::GetVFunc(I::SteamGameCoordinator, VTABLE::SENDMESSAGE), &hkSendMessage))
		return false;

	if (!DTR::RetrieveMessage.Create(MEM::GetVFunc(I::SteamGameCoordinator, VTABLE::RETRIEVEMESSAGE), &hkRetrieveMessage))
		return false;

	if (!DTR::LockCursor.Create(MEM::GetVFunc(I::Surface, VTABLE::LOCKCURSOR), &hkLockCursor))
		return false;

	if (!DTR::PlaySoundSurface.Create(MEM::GetVFunc(I::Surface, VTABLE::PLAYSOUND), &hkPlaySound))
		return false;

	static CConVar *sv_cheats = I::ConVar->FindVar(XorStr("sv_cheats"));

	if (!DTR::SvCheatsGetBool.Create(MEM::GetVFunc(sv_cheats, VTABLE::GETBOOL), &hkSvCheatsGetBool))
		return false;

	return true;

	SEH_END

	return false;
}

void H::Restore()
{
	MH_DisableHook(MH_ALL_HOOKS);
	MH_RemoveHook(MH_ALL_HOOKS);

	MH_Uninitialize();
}
#pragma endregion

#pragma region hooks_handlers
long D3DAPI H::hkReset(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	static auto oReset = DTR::Reset.GetOriginal<decltype(&hkReset)>();

	// check for first initialization
	if (!D::bInitialized)
		return oReset(pDevice, pPresentationParameters);

	// invalidate vertex & index buffer, release fonts texture
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const HRESULT hReset = oReset(pDevice, pPresentationParameters);

	// get directx device and create fonts texture
	if (hReset == D3D_OK)
		ImGui_ImplDX9_CreateDeviceObjects();

	return hReset;
}

long D3DAPI H::hkEndScene(IDirect3DDevice9 *pDevice)
{
	static auto oEndScene = DTR::EndScene.GetOriginal<decltype(&hkEndScene)>();
	static void *pUsedAddress = nullptr;

	SEH_START

	if (pUsedAddress == nullptr)
	{
		// search for gameoverlay address
		MEMORY_BASIC_INFORMATION memInfo;
		VirtualQuery(_ReturnAddress(), &memInfo, sizeof(MEMORY_BASIC_INFORMATION));

		char chModuleName[MAX_PATH];
		GetModuleFileName(static_cast<HMODULE>(memInfo.AllocationBase), chModuleName, MAX_PATH);

		if (strstr(chModuleName, GAMEOVERLAYRENDERER_DLL) != nullptr)
			pUsedAddress = _ReturnAddress();
	}

	// check for called from gameoverlay and render here to bypass capturing programs
	if (_ReturnAddress() == pUsedAddress)
	{
		// init gui (fonts, sizes, styles, colors) once
		if (!D::bInitialized)
			D::Setup(pDevice);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// render cheat menu & visuals
		W::MainWindow(pDevice);

		ImGui::EndFrame();
		ImGui::Render();

		// render draw lists from draw data
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	SEH_END

	return oEndScene(pDevice);
}

void *FASTCALL H::hkAllocKeyValuesMemory(IKeyValuesSystem *thisptr, int edx, int iSize)
{
	static auto oAllocKeyValuesMemory = DTR::AllocKeyValuesMemory.GetOriginal<decltype(&hkAllocKeyValuesMemory)>();

	// return addresses of check function
	// @credits: danielkrupinski
	static const std::uintptr_t uAllocKeyValuesEngine = MEM::GetAbsoluteAddress(MEM::FindPattern(ENGINE_DLL, XorStr("E8 ? ? ? ? 83 C4 08 84 C0 75 10 FF 75 0C")) + 0x1) + 0x4A;
	static const std::uintptr_t uAllocKeyValuesClient = MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, XorStr("E8 ? ? ? ? 83 C4 08 84 C0 75 10")) + 0x1) + 0x3E;

	if (const std::uintptr_t uReturnAddress = reinterpret_cast<std::uintptr_t>(_ReturnAddress()); uReturnAddress == uAllocKeyValuesEngine || uReturnAddress == uAllocKeyValuesClient)
		return nullptr;

	return oAllocKeyValuesMemory(thisptr, edx, iSize);
}

static void STDCALL CreateMove(int nSequenceNumber, float flInputSampleFrametime, bool bIsActive, bool &bSendPacket)
{
	static auto oCreateMove = DTR::CreateMoveProxy.GetOriginal<decltype(&H::hkCreateMoveProxy)>();

	// process original CHLClient::CreateMove -> CInput::CreateMove
	oCreateMove(I::Client, 0, nSequenceNumber, flInputSampleFrametime, bIsActive);

	CUserCmd *pCmd = I::Input->GetUserCmd(nSequenceNumber);
	CVerifiedUserCmd *pVerifiedCmd = I::Input->GetVerifiedCmd(nSequenceNumber);

	// check do we have valid commands, finished signing on to server and not playing back demos (where our commands are ignored)
	if (pCmd == nullptr || pVerifiedCmd == nullptr || !bIsActive)
		return;

	// save global cmd pointer
	G::pCmd = pCmd;

	/*
	 * get global localplayer pointer
	 * @note: dont forget check global localplayer for nullptr when using not in createmove
	 */
	CBaseEntity *pLocal = G::pLocal = CBaseEntity::GetLocalPlayer();

	// netchannel pointer
	INetChannel *pNetChannel = I::ClientState->pNetChannel;

	// save previous view angles for movement correction
	QAngle angOldViewPoint = pCmd->angViewPoint;

	SEH_START

	// @note: need do bunnyhop and other movements before prediction
	CMiscellaneous::Get().Run(pCmd, pLocal, bSendPacket);

	/*
	 * CL_RunPrediction
	 * correct prediction when framerate is lower than tickrate
	 * https://github.com/VSES/SourceEngine2007/blob/master/se2007/engine/cl_pred.cpp#L41
	 */
	if (I::ClientState->iDeltaTick > 0)
		I::Prediction->Update(I::ClientState->iDeltaTick, I::ClientState->iDeltaTick > 0, I::ClientState->iLastCommandAck, I::ClientState->iLastOutgoingCommand + I::ClientState->nChokedCommands);

	CPrediction::Get().Start(pCmd, pLocal);
	{
		if (C::Get<bool>(Vars.bMiscAutoPistol))
			CMiscellaneous::Get().AutoPistol(pCmd, pLocal);

		if (C::Get<bool>(Vars.bMiscFakeLag) || C::Get<bool>(Vars.bAntiAim))
			CMiscellaneous::Get().FakeLag(pLocal, bSendPacket);

		if (C::Get<bool>(Vars.bRage))
			CRageBot::Get().Run(pCmd, pLocal, bSendPacket);

		if (C::Get<bool>(Vars.bLegit))
			CLegitBot::Get().Run(pCmd, pLocal, bSendPacket);

		if (C::Get<bool>(Vars.bTrigger))
			CTriggerBot::Get().Run(pCmd, pLocal);

		if (C::Get<bool>(Vars.bAntiAim))
			CAntiAim::Get().UpdateServerAnimations(pCmd, pLocal);

		if (C::Get<bool>(Vars.bAntiAim))
			CAntiAim::Get().Run(pCmd, pLocal, bSendPacket);
	}
	CPrediction::Get().End(pCmd, pLocal);

	if (pLocal->IsAlive())
		CMiscellaneous::Get().MovementCorrection(pCmd, angOldViewPoint);

	// clamp & normalize view angles
	if (C::Get<bool>(Vars.bMiscAntiUntrusted))
	{
		pCmd->angViewPoint.Normalize();
		pCmd->angViewPoint.Clamp();
	}

	if (C::Get<bool>(Vars.bMiscPingSpike))
		CLagCompensation::Get().UpdateIncomingSequences(pNetChannel);
	else
		CLagCompensation::Get().ClearIncomingSequences();

	// @note: we doesnt need rehook manually cuz detours here
	if (pNetChannel != nullptr)
	{
		if (!DTR::SendNetMsg.IsHooked())
			DTR::SendNetMsg.Create(MEM::GetVFunc(pNetChannel, VTABLE::SENDNETMSG), &H::hkSendNetMsg);

		if (!DTR::SendDatagram.IsHooked())
			DTR::SendDatagram.Create(MEM::GetVFunc(pNetChannel, VTABLE::SENDDATAGRAM), &H::hkSendDatagram);
	}

	// store next tick view angles state
	G::angRealView = pCmd->angViewPoint;

	// store current tick send packet state
	G::bSendPacket = bSendPacket;

	// @note: i seen many times this mistake and please do not set/clamp angles here cuz u get confused with psilent aimbot later!

	SEH_END

	pVerifiedCmd->userCmd = *pCmd;
	pVerifiedCmd->uHashCRC = pCmd->GetChecksum();
}

__declspec(naked) void FASTCALL H::hkCreateMoveProxy([[maybe_unused]] IBaseClientDll *thisptr, [[maybe_unused]] int edx, [[maybe_unused]] int nSequenceNumber, [[maybe_unused]] float flInputSampleFrametime, [[maybe_unused]] bool bIsActive)
{
	__asm
	{
		push	ebp
		mov		ebp, esp; // store the stack
		push	ebx; // bSendPacket
		push	esp; // restore the stack
		push	dword ptr[bIsActive]; // ebp + 16
		push	dword ptr[flInputSampleFrametime]; // ebp + 12
		push	dword ptr[nSequenceNumber]; // ebp + 8
		call	CreateMove
		pop		ebx
		pop		ebp
		retn	0Ch
	}
}

void FASTCALL H::hkPaintTraverse(ISurface *thisptr, int edx, unsigned int uPanel, bool bForceRepaint, bool bForce)
{
	static auto oPaintTraverse = DTR::PaintTraverse.GetOriginal<decltype(&hkPaintTraverse)>();
	const FNV1A_t uPanelHash = FNV1A::Hash(I::Panel->GetName(uPanel));

	// remove zoom panel
	if (!I::Engine->IsTakingScreenshot() && C::Get<bool>(Vars.bWorld) && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SCOPE) && uPanelHash == FNV1A::HashConst("HudZoom"))
		return;

	oPaintTraverse(thisptr, edx, uPanel, bForceRepaint, bForce);

	// @note: we don't render here, only store's data and render it later
	if (uPanelHash == FNV1A::HashConst("FocusOverlayPanel"))
	{
		SEH_START

		// clear data from previous call
		D::ClearDrawData();

		// store data to render
		CVisuals::Get().Store();

		// swap given data to safe container
		D::SwapDrawData();

		SEH_END
	}
}

void FASTCALL H::hkPlaySound(ISurface *thisptr, int edx, const char *szFileName)
{
	static auto oPlaySound = DTR::PlaySoundSurface.GetOriginal<decltype(&hkPlaySound)>();
	oPlaySound(thisptr, edx, szFileName);
}

void FASTCALL H::hkLockCursor(ISurface *thisptr, int edx)
{
	static auto oLockCursor = DTR::LockCursor.GetOriginal<decltype(&hkLockCursor)>();

	if (W::bMainOpened)
	{
		I::Surface->UnLockCursor();
		return;
	}

	oLockCursor(thisptr, edx);
}

void FASTCALL H::hkFrameStageNotify(IBaseClientDll *thisptr, int edx, EClientFrameStage stage)
{
	static auto oFrameStageNotify = DTR::FrameStageNotify.GetOriginal<decltype(&hkFrameStageNotify)>();

	SEH_START

	if (!I::Engine->IsInGame())
	{
		// clear sequences or we get commands overflow on new map connection
		CLagCompensation::Get().ClearIncomingSequences();
		return oFrameStageNotify(thisptr, edx, stage);
	}

	if (I::Engine->IsTakingScreenshot())
		return oFrameStageNotify(thisptr, edx, stage);

	CBaseEntity *pLocal = CBaseEntity::GetLocalPlayer();

	if (pLocal == nullptr)
		return oFrameStageNotify(thisptr, edx, stage);

	static QAngle angOldAimPunch = {}, angOldViewPunch = {};

	switch (stage)
	{
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
	{
		/*
		 * data has been received and we are going to start calling postdataupdate
		 * e.g. resolver or skinchanger and other visuals
		 */

		break;
	}
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
	{
		/*
		 * data has been received and called postdataupdate on all data recipients
		 * e.g. now we can modify interpolation, other lagcompensation stuff
		 */

		break;
	}
	case FRAME_NET_UPDATE_END:
	{
		/*
		 * received all packets, now do interpolation, prediction, etc
		 * e.g. backtrack stuff
		 */

		break;
	}
	case FRAME_RENDER_START:
	{
		/*
		 * start rendering the scene
		 * e.g. remove visual punch, thirdperson, other render/update stuff
		 */

		// set max flash alpha
		*pLocal->GetFlashMaxAlpha() = C::Get<bool>(Vars.bWorld) ? C::Get<int>(Vars.iWorldMaxFlash) * 2.55f : 255.f;

		// no draw smoke
		for (const auto &szSmokeMaterial : arrSmokeMaterials)
		{
			if (IMaterial *pMaterial = I::MaterialSystem->FindMaterial(szSmokeMaterial, TEXTURE_GROUP_OTHER); pMaterial != nullptr && !pMaterial->IsErrorMaterial())
				pMaterial->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, (C::Get<bool>(Vars.bWorld) && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SMOKE)) ? true : false);
		}

		// remove visual punch
		if (pLocal->IsAlive() && C::Get<bool>(Vars.bWorld))
		{
			// save old values
			angOldViewPunch = pLocal->GetViewPunch();
			angOldAimPunch = pLocal->GetPunch();

			if (C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_PUNCH))
			{
				// change current values
				pLocal->GetViewPunch() = QAngle{};
				pLocal->GetPunch() = QAngle{};
			}
		}

		// thirdperson
		if (C::Get<bool>(Vars.bWorld) && C::Get<int>(Vars.iWorldThirdPersonKey) > 0)
		{
			static bool bThirdPerson = false;

			if (!I::Engine->IsConsoleVisible() && IPT::IsKeyReleased(C::Get<int>(Vars.iWorldThirdPersonKey)))
				bThirdPerson = !bThirdPerson;

			// my solution is here cuz camera offset is dynamically by standard functions without any garbage in overrideview hook
			I::Input->bCameraInThirdPerson = bThirdPerson && pLocal->IsAlive() && !I::Engine->IsTakingScreenshot();
			I::Input->vecCameraOffset.z = bThirdPerson ? C::Get<float>(Vars.flWorldThirdPersonOffset) : 150.f;
		}

		break;
	}
	case FRAME_RENDER_END:
	{
		/*
		 * finished rendering the scene
		 * here we can restore our modified things
		 */

		// restore original visual punch values
		if (pLocal->IsAlive() && C::Get<bool>(Vars.bWorld) && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_PUNCH))
		{
			pLocal->GetViewPunch() = angOldViewPunch;
			pLocal->GetPunch() = angOldAimPunch;
		}

		break;
	}
	default:
		break;
	}

	SEH_END

	oFrameStageNotify(thisptr, edx, stage);
}

void FASTCALL H::hkDrawModel(IStudioRender *thisptr, int edx, DrawModelResults_t *pResults, const DrawModelInfo_t &info, matrix3x4_t *pBoneToWorld, float *flFlexWeights, float *flFlexDelayedWeights, const Vector &vecModelOrigin, int nFlags)
{
	static auto oDrawModel = DTR::DrawModel.GetOriginal<decltype(&hkDrawModel)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oDrawModel(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	bool bClearOverride = false;

	if (CBaseEntity *pLocal = CBaseEntity::GetLocalPlayer(); pLocal != nullptr && C::Get<bool>(Vars.bEsp) && C::Get<bool>(Vars.bEspChams))
		bClearOverride = CVisuals::Get().Chams(pLocal, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	oDrawModel(thisptr, edx, pResults, info, pBoneToWorld, flFlexWeights, flFlexDelayedWeights, vecModelOrigin, nFlags);

	if (bClearOverride)
		I::StudioRender->ForcedMaterialOverride(nullptr);
}

void FASTCALL H::hkRenderSmokeOverlay(IViewRender *thisptr, int edx, bool bPreViewModel)
{
	static auto oRenderSmokeOverlay = DTR::RenderSmokeOverlay.GetOriginal<decltype(&hkRenderSmokeOverlay)>();

	if (C::Get<bool>(Vars.bWorld) && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SMOKE))
		// set flSmokeIntensity to 0
		*reinterpret_cast<float *>(reinterpret_cast<std::uintptr_t>(thisptr) + 0x588) = 0.0f;
	else
		oRenderSmokeOverlay(thisptr, edx, bPreViewModel);
}

int FASTCALL H::hkListLeavesInBox(void *thisptr, int edx, const Vector &vecMins, const Vector &vecMaxs, unsigned short *puList, int nListMax)
{
	static auto oListLeavesInBox = DTR::ListLeavesInBox.GetOriginal<decltype(&hkListLeavesInBox)>();

	// @todo: sometimes models doesn't drawn on certain maps (not only me: https://www.unknowncheats.me/forum/counterstrike-global-offensive/330483-disable-model-occulusion-3.html)
	// @test: try to fix z order 11.08.20

	// @credits: soufiw
	// occlusion getting updated on player movement/angle change,
	// in RecomputeRenderableLeaves https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L674
	static std::uintptr_t uInsertIntoTree = (MEM::FindPattern(CLIENT_DLL, XorStr("56 52 FF 50 18")) + 0x5); // @xref: "<unknown renderable>"

	// check for esp state and call from CClientLeafSystem::InsertIntoTree
	if (C::Get<bool>(Vars.bEsp) && C::Get<bool>(Vars.bEspChams) && C::Get<bool>(Vars.bEspChamsDisableOcclusion) && (C::Get<bool>(Vars.bEspChamsEnemies) || C::Get<bool>(Vars.bEspChamsAllies)) && reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uInsertIntoTree)
	{
		// get current renderable info from stack https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L1470
		if (const auto pInfo = *reinterpret_cast<RenderableInfo_t **>(reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress()) + 0x14); pInfo != nullptr)
		{
			if (const auto pRenderable = pInfo->pRenderable; pRenderable != nullptr)
			{
				// check if disabling occlusion for players
				if (const auto pEntity = pRenderable->GetIClientUnknown()->GetBaseEntity(); pEntity != nullptr && pEntity->IsPlayer())
				{
					// fix render order, force translucent group (https://www.unknowncheats.me/forum/2429206-post15.html)
					// AddRenderablesToRenderLists: https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L2473
					// @ida addrenderablestorenderlists: 55 8B EC 83 EC 24 53 56 8B 75 08 57 8B 46
					pInfo->uFlags &= ~RENDER_FLAGS_FORCE_OPAQUE_PASS;
					pInfo->uFlags2 |= RENDER_FLAGS_BOUNDS_ALWAYS_RECOMPUTE;

					// extend world space bounds to maximum https://github.com/pmrowla/hl2sdk-csgo/blob/master/game/client/clientleafsystem.cpp#L707
					constexpr Vector vecMapMin(MIN_COORD_FLOAT, MIN_COORD_FLOAT, MIN_COORD_FLOAT);
					constexpr Vector vecMapMax(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
					return oListLeavesInBox(thisptr, edx, vecMapMin, vecMapMax, puList, nListMax);
				}
			}
		}
	}

	return oListLeavesInBox(thisptr, edx, vecMins, vecMaxs, puList, nListMax);
}

bool FASTCALL H::hkIsConnected(IEngineClient *thisptr, int edx)
{
	static auto oIsConnected = DTR::IsConnected.GetOriginal<decltype(&hkIsConnected)>();

	// @xref: "IsLoadoutAllowed"
	// sub above the string
	// sub in that function
	// .text : 103A2120 84 C0		test    al, al; Logical Compare
	static const std::uintptr_t uLoadoutAllowedReturn = MEM::FindPattern(CLIENT_DLL, XorStr("84 C0 75 05 B0 01 5F"));

	// @credits: gavreel
	if (reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uLoadoutAllowedReturn && C::Get<bool>(Vars.bMiscUnlockInventory))
		return false;

	return oIsConnected(thisptr, edx);
}

bool FASTCALL H::hkSendNetMsg(INetChannel *thisptr, int edx, INetMessage *pMessage, bool bForceReliable, bool bVoice)
{
	static auto oSendNetMsg = DTR::SendNetMsg.GetOriginal<decltype(&hkSendNetMsg)>();

	/*
	 * @note: disable files crc check (sv_pure)
	 * dont send message if it has FileCRCCheck type
	 */
	if (pMessage->GetType() == 14)
		return false;

	/*
	 * @note: fix lag with chocking packets when voice chat is active
	 * check for voicedata group and enable voice stream
	 * @credits: Flaww
	 */
	if (pMessage->GetGroup() == INetChannelInfo::VOICE)
		bVoice = true;

	return oSendNetMsg(thisptr, edx, pMessage, bForceReliable, bVoice);
}

int FASTCALL H::hkSendDatagram(INetChannel *thisptr, int edx, bf_write *pDatagram)
{
	static auto oSendDatagram = DTR::SendDatagram.GetOriginal<decltype(&hkSendDatagram)>();

	INetChannelInfo *pNetChannelInfo = I::Engine->GetNetChannelInfo();
	static CConVar *sv_maxunlag = I::ConVar->FindVar(XorStr("sv_maxunlag"));

	if (!I::Engine->IsInGame() || !C::Get<bool>(Vars.bMiscPingSpike) || pDatagram != nullptr || pNetChannelInfo == nullptr || sv_maxunlag == nullptr)
		return oSendDatagram(thisptr, edx, pDatagram);

	const int iOldInReliableState = thisptr->iInReliableState;
	const int iOldInSequenceNr = thisptr->iInSequenceNr;

	// calculate max available fake latency with our real ping to keep it w/o real lags or delays
	const float flMaxLatency = std::max(0.f, std::clamp(C::Get<float>(Vars.flMiscLatencyFactor), 0.f, sv_maxunlag->GetFloat()) - pNetChannelInfo->GetLatency(FLOW_OUTGOING));
	CLagCompensation::Get().AddLatencyToNetChannel(thisptr, flMaxLatency);

	const int iReturn = oSendDatagram(thisptr, edx, pDatagram);

	thisptr->iInReliableState = iOldInReliableState;
	thisptr->iInSequenceNr = iOldInSequenceNr;

	return iReturn;
}

void FASTCALL H::hkOverrideView(IClientModeShared *thisptr, int edx, CViewSetup *pSetup)
{
	static auto oOverrideView = DTR::OverrideView.GetOriginal<decltype(&hkOverrideView)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oOverrideView(thisptr, edx, pSetup);

	CBaseEntity *pLocal = CBaseEntity::GetLocalPlayer();

	// get camera origin
	G::vecCamera = pSetup->vecOrigin;

	if (pLocal == nullptr || !pLocal->IsAlive())
		return oOverrideView(thisptr, edx, pSetup);

	CBaseCombatWeapon *pWeapon = pLocal->GetWeapon();

	if (pWeapon == nullptr)
		return oOverrideView(thisptr, edx, pSetup);

	if (CCSWeaponData *pWeaponData = I::WeaponSystem->GetWeaponData(pWeapon->GetItemDefinitionIndex());
		pWeaponData != nullptr && C::Get<bool>(Vars.bScreen) && std::fpclassify(C::Get<float>(Vars.flScreenCameraFOV)) != FP_ZERO &&
		// check is we not scoped
		(pWeaponData->nWeaponType == WEAPONTYPE_SNIPER ? !pLocal->IsScoped() : true))
		// set camera fov
		pSetup->flFOV += C::Get<float>(Vars.flScreenCameraFOV);

	oOverrideView(thisptr, edx, pSetup);
}

void FASTCALL H::hkOverrideMouseInput(IClientModeShared *thisptr, int edx, float *x, float *y)
{
	static auto oOverrideMouseInput = DTR::OverrideMouseInput.GetOriginal<decltype(&hkOverrideMouseInput)>();

	if (!I::Engine->IsInGame())
		return oOverrideMouseInput(thisptr, edx, x, y);

	oOverrideMouseInput(thisptr, edx, x, y);
}

float FASTCALL H::hkGetViewModelFOV(IClientModeShared *thisptr, int edx)
{
	static auto oGetViewModelFOV = DTR::GetViewModelFOV.GetOriginal<decltype(&hkGetViewModelFOV)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oGetViewModelFOV(thisptr, edx);

	if (CBaseEntity *pLocal = CBaseEntity::GetLocalPlayer(); pLocal != nullptr && pLocal->IsAlive() && C::Get<bool>(Vars.bScreen) && std::fpclassify(C::Get<float>(Vars.flScreenViewModelFOV)) != FP_ZERO)
		return oGetViewModelFOV(thisptr, edx) + C::Get<float>(Vars.flScreenViewModelFOV);

	return oGetViewModelFOV(thisptr, edx);
}

int FASTCALL H::hkDoPostScreenEffects(IClientModeShared *thisptr, int edx, CViewSetup *pSetup)
{
	static auto oDoPostScreenEffects = DTR::DoPostScreenEffects.GetOriginal<decltype(&hkDoPostScreenEffects)>();

	if (!I::Engine->IsInGame() || I::Engine->IsTakingScreenshot())
		return oDoPostScreenEffects(thisptr, edx, pSetup);

	if (CBaseEntity *pLocal = CBaseEntity::GetLocalPlayer(); pLocal != nullptr && C::Get<bool>(Vars.bEsp) && C::Get<bool>(Vars.bEspGlow))
		CVisuals::Get().Glow(pLocal);

	return oDoPostScreenEffects(thisptr, edx, pSetup);
}

void FASTCALL H::hkRunCommand(IPrediction *thisptr, int edx, CBaseEntity *pEntity, CUserCmd *pCmd, IMoveHelper *pMoveHelper)
{
	static auto oRunCommand = DTR::RunCommand.GetOriginal<decltype(&hkRunCommand)>();

	/* there is tickbase corrections / velocity modifier fix */

	oRunCommand(thisptr, edx, pEntity, pCmd, pMoveHelper);

	// get movehelper interface pointer
	I::MoveHelper = pMoveHelper;
}

int FASTCALL H::hkSendMessage(ISteamGameCoordinator *thisptr, int edx, std::uint32_t uMsgType, const void *pData, std::uint32_t uData)
{
	static auto oSendMessage = DTR::SendMessageGC.GetOriginal<decltype(&hkSendMessage)>();

	std::uint32_t uMessageType = uMsgType & 0x7FFFFFFF;
	void *pDataMutable = const_cast<void *>(pData);

	const int iStatus = oSendMessage(thisptr, edx, uMsgType, pDataMutable, uData);

	if (iStatus != EGCResultOK)
		return iStatus;

#ifdef DEBUG_CONSOLE
	L::PushConsoleColor(FOREGROUND_INTENSE_GREEN | FOREGROUND_RED);
	L::Print(XorStr("[<-] Message sent to GC {:d}!"), uMessageType);
	L::PopConsoleColor();
#endif

	return iStatus;
}

int FASTCALL H::hkRetrieveMessage(ISteamGameCoordinator *thisptr, int edx, std::uint32_t *puMsgType, void *pDest, std::uint32_t uDest, std::uint32_t *puMsgSize)
{
	static auto oRetrieveMessage = DTR::RetrieveMessage.GetOriginal<decltype(&hkRetrieveMessage)>();
	const int iStatus = oRetrieveMessage(thisptr, edx, puMsgType, pDest, uDest, puMsgSize);

	if (iStatus != EGCResultOK)
		return iStatus;

	const std::uint32_t uMessageType = *puMsgType & 0x7FFFFFFF;

#ifdef DEBUG_CONSOLE
	L::PushConsoleColor(FOREGROUND_INTENSE_GREEN | FOREGROUND_RED);
	L::Print(XorStr("[->] Message received from GC {:d}!"), uMessageType);
	L::PopConsoleColor();
#endif

	// check for k_EMsgGCCStrike15_v2_GCToClientSteamdatagramTicket message when we can accept the game
	if (C::Get<bool>(Vars.bMiscAutoAccept) && uMessageType == 9177)
	{
		U::SetLocalPlayerReady();
		Beep(500, 800);
		U::FlashWindow(IPT::hWindow);
	}

	return iStatus;
}

bool FASTCALL H::hkSvCheatsGetBool(CConVar *thisptr, int edx)
{
	static auto oSvCheatsGetBool = DTR::SvCheatsGetBool.GetOriginal<decltype(&hkSvCheatsGetBool)>();
	static std::uintptr_t uCAM_ThinkReturn = (MEM::FindPattern(CLIENT_DLL, XorStr("85 C0 75 30 38 86"))); // @xref: "Pitch: %6.1f   Yaw: %6.1f   Dist: %6.1f %16s"

	if (reinterpret_cast<std::uintptr_t>(_ReturnAddress()) == uCAM_ThinkReturn && C::Get<bool>(Vars.bWorld) && C::Get<int>(Vars.iWorldThirdPersonKey) > 0)
		return true;

	return oSvCheatsGetBool(thisptr, edx);
}

long CALLBACK H::hkWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// process keys
	IPT::Process(uMsg, wParam, lParam);

	// switch window state on key click
	if (C::Get<int>(Vars.iMenuKey) > 0 && IPT::IsKeyReleased(C::Get<int>(Vars.iMenuKey)))
		W::bMainOpened = !W::bMainOpened;

	// disable game input when menu is opened
	I::InputSystem->EnableInput(!W::bMainOpened);

	/*
	 * @note: we can use imgui input handler to our binds if remove menu state check
	 * with ImGui::IsKeyDown, ImGui::IsKeyPressed, etc functions
	 * but imgui api's keys down durations doesnt have forward compatibility
	 * and i dont want spend a lot of time on recode it
	 */
	if (D::bInitialized && W::bMainOpened && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return 1L;

	// return input controls to the game
	return CallWindowProcW(IPT::pOldWndProc, hWnd, uMsg, wParam, lParam);
}
#pragma endregion

#pragma region proxies_get
bool P::Setup()
{
// @note: as example
#if 0
	RecvProp_t* pSmokeEffectTickBegin = CNetvarManager::Get().mapProps[FNV1A::HashConst("CSmokeGrenadeProjectile->m_nSmokeEffectTickBegin")].pRecvProp;
	if (pSmokeEffectTickBegin == nullptr)
		return false;

	RVP::SmokeEffectTickBegin = std::make_shared<CRecvPropHook>(pSmokeEffectTickBegin, P::SmokeEffectTickBegin);
#endif

	return true;
}

void P::Restore()
{
// @note: as example
#if 0
	// restore smoke effect
	RVP::SmokeEffectTickBegin->Restore();
#endif
}
#pragma endregion

#pragma region proxies_handlers
void P::SmokeEffectTickBegin(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	static auto oSmokeEffectTickBegin = RVP::SmokeEffectTickBegin->GetOriginal();

	if (C::Get<bool>(Vars.bWorld) && C::Get<std::vector<bool>>(Vars.vecWorldRemovals).at(REMOVAL_SMOKE))
	{
		if (auto pEntity = static_cast<CBaseEntity *>(pStruct); pEntity != nullptr)
			pEntity->GetOrigin() = Vector(MAX_COORD_FLOAT, MAX_COORD_FLOAT, MAX_COORD_FLOAT);
	}

	oSmokeEffectTickBegin(pData, pStruct, pOut);
}
#pragma endregion
