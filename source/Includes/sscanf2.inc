/*
 *  sscanf 2.8.2
 *  Created by Y_Less, updated by Emmet_.
 *
 *  Version: MPL 1.1
 *
 *  The contents of this file are subject to the Mozilla Public License Version
 *  1.1 (the "License"); you may not use this file except in compliance with
 *  the License. You may obtain a copy of the License at
 *  [url]http://www.mozilla.org/MPL/[/url]
 *
 *  Software distributed under the License is distributed on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 *  for the specific language governing rights and limitations under the
 *  License.
 *
 *  The Original Code is the sscanf 2.0 SA:MP plugin.
 *
 *  The Initial Developer of the Original Code is Alex "Y_Less" Cole.
 *  Portions created by the Initial Developer are Copyright (C) 2010
 *  the Initial Developer. All Rights Reserved.
 *
 *  Contributor(s):
 *
 *  Special Thanks to:
 *
 *      SA:MP Team past, present and future
 */

#if defined _inc_a_npc
	#pragma library sscanf
#elseif !defined _samp_included
	#error Please include <a_npc> or <a_samp> first.
#endif

#define SSCANF:%0(%1) forward sscanf_%0(%1);public sscanf_%0(%1)

#if defined sscanf
	#error sscanf (possibly the PAWN version) already defined.
#endif

native sscanf(const data[], const format[], {Float,_}:...);
native unformat(const data[], const format[], {Float,_}:...) = sscanf;
native SSCANF_Init(players, invalid, len);
native SSCANF_Join(playerid, const ss2_name[], npc);
native SSCANF_Leave(playerid);
native SSCANF_IsConnected(playerid);

native SSCANF_Option(const ss2_name[], value);

stock const
	SSCANF_QUIET[] = "SSCANF_QUIET",
	OLD_DEFAULT_NAME[] = "OLD_DEFAULT_NAME",
	MATCH_NAME_PARTIAL[] = "MATCH_NAME_PARTIAL",
	CELLMIN_ON_MATCHES[] = "CELLMIN_ON_MATCHES",
	OLD_DEFAULT_KUSTOM[] = "OLD_DEFAULT_KUSTOM",
	OLD_DEFAULT_CUSTOM[] = "OLD_DEFAULT_CUSTOM";

static stock
	bool:SSCANF_gInit = false,
	SSCANF_g_sPlayers[MAX_PLAYERS char];

#if defined _inc_a_npc
	forward SSCANF_PlayerCheck();

	/*
	  OnNPCModeInit

	  Called when the script starts if it is a NPC mode, sets up the system,
	  then calls the "real" OnNPCModeInit (using the new ALS 2 hook method).
	*/

	public OnNPCModeInit()
	{
		SSCANF_Init(MAX_PLAYERS, INVALID_PLAYER_ID, MAX_PLAYER_NAME);
		#if !defined SSCANF_NO_PLAYERS
			// Initialise the system.
			SSCANF_PlayerCheck();
			SetTimer("SSCANF_PlayerCheck", 1, 1);
		#endif
		#if defined SSCANF_OnNPCModeInit
			SSCANF_OnNPCModeInit();
		#endif
		return 1;
	}

	#if defined _ALS_OnNPCModeInit
		#undef OnNPCModeInit
	#else
		#define _ALS_OnNPCModeInit
	#endif
	#define OnNPCModeInit SSCANF_OnNPCModeInit
	#if defined SSCANF_OnNPCModeInit
		forward SSCANF_OnNPCModeInit();
	#endif

	/*
	  SSCANF_PlayerCheck

	  NPC modes have no "OnPlayerConnect callback, so we need to simulate one.
	*/

	#if !defined SSCANF_NO_PLAYERS
		public SSCANF_PlayerCheck()
		{
			for (new i = 0; i != MAX_PLAYERS; ++i)
			{
				if (IsPlayerConnected(i))
				{
					if (!SSCANF_g_sPlayers{i})
					{
						new
							ss2_name[MAX_PLAYER_NAME];
						GetPlayerName(i, ss2_name, sizeof (ss2_name));
						// We have no way to know if they are an NPC or not!
						SSCANF_Join(i, ss2_name, 0);
						SSCANF_g_sPlayers{i} = 1;
					}
				}
				else
				{
					if (SSCANF_g_sPlayers{i})
					{
						SSCANF_Leave(i);
						SSCANF_g_sPlayers{i} = 0;
					}
				}
			}
		}
	#endif
#else
	/*
	  OnFilterScriptInit

	  Called when the script starts if it is a filterscript, sets up the system,
	  then calls the "real" OnFilterScriptInit (using the new ALS 2 hook
	  method).
	*/

	public OnFilterScriptInit()
	{
	    new
     		ss2_name[MAX_PLAYER_NAME];
	
		SSCANF_Init(GetMaxPlayers(), INVALID_PLAYER_ID, MAX_PLAYER_NAME);
		SSCANF_gInit = true;
		
		// Check if there are any players that aren't initialized.
		for (new i = 0; i < MAX_PLAYERS; i ++)
		{
		    if (IsPlayerConnected(i) && !SSCANF_IsConnected(i))
		    {
		        GetPlayerName(i, ss2_name, MAX_PLAYER_NAME);
		        SSCANF_Join(i, ss2_name, IsPlayerNPC(i));
		    }
		}
		
		#if defined SSCANF_OnFilterScriptInit
			SSCANF_OnFilterScriptInit();
		#endif
		return 1;
	}

	#if defined _ALS_OnFilterScriptInit
		#undef OnFilterScriptInit
	#else
		#define _ALS_OnFilterScriptInit
	#endif
	#define OnFilterScriptInit SSCANF_OnFilterScriptInit
	#if defined SSCANF_OnFilterScriptInit
		forward SSCANF_OnFilterScriptInit();
	#endif

	/*
	  OnGameModeInit

	  Called when the script starts if it is a gamemode.  This callback is also
	  called in filterscripts so we don't want to reinitialise the system in
	  that case.
	*/

	public OnGameModeInit()
	{
		if (!SSCANF_gInit)
		{
		    new
		        ss2_name[MAX_PLAYER_NAME];
		
			SSCANF_Init(GetMaxPlayers(), INVALID_PLAYER_ID, MAX_PLAYER_NAME);
			SSCANF_gInit = true;
			
			// Check if there are any players that aren't initialized.
			for (new i = 0; i < MAX_PLAYERS; i ++)
			{
			    if (IsPlayerConnected(i) && !SSCANF_IsConnected(i))
			    {
			        GetPlayerName(i, ss2_name, MAX_PLAYER_NAME);
			        SSCANF_Join(i, ss2_name, IsPlayerNPC(i));
			    }
			}
		}
		#if defined SSCANF_OnGameModeInit
			SSCANF_OnGameModeInit();
		#endif
		return 1;
	}

	#if defined _ALS_OnGameModeInit
		#undef OnGameModeInit
	#else
		#define _ALS_OnGameModeInit
	#endif
	#define OnGameModeInit SSCANF_OnGameModeInit
	#if defined SSCANF_OnGameModeInit
		forward SSCANF_OnGameModeInit();
	#endif

	/*
	  OnPlayerConnect

	  Called when a player connects.  Actually increments an internal count so
	  that if a script ends and "OnPlayerDisconnect" is called then "sscanf"
	  still knows that the player is really connected.  Also stores their ss2_name
	  internally.
	*/

	public OnPlayerConnect(playerid)
	{
		new
			ss2_name[MAX_PLAYER_NAME];
		GetPlayerName(playerid, ss2_name, sizeof (ss2_name));
		SSCANF_Join(playerid, ss2_name, IsPlayerNPC(playerid));
		#if defined SSCANF_OnPlayerConnect
			SSCANF_OnPlayerConnect(playerid);
		#endif
		return 1;
	}

	#if defined _ALS_OnPlayerConnect
		#undef OnPlayerConnect
	#else
		#define _ALS_OnPlayerConnect
	#endif
	#define OnPlayerConnect SSCANF_OnPlayerConnect
	#if defined SSCANF_OnPlayerConnect
		forward SSCANF_OnPlayerConnect(playerid);
	#endif

	/*
	  OnPlayerDisconnect

	  Called when a player disconnects, or when a script is ended.
	*/

	public OnPlayerDisconnect(playerid, reason)
	{
		#if defined SSCANF_OnPlayerDisconnect
			SSCANF_OnPlayerDisconnect(playerid, reason);
		#endif
		SSCANF_Leave(playerid);
		return 1;
	}

	#if defined _ALS_OnPlayerDisconnect
		#undef OnPlayerDisconnect
	#else
		#define _ALS_OnPlayerDisconnect
	#endif
	#define OnPlayerDisconnect SSCANF_OnPlayerDisconnect
	#if defined SSCANF_OnPlayerDisconnect
		forward SSCANF_OnPlayerDisconnect(playerid, reason);
	#endif
#endif

#define SSCANF_Init
#define SSCANF_Join
#define SSCANF_Leave

#define extract%0->%1; EXTRN%1;unformat(_:EXTRZ:EXTRV:EXTRX:%0,##,%1,,);
#define unformat(_:EXTRZ:EXTRV:EXTRX:%0,##,%1);%2else if (unformat(_:EXTRV:EXTRX:%0,##,%1))

#define EXTRV:EXTRX:%0<%3>##,%9new%1,%2) EXTRY:%0##P<%3>,|||%1|||%2)
#define EXTRX:%0##,%9new%1,%2) EXTRY:%0##,|||%1|||%2)
#define EXTRY: EXTR8:EXTR9:EXTR0:EXTR1:EXTR2:EXTR3:EXTR4:

#define EXTR8:EXTR9:EXTR0:EXTR1:EXTR2:EXTR3:EXTR4:%0##%1,%2|||%6:%3=%9|||%4) %6_EXTRO:%0##%1,%2|||%3=%9|||%4)
#define EXTR9:EXTR0:EXTR1:EXTR2:EXTR3:EXTR4:%0##%1,%2|||%3=%9|||%4) __EXTRO:%0##%1,%2|||%3=%9|||%4)
#define EXTR0:EXTR1:EXTR2:EXTR3:EXTR4:%0##%1,%2|||%6:%3[%7]|||%4) %6_EXTRW:%0##%1,%2|||%3[%7]|||%4)
#define EXTR1:EXTR2:EXTR3:EXTR4:%0##%1,%2|||%3[%7]|||%4) __EXTRW:%0##%1,%2|||%3|||%4)
#define EXTR2:EXTR3:EXTR4:%0##%1,%2|||%6:%3|||%4) %6_EXTRN:%0##%1,%2|||%3|||%4)
#define EXTR3:EXTR4:%0##%1,,%2||||||%4) %0##%1,%2)
#define EXTR4:%0##%1,%2|||%3|||%4) __EXTRN:%0##%1,%2|||%3|||%4)

// Optional specifiers.
#define __EXTRO:%0##%1,%2|||%3=%9|||%4,%5) EXTRY:%0##%1I"("#%9")"#,%2,%3|||%4|||%5)
#define Float_EXTRO:%0##%1,%2|||%3=%9|||%4,%5) EXTRY:%0##%1F"("#%9")"#,%2,%3|||%4|||%5)
#define player_EXTRO:%0##%1,%2|||%3=%9|||%4,%5) EXTRY:%0##%1U"("#%9")"#,%2,%3|||%4|||%5)
#define string_EXTRO:%0##%1,%2|||%3[%7]=%9|||%4,%5) EXTRY:%0##%1S"("#%9")"#[%7],%2,%3|||%4|||%5)

// Normal specifiers (the double underscore is to work for "_:".
#define __EXTRN:%0##%1,%2|||%3|||%4,%5) EXTRY:%0##%1i,%2,%3|||%4|||%5)
#define Float_EXTRN:%0##%1,%2|||%3|||%4,%5) EXTRY:%0##%1f,%2,%3|||%4|||%5)
#define player_EXTRN:%0##%1,%2|||%3|||%4,%5) EXTRY:%0##%1u,%2,%3|||%4|||%5)
//#define string_EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1s[%7],%2,%3|||%4|||%5)

// Array versions of normal specifiers.
#define __EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1a<i>[%7],%2,%3|||%4|||%5)
#define Float_EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1a<f>[%7],%2,%3|||%4|||%5)
#define player_EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1a<u>[%7],%2,%3|||%4|||%5)
#define string_EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1s[%7],%2,%3|||%4|||%5)

// Get rid of excess leading space which causes warnings.
#define EXTRN%0new%1; new%1;

#if !defined sscanf_string
    #define sscanf_string:
#endif

#define player:

#define hex:
#define hex_EXTRO:%0##%1,%2|||%3=%9|||%4,%5) EXTRY:%0##%1H"("#%9")"#,%2,%3|||%4|||%5)
#define hex_EXTRN:%0##%1,%2|||%3|||%4,%5) EXTRY:%0##%1h,%2,%3|||%4|||%5)
#define hex_EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1a<h>[%7],%2,%3|||%4|||%5)

#define bin:
#define bin_EXTRO:%0##%1,%2|||%3=%9|||%4,%5) EXTRY:%0##%1B"("#%9")"#,%2,%3|||%4|||%5)
#define bin_EXTRN:%0##%1,%2|||%3|||%4,%5) EXTRY:%0##%1b,%2,%3|||%4|||%5)
#define bin_EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1a<b>[%7],%2,%3|||%4|||%5)

#define kustom:%0<%1> %0
#define kustom_EXTRO:%0##%1,%2|||%3<%8>=%9|||%4,%5) EXTRY:%0##%1K<%8>"("#%9")"#,%2,%3|||%4|||%5)
#define kustom_EXTRN:%0##%1,%2|||%3<%8>|||%4,%5) EXTRY:%0##%1k<%8>,%2,%3|||%4|||%5)
//#define bin_EXTRW:%0##%1,%2|||%3[%7]|||%4,%5) EXTRY:%0##%1a<b>[%7],%2,%3|||%4|||%5)

SSCANF:weapon(sscanf_string[])
{
	// This function is VERY basic, needs VASTLY improving to detect variations.
	if ('0' <= sscanf_string[0] <= '9')
	{
		new
			ret = strval(sscanf_string);
		if (0 <= ret <= 18 || 22 <= ret <= 46)
		{
			return ret;
		}
	}
	else if (!strcmp(sscanf_string, "Unarmed")) return 0;
	else if (!strcmp(sscanf_string, "Brass Knuckles")) return 1;
	else if (!strcmp(sscanf_string, "Golf Club")) return 2;
	else if (!strcmp(sscanf_string, "Night Stick")) return 3;
	else if (!strcmp(sscanf_string, "Knife")) return 4;
	else if (!strcmp(sscanf_string, "Baseball Bat")) return 5;
	else if (!strcmp(sscanf_string, "Shovel")) return 6;
	else if (!strcmp(sscanf_string, "Pool cue")) return 7;
	else if (!strcmp(sscanf_string, "Katana")) return 8;
	else if (!strcmp(sscanf_string, "Chainsaw")) return 9;
	else if (!strcmp(sscanf_string, "Purple Dildo")) return 10;
	else if (!strcmp(sscanf_string, "White Dildo")) return 11;
	else if (!strcmp(sscanf_string, "Long White Dildo")) return 12;
	else if (!strcmp(sscanf_string, "White Dildo 2")) return 13;
	else if (!strcmp(sscanf_string, "Flowers")) return 14;
	else if (!strcmp(sscanf_string, "Cane")) return 15;
	else if (!strcmp(sscanf_string, "Grenades")) return 16;
	else if (!strcmp(sscanf_string, "Tear Gas")) return 17;
	else if (!strcmp(sscanf_string, "Molotovs")) return 18;
	else if (!strcmp(sscanf_string, "Pistol")) return 22;
	else if (!strcmp(sscanf_string, "Silenced Pistol")) return 23;
	else if (!strcmp(sscanf_string, "Desert Eagle")) return 24;
	else if (!strcmp(sscanf_string, "Shotgun")) return 25;
	else if (!strcmp(sscanf_string, "Sawn Off Shotgun")) return 26;
	else if (!strcmp(sscanf_string, "Combat Shotgun")) return 27;
	else if (!strcmp(sscanf_string, "Micro Uzi")) return 28;
	else if (!strcmp(sscanf_string, "Mac 10")) return 28;
	else if (!strcmp(sscanf_string, "MP5")) return 29;
	else if (!strcmp(sscanf_string, "AK47")) return 30;
	else if (!strcmp(sscanf_string, "M4")) return 31;
	else if (!strcmp(sscanf_string, "Tec9")) return 32;
	else if (!strcmp(sscanf_string, "Rifle")) return 33;
	else if (!strcmp(sscanf_string, "Sniper Rifle")) return 34;
	else if (!strcmp(sscanf_string, "RPG")) return 35;
	else if (!strcmp(sscanf_string, "Missile Launcher")) return 36;
	else if (!strcmp(sscanf_string, "Flame Thrower")) return 37;
	else if (!strcmp(sscanf_string, "Minigun")) return 38;
	else if (!strcmp(sscanf_string, "Sachel Charges")) return 39;
	else if (!strcmp(sscanf_string, "Detonator")) return 40;
	else if (!strcmp(sscanf_string, "Spray Paint")) return 41;
	else if (!strcmp(sscanf_string, "Fire Extinguisher")) return 42;
	else if (!strcmp(sscanf_string, "Camera")) return 43;
	else if (!strcmp(sscanf_string, "Nightvision Goggles")) return 44;
	else if (!strcmp(sscanf_string, "Thermal Goggles")) return 45;
	else if (!strcmp(sscanf_string, "Parachute")) return 46;
	return -1;
}

SSCANF:vehicle(sscanf_string[])
{
	// This function is VERY basic, needs VASTLY improving to detect variations.
	if ('0' <= sscanf_string[0] <= '9')
	{
		new
			ret = strval(sscanf_string);
		if (400 <= ret <= 611)
		{
			return ret;
		}
	}
	else if (!strcmp(sscanf_string, "Landstalker")) return 400;
	else if (!strcmp(sscanf_string, "Bravura")) return 401;
	else if (!strcmp(sscanf_string, "Buffalo")) return 402;
	else if (!strcmp(sscanf_string, "Linerunner")) return 403;
	else if (!strcmp(sscanf_string, "Perenniel")) return 404;
	else if (!strcmp(sscanf_string, "Sentinel")) return 405;
	else if (!strcmp(sscanf_string, "Dumper")) return 406;
	else if (!strcmp(sscanf_string, "Firetruck")) return 407;
	else if (!strcmp(sscanf_string, "Trashmaster")) return 408;
	else if (!strcmp(sscanf_string, "Stretch")) return 409;
	else if (!strcmp(sscanf_string, "Manana")) return 410;
	else if (!strcmp(sscanf_string, "Infernus")) return 411;
	else if (!strcmp(sscanf_string, "Voodoo")) return 412;
	else if (!strcmp(sscanf_string, "Pony")) return 413;
	else if (!strcmp(sscanf_string, "Mule")) return 414;
	else if (!strcmp(sscanf_string, "Cheetah")) return 415;
	else if (!strcmp(sscanf_string, "Ambulance")) return 416;
	else if (!strcmp(sscanf_string, "Leviathan")) return 417;
	else if (!strcmp(sscanf_string, "Moonbeam")) return 418;
	else if (!strcmp(sscanf_string, "Esperanto")) return 419;
	else if (!strcmp(sscanf_string, "Taxi")) return 420;
	else if (!strcmp(sscanf_string, "Washington")) return 421;
	else if (!strcmp(sscanf_string, "Bobcat")) return 422;
	else if (!strcmp(sscanf_string, "Mr Whoopee")) return 423;
	else if (!strcmp(sscanf_string, "BF Injection")) return 424;
	else if (!strcmp(sscanf_string, "Hunter")) return 425;
	else if (!strcmp(sscanf_string, "Premier")) return 426;
	else if (!strcmp(sscanf_string, "Enforcer")) return 427;
	else if (!strcmp(sscanf_string, "Securicar")) return 428;
	else if (!strcmp(sscanf_string, "Banshee")) return 429;
	else if (!strcmp(sscanf_string, "Predator")) return 430;
	else if (!strcmp(sscanf_string, "Bus")) return 431;
	else if (!strcmp(sscanf_string, "Rhino")) return 432;
	else if (!strcmp(sscanf_string, "Barracks")) return 433;
	else if (!strcmp(sscanf_string, "Hotknife")) return 434;
	else if (!strcmp(sscanf_string, "Article Trailer")) return 435;
	else if (!strcmp(sscanf_string, "Previon")) return 436;
	else if (!strcmp(sscanf_string, "Coach")) return 437;
	else if (!strcmp(sscanf_string, "Cabbie")) return 438;
	else if (!strcmp(sscanf_string, "Stallion")) return 439;
	else if (!strcmp(sscanf_string, "Rumpo")) return 440;
	else if (!strcmp(sscanf_string, "RC Bandit")) return 441;
	else if (!strcmp(sscanf_string, "Romero")) return 442;
	else if (!strcmp(sscanf_string, "Packer")) return 443;
	else if (!strcmp(sscanf_string, "Monster")) return 444;
	else if (!strcmp(sscanf_string, "Admiral")) return 445;
	else if (!strcmp(sscanf_string, "Squallo")) return 446;
	else if (!strcmp(sscanf_string, "Seasparrow")) return 447;
	else if (!strcmp(sscanf_string, "Pizzaboy")) return 448;
	else if (!strcmp(sscanf_string, "Tram")) return 449;
	else if (!strcmp(sscanf_string, "Article Trailer 2")) return 450;
	else if (!strcmp(sscanf_string, "Turismo")) return 451;
	else if (!strcmp(sscanf_string, "Speeder")) return 452;
	else if (!strcmp(sscanf_string, "Reefer")) return 453;
	else if (!strcmp(sscanf_string, "Tropic")) return 454;
	else if (!strcmp(sscanf_string, "Flatbed")) return 455;
	else if (!strcmp(sscanf_string, "Yankee")) return 456;
	else if (!strcmp(sscanf_string, "Caddy")) return 457;
	else if (!strcmp(sscanf_string, "Solair")) return 458;
	else if (!strcmp(sscanf_string, "Berkley's RC Van")) return 459;
	else if (!strcmp(sscanf_string, "Skimmer")) return 460;
	else if (!strcmp(sscanf_string, "PCJ-600")) return 461;
	else if (!strcmp(sscanf_string, "Faggio")) return 462;
	else if (!strcmp(sscanf_string, "Freeway")) return 463;
	else if (!strcmp(sscanf_string, "RC Baron")) return 464;
	else if (!strcmp(sscanf_string, "RC Raider")) return 465;
	else if (!strcmp(sscanf_string, "Glendale")) return 466;
	else if (!strcmp(sscanf_string, "Oceanic")) return 467;
	else if (!strcmp(sscanf_string, "Sanchez")) return 468;
	else if (!strcmp(sscanf_string, "Sparrow")) return 469;
	else if (!strcmp(sscanf_string, "Patriot")) return 470;
	else if (!strcmp(sscanf_string, "Quad")) return 471;
	else if (!strcmp(sscanf_string, "Coastguard")) return 472;
	else if (!strcmp(sscanf_string, "Dinghy")) return 473;
	else if (!strcmp(sscanf_string, "Hermes")) return 474;
	else if (!strcmp(sscanf_string, "Sabre")) return 475;
	else if (!strcmp(sscanf_string, "Rustler")) return 476;
	else if (!strcmp(sscanf_string, "ZR-350")) return 477;
	else if (!strcmp(sscanf_string, "Walton")) return 478;
	else if (!strcmp(sscanf_string, "Regina")) return 479;
	else if (!strcmp(sscanf_string, "Comet")) return 480;
	else if (!strcmp(sscanf_string, "BMX")) return 481;
	else if (!strcmp(sscanf_string, "Burrito")) return 482;
	else if (!strcmp(sscanf_string, "Camper")) return 483;
	else if (!strcmp(sscanf_string, "Marquis")) return 484;
	else if (!strcmp(sscanf_string, "Baggage")) return 485;
	else if (!strcmp(sscanf_string, "Dozer")) return 486;
	else if (!strcmp(sscanf_string, "Maverick")) return 487;
	else if (!strcmp(sscanf_string, "SAN News Maverick")) return 488;
	else if (!strcmp(sscanf_string, "Rancher")) return 489;
	else if (!strcmp(sscanf_string, "FBI Rancher")) return 490;
	else if (!strcmp(sscanf_string, "Virgo")) return 491;
	else if (!strcmp(sscanf_string, "Greenwood")) return 492;
	else if (!strcmp(sscanf_string, "Jetmax")) return 493;
	else if (!strcmp(sscanf_string, "Hotring Racer")) return 494;
	else if (!strcmp(sscanf_string, "Sandking")) return 495;
	else if (!strcmp(sscanf_string, "Blista Compact")) return 496;
	else if (!strcmp(sscanf_string, "Police Maverick")) return 497;
	else if (!strcmp(sscanf_string, "Boxville")) return 498;
	else if (!strcmp(sscanf_string, "Benson")) return 499;
	else if (!strcmp(sscanf_string, "Mesa")) return 500;
	else if (!strcmp(sscanf_string, "RC Goblin")) return 501;
	else if (!strcmp(sscanf_string, "Hotring Racer")) return 502;
	else if (!strcmp(sscanf_string, "Hotring Racer")) return 503;
	else if (!strcmp(sscanf_string, "Bloodring Banger")) return 504;
	else if (!strcmp(sscanf_string, "Rancher")) return 505;
	else if (!strcmp(sscanf_string, "Super GT")) return 506;
	else if (!strcmp(sscanf_string, "Elegant")) return 507;
	else if (!strcmp(sscanf_string, "Journey")) return 508;
	else if (!strcmp(sscanf_string, "Bike")) return 509;
	else if (!strcmp(sscanf_string, "Mountain Bike")) return 510;
	else if (!strcmp(sscanf_string, "Beagle")) return 511;
	else if (!strcmp(sscanf_string, "Cropduster")) return 512;
	else if (!strcmp(sscanf_string, "Stuntplane")) return 513;
	else if (!strcmp(sscanf_string, "Tanker")) return 514;
	else if (!strcmp(sscanf_string, "Roadtrain")) return 515;
	else if (!strcmp(sscanf_string, "Nebula")) return 516;
	else if (!strcmp(sscanf_string, "Majestic")) return 517;
	else if (!strcmp(sscanf_string, "Buccaneer")) return 518;
	else if (!strcmp(sscanf_string, "Shamal")) return 519;
	else if (!strcmp(sscanf_string, "Hydra")) return 520;
	else if (!strcmp(sscanf_string, "FCR-900")) return 521;
	else if (!strcmp(sscanf_string, "NRG-500")) return 522;
	else if (!strcmp(sscanf_string, "HPV1000")) return 523;
	else if (!strcmp(sscanf_string, "Cement Truck")) return 524;
	else if (!strcmp(sscanf_string, "Towtruck")) return 525;
	else if (!strcmp(sscanf_string, "Fortune")) return 526;
	else if (!strcmp(sscanf_string, "Cadrona")) return 527;
	else if (!strcmp(sscanf_string, "FBI Truck")) return 528;
	else if (!strcmp(sscanf_string, "Willard")) return 529;
	else if (!strcmp(sscanf_string, "Forklift")) return 530;
	else if (!strcmp(sscanf_string, "Tractor")) return 531;
	else if (!strcmp(sscanf_string, "Combine Harvester")) return 532;
	else if (!strcmp(sscanf_string, "Feltzer")) return 533;
	else if (!strcmp(sscanf_string, "Remington")) return 534;
	else if (!strcmp(sscanf_string, "Slamvan")) return 535;
	else if (!strcmp(sscanf_string, "Blade")) return 536;
	else if (!strcmp(sscanf_string, "Freight (Train)")) return 537;
	else if (!strcmp(sscanf_string, "Brownstreak (Train)")) return 538;
	else if (!strcmp(sscanf_string, "Vortex")) return 539;
	else if (!strcmp(sscanf_string, "Vincent")) return 540;
	else if (!strcmp(sscanf_string, "Bullet")) return 541;
	else if (!strcmp(sscanf_string, "Clover")) return 542;
	else if (!strcmp(sscanf_string, "Sadler")) return 543;
	else if (!strcmp(sscanf_string, "Firetruck LA")) return 544;
	else if (!strcmp(sscanf_string, "Hustler")) return 545;
	else if (!strcmp(sscanf_string, "Intruder")) return 546;
	else if (!strcmp(sscanf_string, "Primo")) return 547;
	else if (!strcmp(sscanf_string, "Cargobob")) return 548;
	else if (!strcmp(sscanf_string, "Tampa")) return 549;
	else if (!strcmp(sscanf_string, "Sunrise")) return 550;
	else if (!strcmp(sscanf_string, "Merit")) return 551;
	else if (!strcmp(sscanf_string, "Utility Van")) return 552;
	else if (!strcmp(sscanf_string, "Nevada")) return 553;
	else if (!strcmp(sscanf_string, "Yosemite")) return 554;
	else if (!strcmp(sscanf_string, "Windsor")) return 555;
	else if (!strcmp(sscanf_string, "Monster \"A\"")) return 556;
	else if (!strcmp(sscanf_string, "Monster \"B\"")) return 557;
	else if (!strcmp(sscanf_string, "Uranus")) return 558;
	else if (!strcmp(sscanf_string, "Jester")) return 559;
	else if (!strcmp(sscanf_string, "Sultan")) return 560;
	else if (!strcmp(sscanf_string, "Stratum")) return 561;
	else if (!strcmp(sscanf_string, "Elegy")) return 562;
	else if (!strcmp(sscanf_string, "Raindance")) return 563;
	else if (!strcmp(sscanf_string, "RC Tiger")) return 564;
	else if (!strcmp(sscanf_string, "Flash")) return 565;
	else if (!strcmp(sscanf_string, "Tahoma")) return 566;
	else if (!strcmp(sscanf_string, "Savanna")) return 567;
	else if (!strcmp(sscanf_string, "Bandito")) return 568;
	else if (!strcmp(sscanf_string, "Freight Flat Trailer (Train)")) return 569;
	else if (!strcmp(sscanf_string, "Streak Trailer (Train)")) return 570;
	else if (!strcmp(sscanf_string, "Kart")) return 571;
	else if (!strcmp(sscanf_string, "Mower")) return 572;
	else if (!strcmp(sscanf_string, "Dune")) return 573;
	else if (!strcmp(sscanf_string, "Sweeper")) return 574;
	else if (!strcmp(sscanf_string, "Broadway")) return 575;
	else if (!strcmp(sscanf_string, "Tornado")) return 576;
	else if (!strcmp(sscanf_string, "AT400")) return 577;
	else if (!strcmp(sscanf_string, "DFT-30")) return 578;
	else if (!strcmp(sscanf_string, "Huntley")) return 579;
	else if (!strcmp(sscanf_string, "Stafford")) return 580;
	else if (!strcmp(sscanf_string, "BF-400")) return 581;
	else if (!strcmp(sscanf_string, "Newsvan")) return 582;
	else if (!strcmp(sscanf_string, "Tug")) return 583;
	else if (!strcmp(sscanf_string, "Petrol Trailer")) return 584;
	else if (!strcmp(sscanf_string, "Emperor")) return 585;
	else if (!strcmp(sscanf_string, "Wayfarer")) return 586;
	else if (!strcmp(sscanf_string, "Euros")) return 587;
	else if (!strcmp(sscanf_string, "Hotdog")) return 588;
	else if (!strcmp(sscanf_string, "Club")) return 589;
	else if (!strcmp(sscanf_string, "Freight Box Trailer (Train)")) return 590;
	else if (!strcmp(sscanf_string, "Article Trailer 3")) return 591;
	else if (!strcmp(sscanf_string, "Andromada")) return 592;
	else if (!strcmp(sscanf_string, "Dodo")) return 593;
	else if (!strcmp(sscanf_string, "RC Cam")) return 594;
	else if (!strcmp(sscanf_string, "Launch")) return 595;
	else if (!strcmp(sscanf_string, "Police Car (LSPD)")) return 596;
	else if (!strcmp(sscanf_string, "Police Car (SFPD)")) return 597;
	else if (!strcmp(sscanf_string, "Police Car (LVPD)")) return 598;
	else if (!strcmp(sscanf_string, "Police Ranger")) return 599;
	else if (!strcmp(sscanf_string, "Picador")) return 600;
	else if (!strcmp(sscanf_string, "S.W.A.T.")) return 601;
	else if (!strcmp(sscanf_string, "Alpha")) return 602;
	else if (!strcmp(sscanf_string, "Phoenix")) return 603;
	else if (!strcmp(sscanf_string, "Glendale Shit")) return 604;
	else if (!strcmp(sscanf_string, "Sadler Shit")) return 605;
	else if (!strcmp(sscanf_string, "Baggage Trailer \"A\"")) return 606;
	else if (!strcmp(sscanf_string, "Baggage Trailer \"B\"")) return 607;
	else if (!strcmp(sscanf_string, "Tug Stairs Trailer")) return 608;
	else if (!strcmp(sscanf_string, "Boxville")) return 609;
	else if (!strcmp(sscanf_string, "Farm Trailer")) return 610;
	else if (!strcmp(sscanf_string, "Utility Trailer")) return 611;
	return -1;
}

// Fix the compiler crash when both the PAWN and Plugin versions of sscanf are
// found by renaming the old version at declaration.  (fixes.inc compatible
// naming scheme: "BAD_Function()").
#define sscanf(%0:...) BAD_sscanf(%0:...)
