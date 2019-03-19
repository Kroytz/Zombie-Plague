/**
 * ============================================================================
 *
 *  Zombie Plague
 *
 *  File:          particles.cpp
 *  Type:          Module
 *  Description:   Particles dictionary & manager.
 *
 *  Copyright (C) 2015-2019  Greyscale, Richard Helgeby
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
 * Variables to store SDK calls handlers.
 **/
Handle hSDKCallDestructorParticleDictionary;
Handle hSDKCallContainerFindTable;
Handle hSDKCallTableDeleteAllStrings;

/**
 * Variables to store virtual SDK offsets.
 **/
Address particleSystemDictionary;
Address networkStringTable;
int ParticleSystem_Count;

/**
 * @brief Particles module init function.
 **/
void ParticlesOnInit(/*void*/)
{
    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Raw);
    PrepSDKCall_SetFromConf(gServerData.Config, SDKConf_Signature, "CParticleSystemDictionary::~CParticleSystemDictionary");
    
    // Validate call
    if((hSDKCallDestructorParticleDictionary = EndPrepSDKCall()) == null)
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_GAME_EVENTS, LogModule_Effects, "GameData Validation", "Failed to load SDK call \"CParticleSystemDictionary::~CParticleSystemDictionary\". Update signature in \"%s\"", PLUGIN_CONFIG);
        return;
    }
    
    /*_________________________________________________________________________________________________________________________________________*/

    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Raw);
    PrepSDKCall_SetFromConf(gServerData.Config, SDKConf_Virtual, "CNetworkStringTableContainer::FindTable");
    
    // Adds a parameter to the calling convention. This should be called in normal ascending order
    PrepSDKCall_AddParameter(SDKType_String, SDKPass_Pointer);
    PrepSDKCall_SetReturnInfo(SDKType_PlainOldData, SDKPass_Plain);
    
    // Validate call
    if((hSDKCallContainerFindTable = EndPrepSDKCall()) == null)
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_GAME_EVENTS, LogModule_Effects, "GameData Validation", "Failed to load SDK call \"CNetworkStringTableContainer::FindTable\". Update virtual offset in \"%s\"", PLUGIN_CONFIG);
        return;
    }
    
    /*_________________________________________________________________________________________________________________________________________*/

    // Starts the preparation of an SDK call
    StartPrepSDKCall(SDKCall_Raw);
    PrepSDKCall_SetFromConf(gServerData.Config, SDKConf_Signature, "CNetworkStringTable::DeleteAllStrings");
    
    // Validate call
    if((hSDKCallTableDeleteAllStrings = EndPrepSDKCall()) == null)
    {
        // Log failure
        LogEvent(false, LogType_Fatal, LOG_GAME_EVENTS, LogModule_Effects, "GameData Validation", "Failed to load SDK call \"CNetworkStringTable::DeleteAllStrings\". Update signature in \"%s\"", PLUGIN_CONFIG);
        return;
    }
    
    /*_________________________________________________________________________________________________________________________________________*/
    
    // Load other offsets
    fnInitGameConfAddress(gServerData.Config, particleSystemDictionary, "m_pParticleSystemDictionary");
    fnInitGameConfAddress(gServerData.Config, networkStringTable, "s_NetworkStringTable");
    fnInitGameConfOffset(gServerData.Config, ParticleSystem_Count, "CParticleSystemMgr::GetParticleSystemCount");
}

/**
 * @brief Particles module load function.
 **/
void ParticlesOnLoad(/*void*/)
{
    // Initialize buffer char
    static char sBuffer[PLATFORM_LINE_LENGTH];

    // Validate that particles wasn't precache yet
    bool bSave = LockStringTables(false);
    int iCount = LoadFromAddress(particleSystemDictionary + view_as<Address>(ParticleSystem_Count), NumberType_Int16);
    int iTable = SDKCall(hSDKCallContainerFindTable, networkStringTable, "ParticleEffectNames");
    if(!iCount && iTable) /// Validate that table is exist and it empty
    {
        // Opens the file
        File hFile = OpenFile("particles/particles_manifest.txt", "rt", true);
        
        // If doesn't exist stop
        if(hFile == null)
        {
            LogEvent(false, LogType_Fatal, LOG_CORE_EVENTS, LogModule_Effects, "Config Validation", "Error opening file: \"particles/particles_manifest.txt\"");
            return;
        }

        // Read lines in the file
        while(hFile.ReadLine(sBuffer, sizeof(sBuffer)))
        {
            // Checks if string has correct quotes
            int iQuotes = CountCharInString(sBuffer, '"');
            if(iQuotes == 4)
            {
                // Trim string
                TrimString(sBuffer);

                // Copy value string
                strcopy(sBuffer, sizeof(sBuffer), sBuffer[strlen("\"file\"")]);
                
                // Trim string
                TrimString(sBuffer);
                
                // Strips a quote pair off a string 
                StripQuotes(sBuffer);

                // Precache model
                int i; if(sBuffer[i] == '!') i++;
                PrecacheGeneric(sBuffer[i], true);
                SDKCall(hSDKCallTableDeleteAllStrings, iTable); /// HACK~HACK
                /// Clear tables after each file because some of them contains
                /// huge amount of particles and we work around the limit
            }
        }
    }
    
    // Initialize the table index
    static int tableIndex = INVALID_STRING_TABLE;

    // Validate table
    if(tableIndex == INVALID_STRING_TABLE)
    {
        // Searches for a string table
        tableIndex = FindStringTable("ParticleEffectNames");
    }
    
    // If array hasn't been created, then create
    if(gServerData.Particles == null)
    {
        // Initialize a particle list array
        gServerData.Particles = CreateArray(NORMAL_LINE_LENGTH); 

        // i = table string
        iCount = GetStringTableNumStrings(tableIndex);
        for(int i = 0; i < iCount; i++)
        {
            // Gets the string at a given index
            ReadStringTable(tableIndex, i, sBuffer, sizeof(sBuffer));
            
            // Push data into array 
            gServerData.Particles.PushString(sBuffer);
        }
    }
    else
    {
        // i = particle name
        iCount = gServerData.Particles.Length;
        for(int i = 0; i < iCount; i++)
        {
            // Gets the string at a given index
            gServerData.Particles.GetString(i, sBuffer, sizeof(sBuffer));
            
            // Push data into table 
            AddToStringTable(tableIndex, sBuffer);
        }
    }   
    LockStringTables(bSave);
}

/**
 * @brief Particles module purge function.
 **/
void ParticlesOnPurge(/*void*/)
{
    // @link https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/particles/particles.cpp#L81
    SDKCall(hSDKCallDestructorParticleDictionary, particleSystemDictionary);

    /*_________________________________________________________________________________________________________________________________________*/
    
    /// Clear all particles effect table
    bool bSave = LockStringTables(false);
    int iTable = SDKCall(hSDKCallContainerFindTable, networkStringTable, "ParticleEffectNames");
    if(iTable)   SDKCall(hSDKCallTableDeleteAllStrings, iTable);
    LockStringTables(bSave);
}

/*
 * Stocks particles API.
 */ 

/**
 * @brief Create an attached particle entity.
 * 
 * @param clientIndex       The client index.
 * @param sAttach           The attachment name.
 * @param sType             The type of the particle.
 * @param flDurationTime    The duration of light.
 * @return                  The entity index.
 **/
int ParticlesCreate(int clientIndex, char[] sAttach, char[] sType, float flDurationTime)
{
    // Validate name
    if(!hasLength(sType))
    {
        return INVALID_ENT_REFERENCE;
    }
    
    // Create an attach particle entity
    int entityIndex = CreateEntityByName("info_particle_system");
    
    // If entity isn't valid, then skip
    if(entityIndex != INVALID_ENT_REFERENCE)
    {
        // Dispatch main values of the entity
        DispatchKeyValue(entityIndex, "start_active", "1");
        DispatchKeyValue(entityIndex, "effect_name", sType);
        
        // Spawn the entity into the world
        DispatchSpawn(entityIndex);

        // Sets parent to the entity
        SetVariantString("!activator");
        AcceptEntityInput(entityIndex, "SetParent", clientIndex, entityIndex);
        ToolsSetEntityOwner(entityIndex, clientIndex);
        
        // Sets attachment to the entity
        if(hasLength(sAttach))
        { 
            SetVariantString(sAttach); 
            AcceptEntityInput(entityIndex, "SetParentAttachment", clientIndex, entityIndex);
        }
        else
        {
            // Initialize vector variables
            static float vPosition[3];

            // Gets client position
            ToolsGetClientAbsOrigin(clientIndex, vPosition);

            // Teleport the entity
            DispatchKeyValueVector(entityIndex, "origin", vPosition);
        }
        
        // Activate the entity
        ActivateEntity(entityIndex);
        AcceptEntityInput(entityIndex, "Start");
        
        // Initialize time char
        static char sTime[SMALL_LINE_LENGTH];
        FormatEx(sTime, sizeof(sTime), "OnUser1 !self:kill::%f:1", flDurationTime);
        
        // Sets modified flags on the entity
        SetVariantString(sTime);
        AcceptEntityInput(entityIndex, "AddOutput");
        AcceptEntityInput(entityIndex, "FireUser1");
    }
    
    // Return on success
    return entityIndex;
}

/**
 * @brief Delete an attached particle from the entity.
 * 
 * @param clientIndex       The client index.
 **/
void ParticlesRemove(int clientIndex)
{
    // Initialize classname char
    static char sClassname[SMALL_LINE_LENGTH];

    // i = entity index
    int MaxEntities = GetMaxEntities();
    for(int i = MaxClients; i <= MaxEntities; i++)
    {
        // Validate entity
        if(IsValidEdict(i))
        {
            // Gets valid edict classname
            GetEdictClassname(i, sClassname, sizeof(sClassname));

            // If entity is an attach particle entity
            if(sClassname[0] == 'i' && sClassname[5] == 'p' && sClassname[6] == 'a')
            {
                // Validate parent
                if(ToolsGetEntityOwner(i) == clientIndex)
                {
                    AcceptEntityInput(i, "Kill"); /// Destroy
                }
            }
        }
    }
}

/**
 * @brief Create an attached muzzle to the entity.
 * 
 * @param clientIndex       The client index.
 * @param entityIndex       The weapon index.
 * @param sEffect           The effect name.
 **/
void ParticlesMuzzleCreate(int clientIndex, int entityIndex, char[] sEffect)
{
    // Initialize vector variables
    static float vPosition[3];

    // Gets client position
    ToolsGetClientAbsOrigin(clientIndex, vPosition); 

    // Create an effect
    VEffectDispatch(entityIndex, sEffect, "ParticleEffect", vPosition, vPosition, _, 1);
    TE_SendToClient(clientIndex);
}

/**
 * @brief Create an attached muzzlesmoke to the entity.
 * 
 * @param clientIndex       The client index.
 * @param entityIndex       The weapon index.
 **/
void ParticlesMuzzleSmoke(int clientIndex, int entityIndex)
{
    // Initialize vector variables
    static float vPosition[3];

    // Gets client position
    ToolsGetClientAbsOrigin(clientIndex, vPosition); 

    // Create an effect
    VEffectDispatch(entityIndex, "weapon_muzzle_smoke", "ParticleEffect", vPosition, vPosition, _, 1);
    TE_SendToClient(clientIndex);
}

/**
 * @brief Delete an attached muzzlesmoke from the entity.
 * 
 * @param clientIndex       The client index.
 * @param entityIndex       The weapon index.
 **/
void ParticlesMuzzleRemove(int clientIndex, int entityIndex)
{
    // Initialize vector variables
    static float vPosition[3];

    // Gets client position
    ToolsGetClientAbsOrigin(clientIndex, vPosition); 

    // Delete an effect
    VEffectDispatch(entityIndex, _, "ParticleEffectStop", vPosition, vPosition);
    TE_SendToClient(clientIndex);
}