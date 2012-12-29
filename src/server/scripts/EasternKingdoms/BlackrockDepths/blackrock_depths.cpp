/*
 * Copyright (C) 2010-2012 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2010-2012 Oregon <http://www.oregoncore.com/>
 * Copyright (C) 2006-2008 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Blackrock_Depths
SD%Complete: 95
SDComment: Quest support: 4001, 4342, 7604, 4322. Vendor Lokhtos Darkbargainer. Need to rewrite the Jail Break support
SDCategory: Blackrock Depths
EndScriptData */

/* ContentData
go_shadowforge_brazier
at_ring_of_law
npc_grimstone
mob_phalanx
npc_kharan_mighthammer
npc_lokhtos_darkbargainer
npc_dughal_stormwing
npc_marshal_windsor
npc_marshal_reginald_windsor
npc_tobias_seecher
npc_rocknot
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "blackrock_depths.h"

/*######
+## go_shadowforge_brazier
+######*/
class go_shadowforge_brazier : public GameObjectScript
{
public:
    go_shadowforge_brazier() : GameObjectScript("go_shadowforge_brazier") { }

    bool GOHello(Player* /*player*/, GameObject* pGo)
    {
        if (ScriptedInstance* instance = pGo->GetInstanceScript())
        {
            if (instance->GetData(TYPE_LYCEUM) == IN_PROGRESS)
                instance->SetData(TYPE_LYCEUM, DONE);
            else
                instance->SetData(TYPE_LYCEUM, IN_PROGRESS);
            // If used brazier open linked doors (North or South)
            if (pGo->GetGUID() == instance->GetData64(DATA_SF_BRAZIER_N))
                instance->HandleGameObject(instance->GetData64(DATA_GOLEM_DOOR_N), true);
            else if (pGo->GetGUID() == instance->GetData64(DATA_SF_BRAZIER_S))
                instance->HandleGameObject(instance->GetData64(DATA_GOLEM_DOOR_S), true);
        }
        return false;
    }
};

/*######
## npc_grimstone
######*/

enum eGrimstone
{
    NPC_GRIMSTONE                                          = 10096,
    NPC_THELDREN                                           = 16059,

    //4 or 6 in total? 1+2+1 / 2+2+2 / 3+3. Depending on this, code should be changed.
    MAX_MOB_AMOUNT                                         = 4
};

uint32 RingMob[]=
{
    8925,                                                   // Dredge Worm
    8926,                                                   // Deep Stinger
    8927,                                                   // Dark Screecher
    8928,                                                   // Burrowing Thundersnout
    8933,                                                   // Cave Creeper
    8932,                                                   // Borer Beetle
};

uint32 RingBoss[]=
{
    9027,                                                   // Gorosh
    9028,                                                   // Grizzle
    9029,                                                   // Eviscerator
    9030,                                                   // Ok'thor
    9031,                                                   // Anub'shiah
    9032,                                                   // Hedrum
};
class at_ring_of_law : public AreaTriggerScript
{
public:
    at_ring_of_law() : AreaTriggerScript("at_ring_of_law") { }

    bool AreaTrigger(Player* player, const AreaTriggerEntry * /*at*/)
    {
        if (ScriptedInstance* instance = player->GetInstanceScript())
        {
            if (instance->GetData(TYPE_RING_OF_LAW) == IN_PROGRESS || instance->GetData(TYPE_RING_OF_LAW) == DONE)
                return false;

            instance->SetData(TYPE_RING_OF_LAW, IN_PROGRESS);
            player->SummonCreature(NPC_GRIMSTONE, 625.559,-205.618,-52.735, 2.609, TEMPSUMMON_DEAD_DESPAWN, 0);

            return false;
        }
        return false;
    }
};

/*######
## npc_grimstone
######*/

enum GrimstoneTexts
{
    SCRIPT_TEXT1                                           = -1000000,
    SCRIPT_TEXT2                                           = -1000001,
    SCRIPT_TEXT3                                           = -1000002,
    SCRIPT_TEXT4                                           = -1000003,
    SCRIPT_TEXT5                                           = -1000004,
    SCRIPT_TEXT6                                           = -1000005
};

//TODO: implement quest part of event (different end boss)class npc_grimstone : public CreatureScript
{
public:
    npc_grimstone() : CreatureScript("npc_grimstone") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_grimstoneAI(creature);
    }

    struct npc_grimstoneAI : public npc_escortAI
    {
        npc_grimstoneAI(Creature *c) : npc_escortAI(c)
        {
            instance = c->GetInstanceScript();
            MobSpawnId = rand()%6;
        }

        ScriptedInstance* instance;

        uint8 EventPhase;
        uint32 Event_Timer;

        uint8 MobSpawnId;
        uint8 MobCount;
        uint32 MobDeath_Timer;

        uint64 RingMobGUID[4];
        uint64 RingBossGUID;

        bool CanWalk;

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            EventPhase = 0;
            Event_Timer = 1000;

            MobCount = 0;
            MobDeath_Timer = 0;

            for (uint8 i = 0; i < MAX_MOB_AMOUNT; ++i)
                RingMobGUID[i] = 0;

            RingBossGUID = 0;

            CanWalk = false;
        }

        //TODO: move them to center
        void SummonRingMob()
        {
            if (Creature* tmp = me->SummonCreature(RingMob[MobSpawnId],608.960,-235.322,-53.907, 1.857, TEMPSUMMON_DEAD_DESPAWN, 0))
                RingMobGUID[MobCount] = tmp->GetGUID();

            ++MobCount;

            if (MobCount == MAX_MOB_AMOUNT)
                MobDeath_Timer = 2500;
        }

        //TODO: move them to center
        void SummonRingBoss()
        {
            if (Creature* tmp = me->SummonCreature(RingBoss[rand()%6],644.300,-175.989,-53.739, 3.418, TEMPSUMMON_DEAD_DESPAWN, 0))
                RingBossGUID = tmp->GetGUID();

            MobDeath_Timer = 2500;
        }

        void WaypointReached(uint32 i)
        {
            switch (i)
            {
            case 0:
                DoScriptText(SCRIPT_TEXT1, me);//2
                CanWalk = false;
                Event_Timer = 5000;
                break;
            case 1:
                DoScriptText(SCRIPT_TEXT2, me);//4
                CanWalk = false;
                Event_Timer = 5000;
                break;
            case 2:
                CanWalk = false;
                break;
            case 3:
                DoScriptText(SCRIPT_TEXT3, me);//5
                break;
            case 4:
                DoScriptText(SCRIPT_TEXT4, me);//6
                CanWalk = false;
                Event_Timer = 5000;
                break;
            case 5:
                if (instance)
                {
                    instance->SetData(TYPE_RING_OF_LAW, DONE);
                    sLog->outDebug("TSCR: npc_grimstone: event reached end and set complete.");
                }
                break;
            }
        }

        void HandleGameObject(uint32 id, bool open)
        {
            instance->HandleGameObject(instance->GetData64(id), open);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!instance)
                return;

            if (MobDeath_Timer)
            {
                if (MobDeath_Timer <= diff)
                {
                    MobDeath_Timer = 2500;

                    if (RingBossGUID)
                    {
                        Creature *boss = Unit::GetCreature(*me, RingBossGUID);
                        if (boss && !boss->isAlive() && boss->isDead())
                        {
                            RingBossGUID = 0;
                            Event_Timer = 5000;
                            MobDeath_Timer = 0;
                            return;
                        }
                        return;
                    }

                    for (uint8 i = 0; i < MAX_MOB_AMOUNT; ++i)
                    {
                        Creature *mob = Unit::GetCreature(*me, RingMobGUID[i]);
                        if (mob && !mob->isAlive() && mob->isDead())
                        {
                            RingMobGUID[i] = 0;
                            --MobCount;

                            //seems all are gone, so set timer to continue and discontinue this
                            if (!MobCount)
                            {
                                Event_Timer = 5000;
                                MobDeath_Timer = 0;
                            }
                        }
                    }
                } else MobDeath_Timer -= diff;
            }

            if (Event_Timer)
            {
                if (Event_Timer <= diff)
                {
                    switch (EventPhase)
                    {
                    case 0:
                        DoScriptText(SCRIPT_TEXT5, me);//1
                        HandleGameObject(DATA_ARENA4, false);
                        Start(false, false);
                        CanWalk = true;
                        Event_Timer = 0;
                        break;
                    case 1:
                        CanWalk = true;
                        Event_Timer = 0;
                        break;
                    case 2:
                        Event_Timer = 2000;
                        break;
                    case 3:
                        HandleGameObject(DATA_ARENA1, true);
                        Event_Timer = 3000;
                        break;
                    case 4:
                        CanWalk = true;
                        me->SetVisibility(VISIBILITY_OFF);
                        SummonRingMob();
                        Event_Timer = 8000;
                        break;
                    case 5:
                        SummonRingMob();
                        SummonRingMob();
                        Event_Timer = 8000;
                        break;
                    case 6:
                        SummonRingMob();
                        Event_Timer = 0;
                        break;
                    case 7:
                        me->SetVisibility(VISIBILITY_ON);
                        HandleGameObject(DATA_ARENA1, false);
                        DoScriptText(SCRIPT_TEXT6, me);//4
                        CanWalk = true;
                        Event_Timer = 0;
                        break;
                    case 8:
                        HandleGameObject(DATA_ARENA2, true);
                        Event_Timer = 5000;
                        break;
                    case 9:
                        me->SetVisibility(VISIBILITY_OFF);
                        SummonRingBoss();
                        Event_Timer = 0;
                        break;
                    case 10:
                        //if quest, complete
                        HandleGameObject(DATA_ARENA2, false);
                        HandleGameObject(DATA_ARENA3, true);
                        HandleGameObject(DATA_ARENA4, true);
                        CanWalk = true;
                        Event_Timer = 0;
                        break;
                    }
                    ++EventPhase;
                } else Event_Timer -= diff;
            }

            if (CanWalk)
                npc_escortAI::UpdateAI(diff);
           }
    };
};

/*######
## mob_phalanx
######*/

enum PhalanxSpells
{
    SPELL_THUNDERCLAP                                      = 8732,
    SPELL_FIREBALLVOLLEY                                   = 22425,
    SPELL_MIGHTYBLOW                                       = 14099
};
class mob_phalanx : public CreatureScript
{
public:
    mob_phalanx() : CreatureScript("mob_phalanx") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new mob_phalanxAI (creature);
    }

    struct mob_phalanxAI : public ScriptedAI
    {
        mob_phalanxAI(Creature *c) : ScriptedAI(c) {}

        uint32 ThunderClap_Timer;
        uint32 FireballVolley_Timer;
        uint32 MightyBlow_Timer;

        void Reset()
        {
            ThunderClap_Timer = 12000;
            FireballVolley_Timer =0;
            MightyBlow_Timer = 15000;
        }

        void UpdateAI(const uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            //ThunderClap_Timer
            if (ThunderClap_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_THUNDERCLAP);
                ThunderClap_Timer = 10000;
            } else ThunderClap_Timer -= diff;

            //FireballVolley_Timer
            if (me->GetHealth()*100 / me->GetMaxHealth() < 51)
            {
                if (FireballVolley_Timer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_FIREBALLVOLLEY);
                    FireballVolley_Timer = 15000;
                } else FireballVolley_Timer -= diff;
            }

            //MightyBlow_Timer
            if (MightyBlow_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_MIGHTYBLOW);
                MightyBlow_Timer = 10000;
            } else MightyBlow_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_kharan_mighthammer
######*/

enum KharamQuests
{
    QUEST_4001                                             = 4001,
    QUEST_4342                                             = 4342
};

#define GOSSIP_ITEM_KHARAN_1    "I need to know where the princess are, Kharan!"
#define GOSSIP_ITEM_KHARAN_2    "All is not lost, Kharan!"

#define GOSSIP_ITEM_KHARAN_3    "Gor'shak is my friend, you can trust me."
#define GOSSIP_ITEM_KHARAN_4    "Not enough, you need to tell me more."
#define GOSSIP_ITEM_KHARAN_5    "So what happened?"
#define GOSSIP_ITEM_KHARAN_6    "Continue..."
#define GOSSIP_ITEM_KHARAN_7    "So you suspect that someone on the inside was involved? That they were tipped off?"
#define GOSSIP_ITEM_KHARAN_8    "Continue with your story please."
#define GOSSIP_ITEM_KHARAN_9    "Indeed."
#define GOSSIP_ITEM_KHARAN_10   "The door is open, Kharan. You are a free man."
class npc_kharan_mighthammer : public CreatureScript
{
public:
    npc_kharan_mighthammer() : CreatureScript("npc_kharan_mighthammer") { }

    bool GossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        switch (uiAction)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                 player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);
                player->SEND_GOSSIP_MENU(2475, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);
                player->SEND_GOSSIP_MENU(2476, creature->GetGUID());
                break;

            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+4);
                player->SEND_GOSSIP_MENU(2477, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_6, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+5);
                player->SEND_GOSSIP_MENU(2478, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+5:
                 player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_7, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+6);
                player->SEND_GOSSIP_MENU(2479, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+6:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_8, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+7);
                player->SEND_GOSSIP_MENU(2480, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+7:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_9, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+8);
                player->SEND_GOSSIP_MENU(2481, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+8:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_10, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+9);
                player->SEND_GOSSIP_MENU(2482, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+9:
                player->CLOSE_GOSSIP_MENU();
                if (player->GetTeam() == HORDE)
                    player->AreaExploredOrEventHappens(QUEST_4001);
                else
                    player->AreaExploredOrEventHappens(QUEST_4342);
                break;
        }
        return true;
    }

    bool GossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_4001) == QUEST_STATUS_INCOMPLETE)
             player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        if (player->GetQuestStatus(4342) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_KHARAN_2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+3);

        if (player->GetTeam() == HORDE)
            player->SEND_GOSSIP_MENU(2473, creature->GetGUID());
        else
            player->SEND_GOSSIP_MENU(2474, creature->GetGUID());

        return true;
    }
};

/*######
## npc_lokhtos_darkbargainer
######*/

enum LokhtosItems
{
    ITEM_THRORIUM_BROTHERHOOD_CONTRACT                     = 18628,
    ITEM_SULFURON_INGOT                                    = 17203
};

enum LokhtosQuests
{
    QUEST_A_BINDING_CONTRACT                               = 7604
};

enum LokhtosSpells
{
    SPELL_CREATE_THORIUM_BROTHERHOOD_CONTRACT_DND          = 23059
};

#define GOSSIP_ITEM_SHOW_ACCESS     "Show me what I have access to, Lothos."
#define GOSSIP_ITEM_GET_CONTRACT    "Get Thorium Brotherhood Contract"
class npc_lokhtos_darkbargainer : public CreatureScript
{
public:
    npc_lokhtos_darkbargainer() : CreatureScript("npc_lokhtos_darkbargainer") { }

    bool GossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            player->CastSpell(player, SPELL_CREATE_THORIUM_BROTHERHOOD_CONTRACT_DND, false);
        }
        if (uiAction == GOSSIP_ACTION_TRADE)
            player->SEND_VENDORLIST(creature->GetGUID());

        return true;
    }

    bool GossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->isVendor() && player->GetReputationRank(59) >= REP_FRIENDLY)
              player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_ITEM_SHOW_ACCESS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        if (player->GetQuestRewardStatus(QUEST_A_BINDING_CONTRACT) != 1 &&
            !player->HasItemCount(ITEM_THRORIUM_BROTHERHOOD_CONTRACT, 1, true) &&
            player->HasItemCount(ITEM_SULFURON_INGOT, 1))
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GET_CONTRACT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        }

        if (player->GetReputationRank(59) < REP_FRIENDLY)
            player->SEND_GOSSIP_MENU(3673, creature->GetGUID());
        else
            player->SEND_GOSSIP_MENU(3677, creature->GetGUID());

        return true;
    }
};

/*######
## npc_dughal_stormwing
######*/

enum DughalQuests
{
    QUEST_JAIL_BREAK                                       = 4322
};

#define SAY_DUGHAL_FREE         "Thank you, $N! I'm free!!!"
#define GOSSIP_DUGHAL           "You're free, Dughal! Get out of here!"

/*class npc_dughal_stormwing : public CreatureScript
{
public:
    npc_dughal_stormwing() : CreatureScript("npc_dughal_stormwing") { }

    CreatureAI* GetAI(Creature* creature)
    {
        npc_dughal_stormwingAI* dughal_stormwingAI = new npc_dughal_stormwingAI(creature);

        dughal_stormwingAI->AddWaypoint(0, 280.42,-82.86, -77.12, 0);
        dughal_stormwingAI->AddWaypoint(1, 287.64,-87.01, -76.79, 0);
        dughal_stormwingAI->AddWaypoint(2, 354.63,-64.95, -67.53, 0);

        return dughal_stormwingAI;
    }

    bool GossipSelect(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_escortAI, (creature->AI()))->Start(false, true, player->GetGUID());
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            instance->SetData(DATA_QUEST_JAIL_BREAK, ENCOUNTER_STATE_IN_PROGRESS);
        }
        return true;
    }

    bool GossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_JAIL_BREAK) == QUEST_STATUS_INCOMPLETE && instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_IN_PROGRESS)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_DUGHAL, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(2846, creature->GetGUID());
        }
        return true;
    }

    struct npc_dughal_stormwingAI : public npc_escortAI
    {
        npc_dughal_stormwingAI(Creature *c) : npc_escortAI(c) {}

        void WaypointReached(uint32 i)
        {
        switch (i)
            {
            case 0:me->Say(SAY_DUGHAL_FREE, LANG_UNIVERSAL, PlayerGUID); break;
            case 1:instance->SetData(DATA_DUGHAL, ENCOUNTER_STATE_OBJECTIVE_COMPLETED);break;
            case 2:
                me->SetVisibility(VISIBILITY_OFF);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                instance->SetData(DATA_DUGHAL, ENCOUNTER_STATE_ENDED);
                break;
            }
        }

        void EnterCombat(Unit* who) {}
        void Reset() {}

        void JustDied(Unit* killer)
        {
            if (IsBeingEscorted && killer == me)
            {
                me->SetVisibility(VISIBILITY_OFF);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                instance->SetData(DATA_DUGHAL, ENCOUNTER_STATE_ENDED);
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_NOT_STARTED) return;
            if ((instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_IN_PROGRESS || instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_FAILED || instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_ENDED)&& instance->GetData(DATA_DUGHAL) == ENCOUNTER_STATE_ENDED)
            {
                me->SetVisibility(VISIBILITY_OFF);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            else
            {
                me->SetVisibility(VISIBILITY_ON);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            npc_escortAI::UpdateAI(diff);
        }
    };
};

 */
/*######
## npc_marshal_windsor
######*/

#define SAY_WINDSOR_AGGRO1          "You locked up the wrong Marshal. Prepare to be destroyed!"
#define SAY_WINDSOR_AGGRO2          "I bet you're sorry now, aren't you !?!!"
#define SAY_WINDSOR_AGGRO3          "You better hold me back $N or they are going to feel some prison house beatings."
#define SAY_WINDSOR_1               "Let's get a move on. My gear should be in the storage area up this way..."
#define SAY_WINDSOR_4_1             "Check that cell, $N. If someone is alive in there, we need to get them out."
#define SAY_WINDSOR_4_2             "Get him out of there!"
#define SAY_WINDSOR_4_3             "Good work! We're almost there, $N. This way."
#define SAY_WINDSOR_6               "This is it, $N. My stuff should be in that room. Cover me, I'm going in!"
#define SAY_WINDSOR_9               "Ah, there it is!"
#define MOB_ENTRY_REGINALD_WINDSOR  9682

Player* pPlayerStart;
/*class npc_marshal_windsor : public CreatureScript
{
public:
    npc_marshal_windsor() : CreatureScript("npc_marshal_windsor") { }

    CreatureAI* GetAI(Creature* creature)
    {
        npc_marshal_windsorAI* marshal_windsorAI = new npc_marshal_windsorAI(creature);

        marshal_windsorAI->AddWaypoint(0, 316.336,-225.528, -77.7258, 7000);
        marshal_windsorAI->AddWaypoint(1, 316.336,-225.528, -77.7258, 2000);
        marshal_windsorAI->AddWaypoint(2, 322.96,-207.13, -77.87, 0);
        marshal_windsorAI->AddWaypoint(3, 281.05,-172.16, -75.12, 0);
        marshal_windsorAI->AddWaypoint(4, 272.19,-139.14, -70.61, 0);
        marshal_windsorAI->AddWaypoint(5, 283.62,-116.09, -70.21, 0);
        marshal_windsorAI->AddWaypoint(6, 296.18,-94.30, -74.08, 0);
        marshal_windsorAI->AddWaypoint(7, 294.57,-93.11, -74.08, 0);
        marshal_windsorAI->AddWaypoint(8, 314.31,-74.31, -76.09, 0);
        marshal_windsorAI->AddWaypoint(9, 360.22,-62.93, -66.77, 0);
        marshal_windsorAI->AddWaypoint(10, 383.38,-69.40, -63.25, 0);
        marshal_windsorAI->AddWaypoint(11, 389.99,-67.86, -62.57, 0);
        marshal_windsorAI->AddWaypoint(12, 400.98,-72.01, -62.31, 0);
        marshal_windsorAI->AddWaypoint(13, 404.22,-62.30, -63.50, 2300);
        marshal_windsorAI->AddWaypoint(14, 404.22,-62.30, -63.50, 1500);
        marshal_windsorAI->AddWaypoint(15, 407.65,-51.86, -63.96, 0);
        marshal_windsorAI->AddWaypoint(16, 403.61,-51.71, -63.92, 1000);
        marshal_windsorAI->AddWaypoint(17, 403.61,-51.71, -63.92, 2000);
        marshal_windsorAI->AddWaypoint(18, 403.61,-51.71, -63.92, 1000);
        marshal_windsorAI->AddWaypoint(19, 403.61,-51.71, -63.92, 0);

        return marshal_windsorAI;
    }

    bool QuestAccept(Player* player, Creature* creature, Quest const *quest)
    {
        if (quest->GetQuestId() == 4322)
            {PlayerStart = player;
            if (instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_NOT_STARTED)
            {
                    CAST_AI(npc_escortAI, (creature->AI()))->Start(true, false, player->GetGUID());
                    instance->SetData(DATA_QUEST_JAIL_BREAK, ENCOUNTER_STATE_IN_PROGRESS);
                    creature->setFaction(11);
            }
            }
        return false;
    }

    struct npc_marshal_windsorAI : public npc_escortAI
    {
        npc_marshal_windsorAI(Creature *c) : npc_escortAI(c)
        {
            instance = c->GetInstanceScript();
        }

        void WaypointReached(uint32 i)
        {
        switch (i)
            {
            case 1:
                me->Say(SAY_WINDSOR_1, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 7:
                me->HandleEmoteCommand(EMOTE_STATE_POINT);
                me->Say(SAY_WINDSOR_4_1, LANG_UNIVERSAL, PlayerGUID);
                IsOnHold=true;
                break;
            case 10:
                me->setFaction(534);
                break;
            case 12:
                me->Say(SAY_WINDSOR_6, LANG_UNIVERSAL, PlayerGUID);
                instance->SetData(DATA_SUPPLY_ROOM, ENCOUNTER_STATE_IN_PROGRESS);
                break;
            case 13:
                me->HandleEmoteCommand(EMOTE_STATE_USESTANDING);//EMOTE_STATE_WORK
                break;
            case 14:
                instance->SetData(DATA_GATE_SR, 0);
                me->setFaction(11);
                break;
            case 16:
                me->Say(SAY_WINDSOR_9, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 17:
                me->HandleEmoteCommand(EMOTE_STATE_USESTANDING);//EMOTE_STATE_WORK
                break;
            case 18:
                instance->SetData(DATA_GATE_SC, 0);
                break;
            case 19:
                me->SetVisibility(VISIBILITY_OFF);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                me->SummonCreature(MOB_ENTRY_REGINALD_WINDSOR, 403.61,-51.71,-63.92, 3.600434, TEMPSUMMON_DEAD_DESPAWN , 0);
                instance->SetData(DATA_SUPPLY_ROOM, ENCOUNTER_STATE_ENDED);
                break;
            }
        }

        void EnterCombat(Unit* who)
            {
            switch (urand(0, 2))
            {
                case 0: me->Say(SAY_WINDSOR_AGGRO1, LANG_UNIVERSAL, PlayerGUID); break;
                case 1: me->Say(SAY_WINDSOR_AGGRO2, LANG_UNIVERSAL, PlayerGUID); break;
                case 2: me->Say(SAY_WINDSOR_AGGRO3, LANG_UNIVERSAL, PlayerGUID); break;
            }
            }

        void Reset() {}

        void JustDied(Unit *slayer)
        {
            instance->SetData(DATA_QUEST_JAIL_BREAK, ENCOUNTER_STATE_FAILED);
        }

        void UpdateAI(const uint32 diff)
        {
            if (instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_NOT_STARTED) return;
            if (instance->GetData(DATA_DUGHAL) == ENCOUNTER_STATE_OBJECTIVE_COMPLETED)
                SetEscortPaused(false);
            if (!instance->GetData(DATA_GATE_D) && instance->GetData(DATA_DUGHAL) == ENCOUNTER_STATE_NOT_STARTED)
                {
                me->Say(SAY_WINDSOR_4_2, LANG_UNIVERSAL, PlayerGUID);
                instance->SetData(DATA_DUGHAL, ENCOUNTER_STATE_BEFORE_START);
                }
            if (instance->GetData(DATA_DUGHAL) == ENCOUNTER_STATE_OBJECTIVE_COMPLETED)
                {
                me->Say(SAY_WINDSOR_4_3, LANG_UNIVERSAL, PlayerGUID);
                instance->SetData(DATA_DUGHAL, ENCOUNTER_STATE_ENDED);
                }
            if ((instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_IN_PROGRESS || instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_FAILED || instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_ENDED)&& instance->GetData(DATA_SUPPLY_ROOM) == ENCOUNTER_STATE_ENDED)
            {
                me->SetVisibility(VISIBILITY_OFF);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            else
            {
                me->SetVisibility(VISIBILITY_ON);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            npc_escortAI::UpdateAI(diff);
        }
    };
};

  */
/*######
## npc_marshal_reginald_windsor
######*/

#define SAY_REGINALD_WINDSOR_0_1    "Can you feel the power, $N??? It's time to ROCK!"
#define SAY_REGINALD_WINDSOR_0_2    "Now we just have to free Tobias and we can get out of here. This way!"
#define SAY_REGINALD_WINDSOR_5_1    "Open it."
#define SAY_REGINALD_WINDSOR_5_2    "I never did like those two. Let's get moving."
#define SAY_REGINALD_WINDSOR_7_1    "Open it and be careful this time!"
#define SAY_REGINALD_WINDSOR_7_2    "That intolerant dirtbag finally got what was coming to him. Good riddance!"
#define SAY_REGINALD_WINDSOR_7_3    "Alright, let's go."
#define SAY_REGINALD_WINDSOR_13_1   "Open it. We need to hurry up. I can smell those Dark Irons coming a mile away and I can tell you one thing, they're COMING!"
#define SAY_REGINALD_WINDSOR_13_2   "Administering fists of fury on Crest Killer!"
#define SAY_REGINALD_WINDSOR_13_3   "He has to be in the last cell. Unless... they killed him."
#define SAY_REGINALD_WINDSOR_14_1   "Get him out of there!"
#define SAY_REGINALD_WINDSOR_14_2   "Excellent work, $N. Let's find the exit. I think I know the way. Follow me!"
#define SAY_REGINALD_WINDSOR_20_1   "We made it!"
#define SAY_REGINALD_WINDSOR_20_2   "Meet me at Maxwell's encampment. We'll go over the next stages of the plan there and figure out a way to decode my tablets without the decryption ring."
#define MOB_ENTRY_SHILL_DINGER      9678
#define MOB_ENTRY_CREST_KILLER      9680

int wp = 0;
/*class npc_marshal_reginald_windsor : public CreatureScript
{
public:
    npc_marshal_reginald_windsor() : CreatureScript("npc_marshal_reginald_windsor") { }

    CreatureAI* GetAI(Creature* creature)
    {
        npc_marshal_reginald_windsorAI* marshal_reginald_windsorAI = new npc_marshal_reginald_windsorAI(creature);

        marshal_reginald_windsorAI->AddWaypoint(0, 403.61,-52.71, -63.92, 4000);
        marshal_reginald_windsorAI->AddWaypoint(1, 403.61,-52.71, -63.92, 4000);
        marshal_reginald_windsorAI->AddWaypoint(2, 406.33,-54.87, -63.95, 0);
        marshal_reginald_windsorAI->AddWaypoint(3, 407.99,-73.91, -62.26, 0);
        marshal_reginald_windsorAI->AddWaypoint(4, 557.03,-119.71, -61.83, 0);
        marshal_reginald_windsorAI->AddWaypoint(5, 573.40,-124.39, -65.07, 0);
        marshal_reginald_windsorAI->AddWaypoint(6, 593.91,-130.29, -69.25, 0);
        marshal_reginald_windsorAI->AddWaypoint(7, 593.21,-132.16, -69.25, 0);
        marshal_reginald_windsorAI->AddWaypoint(8, 593.21,-132.16, -69.25, 3000);
        marshal_reginald_windsorAI->AddWaypoint(9, 622.81,-135.55, -71.92, 0);
        marshal_reginald_windsorAI->AddWaypoint(10, 634.68,-151.29, -70.32, 0);
        marshal_reginald_windsorAI->AddWaypoint(11, 635.06,-153.25, -70.32, 0);
        marshal_reginald_windsorAI->AddWaypoint(12, 635.06,-153.25, -70.32, 3000);
        marshal_reginald_windsorAI->AddWaypoint(13, 635.06,-153.25, -70.32, 1500);
        marshal_reginald_windsorAI->AddWaypoint(14, 655.25,-172.39, -73.72, 0);
        marshal_reginald_windsorAI->AddWaypoint(15, 654.79,-226.30, -83.06, 0);
        marshal_reginald_windsorAI->AddWaypoint(16, 622.85,-268.85, -83.96, 0);
        marshal_reginald_windsorAI->AddWaypoint(17, 579.45,-275.56, -80.44, 0);
        marshal_reginald_windsorAI->AddWaypoint(18, 561.19,-266.85, -75.59, 0);
        marshal_reginald_windsorAI->AddWaypoint(19, 547.91,-253.92, -70.34, 0);
        marshal_reginald_windsorAI->AddWaypoint(20, 549.20,-252.40, -70.34, 0);
        marshal_reginald_windsorAI->AddWaypoint(21, 549.20,-252.40, -70.34, 4000);
        marshal_reginald_windsorAI->AddWaypoint(22, 555.33,-269.16, -74.40, 0);
        marshal_reginald_windsorAI->AddWaypoint(23, 554.31,-270.88, -74.40, 0);
        marshal_reginald_windsorAI->AddWaypoint(24, 554.31,-270.88, -74.40, 4000);
        marshal_reginald_windsorAI->AddWaypoint(25, 536.10,-249.60, -67.47, 0);
        marshal_reginald_windsorAI->AddWaypoint(26, 520.94,-216.65, -59.28, 0);
        marshal_reginald_windsorAI->AddWaypoint(27, 505.99,-148.74, -62.17, 0);
        marshal_reginald_windsorAI->AddWaypoint(28, 484.21,-56.24, -62.43, 0);
        marshal_reginald_windsorAI->AddWaypoint(29, 470.39,-6.01, -70.10, 0);
        marshal_reginald_windsorAI->AddWaypoint(30, 451.27, 30.85, -70.07, 0);
        marshal_reginald_windsorAI->AddWaypoint(31, 452.45, 29.85, -70.37, 1500);
        marshal_reginald_windsorAI->AddWaypoint(32, 452.45, 29.85, -70.37, 7000);
        marshal_reginald_windsorAI->AddWaypoint(33, 452.45, 29.85, -70.37, 10000);
        marshal_reginald_windsorAI->AddWaypoint(34, 451.27, 31.85, -70.07, 0);

        return marshal_reginald_windsorAI;
    }

    struct npc_marshal_reginald_windsorAI : public npc_escortAI
    {
        npc_marshal_reginald_windsorAI(Creature *c) : npc_escortAI(c)
        {
        }

        void WaypointReached(uint32 i)
        {
        wp=i;
        switch (i)
            {
            case 0:
                me->setFaction(11);
                me->Say(SAY_REGINALD_WINDSOR_0_1, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 1:
                me->Say(SAY_REGINALD_WINDSOR_0_2, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 7:
                me->HandleEmoteCommand(EMOTE_STATE_POINT);
                me->Say(SAY_REGINALD_WINDSOR_5_1, LANG_UNIVERSAL, PlayerGUID);
                IsOnHold=true;
                break;
            case 8:
                me->Say(SAY_REGINALD_WINDSOR_5_2, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 11:
                me->HandleEmoteCommand(EMOTE_STATE_POINT);
                me->Say(SAY_REGINALD_WINDSOR_7_1, LANG_UNIVERSAL, PlayerGUID);
                IsOnHold=true;
                break;
            case 12:
                me->Say(SAY_REGINALD_WINDSOR_7_2, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 13:
                me->Say(SAY_REGINALD_WINDSOR_7_3, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 20:
                me->HandleEmoteCommand(EMOTE_STATE_POINT);
                me->Say(SAY_REGINALD_WINDSOR_13_1, LANG_UNIVERSAL, PlayerGUID);
                IsOnHold=true;
                break;
            case 21:
                me->Say(SAY_REGINALD_WINDSOR_13_3, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 23:
                me->HandleEmoteCommand(EMOTE_STATE_POINT);
                me->Say(SAY_REGINALD_WINDSOR_14_1, LANG_UNIVERSAL, PlayerGUID);
                IsOnHold=true;
                break;
            case 24:
                me->Say(SAY_REGINALD_WINDSOR_14_2, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 31:
                me->Say(SAY_REGINALD_WINDSOR_20_1, LANG_UNIVERSAL, PlayerGUID);
                break;
            case 32:
                me->Say(SAY_REGINALD_WINDSOR_20_2, LANG_UNIVERSAL, PlayerGUID);
                PlayerStart->GroupEventHappens(QUEST_JAIL_BREAK, me);
                instance->SetData(DATA_SHILL, ENCOUNTER_STATE_ENDED);
                break;
            }
        }

        void MoveInLineOfSight(Unit *who)
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
                return;

            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (CAST_PLR(who)->GetQuestStatus(4322) == QUEST_STATUS_INCOMPLETE)
                {
                    float Radius = 10.0;
                    if (me->IsWithinDistInMap(who, Radius))
                    {
                        SetEscortPaused(false);
                        Start(true, false, who->GetGUID());
                    }
                }
            }
        }

        void EnterCombat(Unit* who)
            {
            switch (urand(0, 2))
            {
                case 0: me->Say(SAY_WINDSOR_AGGRO1, LANG_UNIVERSAL, PlayerGUID); break;
                case 1: me->Say(SAY_WINDSOR_AGGRO2, LANG_UNIVERSAL, PlayerGUID); break;
                case 2: me->Say(SAY_WINDSOR_AGGRO3, LANG_UNIVERSAL, PlayerGUID); break;
            }
            }
        void Reset() {}

        void JustDied(Unit *slayer)
        {
            instance->SetData(DATA_QUEST_JAIL_BREAK, ENCOUNTER_STATE_FAILED);
        }

        void UpdateAI(const uint32 diff)
        {
            if (instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_NOT_STARTED) return;
            if (wp == 7)
                {
                if (!instance->GetData(DATA_GATE_J) && instance->GetData(DATA_JAZ) == ENCOUNTER_STATE_NOT_STARTED)
                    {
                        instance->SetData(DATA_CREATURE_JAZ, 1);
                        instance->SetData(DATA_JAZ, ENCOUNTER_STATE_IN_PROGRESS);
                    }
                if (instance->GetData(DATA_CREATURE_JAZ) && instance->GetData(DATA_CREATURE_OGRABISI) && instance->GetData(DATA_JAZ) == ENCOUNTER_STATE_IN_PROGRESS)
                    {
                        SetEscortPaused(false);
                        instance->SetData(DATA_JAZ, ENCOUNTER_STATE_ENDED);
                    }
                }
            else if (wp == 11)
                {
                if (!instance->GetData(DATA_GATE_S) && instance->GetData(DATA_SHILL) == ENCOUNTER_STATE_NOT_STARTED)
                    {
                        instance->SetData(DATA_CREATURE_SHILL, 1);
                        instance->SetData(DATA_SHILL, ENCOUNTER_STATE_IN_PROGRESS);
                    }
                if (instance->GetData(DATA_CREATURE_SHILL) && instance->GetData(DATA_SHILL) == ENCOUNTER_STATE_IN_PROGRESS)
                    {
                        instance->SetData(DATA_SHILL, ENCOUNTER_STATE_ENDED);
                        SetEscortPaused(false);
                    }
                }
            else if (wp == 20)
                {
                if (!instance->GetData(DATA_GATE_C) && instance->GetData(DATA_CREST) == ENCOUNTER_STATE_NOT_STARTED)
                    {
                        instance->SetData(DATA_CREATURE_CREST, 1);
                        me->Say(SAY_REGINALD_WINDSOR_13_2, LANG_UNIVERSAL, PlayerGUID);
                        instance->SetData(DATA_CREST, ENCOUNTER_STATE_IN_PROGRESS);
                    }
                if (instance->GetData(DATA_CREATURE_CREST) && instance->GetData(DATA_CREST) == ENCOUNTER_STATE_IN_PROGRESS)
                    {
                        SetEscortPaused(false);
                        instance->SetData(DATA_CREST, ENCOUNTER_STATE_ENDED);
                    }
                }
            if (instance->GetData(DATA_TOBIAS) == ENCOUNTER_STATE_OBJECTIVE_COMPLETED) SetEscortPaused(false);
            npc_escortAI::UpdateAI(diff);
        }
    };
};
*/
/*######
## npc_tobias_seecher
######*/

#define SAY_TOBIAS_FREE         "Thank you! I will run for safety immediately!"
/*class npc_tobias_seecher : public CreatureScript
{
public:
    npc_tobias_seecher() : CreatureScript("npc_tobias_seecher") { }

    CreatureAI* GetAI(Creature* creature)
    {
        npc_tobias_seecherAI* tobias_seecherAI = new npc_tobias_seecherAI(creature);

        tobias_seecherAI->AddWaypoint(0, 549.21, -281.07, -75.27);
        tobias_seecherAI->AddWaypoint(1, 554.39, -267.39, -73.68);
        tobias_seecherAI->AddWaypoint(2, 533.59, -249.38, -67.04);
        tobias_seecherAI->AddWaypoint(3, 519.44, -217.02, -59.34);
        tobias_seecherAI->AddWaypoint(4, 506.55, -153.49, -62.34);

        return tobias_seecherAI;
    }

    bool GossipSelect(Player* player, Creature* creature, uint32 uiSender, uint32 uiAction)
    {
        if (uiAction == GOSSIP_ACTION_INFO_DEF + 1)
        {
            player->CLOSE_GOSSIP_MENU();
            CAST_AI(npc_escortAI, (creature->AI()))->Start(false, true, player->GetGUID());
            creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            instance->SetData(DATA_TOBIAS, ENCOUNTER_STATE_IN_PROGRESS);
        }
        return true;
    }

    bool GossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(QUEST_JAIL_BREAK) == QUEST_STATUS_INCOMPLETE && instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_IN_PROGRESS)
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Get out of here, Tobias, you're free!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(2847, creature->GetGUID());
        }
        return true;
    }

    struct npc_tobias_seecherAI : public npc_escortAI
    {
        npc_tobias_seecherAI(Creature *c) :npc_escortAI(c) {}

        void EnterCombat(Unit* who) {}
        void Reset() {}

        void JustDied(Unit* killer)
        {
            if (IsBeingEscorted && killer == me)
            {
                me->SetVisibility(VISIBILITY_OFF);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                instance->SetData(DATA_TOBIAS, ENCOUNTER_STATE_ENDED);
            }
        }

        void WaypointReached(uint32 i)
        {
        switch (i)
            {
            case 0:me->Say(SAY_TOBIAS_FREE, LANG_UNIVERSAL, PlayerGUID); break;
            case 2:
                instance->SetData(DATA_TOBIAS, ENCOUNTER_STATE_OBJECTIVE_COMPLETED);break;
            case 4:
                me->SetVisibility(VISIBILITY_OFF);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                instance->SetData(DATA_TOBIAS, ENCOUNTER_STATE_ENDED);
                break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_NOT_STARTED) return;
            if ((instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_IN_PROGRESS || instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_FAILED || instance->GetData(DATA_QUEST_JAIL_BREAK) == ENCOUNTER_STATE_ENDED)&& instance->GetData(DATA_TOBIAS) == ENCOUNTER_STATE_ENDED)
            {
                me->SetVisibility(VISIBILITY_OFF);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            else
            {
                me->SetVisibility(VISIBILITY_ON);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }
            npc_escortAI::UpdateAI(diff);
        }
    };
};

*/

/*######
## npc_rocknot
######*/

enum RocknotSays
{
    SAY_GOT_BEER                                           = -1230000
};

enum RocknotSpells
{
    SPELL_DRUNKEN_RAGE                                     = 14872
};

enum RocknotQuests
{
    QUEST_ALE                                              = 4295
};
class npc_rocknot : public CreatureScript
{
public:
    npc_rocknot() : CreatureScript("npc_rocknot") { }

    bool ChooseReward(Player* /*player*/, Creature* creature, const Quest *_Quest, uint32 /*item*/)
    {
        ScriptedInstance* instance = creature->GetInstanceScript();

        if (!instance)
            return true;

        if (instance->GetData(TYPE_BAR) == DONE || instance->GetData(TYPE_BAR) == SPECIAL)
            return true;

        if (_Quest->GetQuestId() == QUEST_ALE)
        {
            if (instance->GetData(TYPE_BAR) != IN_PROGRESS)
                instance->SetData(TYPE_BAR, IN_PROGRESS);

            instance->SetData(TYPE_BAR, SPECIAL);

            //keep track of amount in instance script, returns SPECIAL if amount ok and event in progress
            if (instance->GetData(TYPE_BAR) == SPECIAL)
            {
                DoScriptText(SAY_GOT_BEER, creature);
                creature->CastSpell(creature, SPELL_DRUNKEN_RAGE, false);
                if (npc_escortAI* pEscortAI = CAST_AI(npc_rocknotAI, creature->AI()))
                    pEscortAI->Start(false, false);
            }
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_rocknotAI(creature);
    }

    struct npc_rocknotAI : public npc_escortAI
    {
        npc_rocknotAI(Creature *c) : npc_escortAI(c)
        {
            instance = c->GetInstanceScript();
        }

        ScriptedInstance* instance;

        uint32 BreakKeg_Timer;
        uint32 BreakDoor_Timer;

        void Reset()
        {
            if (HasEscortState(STATE_ESCORT_ESCORTING))
                return;

            BreakKeg_Timer = 0;
            BreakDoor_Timer = 0;
        }

        void DoGo(uint32 id, uint32 state)
        {
            if (GameObject* pGo = instance->instance->GetGameObject(instance->GetData64(id)))
                pGo->SetGoState((GOState)state);
        }

        void WaypointReached(uint32 i)
        {
            if (!instance)
                return;

            switch (i)
            {
            case 1:
                me->HandleEmoteCommand(EMOTE_ONESHOT_KICK);
                break;
            case 2:
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACKUNARMED);
                break;
            case 3:
                me->HandleEmoteCommand(EMOTE_ONESHOT_ATTACKUNARMED);
                break;
            case 4:
                me->HandleEmoteCommand(EMOTE_ONESHOT_KICK);
                break;
            case 5:
                me->HandleEmoteCommand(EMOTE_ONESHOT_KICK);
                BreakKeg_Timer = 2000;
                break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!instance)
                return;

            if (BreakKeg_Timer)
            {
                if (BreakKeg_Timer <= diff)
                {
                    DoGo(DATA_GO_BAR_KEG, 0);
                    BreakKeg_Timer = 0;
                    BreakDoor_Timer = 1000;
                } else BreakKeg_Timer -= diff;
            }

            if (BreakDoor_Timer)
            {
                if (BreakDoor_Timer <= diff)
                {
                    DoGo(DATA_GO_BAR_DOOR, 2);
                    DoGo(DATA_GO_BAR_KEG_TRAP, 0);               //doesn't work very well, leaving code here for future
                    //spell by trap has effect61, this indicate the bar go hostile

                    if (Unit *tmp = Unit::GetUnit(*me, instance->GetData64(DATA_PHALANX)))
                        tmp->setFaction(14);

                    //for later, this event(s) has alot more to it.
                    //optionally, DONE can trigger bar to go hostile.
                    instance->SetData(TYPE_BAR, DONE);

                    BreakDoor_Timer = 0;
                } else BreakDoor_Timer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };
};

/*######
##
######*/

void AddSC_blackrock_depths()
{
    new go_shadowforge_brazier();
    new at_ring_of_law();
    new npc_grimstone();
    new mob_phalanx();
    new npc_kharan_mighthammer();
    new npc_lokhtos_darkbargainer();
    new npc_dughal_stormwing();
    new npc_tobias_seecher();
    new npc_marshal_windsor();
    new npc_marshal_reginald_windsor();
    new npc_rocknot();
}
