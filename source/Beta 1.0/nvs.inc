/***********************************************************************************************************************

	Include: Nickk's Vehicle Streamer
		Version: Beta 1.0
		
Dependencies:
-	SSCANF2 Plugin: https://github.com/maddinat0r/sscanf/releases
-	Streamer Plugin: https://github.com/samp-incognito/samp-streamer-plugin/releases

Functions:
-	CreateDynamicVehicle(model, Float:x, Float:y, Float:z, Float:rot, color1, color2, respawndelay = -1, addsiren = 0)
-	DestroyDynamicVehicle(vehicleid)
-	SetDynamicVehiclePos(vehicleid, Float:x, Float:y, Float:z)
-	SetDynamicVehicleZAngle(vehicleid, Float:zangle)
-	SetDynamicVehicleHealth(vehicleid, Float:health)
-	ChangeDynamicVehicleColor(vehicleid, color1, color2)
-	AddDynamicVehicleComponent(vehicleid, componentid)
-	PutPlayerInDynamicVehicle(playerid, vehicleid, seatid)
-	SetDynamicVehicleToRespawn(vehicleid)
-	ChangeDynamicVehiclePaintjob(vehicleid, paintjobid)
-	LinkDynamicVehicleToInterior(vehicleid, interiorid)
-	SetDynamicVehicleVirtualWorld(vehicleid, worldid)
-	CountDynamicVehicles()
-	CountSpawnedDynamicVehicles()
-	GetDynamicVehicleInternalID(vehicleid)
-	GetDynamicVehicleModel(vehicleid)
-	GetDynamicVehicleVirtualWorld(vehicleid)
-	GetDynamicVehicleInterior(vehicleid)
-	GetDynamicVehiclePos(vehicleid, &Float:x, &Float:y, &Float:z, &Float:angle)
-	GetDynamicVehicleColor(vehicleid, &color1, &color2)

Callbacks:
-	OnDynamicVehicleSpawn(vehicleid, internal_id)
-	OnDynamicVehicleDespawn(vehicleid, internal_id)

***********************************************************************************************************************/

#include <sscanf2>

new nvs_ret_code, nvs_ret_str[128];

//Forwards
forward OnDynamicVehicleSpawn(vehicleid, internal_id);
forward OnDynamicVehicleDespawn(vehicleid, internal_id);

/*******************************************
*				SET FUNCTIONS			   *
********************************************/
stock CreateDynamicVehicle(model, Float:x, Float:y, Float:z, Float:rot, color1, color2, respawndelay = -1, addsiren = 0)
	return CallRemoteFunction("VS_CreateDynamicVehicle", "iffffiiii", model, x, y, z, rot, color1, color2, respawndelay, addsiren);
	
stock DestroyDynamicVehicle(vehicleid)
	return CallRemoteFunction("VS_DestroyDynamicVehicle", "i", vehicleid);
	
stock SetDynamicVehiclePos(vehicleid, Float:x, Float:y, Float:z)
	return CallRemoteFunction("VS_SetDynamicVehiclePos", "ifff", vehicleid, x, y, z);
	
stock SetDynamicVehicleZAngle(vehicleid, Float:zangle)
	return CallRemoteFunction("VS_SetDynamicVehicleZAngle", "if", vehicleid, zangle);
	
stock SetDynamicVehicleHealth(vehicleid, Float:health)
	return CallRemoteFunction("VS_SetDynamicVehicleHealth", "if", vehicleid, health);
	
stock ChangeDynamicVehicleColor(vehicleid, color1, color2)
	return CallRemoteFunction("VS_ChangeDynamicVehicleColor", "iii", vehicleid, color1, color2);
	
stock AddDynamicVehicleComponent(vehicleid, componentid)
	return CallRemoteFunction("VS_AddDynamicVehicleComponent", "ii", vehicleid, componentid);
	
stock PutPlayerInDynamicVehicle(playerid, vehicleid, seatid)
	return CallRemoteFunction("VS_PutPlayerInDynamicVehicle", "iii", playerid, vehicleid, seatid);
	
stock SetDynamicVehicleToRespawn(vehicleid)
	return CallRemoteFunction("VS_SetDynamicVehicleToRespawn", "i", vehicleid);
	
stock ChangeDynamicVehiclePaintjob(vehicleid, paintjobid)
	return CallRemoteFunction("VS_ChangeDynamicVehiclePaintjob", "ii", vehicleid, paintjobid);

stock LinkDynamicVehicleToInterior(vehicleid, interiorid)
	return CallRemoteFunction("VS_LinkDynamicVehicleToInterior", "ii", vehicleid, interiorid);
	
stock SetDynamicVehicleVirtualWorld(vehicleid, worldid)
	return CallRemoteFunction("VS_SetDynamicVehicleVW", "ii", vehicleid, worldid);

/*******************************************
*				GET FUNCTIONS			   *
********************************************/

stock CountDynamicVehicles()
	return CallRemoteFunction("VS_CountDynamicVehicles", "", #);
	
stock CountSpawnedDynamicVehicles()
	return CallRemoteFunction("VS_CountSpawnedDynamicVehicles", "", #);
	
stock GetDynamicVehicleInternalID(vehicleid)
	return CallRemoteFunction("VS_GetDynamicVehicleInternalID", "i", vehicleid);

stock GetDynamicVehicleModel(vehicleid)
	return CallRemoteFunction("VS_GetDynamicVehicleModel", "i", vehicleid);
	
stock GetDynamicVehicleVirtualWorld(vehicleid)
	return CallRemoteFunction("VS_GetDynamicVehicleVW", "i", vehicleid);
	
stock GetDynamicVehicleInterior(vehicleid)
	return CallRemoteFunction("VS_GetDynamicVehicleInterior", "i", vehicleid);
	
stock GetDynamicVehicleComponent(vehicleid, slot)
	return CallRemoteFunction("VS_GetDynVehicleComponentInSlot", "ii", vehicleid, slot);

stock GetDynamicVehiclePos(vehicleid, &Float:x, &Float:y, &Float:z, &Float:angle)
{
	nvs_ret_code = CallRemoteFunction("VS_GetDynamicVehiclePos", "i", vehicleid);
	switch(nvs_ret_code)
	{
		case 0: return 0;
		default:
		{
			if(existproperty(0, "", nvs_ret_code))
			{
				getproperty(0, "", nvs_ret_code, nvs_ret_str);
				deleteproperty(0, "", nvs_ret_code);
				strunpack(nvs_ret_str, nvs_ret_str, sizeof(nvs_ret_str));
				sscanf(nvs_ret_str, "ffff", x, y, z, angle);
				return 1;
			}
		}
	}
	return 0;
}

stock GetDynamicVehicleColor(vehicleid, &color1, &color2)
{
	nvs_ret_code = CallRemoteFunction("VS_GetDynamicVehicleColor", "i", vehicleid);
	switch(nvs_ret_code)
	{
		case 0: return 0;
		default:
		{
			if(existproperty(0, "", nvs_ret_code))
			{
				getproperty(0, "", nvs_ret_code, nvs_ret_str);
				deleteproperty(0, "", nvs_ret_code);
				strunpack(nvs_ret_str, nvs_ret_str, sizeof(nvs_ret_str));
				sscanf(nvs_ret_str, "ii", color1, color2);
				return 1;
			}
		}
	}
	return 0;
}

	