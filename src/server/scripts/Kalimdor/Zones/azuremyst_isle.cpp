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
SDName: Azuremyst_Isle
SD%Complete: 75
SDComment: Quest support: 9283, 9537, 9582, 9554, 9531, 9303(special flight path, proper model for mount missing). Injured Draenei cosmetic only, 9582.
SDCategory: Azuremyst Isle
EndScriptData */

/* ContentData
npc_draenei_survivor
npc_engineer_spark_overgrind
npc_injured_draenei
npc_magwin
npc_geezle
mob_nestlewood_owlkinclass go_ravager_cage : public GameObjectScript
{
public:
    go_ravager_cage() : GameObjectScript("go_ravager_cage") { }

    go_ravager_cage
    npc_death_ravager
    EndContentData */

    #include "ScriptPCH.h"
    #include "ScriptedEscortAI.h"
    #include <cmath>

    /*######
    ## npc_draenei_survivor
    ######*/

    enum eSurv
    {
        HEAL1           = -1000176,
        HEAL2           = -1000177,
        HEAL3           = -1000178,
        HEAL4           = -1000179,

        HELP1           = -1000180,
        HELP2           = -1000181,
        HELP3           = -1000182,
        HELP4           = -1000183
    };
};
class npc_draenei_survivor : public CreatureScript
{
public:
    npc_draenei_survivor() : CreatureScript("npc_draenei_survivor") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_draenei_survivorAI (creature);
    }

    struct npc_draenei_survivorAI : public ScriptedAI
    {
        npc_draenei_survivorAI(Creature *c) : ScriptedAI(c) {}

        uint32 UnSpawnTimer;
        uint32 ResetlifeTimer;
        uint32 SayingTimer;
        uint32 HealSayTimer;
        bool UnSpawn;
        bool say;
        bool HealSay;
        bool isRun;
        bool isMove;

        void Reset()
        {
            UnSpawnTimer = 2500;
            ResetlifeTimer= 60000;
            SayingTimer    = 5000;
            HealSayTimer = 6000;
            say = false;
            isRun = false;
            isMove = false;
            UnSpawn    = false;
            HealSay = false;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            //cast red shining
            me->CastSpell(me, 29152, false, NULL);
            //set creature health
            me->SetHealth(int(me->GetMaxHealth()*.1));
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, 3);
        }

        void EnterCombat(Unit *who) {}

        void MoveInLineOfSight(Unit *who)                       //MoveInLineOfSight is called if creature could see you, updated all 100 ms
        {
            if (!who)
                return;

            if (who->GetTypeId() == TYPEID_PLAYER && me->IsFriendlyTo(who) && me->IsWithinDistInMap(who, 15) && say && !isRun)
            {
                switch (rand()%4)                               //Random switch between 4 texts
                {
                    case 0:
                        DoScriptText(HELP1, me);
                        SayingTimer = 15000;
                        say = false;
                        break;
                    case 1:
                        DoScriptText(HELP2, me);
                        SayingTimer = 15000;
                        say = false;
                        break;
                    case 2:
                        DoScriptText(HELP3, me);
                        SayingTimer = 15000;
                        say = false;
                        break;
                    case 3:
                        DoScriptText(HELP4, me);
                        SayingTimer = 15000;
                        say = false;
                        break;
                }
            }
            else
            {
                isRun = false;
            }
        }

        void UpdateAI(const uint32 diff)                        //Is also called each ms for Creature AI Updates...
        {
            if (me->GetHealth() > 50)
            {
                if (ResetlifeTimer <= diff)
                {
                    ResetlifeTimer = 60000;
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
                    //set creature health
                    me->SetHealth(int(me->GetMaxHealth()*.1));
                    // ley down
                    me->SetUInt32Value(UNIT_FIELD_BYTES_1, 3);
                }
                else ResetlifeTimer -= diff;
            }

            if (HealSay)
            {
                if (HealSayTimer <= diff)
                {
                    UnSpawn = true;
                    isRun = true;
                    isMove = true;
                } else HealSayTimer -= diff;
            }

            if (UnSpawn)
            {
                if (isMove)
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(0, -4115.053711f, -13754.831055f, 73.508949f);
                    isMove = false;
                }

                if (UnSpawnTimer <= diff)
                {
                    me->StopMoving();
                    EnterEvadeMode();
                    //set creature health
                    me->SetHealth(int(me->GetMaxHealth()*.1));
                    return;
                } else UnSpawnTimer -= diff;
            }

            if (SayingTimer <= diff)
            {
                say = true;
            } else SayingTimer -= diff;
        }

        void SpellHit(Unit *Hitter, const SpellEntry *Spellkind)//Called if you cast a spell and do some things if Specified spell is true!
        {
            if (Hitter && Spellkind->Id == 28880)
            {
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, 0);
                me->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_CONFUSED);
                me->HandleEmoteCommand(ANIM_RISE);
                switch (rand()%4)                               //This switch doesn't work at all, creature say nothing!
                {
                    case 0: DoScriptText(HEAL1, me, Hitter); break;
                    case 1: DoScriptText(HEAL2, me, Hitter); break;
                    case 2: DoScriptText(HEAL3, me, Hitter); break;
                    case 3: DoScriptText(HEAL4, me, Hitter); break;
                }
                HealSay    = true;
            }
        }
    };
};

/*######
## npc_engineer_spark_overgrind
######*/

enum eSpark
{
    SAY_TEXT                = -1000184,
    SAY_EMOTE               = -1000185,
    ATTACK_YELL             = -1000186,

    SPELL_DYNAMITE          = 7978
};

#define GOSSIP_FIGHT    "Traitor! You will be brought to justice!"
class npc_engineer_spark_overgrind : public CreatureScript
{
public:
    npc_engineer_spark_overgrind() : CreatureScript("npc_engineer_spark_overgrind") { }

    bool GossipSelect(Player* player, Creature* creature, uint32 sender, uint32 action)
    {
        if (action == GOSSIP_ACTION_INFO_DEF)
        {
            player->CLOSE_GOSSIP_MENU();
            creature->setFaction(14);
            DoScriptText(ATTACK_YELL, creature, player);
            ((npc_engineer_spark_overgrindAI*)creature->AI())->AttackStart(player);
        }
        return true;
    }

    bool GossipHello(Player* player, Creature* creature)
    {
        if (player->GetQuestStatus(9537) == QUEST_STATUS_INCOMPLETE)
            player->ADD_GOSSIP_ITEM(0, GOSSIP_FIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_engineer_spark_overgrindAI (creature);
    }

    struct npc_engineer_spark_overgrindAI : public ScriptedAI
    {
        npc_engineer_spark_overgrindAI(Creature *c) : ScriptedAI(c) {}

        uint32 Dynamite_Timer;
        uint32 Emote_Timer;

        void Reset()
        {
            Dynamite_Timer = 8000;
            Emote_Timer = 120000 + rand()%30000;
            me->setFaction(875);
        }

        void EnterCombat(Unit *who) { }

        void UpdateAI(const uint32 diff)
        {
            if (!me->isInCombat())
            {
                if (Emote_Timer <= diff)
                {
                    DoScriptText(SAY_TEXT, me);
                    DoScriptText(SAY_EMOTE, me);
                    Emote_Timer = 120000 + rand()%30000;
                } else Emote_Timer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (Dynamite_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_DYNAMITE);
                Dynamite_Timer = 8000;
            } else Dynamite_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

/*######
## npc_injured_draenei
######*/
class npc_injured_draenei : public CreatureScript
{
public:
    npc_injured_draenei() : CreatureScript("npc_injured_draenei") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_injured_draeneiAI (creature);
    }

    struct npc_injured_draeneiAI : public ScriptedAI
    {
        npc_injured_draeneiAI(Creature *c) : ScriptedAI(c) {}

        void Reset()
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
            me->SetHealth(int(me->GetMaxHealth()*.15));
            switch (rand()%2)
            {
                case 0: me->SetUInt32Value(UNIT_FIELD_BYTES_1, 1); break;
                case 1: me->SetUInt32Value(UNIT_FIELD_BYTES_1, 3); break;
            }
        }

        void EnterCombat(Unit * /*who*/) {}

        void MoveInLineOfSight(Unit * /*who*/)
        {
        }

        void UpdateAI(const uint32 /*diff*/)
        {
        }
    };
};

/*######
## npc_magwin
######*/

enum eMagwin
{
    SAY_START                   = -1000111,
    SAY_AGGRO                   = -1000112,
    SAY_PROGRESS                = -1000113,
    SAY_END1                    = -1000114,
    SAY_END2                    = -1000115,
    EMOTE_HUG                   = -1000116,

    QUEST_A_CRY_FOR_SAY_HELP    = 9528
};

struct npc_magwinAI : public npc_escortAI
{
    npc_magwinAI(Creature *c) : npc_escortAI(c) {}

    void WaypointReached(uint32 i)
    {
        Player* player = GetPlayerForEscort();

        if (!player)
            return;

        switch (i)
        {
        case 0:
            DoScriptText(SAY_START, me, player);
            break;
        case 17:
            DoScriptText(SAY_PROGRESS, me, player);
            break;
        case 28:
            DoScriptText(SAY_END1, me, player);
            break;
        case 29:
            DoScriptText(EMOTE_HUG, me, player);
            DoScriptText(SAY_END2, me, player);
            player->GroupEventHappens(QUEST_A_CRY_FOR_SAY_HELP, me);
            break;
        }
    }

    void EnterCombat(Unit* who)
    {
        DoScriptText(SAY_AGGRO, me, who);
    }

    void Reset() { }
};
class npc_magwin : public CreatureScript
{
public:
    npc_magwin() : CreatureScript("npc_magwin") { }

    bool QuestAccept(Player* player, Creature* creature, Quest const* quest)
    {
        if (quest->GetQuestId() == QUEST_A_CRY_FOR_SAY_HELP)
        {
            creature->setFaction(113);
            if (npc_escortAI* pEscortAI = CAST_AI(npc_escortAI, creature->AI()))
                pEscortAI->Start(true, false, player->GetGUID());
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_magwinAI(creature);
    }
};

/*######
## npc_geezle
######*/

#define GEEZLE_SAY_1    -1000728
#define SPARK_SAY_2     -1000729
#define SPARK_SAY_3     -1000730
#define GEEZLE_SAY_4    -1000731
#define SPARK_SAY_5     -1000732
#define SPARK_SAY_6     -1000733
#define GEEZLE_SAY_7    -1000734

#define EMOTE_SPARK     -1000735

#define MOB_SPARK       17243
#define GO_NAGA_FLAG    181694

static float SparkPos[3] = {-5030.95f, -11291.99f, 7.97f};

struct npc_geezleAI : public ScriptedAI
{
    npc_geezleAI(Creature *c) : ScriptedAI(c) {}

    uint64 SparkGUID;

    uint32 Step;
    uint32 SayTimer;

    bool EventStarted;

    void Reset()
    {
        SparkGUID = 0;
        Step = 0;
        StartEvent();
    }

    void EnterCombat(Unit* /*who*/){}

    void StartEvent()
    {
        Step = 1;
        EventStarted = true;
        Creature* Spark = me->SummonCreature(MOB_SPARK, SparkPos[0], SparkPos[1], SparkPos[2], 0, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 1000);
        if (Spark)
        {
            SparkGUID = Spark->GetGUID();
            Spark->setActive(true);
            Spark->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            Spark->GetMotionMaster()->MovePoint(0, -5080.70f, -11253.61f, 0.56f);
        }
        me->GetMotionMaster()->MovePoint(0, -5092.26f, -11252, 0.71f);
        SayTimer = 23000;
    }

    uint32 NextStep(uint32 Step)
    {
        Unit* Spark = Unit::GetUnit((*me), SparkGUID);

        switch (Step)
        {
        case 0: return 99999;
        case 1:
            //DespawnNagaFlag(true);
            DoScriptText(EMOTE_SPARK, Spark);
            return 1000;
        case 2:
            DoScriptText(GEEZLE_SAY_1, me, Spark);
            if (Spark)
            {
                Spark->SetInFront(me);
                me->SetInFront(Spark);
            }
            return 5000;
        case 3: DoScriptText(SPARK_SAY_2, Spark); return 7000;
        case 4: DoScriptText(SPARK_SAY_3, Spark); return 8000;
        case 5: DoScriptText(GEEZLE_SAY_4, me, Spark); return 8000;
        case 6: DoScriptText(SPARK_SAY_5, Spark); return 9000;
        case 7: DoScriptText(SPARK_SAY_6, Spark); return 8000;
        case 8: DoScriptText(GEEZLE_SAY_7, me, Spark); return 2000;
        case 9:
            me->GetMotionMaster()->MoveTargetedHome();
            if (Spark)
                Spark->GetMotionMaster()->MovePoint(0, -5030.95f, -11291.99f, 7.97f);
            return 20000;
        case 10:
            if (Spark)
                Spark->DealDamage(Spark, Spark->GetHealth(),NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            //DespawnNagaFlag(false);
            me->SetVisibility(VISIBILITY_OFF);
        default: return 99999999;
        }
    }

    void DespawnNagaFlag(bool despawn)
    {
        std::list<GameObject*> FlagList;
        me->GetGameObjectListWithEntryInGrid(FlagList, GO_NAGA_FLAG, 100.0f);

        if (!FlagList.empty())
        {
            for (std::list<GameObject*>::iterator itr = FlagList.begin(); itr != FlagList.end(); ++itr)
            {
                //TODO: Found how to despawn and respawn
                if (despawn)
                    (*itr)->RemoveFromWorld();
                else
                    (*itr)->Respawn();
            }
        } else sLog->outError("SD2 ERROR: FlagList is empty!");
    }

    void UpdateAI(const uint32 diff)
    {
        if (SayTimer <= diff)
        {
            if (EventStarted)
            {
                SayTimer = NextStep(++Step);
            }
        } else SayTimer -= diff;
    }
};
class npc_geezle : public CreatureScript
{
public:
    npc_geezle() : CreatureScript("npc_geezle") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_geezleAI(creature);
    }
};

/*######
## mob_nestlewood_owlkin
######*/

#define INOCULATION_CHANNEL 29528
#define INOCULATED_OWLKIN   16534

struct mob_nestlewood_owlkinAI : public ScriptedAI
{
    mob_nestlewood_owlkinAI(Creature *c) : ScriptedAI(c) {}

    uint32 ChannelTimer;
    bool Channeled;
    bool Hitted;

    void Reset()
    {
        ChannelTimer = 0;
        Channeled = false;
        Hitted = false;
    }

    void EnterCombat(Unit *who){}

    void SpellHit(Unit* caster, const SpellEntry* spell)
    {
        if (!caster)
            return;

        if (caster->GetTypeId() == TYPEID_PLAYER && spell->Id == INOCULATION_CHANNEL)
        {
            ChannelTimer = 3000;
            Hitted = true;
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (ChannelTimer <= diff && !Channeled && Hitted)
        {
            me->DealDamage(me, me->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
            me->RemoveCorpse();
            me->SummonCreature(INOCULATED_OWLKIN, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 180000);
            Channeled = true;
        } else ChannelTimer -= diff;

        DoMeleeAttackIfReady();
    }
};
class mob_nestlewood_owlkin : public CreatureScript
{
public:
    mob_nestlewood_owlkin() : CreatureScript("mob_nestlewood_owlkin") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new mob_nestlewood_owlkinAI (creature);
    }
};

enum eRavegerCage
{
    NPC_DEATH_RAVAGER       = 17556,

    SPELL_REND              = 13443,
    SPELL_ENRAGING_BITE     = 30736,

    QUEST_STRENGTH_ONE      = 9582
};

bool go_ravager_cage(Player* player, GameObject* pGo)
{
    if (player->GetQuestStatus(QUEST_STRENGTH_ONE) == QUEST_STATUS_INCOMPLETE)
    {
        if (Creature* ravager = pGo->FindNearestCreature(NPC_DEATH_RAVAGER, 5.0f, true))
        {
            ravager->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            ravager->SetReactState(REACT_AGGRESSIVE);
            ravager->AI()->AttackStart(player);
        }
    }
    return true ;
}

struct npc_death_ravagerAI : public ScriptedAI
{
    npc_death_ravagerAI(Creature *c) : ScriptedAI(c){}

    uint32 RendTimer;
    uint32 EnragingBiteTimer;

    void Reset()
    {
        RendTimer = 30000;
        EnragingBiteTimer = 20000;

        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void UpdateAI(const uint32 diff)
    {
        if (!UpdateVictim())
            return;

        if (RendTimer <= diff)
        {
            DoCast(me->getVictim(), SPELL_REND);
            RendTimer = 30000;
        }
        else RendTimer -= diff;

        if (EnragingBiteTimer <= diff)
        {
            DoCast(me->getVictim(), SPELL_ENRAGING_BITE);
            EnragingBiteTimer = 15000;
        }
        else EnragingBiteTimer -= diff;

        DoMeleeAttackIfReady();
    }
};
class npc_death_ravager : public CreatureScript
{
public:
    npc_death_ravager() : CreatureScript("npc_death_ravager") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_death_ravagerAI(creature);
    }
};

/*########
## Quest: The Prophecy of Akida
########*/

enum BristlelimbCage
{
    CAPITIVE_SAY_1                      = -1000602,
    CAPITIVE_SAY_2                      = -1000603,
    CAPITIVE_SAY_3                      = -1000604,

    QUEST_THE_PROPHECY_OF_AKIDA         = 9544,
    NPC_STILLPINE_CAPITIVE              = 17375,
    GO_BRISTELIMB_CAGE                  = 181714
};

struct npc_stillpine_capitiveAI : public ScriptedAI
{
    npc_stillpine_capitiveAI(Creature *c) : ScriptedAI(c){}

    uint32 FleeTimer;

    void Reset()
    {
        FleeTimer = 0;
        GameObject* cage = me->FindNearestGameObject(GO_BRISTELIMB_CAGE, 5.0f);
        if (cage)
            cage->ResetDoorOrButton();
    }

    void UpdateAI(const uint32 diff)
    {
        if (FleeTimer)
        {
            if (FleeTimer <= diff)
                me->ForcedDespawn();
            else FleeTimer -= diff;
        }
    }
};
class npc_stillpine_capitive : public CreatureScript
{
public:
    npc_stillpine_capitive() : CreatureScript("npc_stillpine_capitive") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new npc_stillpine_capitiveAI(creature);
    }
};
class go_bristlelimb_cage : public GameObjectScript
{
public:
    go_bristlelimb_cage() : GameObjectScript("go_bristlelimb_cage") { }

    bool go_bristlelimb_cage(Player* player, GameObject* pGo)
    {
        if (player->GetQuestStatus(QUEST_THE_PROPHECY_OF_AKIDA) == QUEST_STATUS_INCOMPLETE)
        {
            Creature* creature = pGo->FindNearestCreature(NPC_STILLPINE_CAPITIVE, 5.0f, true);
            if (creature)
            {
                DoScriptText(RAND(CAPITIVE_SAY_1, CAPITIVE_SAY_2, CAPITIVE_SAY_3), creature, player);
                creature->GetMotionMaster()->MoveFleeing(player, 3500);
                player->KilledMonsterCredit(creature->GetEntry(), creature->GetGUID());
                CAST_AI(npc_stillpine_capitiveAI, creature->AI())->FleeTimer = 3500;
                return false;
            }
        }
        return true;
    }
};

void AddSC_azuremyst_isle()
{
    new npc_draenei_survivor();
    new npc_engineer_spark_overgrind();
    new npc_injured_draenei();
    new npc_magwin();
    new npc_geezle();
    new mob_nestlewood_owlkin();
    new npc_death_ravager();
    new go_ravager_cage();
    new npc_stillpine_capitive();
    new go_bristlelimb_cage();
}
