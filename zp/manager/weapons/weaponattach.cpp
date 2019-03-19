/**
 * ============================================================================
 *
 *  Zombie Plague
 *
 *  File:          weaponattach.cpp
 *  Type:          Module
 *  Description:   Weapon attachments functions.
 *
 *  Copyright (C) 2015-2019 Nikita Ushakov (Ireland, Dublin), Mitchell
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 **/

/**
 * @section Weapon addon bits.
 **/
#define CSAddon_NONE                0
#define CSAddon_Flashbang1          (1<<0)
#define CSAddon_Flashbang2          (1<<1)
#define CSAddon_HEGrenade           (1<<2)
#define CSAddon_SmokeGrenade        (1<<3)
#define CSAddon_C4                  (1<<4)
#define CSAddon_DefuseKit           (1<<5)
#define CSAddon_PrimaryWeapon       (1<<6)
#define CSAddon_SecondaryWeapon     (1<<7)
#define CSAddon_Holster             (1<<8) 
#define CSAddon_Decoy               (1<<9)
#define CSAddon_Knife               (1<<10)
#define CSAddon_FaceMask            (1<<11) // I'am guess
#define CSAddon_TaGrenade           (1<<12)
/**
 * @endsection
 **/
 
/**
 * @section Number of valid addons.
 **/
enum BitType
{
    BitType_Invalid = -1,         /** Used as return value when an bit doens't exist. */
    
    BitType_PrimaryWeapon,        /** Primary bit */
    BitType_SecondaryWeapon,      /** Secondary bit */
    BitType_Flashbang1,           /** Flashbang1 bit */
    BitType_Flashbang2,           /** Flashbang2 bit */
    BitType_HEGrenade,            /** Hegrenade bit */
    BitType_SmokeGrenade,         /** Smokegrenade bit */
    BitType_Decoy,                /** Decoy bit */
    BitType_Knife,                /** Knife bit */
    BitType_TaGrenade,            /** Tagrenade bit */
    BitType_C4,                   /** C4 bit */
    BitType_DefuseKit             /** Defuse bit */
};
/**
 * @endsection
 **/

/**
 * @brief Destroy weapon attachments.
 **/
void WeaponAttachOnUnload(/*void*/) 
{
    // i = client index
    for(int i = 1; i <= MaxClients; i++) 
    {
        // Validate client
        if(IsPlayerExist(i, false)) 
        {
            // Remove current attachment
            WeaponAttachRemoveAddons(i);
        }
    }
}

/**
 * Hook: SetTransmit
 * @brief Called right before the entity transmitting to other entities.
 *
 * @param entityIndex       The entity index.
 * @param clientIndex       The client index.
 **/
public Action WeaponAttachOnTransmit(int entityIndex, int clientIndex)
{
    // i = slot index
    for(BitType i = BitType_PrimaryWeapon; i <= BitType_DefuseKit; i++)
    {
        // Validate addons
        if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[i]) == entityIndex)
        {
            // Validate observer mode
            if(ToolsGetClientObserverMode(clientIndex))
            {
                // Allow transmitting    
                return Plugin_Continue;
            }

            // Block transmitting
            return Plugin_Handled;
        }
    }
    
    // Gets the owner of the entity
    int ownerIndex = ToolsGetEntityOwner(entityIndex);

    // Validate dead owner
    if(!IsPlayerAlive(ownerIndex))
    {
        // Block transmitting
        return Plugin_Handled;
    }
    
    // Validate observer mode
    if(ToolsGetClientObserverMode(clientIndex) == SPECMODE_FIRSTPERSON && ownerIndex == ToolsGetClientObserverTarget(clientIndex))
    {
        // Block transmitting
        return Plugin_Handled;
    }

    // Allow transmitting
    return Plugin_Continue;
}

/*
 * Stocks attachment API.
 */

/**
 * @brief Sets addons attachment.
 *
 * @param clientIndex       The client index.
 **/
void WeaponAttachSetAddons(int clientIndex)
{
    // Gets current bits
    int iBits = ToolsGetClientAddonBits(clientIndex); int iBitPurge; static int weaponIndex; static int iD;
    
    /*____________________________________________________________________________________________*/
    
    // Validate primary bits
    if(iBits & CSAddon_PrimaryWeapon)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_PrimaryWeapon))
        {
            // Gets weapon index
            weaponIndex = GetPlayerWeaponSlot(clientIndex, view_as<int>(SlotType_Primary));
            
            // Validate weapon
            if(IsValidEdict(weaponIndex))
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_PrimaryWeapon, "primary");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_PrimaryWeapon)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_PrimaryWeapon);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate secondary bits
    if(iBits & CSAddon_SecondaryWeapon)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_SecondaryWeapon))
        {
            // Gets weapon index
            weaponIndex = GetPlayerWeaponSlot(clientIndex, view_as<int>(SlotType_Secondary));

            // Validate taser slot
            if(weaponIndex == ToolsGetClientActiveWeapon(clientIndex))
            {
                // Gets weapon index
                weaponIndex = WeaponsGetIndex(clientIndex, "weapon_taser");
            }
            
            // Validate weapon
            if(weaponIndex != INVALID_ENT_REFERENCE)
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_SecondaryWeapon, "pistol");
                }
            }
        }
        }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_SecondaryWeapon)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_SecondaryWeapon);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate flashbang1 bits
    if(iBits & CSAddon_Flashbang1)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_Flashbang1))
        {
            // Gets weapon index
            weaponIndex = WeaponsGetIndex(clientIndex, "weapon_flashbang");
            
            // Validate weapon
            if(weaponIndex != INVALID_ENT_REFERENCE)
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_Flashbang1, "grenade0");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_Flashbang1)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_Flashbang1);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate flashbang2 bits
    if(iBits & CSAddon_Flashbang2)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_Flashbang2))
        {
            // Gets weapon index
            weaponIndex = WeaponsGetIndex(clientIndex, "weapon_flashbang");
            
            // Validate weapon
            if(weaponIndex != INVALID_ENT_REFERENCE)
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_Flashbang2, "eholster");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_Flashbang2)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_Flashbang2);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate hegrenade bits
    if(iBits & CSAddon_HEGrenade)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_HEGrenade))
        {
            // Gets weapon index
            weaponIndex = WeaponsGetIndex(clientIndex, "weapon_hegrenade");
            
            // Validate weapon
            if(weaponIndex != INVALID_ENT_REFERENCE)
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_HEGrenade, "grenade1");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_HEGrenade)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_HEGrenade);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate smokegrenade bits
    if(iBits & CSAddon_SmokeGrenade)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_SmokeGrenade))
        {
            // Gets weapon index
            weaponIndex = WeaponsGetIndex(clientIndex, "weapon_smokegrenade");
            
            // Validate weapon
            if(weaponIndex != INVALID_ENT_REFERENCE)
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_SmokeGrenade, "grenade2");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_SmokeGrenade)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_SmokeGrenade);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate decoy bits
    if(iBits & CSAddon_Decoy)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_Decoy))
        {
            // Gets weapon index
            weaponIndex = WeaponsGetIndex(clientIndex, "weapon_decoy");
            
            // Validate weapon
            if(weaponIndex != INVALID_ENT_REFERENCE)
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_Decoy, "grenade3");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_Decoy)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_Decoy);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate knife bits
    if(iBits & CSAddon_Knife)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_Knife))
        {
            // Gets weapon index
            weaponIndex = GetPlayerWeaponSlot(clientIndex, view_as<int>(SlotType_Melee));
            
            // Validate weapon
            if(IsValidEdict(weaponIndex))
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_Knife, "knife");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_Knife)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_Knife);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate tagrenade bits
    if(iBits & CSAddon_TaGrenade)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_TaGrenade))
        {
            // Gets weapon index
            weaponIndex = WeaponsGetIndex(clientIndex, "weapon_tagrenade");
            
            // Validate weapon
            if(weaponIndex != INVALID_ENT_REFERENCE)
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_TaGrenade, "grenade4");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_TaGrenade)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_TaGrenade);
    }
    
    /*____________________________________________________________________________________________*/
    
    // Validate c4 bits 
    if(iBits & CSAddon_C4)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_C4))
        {
            // Gets weapon index
            weaponIndex = GetPlayerWeaponSlot(clientIndex, view_as<int>(SlotType_C4));
            
            // Validate weapon
            if(IsValidEdict(weaponIndex))
            {
                // Validate custom index
                iD = WeaponsGetCustomID(weaponIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_C4, "c4");
                }
            }
        }
    }
    else if(gClientData[clientIndex].AttachmentBits & CSAddon_C4)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_C4);
    }

    /*____________________________________________________________________________________________*/
    
    // Validate defuser bits 
    if(iBits & CSAddon_DefuseKit)
    {
        // Gets client bits
        if(!(gClientData[clientIndex].AttachmentBits & CSAddon_DefuseKit))
        {
            // Validate defuser
            if(ToolsGetClientDefuser(clientIndex))
            {
                // Validate custom index
                iD = WeaponsGetCustomID(clientIndex);
                if(iD != -1)
                {
                    // Create weapon addons
                    WeaponAttachCreateAddons(clientIndex, iD, BitType_DefuseKit, "c4");
                }
            }
        }
    }
    /*else if(gClientData[clientIndex].AttachmentBits & CSAddon_DefuseKit)
    {
        // Remove current addons
        WeaponAttachRemoveAddons(clientIndex, BitType_DefuseKit);
    }*/
    
    /*____________________________________________________________________________________________*/
    
    // Validate addons
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_PrimaryWeapon]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_PrimaryWeapon;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_SecondaryWeapon]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_SecondaryWeapon;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_Flashbang1]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_Flashbang1;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_Flashbang2]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_Flashbang2;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_HEGrenade]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_HEGrenade;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_SmokeGrenade]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_SmokeGrenade;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_Decoy]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_Decoy;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_Knife]) != INVALID_ENT_REFERENCE || gClientData[clientIndex].Zombie)
    {
        iBitPurge |= CSAddon_Knife; iBitPurge |= CSAddon_Holster;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_TaGrenade]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_TaGrenade;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_C4]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_C4;
    }
    if(EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[BitType_DefuseKit]) != INVALID_ENT_REFERENCE)
    {
        iBitPurge |= CSAddon_DefuseKit; if(!ToolsGetClientDefuser(clientIndex)) WeaponAttachRemoveAddons(clientIndex, BitType_DefuseKit);
    }
    
    // Store the bits for next usage
    gClientData[clientIndex].AttachmentBits = iBits;
    ToolsSetClientAddonBits(clientIndex, iBits &~ iBitPurge);
}

/**
 * @brief Create an attachment addons entities for the client.
 *
 * @param clientIndex       The client index.
 * @param iD                The weapon id.
 * @param mBits             The bits type.
 * @param sAttach           The attachment name.
 **/
void WeaponAttachCreateAddons(int clientIndex, int iD, BitType mBits, char[] sAttach)
{
    // Remove current addons
    WeaponAttachRemoveAddons(clientIndex, mBits);

    // If dropmodel exist, then apply it
    if(WeaponsGetModelDropID(iD))
    {
        // Validate attachment
        if(ToolsLookupAttachment(clientIndex, sAttach))
        {
            // Create an attach addon entity 
            int entityIndex = CreateEntityByName("prop_dynamic_override");
            
            // If entity isn't valid, then skip
            if(entityIndex != INVALID_ENT_REFERENCE)
            {
                // Gets weapon dropmodel
                static char sModel[PLATFORM_LINE_LENGTH];
                WeaponsGetModelDrop(iD, sModel, sizeof(sModel)); 

                // Dispatch main values of the entity
                DispatchKeyValue(entityIndex, "model", sModel);
                DispatchKeyValue(entityIndex, "spawnflags", "256"); /// Start with collision disabled
                DispatchKeyValue(entityIndex, "solid", "0");
               
                // Sets bodygroup of the entity
                SetVariantInt(WeaponsGetModelBody(iD, ModelType_Drop));
                AcceptEntityInput(entityIndex, "SetBodyGroup");
                
                // Sets skin of the entity
                SetVariantInt(WeaponsGetModelSkin(iD, ModelType_Drop));
                AcceptEntityInput(entityIndex, "ModelSkin");
                
                // Spawn the entity into the world
                DispatchSpawn(entityIndex);
 
                // Sets parent to the entity
                SetVariantString("!activator");
                AcceptEntityInput(entityIndex, "SetParent", clientIndex, entityIndex);
                ToolsSetEntityOwner(entityIndex, clientIndex);
                
                // Sets attachment to the entity
                SetVariantString(sAttach);
                AcceptEntityInput(entityIndex, "SetParentAttachment", clientIndex, entityIndex);
                
                // Hook entity callbacks
                SDKHook(entityIndex, SDKHook_SetTransmit, WeaponAttachOnTransmit);
                
                // Store the client cache
                gClientData[clientIndex].AttachmentAddons[mBits] = EntIndexToEntRef(entityIndex);
            }
        }
    }
}

/**
 * @brief Remove an attachment addons entities from the client.
 *
 * @param clientIndex       The client index.
 * @param mBits             The bits type.
 **/
void WeaponAttachRemoveAddons(int clientIndex, BitType mBits = BitType_Invalid) 
{
    // Validate all
    if(mBits == BitType_Invalid)
    {
        // i = slot index
        for(BitType i = BitType_PrimaryWeapon; i <= BitType_DefuseKit; i++)
        {
            // Gets current addon from the client reference
            int entityIndex = EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[i]);
    
            // Validate addon
            if(entityIndex != INVALID_ENT_REFERENCE) 
            {
                AcceptEntityInput(entityIndex, "Kill");
            }

            // Clear the client cache
            gClientData[clientIndex].AttachmentBits = CSAddon_NONE;
            gClientData[clientIndex].AttachmentAddons[i] = INVALID_ENT_REFERENCE;
        }
    }
    else
    {
        // Gets current addon from the client reference
        int entityIndex = EntRefToEntIndex(gClientData[clientIndex].AttachmentAddons[mBits]);

        // Validate addon
        if(entityIndex != INVALID_ENT_REFERENCE) 
        {
            AcceptEntityInput(entityIndex, "Kill");
        }

        // Clear the client cache
        gClientData[clientIndex].AttachmentBits = CSAddon_NONE;
        gClientData[clientIndex].AttachmentAddons[mBits] = INVALID_ENT_REFERENCE;
    }
}