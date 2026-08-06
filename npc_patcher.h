#ifndef NPCPATCHER
#define NPCPATCHER

using namespace std;

#include "PCH.h"
#include "npc.h"
#include <string>

void* patch(const NPC::patch_instruction& line, RE::TESNPC* curobj, bool bEnableSetLevelDirectlyByPCMult, const NPC::ActorData& aData);


#endif
