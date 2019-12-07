/***********************************************************************************************************************

	Script: Nickk's Vehicle Streamer
		Version: Beta 1.0
		
	Author: Nickk888
	Facebook: https://www.facebook.com/Nickk888FP/
		
Dependencies:
-	SSCANF2 Plugin: https://github.com/maddinat0r/sscanf/releases
-	Streamer Plugin: https://github.com/samp-incognito/samp-streamer-plugin/releases

***********************************************************************************************************************/

//Includes
#include <a_samp>
#include <streamer>
#include <zcmd>
#include <YSI\y_iterate>
#include <dfile>

//Defines
#define CHUNK_SIZE 300 //300 = Minimum | 600 = Maximum
#define MAX_STREAMER_CHUNKS 17956
#define TICK_RATE 1000
#define MAX_STREAMED_VEHICLES 100000
#define MAX_VEHICLES_PER_CHUNK (MAX_VEHICLES)
#define CHUNK_CHECKSUM 924573 //Don't change please!
#define PROPERTY_CODE 8421348478 //Don't change please!
#define MAP_SIZE 40000 //The map size, don't change!
#define CONFIG_FILE "nvs_config.ini"
#define SCRIPT_VERSION "1.0 Beta"

//Pragmas
#pragma dynamic 4194304

//Iterators
new Iterator:VS_I_Vehicles_A<MAX_STREAMED_VEHICLES>;
new Iterator:VS_I_Vehicles_L<MAX_VEHICLES>;
new Iterator:VS_I_Chunks<MAX_STREAMER_CHUNKS>;
new Iterator:VS_I_ChunkVehicles[MAX_STREAMER_CHUNKS]<MAX_VEHICLES_PER_CHUNK>;

//Global Variables
new VS_StreamerChunk[MAX_STREAMER_CHUNKS];
new VS_ChunkVehicleID[MAX_STREAMER_CHUNKS][MAX_VEHICLES_PER_CHUNK];
new VS_VehicleInternalID[MAX_VEHICLES];

//Debug Variables
new VS_ActiveChunks;
new VS_ChunkDebugZone[MAX_STREAMER_CHUNKS];

//Enumerators
enum VS_E_VEHICLEDATA
{
	bool:vsv_isvalid,
	vsv_vehicleid,
	vsv_modelid,
	vsv_chunkid,
	Float:vsv_spawn_x,
	Float:vsv_spawn_y,
	Float:vsv_spawn_z,
	Float:vsv_spawn_a,
	Float:vsv_x,
	Float:vsv_y,
	Float:vsv_z,
	Float:vsv_a,
	Float:vsv_health,
	vsv_component[14],
	vsv_color1,
	vsv_color2,
	vsv_respawndelay,
	vsv_addsiren,
	vsv_paintjob,
	vsv_interior,
	vsv_virtualworld,
	vsv_engine,
	vsv_lights,
	vsv_alarm,
	vsv_doors,
	vsv_bonnet,
	vsv_boot,
	vsv_objective
};
new VS_V[MAX_STREAMED_VEHICLES][VS_E_VEHICLEDATA];

enum VS_E_CHUNKDATA
{
	bool:vsc_active,
	Float:vsc_minx,
	Float:vsc_miny,
	Float:vsc_maxx,
	Float:vsc_maxy,
	Float:vsc_centerx,
	Float:vsc_centery,
	vsc_north,
	vsc_ost,
	vsc_south,
	vsc_west,
};
new VS_C[MAX_STREAMER_CHUNKS][VS_E_CHUNKDATA];

enum VS_E_SETTINGSDATA
{
	bool:vss_debug,
	Float:vss_chunksize,
	vss_tickrate
};
new VS_S[VS_E_SETTINGSDATA];

//Natives
native IsValidVehicle(vehicleid);

//Callbacks
public OnFilterScriptInit()
{
	print("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
	printf("\n[NVS] Vehicle Streamer by Nickk888 v.%s - LOADED!", SCRIPT_VERSION);
	//Initalization
	Iter_Init(VS_I_ChunkVehicles);
	VS_LoadConfig();
	if(VS_CreateChunks())
	{
		foreach(new i : Player)
			Streamer_Update(i);
	}
	
	//
	print("\n* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
	
	//Timers
	SetTimer("VS_Tick", VS_S[vss_tickrate], true);
	return 1;
}

public OnFilterScriptExit()
{
	//
	print("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
	print("\n[VSTREAMER EXIT] Vehicle Streamer by Nickk888 - UNLOADED!\n");
	print("* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *");
	//
	if(VS_S[vss_debug])
	{
		foreach(new i : VS_I_Chunks)
			GangZoneDestroy(VS_ChunkDebugZone[i]);
	}
	foreach(new i : VS_I_Vehicles_L)
	{
		DestroyVehicle(i);
	}
		
	return 1;
}

public OnVehicleSpawn(vehicleid)
{
	if(IsValidVehicle(vehicleid))
	{
		new int_vehicleid = VS_IsVehicleStreamerVehicle(vehicleid);
		if(int_vehicleid != -1)
		{
			VS_SetDynamicVehicleToRespawn(int_vehicleid);
		}
	}
	return 1;
}

//Timers
forward VS_Tick();
public VS_Tick()
{
	new tick = GetTickCount();
	new intid, working, Float:tmp_pos[4];
	//Vehicle Save
	foreach(new i : VS_I_Vehicles_L)
	{
		intid = VS_VehicleInternalID[i];
		GetVehiclePos(i, tmp_pos[0], tmp_pos[1], tmp_pos[2]);
		GetVehicleZAngle(i, tmp_pos[3]);
		GetVehicleHealth(i, VS_V[intid][vsv_health]);
		if(VS_V[intid][vsv_x] != tmp_pos[0] || VS_V[intid][vsv_y] != tmp_pos[1] || VS_V[intid][vsv_z] != tmp_pos[2] || VS_V[intid][vsv_a] != tmp_pos[3])
		{
			VS_V[intid][vsv_x] = tmp_pos[0];
			VS_V[intid][vsv_y] = tmp_pos[1];
			VS_V[intid][vsv_z] = tmp_pos[2];
			VS_V[intid][vsv_a] = tmp_pos[3];
			VS_UpdateVehicleChunkData(intid, true);
		}
		
	}
	//Chunk check
	foreach(new i : VS_I_Chunks)
	{
		//Try Load
		if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[i]))
		{
			working+=VS_LoadChunk(i); //Current/Main Chunk
			working+=VS_LoadChunk(VS_C[i][vsc_north]); //North Chunk
			working+=VS_LoadChunk(VS_C[i][vsc_ost]); //Ost Chunk
			working+=VS_LoadChunk(VS_C[i][vsc_south]); //South Chunk
			working+=VS_LoadChunk(VS_C[i][vsc_west]); //West Chunk
			if(VS_C[i][vsc_north] != -1) 
			{
				working+=VS_LoadChunk(VS_C[VS_C[i][vsc_north]][vsc_ost]); //North Ost Chunk
				working+=VS_LoadChunk(VS_C[VS_C[i][vsc_north]][vsc_west]); //North West Chunk
			}
			if(VS_C[i][vsc_south] != -1)
			{
				working+=VS_LoadChunk(VS_C[VS_C[i][vsc_south]][vsc_ost]); //South Ost Chunk
				working+=VS_LoadChunk(VS_C[VS_C[i][vsc_south]][vsc_west]); //South West Chunk
			}
		}
		//Try Unload
		else if(VS_C[i][vsc_active] == true)
		{
			if(VS_C[i][vsc_north] != -1) //North Chunk
				if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[i][vsc_north]]))
					continue;
			if(VS_C[i][vsc_ost] != -1) //Ost Chunk
				if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[i][vsc_ost]]))
					continue;
			if(VS_C[i][vsc_south] != -1) //South Chunk
				if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[i][vsc_south]]))
					continue;
			if(VS_C[i][vsc_west] != -1) //West Chunk
				if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[i][vsc_west]]))
					continue;
			if(VS_C[i][vsc_north] != -1)
			{
				if(VS_C[VS_C[i][vsc_north]][vsc_ost] != -1) //North Ost Chunk
					if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[VS_C[i][vsc_north]][vsc_ost]]))
						continue;
				if(VS_C[VS_C[i][vsc_north]][vsc_west] != -1) //North west Chunk
					if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[VS_C[i][vsc_north]][vsc_west]]))
						continue;
			}
			if(VS_C[i][vsc_south] != -1)
			{
				if(VS_C[VS_C[i][vsc_south]][vsc_ost] != -1) //South Ost Chunk
					if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[VS_C[i][vsc_south]][vsc_ost]]))
						continue;
				if(VS_C[VS_C[i][vsc_south]][vsc_west] != -1) //South West Chunk
					if(IsAnyPlayerInDynamicArea(VS_StreamerChunk[VS_C[VS_C[i][vsc_south]][vsc_west]]))
						continue;
			}
			//Unload Chunk
			working+=VS_UnloadChunk(i);
		}
	}
	//Debug
	if(VS_S[vss_debug])
	{
		new string[128];
		format(string, sizeof string, "~w~NVS~n~~y~Created: ~w~%i~n~\
		~y~Spawned: ~w~%i~n~\
		~y~Lag: ~w~%i ms~n~\
		%s", 
		Iter_Count(VS_I_Vehicles_A),
		Iter_Count(VS_I_Vehicles_L),
		GetTickCount() - tick,
		(working) ? ("~r~Working...") : (""));
		GameTextForAll(string, TICK_RATE + 125, 3);
	}
	return 1;
}


//Set Functions
forward VS_CreateDynamicVehicle(modelid, Float:x, Float:y, Float:z, Float:rot, color1, color2, respawndelay, addsiren);
public VS_CreateDynamicVehicle(modelid, Float:x, Float:y, Float:z, Float:rot, color1, color2, respawndelay, addsiren)
{
	new int_vehicleid = Iter_Free(VS_I_Vehicles_A);
	if(int_vehicleid >= 0 && int_vehicleid < MAX_STREAMED_VEHICLES)
	{
		Iter_Add(VS_I_Vehicles_A, int_vehicleid);
		//Vehicle Data
		VS_V[int_vehicleid][vsv_isvalid] = true;
		VS_V[int_vehicleid][vsv_vehicleid] = INVALID_VEHICLE_ID;
		VS_V[int_vehicleid][vsv_modelid] = modelid;
		VS_V[int_vehicleid][vsv_spawn_x] = x;
		VS_V[int_vehicleid][vsv_spawn_y] = y;
		VS_V[int_vehicleid][vsv_spawn_z] = z;
		VS_V[int_vehicleid][vsv_spawn_a] = rot;
		VS_V[int_vehicleid][vsv_x] = VS_V[int_vehicleid][vsv_spawn_x];
		VS_V[int_vehicleid][vsv_y] = VS_V[int_vehicleid][vsv_spawn_y];
		VS_V[int_vehicleid][vsv_z] = VS_V[int_vehicleid][vsv_spawn_z];
		VS_V[int_vehicleid][vsv_a] = VS_V[int_vehicleid][vsv_spawn_a];
		VS_V[int_vehicleid][vsv_color1] = color1;
		VS_V[int_vehicleid][vsv_color2] = color2;
		VS_V[int_vehicleid][vsv_respawndelay] = respawndelay;
		VS_V[int_vehicleid][vsv_addsiren] = addsiren;
		VS_V[int_vehicleid][vsv_health] = 1000;
		VS_V[int_vehicleid][vsv_interior] = 0;
		VS_V[int_vehicleid][vsv_virtualworld] = 0;
		VS_V[int_vehicleid][vsv_chunkid] = -1;
		for(new i; i < 14; i++)
			VS_V[int_vehicleid][vsv_component][i] = 0;
		//Chunk Register
		VS_UpdateVehicleChunkData(int_vehicleid);
		//Check & Spawn
		if(VS_V[int_vehicleid][vsv_chunkid] == -1)
		{
			Iter_Remove(VS_I_Vehicles_A, int_vehicleid);
			return -1;
		}
	}
	else 
	{
		int_vehicleid = -1;
		printf("[NVS - ERROR] Can't create dynamic vehicle! Limit (%i) exceeded!", MAX_STREAMED_VEHICLES);
	}
	return int_vehicleid;
}

forward VS_DestroyDynamicVehicle(int_vehicleid);
public VS_DestroyDynamicVehicle(int_vehicleid)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			VS_DeSpawnVehicle(int_vehicleid);
		Iter_Remove(VS_I_Vehicles_A, int_vehicleid);
		VS_RemoveVehicleFromChunk(int_vehicleid, VS_V[int_vehicleid][vsv_chunkid]);
		return 1;
	}
	return 0;
}

forward VS_SetDynamicVehiclePos(int_vehicleid, Float:x, Float:y, Float:z);
public VS_SetDynamicVehiclePos(int_vehicleid, Float:x, Float:y, Float:z)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_V[int_vehicleid][vsv_x] = x;
		VS_V[int_vehicleid][vsv_y] = y;
		VS_V[int_vehicleid][vsv_z] = z;
		VS_UpdateVehicleChunkData(int_vehicleid);
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			SetVehiclePos(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_x], VS_V[int_vehicleid][vsv_y], VS_V[int_vehicleid][vsv_z]);
		return 1;
	}
	return 0;
}

forward VS_SetDynamicVehicleZAngle(int_vehicleid, Float:angle);
public VS_SetDynamicVehicleZAngle(int_vehicleid, Float:angle)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_V[int_vehicleid][vsv_a] = angle;
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			SetVehicleZAngle(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_a]);
		return 1;
	}
	return 0;
}

forward VS_SetDynamicVehicleHealth(int_vehicleid, Float:health);
public VS_SetDynamicVehicleHealth(int_vehicleid, Float:health)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_V[int_vehicleid][vsv_health] = health;
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			SetVehicleHealth(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_health]);
		return 1;
	}
	return 0;
}

forward VS_ChangeDynamicVehicleColor(int_vehicleid, color1, color2);
public VS_ChangeDynamicVehicleColor(int_vehicleid, color1, color2)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_V[int_vehicleid][vsv_color1] = color1;
		VS_V[int_vehicleid][vsv_color2] = color2;
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			ChangeVehicleColor(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_color1], VS_V[int_vehicleid][vsv_color2]);
		return 1;
	}
	return 0;
}

forward VS_ChangeDynamicVehiclePaintjob(int_vehicleid, paintjob);
public VS_ChangeDynamicVehiclePaintjob(int_vehicleid, paintjob)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_V[int_vehicleid][vsv_paintjob] = paintjob;
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			ChangeVehiclePaintjob(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_paintjob]);
		return 1;
	}
	return 0;
}

forward VS_LinkDynamicVehicleToInterior(int_vehicleid, interiorid);
public VS_LinkDynamicVehicleToInterior(int_vehicleid, interiorid)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_V[int_vehicleid][vsv_interior] = interiorid;
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			LinkVehicleToInterior(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_interior]);
		return 1;
	}
	return 0;
}

forward VS_SetDynamicVehicleVW(int_vehicleid, virtualworld);
public VS_SetDynamicVehicleVW(int_vehicleid, virtualworld)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_V[int_vehicleid][vsv_virtualworld] = virtualworld;
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			SetVehicleVirtualWorld(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_virtualworld]);
		return 1;
	}
	return 0;
}

forward VS_PutPlayerInDynamicVehicle(playerid, int_vehicleid, seatid);
public VS_PutPlayerInDynamicVehicle(playerid, int_vehicleid, seatid)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		if(!VS_IsInternalVehicleSpawned(int_vehicleid))
			VS_SpawnVehicle(int_vehicleid);
		PutPlayerInVehicle(playerid, VS_V[int_vehicleid][vsv_vehicleid], seatid);
		return 1;
	}
	return 0;
}

forward VS_SetDynamicVehicleToRespawn(int_vehicleid);
public VS_SetDynamicVehicleToRespawn(int_vehicleid)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		VS_SetDynamicVehiclePos(int_vehicleid, VS_V[int_vehicleid][vsv_spawn_x], VS_V[int_vehicleid][vsv_spawn_y], VS_V[int_vehicleid][vsv_spawn_z]);
		VS_SetDynamicVehicleZAngle(int_vehicleid, VS_V[int_vehicleid][vsv_spawn_a]);
		return 1;
	}
	return 0;
}

forward VS_AddDynamicVehicleComponent(int_vehicleid, componentid);
public VS_AddDynamicVehicleComponent(int_vehicleid, componentid)
{
	if(VS_IsInternalVehicleIDValid(int_vehicleid))
	{
		new type = GetVehicleComponentType(componentid);
		if(type == -1)
			return 0;
		VS_V[int_vehicleid][vsv_component][type] = componentid;
		if(VS_IsInternalVehicleSpawned(int_vehicleid))
			AddVehicleComponent(VS_V[int_vehicleid][vsv_vehicleid], VS_V[int_vehicleid][vsv_component][type]);
		return 1;
	}
	return 0;
}

forward VS_SetDynamicVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective);
public VS_SetDynamicVehicleParamsEx(vehicleid, engine, lights, alarm, doors, bonnet, boot, objective)
{
	if(VS_IsInternalVehicleIDValid(vehicleid))
	{
		VS_V[vehicleid][vsv_engine] = engine;
		VS_V[vehicleid][vsv_lights] = lights;
		VS_V[vehicleid][vsv_alarm] = alarm;
		VS_V[vehicleid][vsv_doors] = doors;
		VS_V[vehicleid][vsv_bonnet] = bonnet;
		VS_V[vehicleid][vsv_boot] = boot;
		VS_V[vehicleid][vsv_objective] = objective;
		if(VS_IsInternalVehicleSpawned(vehicleid))
			SetVehicleParamsEx(VS_V[vehicleid][vsv_vehicleid], 
			VS_V[vehicleid][vsv_engine],
			VS_V[vehicleid][vsv_lights],
			VS_V[vehicleid][vsv_alarm],
			VS_V[vehicleid][vsv_doors],
			VS_V[vehicleid][vsv_bonnet],
			VS_V[vehicleid][vsv_boot],
			VS_V[vehicleid][vsv_objective]);
	}
	return 1;
}

//Get Functions
forward VS_CountDynamicVehicles();
public VS_CountDynamicVehicles()
	return Iter_Count(VS_I_Vehicles_A);
	
forward VS_CountSpawnedDynamicVehicles();
public VS_CountSpawnedDynamicVehicles()
	return Iter_Count(VS_I_Vehicles_L);
	
forward VS_GetDynamicVehicleInternalID(vehicleid);
public VS_GetDynamicVehicleInternalID(vehicleid)
{
	if(IsValidVehicle(vehicleid))
		return VS_VehicleInternalID[vehicleid];
	return 0;
}
	
forward VS_GetDynamicVehicleModel(vehicleid);
public VS_GetDynamicVehicleModel(vehicleid)
{
	if(VS_IsInternalVehicleIDValid(vehicleid))
		return VS_V[vehicleid][vsv_modelid];
	return 0;
}
	
	
forward VS_GetDynamicVehicleVW(vehicleid);
public VS_GetDynamicVehicleVW(vehicleid)
{
	if(VS_IsInternalVehicleIDValid(vehicleid))
		return VS_V[vehicleid][vsv_virtualworld];
	return 0;
}
	
forward VS_GetDynamicVehicleInterior(vehicleid);
public VS_GetDynamicVehicleInterior(vehicleid)
{
	if(VS_IsInternalVehicleIDValid(vehicleid))
		return VS_V[vehicleid][vsv_interior];
	return 0;
}
	
forward VS_GetDynVehicleComponentInSlot(vehicleid, slot);
public VS_GetDynVehicleComponentInSlot(vehicleid, slot)
{
	if(VS_IsInternalVehicleIDValid(vehicleid))
		if(slot >= 0 && slot <= 13)
			return VS_V[vehicleid][vsv_component][slot];
	return 0;
}
	
forward VS_GetDynamicVehiclePos(vehicleid);
public VS_GetDynamicVehiclePos(vehicleid)
{
	new string[128];
	if(VS_IsInternalVehicleIDValid(vehicleid))
	{
		if(VS_IsInternalVehicleSpawned(vehicleid))
		{
			GetVehiclePos(VS_V[vehicleid][vsv_vehicleid], VS_V[vehicleid][vsv_x], VS_V[vehicleid][vsv_y], VS_V[vehicleid][vsv_z]);
			GetVehicleZAngle(VS_V[vehicleid][vsv_vehicleid], VS_V[vehicleid][vsv_a]);
		}
		format(string, sizeof string, "%f %f %f %f", VS_V[vehicleid][vsv_x], VS_V[vehicleid][vsv_y], VS_V[vehicleid][vsv_z], VS_V[vehicleid][vsv_a]);
		setproperty(0, "", PROPERTY_CODE, string);
		return PROPERTY_CODE;
	}
	return 0;
}

forward VS_GetDynamicVehicleColor(vehicleid);
public VS_GetDynamicVehicleColor(vehicleid)
{
	new string[128];
	if(VS_IsInternalVehicleIDValid(vehicleid))
	{
		format(string, sizeof string, "%i %i", VS_V[vehicleid][vsv_color1], VS_V[vehicleid][vsv_color1]);
		setproperty(0, "", PROPERTY_CODE, string);
		return PROPERTY_CODE;
	}
	return 0;
}
	

	
//Script Functions
stock VS_UpdateVehicleChunkData(int_vehicleid, bool:onlyupdate = false)
{
	new areas[100], chunkid;
	GetDynamicAreasForPoint(VS_V[int_vehicleid][vsv_x], VS_V[int_vehicleid][vsv_y], VS_V[int_vehicleid][vsv_z], areas);
	for(new i; i < sizeof areas; i++)
	{
		chunkid = VS_GetChunkID(areas[i]);
		if(chunkid != -1)
		{
			
			if(VS_V[int_vehicleid][vsv_chunkid] == chunkid)
				break;
			VS_RemoveVehicleFromChunk(int_vehicleid, VS_V[int_vehicleid][vsv_chunkid]);
			if(VS_AddVehicleToChunk(int_vehicleid, chunkid) != -1)
			{
				if(!onlyupdate)
				{
					if(!VS_IsInternalVehicleSpawned(int_vehicleid))
					{
						if(VS_C[chunkid][vsc_active])
							VS_SpawnVehicle(int_vehicleid);
					}
					else
					{
						if(!VS_C[chunkid][vsc_active])
							VS_DeSpawnVehicle(int_vehicleid);
					}
				}
			}
			break;
		}
	}
	return 1;
}

stock VS_IsInternalVehicleSpawned(int_vehicleid)
{
	if(VS_V[int_vehicleid][vsv_vehicleid] != INVALID_VEHICLE_ID)
		return 1;
	return 0;
}

stock VS_IsInternalVehicleIDValid(int_vehicleid)
{
	if(int_vehicleid < 0 || int_vehicleid > MAX_STREAMED_VEHICLES)
		return 0;
	if(VS_V[int_vehicleid][vsv_isvalid] == false)
		return 0;
	return 1;
}

stock VS_DeSpawnVehicle(int_vehid)
{
	//Callback Call
	CallRemoteFunction("OnDynamicVehicleDespawn", "ii", VS_V[int_vehid][vsv_vehicleid], int_vehid);
	//
	DestroyVehicle(VS_V[int_vehid][vsv_vehicleid]);
	Iter_Remove(VS_I_Vehicles_L, VS_V[int_vehid][vsv_vehicleid]);
	VS_V[int_vehid][vsv_vehicleid] = INVALID_VEHICLE_ID;
	return 1;
}

stock VS_SpawnVehicle(int_vehid)
{
	if(VS_V[int_vehid][vsv_vehicleid] != INVALID_VEHICLE_ID)
		return 0;
	//Create
	VS_V[int_vehid][vsv_vehicleid] = CreateVehicle(
	VS_V[int_vehid][vsv_modelid], 
	VS_V[int_vehid][vsv_x],
	VS_V[int_vehid][vsv_y],
	VS_V[int_vehid][vsv_z],
	VS_V[int_vehid][vsv_a],
	VS_V[int_vehid][vsv_color1],
	VS_V[int_vehid][vsv_color2],
	VS_V[int_vehid][vsv_respawndelay],
	VS_V[int_vehid][vsv_addsiren]);
	//Check for Failure
	if(VS_V[int_vehid][vsv_vehicleid] == INVALID_VEHICLE_ID || VS_V[int_vehid][vsv_vehicleid] == 0)
	{
		VS_V[int_vehid][vsv_vehicleid] = INVALID_VEHICLE_ID;
		return -1;
	}
	//Callback Call
	CallRemoteFunction("OnDynamicVehicleSpawn", "ii", VS_V[int_vehid][vsv_vehicleid], int_vehid);
	//Add to memory
	Iter_Add(VS_I_Vehicles_L, VS_V[int_vehid][vsv_vehicleid]);
	//Health
	SetVehicleHealth(VS_V[int_vehid][vsv_vehicleid], VS_V[int_vehid][vsv_health]);
	//Components
	for(new i; i < 14; i++)
		if(VS_V[int_vehid][vsv_component][i] != 0)
			AddVehicleComponent(VS_V[int_vehid][vsv_vehicleid], VS_V[int_vehid][vsv_component][i]);
	//Paintjob
	ChangeVehiclePaintjob(VS_V[int_vehid][vsv_vehicleid], VS_V[int_vehid][vsv_paintjob]);
	//Interior
	LinkVehicleToInterior(VS_V[int_vehid][vsv_vehicleid], VS_V[int_vehid][vsv_interior]);
	//Virtual World
	SetVehicleVirtualWorld(VS_V[int_vehid][vsv_vehicleid], VS_V[int_vehid][vsv_virtualworld]);
	//
	VS_VehicleInternalID[VS_V[int_vehid][vsv_vehicleid]] = int_vehid;
	return VS_V[int_vehid][vsv_vehicleid];
}

stock VS_UnloadChunk(chunkid)
{
	if(chunkid == -1)
		return 0;
	if(!VS_C[chunkid][vsc_active])
		return 0;
	new unl_count;
	VS_C[chunkid][vsc_active] = false;
	VS_ActiveChunks--;
	foreach(new i : VS_I_ChunkVehicles[chunkid])
	{
		new int_vehid = VS_ChunkVehicleID[chunkid][i];
		VS_DeSpawnVehicle(int_vehid);
		unl_count++;
	}
	if(VS_S[vss_debug])
	{
		GangZoneDestroy(VS_ChunkDebugZone[chunkid]);
		printf("[NVS] Chunk ID %i unloaded. Despawned vehicles: %i", chunkid, unl_count);
	}
	return 1;
}

stock VS_LoadChunk(chunkid)
{
	if(chunkid == -1)
		return 0;
	if(VS_C[chunkid][vsc_active])
		return 0;
	new l_count;
	VS_C[chunkid][vsc_active] = true;
	VS_ActiveChunks++;
	foreach(new i : VS_I_ChunkVehicles[chunkid])
	{
		new int_vehid = VS_ChunkVehicleID[chunkid][i];
		VS_SpawnVehicle(int_vehid);
		l_count++;
	}
	if(VS_S[vss_debug])
	{
		new ColorPalette[][] =
		{
			0x00000099,0xF5F5F599,0x2A77A199, 0x84041099,0x26373999,
			0x86446E99, 0xD78E1099,0x4C75B799,0xBDBEC699
		};
		//Debug VS_C[]
		VS_ChunkDebugZone[chunkid] = GangZoneCreate(VS_C[chunkid][vsc_minx], VS_C[chunkid][vsc_miny], VS_C[chunkid][vsc_maxx], VS_C[chunkid][vsc_maxy]);
		GangZoneShowForAll(VS_ChunkDebugZone[chunkid], ColorPalette[random(sizeof ColorPalette)][0]);
		//
		printf("[NVS] Chunk ID %i loaded. Spawned vehicles: %i", chunkid, l_count);
	}
	return 1;
}

stock VS_RemoveVehicleFromChunk(int_vehicleid, chunkid)
{
	if(VS_V[int_vehicleid][vsv_chunkid] == -1)
		return 0;
	foreach(new i : VS_I_ChunkVehicles[chunkid])
	{
		if(VS_ChunkVehicleID[chunkid][i] == int_vehicleid)
		{
			Iter_Remove(VS_I_ChunkVehicles[chunkid], i);
			VS_ChunkVehicleID[chunkid][i] = INVALID_VEHICLE_ID;
			if(VS_S[vss_debug]) printf("[NVS] Vehicle ID %i unregistered: Chunk ID %i", int_vehicleid, chunkid);
			break;
		}
	}
	
	return 1;
}

stock VS_AddVehicleToChunk(int_vehicleid, chunkid)
{
	VS_V[int_vehicleid][vsv_chunkid] = chunkid;
	new chunkslot = Iter_Free(VS_I_ChunkVehicles[chunkid]);
	if(chunkslot >= 0 && chunkslot < MAX_VEHICLES_PER_CHUNK)
	{
		VS_ChunkVehicleID[chunkid][chunkslot] = int_vehicleid;
		Iter_Add(VS_I_ChunkVehicles[chunkid], chunkslot);
		if(VS_S[vss_debug]) printf("[NVS] Vehicle Internal ID %i registered: Chunk ID %i | Slot %i", int_vehicleid, chunkid, chunkslot);
	}
	else 
		chunkslot = -1;
	return chunkslot;
}

stock VS_CreateChunks()
{
	print("[NVS] Creating chunks...");
	new chunk_array_data[3], chunkid;
	for(new Float:x = -(MAP_SIZE/2); x < (MAP_SIZE/2); x += VS_S[vss_chunksize]) // X
	{
		for(new Float:y = -(MAP_SIZE/2); y < (MAP_SIZE/2); y += VS_S[vss_chunksize]) // Y
		{
			chunkid = Iter_Free(VS_I_Chunks);
			if(chunkid >= 0 && chunkid < MAX_STREAMER_CHUNKS)
			{
				Iter_Add(VS_I_Chunks, chunkid);
				//
				VS_StreamerChunk[chunkid] = CreateDynamicRectangle(x, y, x + VS_S[vss_chunksize], y + VS_S[vss_chunksize]);
				//Inser Data to ArrayData
				chunk_array_data[0] = CHUNK_CHECKSUM;
				chunk_array_data[1] = -CHUNK_CHECKSUM;
				chunk_array_data[2] = chunkid;
				Streamer_SetArrayData(STREAMER_TYPE_AREA, VS_StreamerChunk[chunkid], E_STREAMER_EXTRA_ID, chunk_array_data);
				//Insert data to Enum
				VS_C[chunkid][vsc_minx] = x;
				VS_C[chunkid][vsc_miny] = y;
				VS_C[chunkid][vsc_maxx] = x + VS_S[vss_chunksize];
				VS_C[chunkid][vsc_maxy] = y + VS_S[vss_chunksize];
				//Calculate center
				VS_CalculateAreaCenter(VS_C[chunkid][vsc_minx], VS_C[chunkid][vsc_miny], VS_C[chunkid][vsc_maxx], VS_C[chunkid][vsc_maxy], VS_C[chunkid][vsc_centerx], VS_C[chunkid][vsc_centery]);
			}
			else break;
		}
	}
	printf("[NVS] Chunks Created: %i", Iter_Count(VS_I_Chunks));
	VS_ConnectChunks();
	return 1;
}

stock VS_ConnectChunks()
{
	new areas[100], amount;
	foreach(new i : VS_I_Chunks)
	{
		//North
		amount = GetDynamicAreasForPoint(VS_C[i][vsc_centerx], VS_C[i][vsc_centery] + VS_S[vss_chunksize], 0.0, areas);
		if(amount > 0)
		{
			for(new a; a < amount; a++)
			{
				VS_C[i][vsc_north] = VS_GetChunkID(areas[a]);
				if(VS_C[i][vsc_north] != -1)
					break;
			}
		}
		else VS_C[i][vsc_north] = -1;
		//South
		amount = GetDynamicAreasForPoint(VS_C[i][vsc_centerx], VS_C[i][vsc_centery] - VS_S[vss_chunksize], 0.0, areas);
		if(amount > 0)
		{
			for(new a; a < amount; a++)
			{
				VS_C[i][vsc_south] = VS_GetChunkID(areas[a]);
				if(VS_C[i][vsc_south] != -1)
					break;
			}
		}
		else VS_C[i][vsc_south] = -1;
		//Ost
		amount = GetDynamicAreasForPoint(VS_C[i][vsc_centerx] + VS_S[vss_chunksize], VS_C[i][vsc_centery], 0.0, areas);
		if(amount > 0)
		{
			for(new a; a < amount; a++)
			{
				VS_C[i][vsc_ost] = VS_GetChunkID(areas[a]);
				if(VS_C[i][vsc_ost] != -1)
					break;
			}
		}
		else VS_C[i][vsc_ost] = -1;
		//West
		amount = GetDynamicAreasForPoint(VS_C[i][vsc_centerx] - VS_S[vss_chunksize], VS_C[i][vsc_centery], 0.0, areas);
		if(amount > 0)
		{
			for(new a; a < amount; a++)
			{
				VS_C[i][vsc_west] = VS_GetChunkID(areas[a]);
				if(VS_C[i][vsc_west] != -1)
					break;
			}
		}
		else VS_C[i][vsc_west] = -1;
	}
	return 1;
}

stock VS_LoadConfig()
{
	VS_S[vss_debug] = false;
	VS_S[vss_chunksize] = CHUNK_SIZE;
	VS_S[vss_tickrate] = TICK_RATE;
	//
	if(!dfile_FileExists(CONFIG_FILE))
	{
		dfile_Create(CONFIG_FILE);
		dfile_Open(CONFIG_FILE);
		//
		dfile_WriteBool("debug_mode", false);
		dfile_WriteInt("chunk_size", CHUNK_SIZE);
		dfile_WriteInt("tick_rate", TICK_RATE);
		//
		dfile_SaveFile();
		dfile_CloseFile();
	}
	if(dfile_FileExists(CONFIG_FILE))
	{
		dfile_Open(CONFIG_FILE);
		//
		VS_S[vss_debug] = dfile_ReadBool("debug_mode");
		VS_S[vss_chunksize] = dfile_ReadFloat("chunk_size");
		if(VS_S[vss_chunksize] < 300 || VS_S[vss_chunksize] > 600)
			VS_S[vss_chunksize] = CHUNK_SIZE;
		VS_S[vss_tickrate] = dfile_ReadInt("tick_rate");
		//
		dfile_CloseFile();
	}
	printf("[NVS] Debug Mode: %s | Chunksize: %f | Tickrate: %i", (VS_S[vss_debug] == true) ? ("Enabled") : ("Disabled"), VS_S[vss_chunksize], VS_S[vss_tickrate]);
	return 1;
}

//Miscellaneous Functions
stock VS_GetChunkID(areaid)
{
	new chunk_array_data[3];
	Streamer_GetArrayData(STREAMER_TYPE_AREA, areaid, E_STREAMER_EXTRA_ID, chunk_array_data);
	if(chunk_array_data[0] == CHUNK_CHECKSUM && chunk_array_data[1] == -CHUNK_CHECKSUM)
		return chunk_array_data[2];
	return -1;
}

stock VS_IsVehicleStreamerVehicle(vehicleid)
{
	foreach(new i : VS_I_Vehicles_A)
	{
		if(VS_V[i][vsv_vehicleid] == vehicleid)
			return i;
	}
	return 0;
}

stock VS_CalculateAreaCenter(Float:minx, Float:miny, Float:maxx, Float:maxy, &Float:ret_x, &Float:ret_y)
{
	ret_x = minx + ((maxx - minx) / 2);
	ret_y = miny + ((maxy - miny) / 2);
	return 1;
}
