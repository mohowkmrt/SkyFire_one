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
SDName: Boss_Moam
SD%Complete: 100
SDComment: VERIFY SCRIPT AND SQL
SDCategory: Ruins of Ahn'Qiraj
EndScriptData */

#include "ScriptPCH.h"

#define EMOTE_AGGRO             -1509000
#define EMOTE_MANA_FULL         -1509001

#define SPELL_TRAMPLE           15550
#define SPELL_DRAINMANA         27256
#define SPELL_ARCANEERUPTION    25672
#define SPELL_SUMMONMANA        25681
#define SPELL_GRDRSLEEP         24360                       //Greater Dreamless Sleep
class boss_moam : public CreatureScript
{
public:
    boss_moam() : CreatureScript("boss_moam") { }

    CreatureAI* GetAI(Creature* creature)
    {
        return new boss_moamAI (creature);
    }

    struct boss_moamAI : public ScriptedAI
    {
        boss_moamAI(Creature *c) : ScriptedAI(c) {}

        Unit *pTarget;
        uint32 TRAMPLE_Timer;
        uint32 DRAINMANA_Timer;
        uint32 SUMMONMANA_Timer;
        uint32 i;
        uint32 j;

        void Reset()
        {
            i = 0;
            j=0;
            pTarget = NULL;
            TRAMPLE_Timer = 30000;
            DRAINMANA_Timer = 30000;
        }

        void EnterCombat(Unit *who)
        {
            DoScriptText(EMOTE_AGGRO, me);
            pTarget = who;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            //If we are 100%MANA cast Arcane Erruption
            //if (j == 1 && me->GetMana()*100 / me->GetMaxMana() == 100 && !me->IsNonMeleeSpellCasted(false))
            {
                DoCast(me->getVictim(),SPELL_ARCANEERUPTION);
                DoScriptText(EMOTE_MANA_FULL, me);
            }

            //If we are <50%HP cast MANA FIEND (Summon Mana) and Sleep
            //if (i == 0 && me->GetHealth()*100 / me->GetMaxHealth() <= 50 && !me->IsNonMeleeSpellCasted(false))
            {
                i=1;
                DoCast(me->getVictim(),SPELL_SUMMONMANA);
                DoCast(me->getVictim(),SPELL_GRDRSLEEP);
            }

            //SUMMONMANA_Timer
            if (i == 1 && SUMMONMANA_Timer <= diff)
            {
                DoCast(me->getVictim(),SPELL_SUMMONMANA);
                SUMMONMANA_Timer = 90000;
            } else SUMMONMANA_Timer -= diff;

            //TRAMPLE_Timer
            if (TRAMPLE_Timer <= diff)
            {
                DoCast(me->getVictim(),SPELL_TRAMPLE);
                j=1;

                TRAMPLE_Timer = 30000;
            } else TRAMPLE_Timer -= diff;

            //DRAINMANA_Timer
            if (DRAINMANA_Timer <= diff)
            {
                DoCast(me->getVictim(),SPELL_DRAINMANA);
                DRAINMANA_Timer = 30000;
            } else DRAINMANA_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };
};

void AddSC_boss_moam()
{
    new boss_moam();
}
