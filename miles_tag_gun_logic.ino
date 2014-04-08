//milestag protocol 1 documented at http://www.lasertagparts.com/mtformat.htm
//milestag protocol 2 documented at http://www.lasertagparts.com/mtformat-2.htm
// we currently only implement MT1 as MT2 seems incomplete.

#include <util/parity.h>
#include "miles_tag_structs.h"

byte TeamID;
byte PlayerID;

unsigned int Life; //[0,999]
byte Armor; //[0,200]

//Gun Characteristics
byte Gun_ClipSize; //[1,250], else UNL
byte Gun_Clips; //[2,200], else UNL

//Actual Status
byte Ammo; //[0,ClipSize]
unsigned int AmmoRemaining;

boolean FriendlyFire;

//ignore the shot if friendlyfire is off and this is a shot from our team.
boolean defaultPreReceiveShot(byte teamId, byte playerId) {
  return !(FriendlyFire && (TeamID == teamId));
}

void defaultReceiveShot(struct shot *receivedShot) {
  Serial.print("Received ");
  Serial.print(receivedShot->damage,DEC);
  Serial.print(" damage from player ");
  Serial.print(receivedShot->playerId,DEC);
  Serial.print(" on team ");
  Serial.println(receivedShot->teamId,DEC);
  
  if (receivedShot->damage == MT1_DAMAGE_RESURRECT_OPPONENT) {
    Life = 100;
  }
  else {
    if (Armor) {
      byte ArmorDeflect = min(Armor, receivedShot->damage);
      Armor -= ArmorDeflect;
      Life -= ArmorDeflect / 4;
      receivedShot->damage -= ArmorDeflect;
    }
    Life = max(0, receivedShot->damage);
  }
  Serial.print("Health remaining: ");
  Serial.println(Life,DEC);
}

logicFunctions gameLogic = {&defaultPreReceiveShot, &defaultReceiveShot};

void mt_setup()
{
    TeamID = 0;
    PlayerID = 0;
    Life = 100;
    Armor = 100;
    
    Gun_ClipSize = 10;
    Gun_Clips = 255;
    
    
    Ammo = Gun_ClipSize;
    AmmoRemaining = Gun_Clips * Gun_ClipSize;
    
    FriendlyFire = false;
}

void mt_setPlayerID(unsigned int ID)
{
  PlayerID = ID;
}

unsigned int mt_getPlayerID()
{
  return PlayerID;
}

void mt_setTeamID(byte ID)
{
  TeamID = ID;
}

byte mt_getTeamID()
{
  return TeamID;
}

void mt_parseIRMessage(unsigned long recvBuffer)
{
    Serial.print("Received:");
    Serial.println(recvBuffer,HEX);
    byte recv_TeamID = (recvBuffer & MT1_TEAM_MASK) >> MT1_TEAM_SHIFT;
    unsigned long DataByte2 = recvBuffer & 0xff;
    
    if (recv_TeamID == SYSTEM_MESSAGE) {
        Serial.println("System message.");
        byte recv_SystemMessage = (recvBuffer >> SYSTEM_MESSAGE_SHIFT) & SYSTEM_MESSAGE_MASK;
        
        switch (recv_SystemMessage) {
            case SYSTEM_MESSAGE_SET_TEAM_ID:
                TeamID = DataByte2;
                break;
            case SYSTEM_MESSAGE_SET_PLAYER_ID:
                PlayerID = DataByte2;
                break;
            case SYSTEM_MESSAGE_ADD_HEALTH:
                Life += DataByte2;
                break;
            case SYSTEM_MESSAGE_ADD_CLIPS:
            {
                if (Gun_Clips == UNLIMITED_CLIPS) break;
                AmmoRemaining = min(Gun_Clips * Gun_ClipSize, AmmoRemaining + (DataByte2 * Gun_ClipSize));
                break;
            }
            case SYSTEM_MESSAGE_GOD_GUN:
            {
                byte recv_GodGun = DataByte2;
                switch (recv_GodGun) {
                    case GOD_GUN_KILL_PLAYER:
                        Serial.println("God killed me!");
                        Life = 0;
                        break;
                    case GOD_GUN_FULL_AMMO:
                        Ammo = Gun_ClipSize;
                        AmmoRemaining = Gun_Clips * Gun_ClipSize;
                        break;
                    case GOD_GUN_RESPAWN_PLAYER:
                        Life = 100;
                        break;
                    case GOD_GUN_PAUSE_PLAYER:
                    case GOD_GUN_START_GAME:
                    case GOD_GUN_INIT_PLAYER:
                    case GOD_GUN_END_PLAYER:
                    default:
                        Serial.println("Unknown GGM");
                        break;
                }
                
                break;
            }
            case SYSTEM_MESSAGE_ADD_ROUNDS:
                AmmoRemaining = min(Gun_Clips * Gun_ClipSize, AmmoRemaining + DataByte2);
                break;
            case SYSTEM_MESSAGE_ADD_RPG_ROUNDS:
            case SYSTEM_MESSAGE_SCORE_DATA_HEADER:
            case SYSTEM_MESSAGE_SCORE_REQUEST:
            default:
                Serial.println("Unknown SM");
                break;
        }
    } else {
        Serial.println("Shot");
        byte recv_PlayerID = (recvBuffer & MT1_PLAYER_MASK) >> MT1_PLAYER_SHIFT;

        if (!gameLogic.preRecieveShot(recv_TeamID, recv_PlayerID)) {
          return;
        }

        signed char damage;
        
        byte recv_PlayerWeaponHit = DataByte2;
        switch (recv_PlayerWeaponHit) {
            case 0:
            {
                damage = MT1_DAMAGE_RESURRECT_OPPONENT;
                break;
            }
            case 1 ... 100:
            {
                damage = recv_PlayerWeaponHit;
                break;
            }
            //No 'Base' Mode
            /*case 101 ... 200:
            {
                recv_PlayerWeaponHit -= 100;
                baseDamage = recv_PlayerWeaponHit;
            }
            case 255:
                baseDamage = MT1_DAMAGE_RESURRECT_ENEMY_BASE;
            */

            default:
                Serial.println("Unknown Message");
                break;
        }
        
        shot currShot = {recv_PlayerID, recv_TeamID, damage/*, baseDamage*/};
        
        gameLogic.recieveShot(&currShot);
    }
}

unsigned long mt_buildShot()
{
  unsigned long data = TeamID << 5 | PlayerID;
  data <<= 8;
  data += 12;
  
  return data;
}

/* Function to get parity of number n. It returns 1
   if n has odd parity, and returns 0 if n has even
   parity */
boolean getParity(unsigned long n)
{
    boolean parity = false;
    while (n != 0)
    {
        parity = !parity;
        n &= (n - 1);
    }
    return parity;
}
  
