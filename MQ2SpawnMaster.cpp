// MQ2SpawnMaster - Spawn tracking/analysis utility
//
// Commands:
//  /spawnmaster - display usage information
//
// MQ2Data Variables
//     bool SpawnMaster - True if spawn monitoring is active. NULL if plugin not loaded.
// Members:
//     int    Search      - Number of search strings being monitored for the current zone
//     int    UpList      - Number of matching spawns currently up
//     int    DownList    - Number of matching spawns that have died or depopped
//     string LastMatch   - The name of the last spawn to match a search
//
// Changes:
// 11/02/2004
//  Restructured initialization and zoning to utilize Cronic's new zoning callbacks
//  New INI file handling.  Entries MUST start with spawn0 and count up sequentially
// 10/27/2004
//  changes to formatting and colors, thanks to Chill for suggestions
//  added location of death/despawn to the struct
// 10/23/2004
//  initial "proof of concept" release
//  some code borrowed from Digitalxero's SpawnAlert plugin
//
///////////////////////////////////////////////////////////////////////////////////////////

//24/04/2017 Version 11.0 by eqmule - Added sound support for mob spawning
//22/06/2017 Version 11.1 by eqmule - Added settings per server.character

#include <mq/Plugin.h>

#include <list>

constexpr auto PLUGIN_NAME = "MQ2SpawnMaster";
PLUGIN_VERSION(11.3);

PreSetup(PLUGIN_NAME);

constexpr auto SKIP_PULSES = 10;

float fMasterVolume = 1.0;
bool bMasterVolumeSet = false;

struct _SEARCH_STRINGS {
    CHAR SearchString[MAX_STRING];
    CHAR SearchSound[MAX_STRING];
};

typedef struct _GAMETIME {
    BYTE Hour;
    BYTE Minute;
    BYTE Day;
    BYTE Month;
    DWORD Year;
} GAMETIME, *PGAMETIME;

typedef struct _SPAWN_DATA {
    CHAR SpawnTime[MAX_STRING];
    CHAR DeSpawnTime[MAX_STRING];
    CHAR Name[MAX_STRING];
    GAMETIME SpawnGameTime;
    GAMETIME DeSpawnGameTime;
    DWORD SpawnID;
    FLOAT SpawnedX;
    FLOAT SpawnedY;
    FLOAT SpawnedZ;
    FLOAT LastX;
    FLOAT LastY;
    FLOAT LastZ;
} SPAWN_DATA, *PSPAWN_DATA;

std::list<_SEARCH_STRINGS> SearchStrings;
std::list<SPAWN_DATA> SpawnUpList;
std::list<SPAWN_DATA> SpawnDownList;

bool bSpawnMasterOn = false;
bool bUseExactCase = false;

// TODO:  Replace with the built-in methods
// case-insensitive string search, the first argument is the string
// to be searched, the second is the string to search for
bool StringCompare(PCHAR string, PCHAR strSearch)
{
    CHAR szTemp1[MAX_STRING];
    CHAR szTemp2[MAX_STRING];
    strcpy_s(szTemp1,string);
    strcpy_s(szTemp2,strSearch);
    if(!bUseExactCase) {
        _strlwr_s(szTemp1);
        _strlwr_s(szTemp2);
    }
    return strstr(szTemp1,szTemp2)?true:false;
}

void AddNameToSearchList(PCHAR SpawnName)
{
	CHAR szSound[MAX_STRING] = { 0 };
	if (char*pDest = strchr(SpawnName, '|')) {
		pDest[0] = '\0';
		strcpy_s(szSound, &pDest[1]);
	}
    std::list<_SEARCH_STRINGS>::iterator pSearchStrings = SearchStrings.begin();
    while (pSearchStrings!=SearchStrings.end())
    {
        if (!strcmp(SpawnName,pSearchStrings->SearchString))
        {
            WriteChatf("\at%s\ax::\aoAlready watching for \"\ay%s\ao\"", PLUGIN_NAME, SpawnName);
            return;
        }
        pSearchStrings++;
    }
    _SEARCH_STRINGS NewString;
    strcpy_s(NewString.SearchString,SpawnName);
    strcpy_s(NewString.SearchSound,szSound);
    SearchStrings.push_back(NewString);
    WriteChatf("\at%s\ax::\aoNow watching for \"\ag%s\ao\"", PLUGIN_NAME, SpawnName);
}

void WriteINI()
{
    CHAR szTemp[MAX_STRING];
    if(GetCharInfo()->zoneId > MAX_ZONES)
        return;

    EQWorldData* psWorldData = pWorldData;
    CHAR ZoneName[MAX_STRING] = {0};
    strcpy_s(ZoneName,psWorldData->ZoneArray[GetCharInfo()->zoneId]->LongName);

    // clear the INI section
    WritePrivateProfileString(ZoneName,NULL,NULL,INIFileName);
    if (SearchStrings.empty()) return;
    std::list<_SEARCH_STRINGS>::iterator pSearchStrings = SearchStrings.begin();
    int i=0;
    while (pSearchStrings!=SearchStrings.end())
    {
        sprintf_s(szTemp,"Spawn%d",i);
        WritePrivateProfileString(ZoneName,szTemp,pSearchStrings->SearchString,INIFileName);
		if (pSearchStrings->SearchSound[0] != '\0') {
			sprintf_s(szTemp,"Sound%d",i);
			WritePrivateProfileString(ZoneName,szTemp,pSearchStrings->SearchSound,INIFileName);
		}
        i++;
        pSearchStrings++;
    }
    return;
}

// FIXME: MAX_STRING everywhere
void ReadSpawnListFromINI()
{
    if(GetCharInfo()->zoneId > MAX_ZONES)
        return;
    CHAR ZoneName[MAX_STRING];
    CHAR szTemp[MAX_STRING];
    CHAR szBuffer[MAX_STRING];
    CHAR szSound[MAX_STRING];
    EQWorldData* psWorldData = pWorldData;

    strcpy_s(ZoneName,psWorldData->ZoneArray[GetCharInfo()->zoneId]->LongName);
    //WriteChatf("\at%s\ax::\am%s", PLUGIN_NAME, ZoneName);
    int i=0;
    do {
        sprintf_s(szTemp,"spawn%d",i);
        GetPrivateProfileString(ZoneName,szTemp,"notfound",szBuffer,MAX_STRING,INIFileName);
        if (!strcmp(szBuffer,"notfound"))
			break;
        sprintf_s(szTemp,"sound%d",i);
        GetPrivateProfileString(ZoneName,szTemp,NULL,szSound,MAX_STRING,INIFileName);
		if (szSound[0] != '\0') {
			strcat_s(szBuffer, "|");
			strcat_s(szBuffer, szSound);
		}
        AddNameToSearchList(szBuffer);
    } while (++i);
}

void RemoveNameFromSearchList(PCHAR SpawnName)
{
    std::list<_SEARCH_STRINGS>::iterator pSearchStrings = SearchStrings.begin();
    while (pSearchStrings!=SearchStrings.end())
    {
        if (!strcmp(SpawnName,pSearchStrings->SearchString))
        {
            WriteChatf("\at%s\ax::\aoNo longer watching for \"\ay%s\ao\"", PLUGIN_NAME, SpawnName);
            SearchStrings.erase(pSearchStrings);
            return;
        }
        pSearchStrings++;
    }
    WriteChatf("\at%s\ax::\aoCannot delete \"\ay%s\ao\", not found on watch list", PLUGIN_NAME, SpawnName);
}

template <unsigned int _Size>VOID GetLocalTimeHHMMSS(CHAR(&SysTime)[_Size])
{
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    WORD Hour = st.wHour%12;
    if (!Hour) Hour=12;
    sprintf_s(SysTime, "%2d:%02d:%02d", Hour, st.wMinute, st.wSecond );
}

// TODO:  Why is Sound required to determine if something is a watched spawn?
template<unsigned int _Size>BOOL IsWatchedSpawn(PSPAWNINFO pSpawn,char(&Sound)[_Size])
{
    if (pSpawn->Type==SPAWN_CORPSE)
		return false;
    std::list<_SEARCH_STRINGS>::iterator pSearchStrings = SearchStrings.begin();
    while (pSearchStrings!=SearchStrings.end())
    {
        PCHAR p = pSearchStrings->SearchString;
        if (p[0] == '#') // exact match
        {
            p++; // skip over the #
			if (!strcmp(pSpawn->DisplayedName, p)) {
				strcpy_s(Sound, _Size, pSearchStrings->SearchSound);
				return true;
			}
        }
        else
        {
			if (StringCompare(pSpawn->DisplayedName, p)) {
				strcpy_s(Sound, _Size, pSearchStrings->SearchSound);
				return true;
			}
        }
        pSearchStrings++;
    }
    return false;
}

template<unsigned int _Size>VOID AddSpawnToUpList(PSPAWNINFO pSpawn,char(&Sound)[_Size])
{
    // Don't add the spawn if its already a corpse
    if (pSpawn->Type!=SPAWN_CORPSE)
    {
        CHAR szTemp[MAX_STRING] = {0};
        SPAWN_DATA UpListTemp;

        UpListTemp.SpawnID = pSpawn->SpawnID;
        strcpy_s(UpListTemp.Name,pSpawn->DisplayedName);
        GetLocalTimeHHMMSS(UpListTemp.SpawnTime);

        UpListTemp.LastX=pSpawn->X;
        UpListTemp.LastY=pSpawn->Y;
        UpListTemp.LastZ=pSpawn->Z;
        UpListTemp.SpawnGameTime.Hour=pWorldData->Hour;
        UpListTemp.SpawnGameTime.Minute=pWorldData->Minute;
        UpListTemp.SpawnGameTime.Day=pWorldData->Day;
        UpListTemp.SpawnGameTime.Month=pWorldData->Month;
        UpListTemp.SpawnGameTime.Year=pWorldData->Year;

        SpawnUpList.push_back(UpListTemp);

        // Send alert to chat window

        INT Angle = (INT)((atan2f(GetCharInfo()->pSpawn->X - pSpawn->X, GetCharInfo()->pSpawn->Y - pSpawn->Y) * 180.0f / PI + 360.0f) / 22.5f + 0.5f) % 16;
        WriteChatf("\at%s\ax::\aySPAWN\am[%s] (%d) %s (%1.2f %s, %1.2fZ)", PLUGIN_NAME, UpListTemp.SpawnTime, UpListTemp.SpawnID, UpListTemp.Name,
            GetDistance(GetCharInfo()->pSpawn,pSpawn), szHeadingShort[Angle], pSpawn->Z-GetCharInfo()->pSpawn->Z);

        // Do a /highlight command for the map
        sprintf_s(szTemp, "/squelch /highlight \"%s\"", pSpawn->DisplayedName);
        DoCommand((PSPAWNINFO)pCharSpawn, szTemp);
        // play sound, this can play any mp3 file from the \Voice\Default dir (in the eq dir)
		if (Sound[0] != '\0') {
			if (EqSoundManager*peqs = pEqSoundManager) {
				float fOrgVol = peqs->fWaveVolumeLevel;
				peqs->fWaveVolumeLevel = fMasterVolume;
				peqs->PlayScriptMp3(Sound);
				peqs->fWaveVolumeLevel = fOrgVol;//better not mess with peoples mastervolume...
			}
		}
		CHAR szProfile[MAX_STRING] = { 0 };
		sprintf_s(szProfile,"%s.%s",EQADDR_SERVERNAME,((PCHARINFO)pCharData)->Name);
        GetPrivateProfileString(szProfile,"OnSpawnCommand","notfound",szTemp,MAX_STRING,INIFileName);
        if(!strcmp(szTemp,"notfound"))
			return;
        DoCommand((PSPAWNINFO)pCharSpawn, szTemp);

    }
}

void WalkSpawnList()
{
    CHAR szMsg[MAX_STRING] = {0};
    CHAR szCommand[MAX_STRING] = {0};
    CHAR szSound[MAX_STRING] = {0};

    // Reset the hightlighting, incase weve removed a spawn
    strcpy_s(szCommand, "/squelch /highlight reset");
    DoCommand((PSPAWNINFO)pCharSpawn, szCommand);

    // Make fresh list from current spawns.
    PSPAWNINFO pSpawn=(PSPAWNINFO)pSpawnList;

    WriteChatf("\at%s\ax::\aoSpawns currently up:",PLUGIN_NAME);

    if (SearchStrings.empty())
    {
        WriteChatf("\aoNone");
    }
    else
    {
        while(pSpawn)
        {
            if(IsWatchedSpawn(pSpawn,szSound))
            {
                // Check IDs to see if we're already watching it
                bool found = false;
                if (!SpawnUpList.empty())
                {
                    std::list<SPAWN_DATA>::iterator pSpawnUpList = SpawnUpList.begin();
                    while (pSpawnUpList!=SpawnUpList.end())
                    {
                        if (pSpawnUpList->SpawnID==pSpawn->SpawnID)
                        {
                            found = true;
                            break;
                        }
                        pSpawnUpList++;
                    }
                }
                if (!found)
					AddSpawnToUpList(pSpawn,szSound);
            }
            pSpawn = pSpawn->pNext;
        }
    }
}

void CheckForCorpse()
{
    PSPAWNINFO pSpawnTemp = nullptr;
    if (SpawnUpList.empty()) return;
    std::list<SPAWN_DATA>::iterator pSpawnUpList = SpawnUpList.begin();
    // For each node in list, check it to see if its a corpse

    while (pSpawnUpList != SpawnUpList.end())
    {
        PlayerClient* pSpawn = GetSpawnByID(pSpawnUpList->SpawnID);
        if(pSpawn && pSpawn->Type==SPAWN_CORPSE) {
            char LocalTime[MAX_STRING];
            GetLocalTimeHHMMSS(LocalTime);
            WriteChatf("\at%s\ax::\ar*[%s]\ax Spawn Killed: %s (%d)", PLUGIN_NAME, LocalTime, pSpawn->DisplayedName, pSpawn->SpawnID);
            strcpy_s(pSpawnUpList->DeSpawnTime,LocalTime);
            pSpawnUpList->DeSpawnGameTime.Minute=pWorldData->Minute;
            pSpawnUpList->DeSpawnGameTime.Day=pWorldData->Day;
            pSpawnUpList->DeSpawnGameTime.Month=pWorldData->Month;
            pSpawnUpList->DeSpawnGameTime.Year=pWorldData->Year;
            pSpawnUpList->LastX=pSpawn->X;
            pSpawnUpList->LastY=pSpawn->Y;
            pSpawnUpList->LastZ=pSpawn->Z;

            SpawnDownList.splice(SpawnDownList.end(),SpawnUpList,pSpawnUpList);
            return;
        }
        ++pSpawnUpList;
    }
}

// FIXME: MAX_STRING everywhere
void SpawnMasterCmd(PSPAWNINFO pChar, PCHAR szLine)
{
    CHAR Arg1[MAX_STRING] = {0};
    CHAR Arg2[MAX_STRING] = {0};
    CHAR szMsg[MAX_STRING] = {0};
    CHAR szTemp[MAX_STRING] = {0};

    GetArg(Arg1,szLine,1);
    GetArg(Arg2,szLine,2);
	CHAR szProfile[MAX_STRING] = { 0 };
	sprintf_s(szProfile,"%s.%s",EQADDR_SERVERNAME,((PCHARINFO)pCharData)->Name);
    if (!_stricmp(Arg1,"off"))
    {
        bSpawnMasterOn = false;
        WritePrivateProfileString(szProfile,"Enabled","off",INIFileName);
        WriteChatf("\at%s\ax::\arDisabled",PLUGIN_NAME);
    }
    else if (!_stricmp(Arg1,"on"))
    {
        bSpawnMasterOn = true;
        WritePrivateProfileString(szProfile,"Enabled","on",INIFileName);
        WriteChatf("\at%s\ax::\agEnabled",PLUGIN_NAME);

    }
    else if (!_stricmp(Arg1,"case"))
    {
        if(Arg2[0])
            bUseExactCase=!bUseExactCase;
        else if(!_strnicmp(Arg2, "on", 2))
            bUseExactCase=true;
        else if(!_strnicmp(Arg2, "off", 3))
            bUseExactCase=false;
        WritePrivateProfileString(szProfile,"ExactCase",bUseExactCase?"on":"off",INIFileName);
        WriteChatf("\at%s\ax::\amExactCast=%s",PLUGIN_NAME,bUseExactCase?"\agON":"\ayOFF");
    }
	else if (!_stricmp(Arg1,"vol"))
    {
		if (Arg2[0]!='\0') {
			WritePrivateProfileString(szProfile, "MasterVolume", Arg2, INIFileName);
			WriteChatf("\at%s\ax::\amMasterVolume=%s", PLUGIN_NAME, Arg2);
			fMasterVolume = GetFloatFromString(Arg2, fMasterVolume);
		} else {
			WriteChatf("Usage: /spawnmaster vol 0.0 to 1.0");
		}
    }
    else if (!_stricmp(Arg1,"add"))
    {
        if (strlen(Arg2))
        {
            AddNameToSearchList(Arg2);
        }
        else
        {
            if(pTarget)
            {
                PSPAWNINFO psTarget = (PSPAWNINFO)pTarget;
                sprintf_s(szTemp,"#%s",psTarget->DisplayedName);
                AddNameToSearchList(szTemp);
            }
            else
            {
                WriteChatf("\at%s\ax::\ayYou must have a target or specify a spawn!",PLUGIN_NAME);
                return;
            }
        }
        WriteINI();
        if (bSpawnMasterOn) WalkSpawnList();
    }
    else if (!_stricmp(Arg1,"delete"))
    {
        if (strlen(Arg2))
        {
            RemoveNameFromSearchList(Arg2);
        }
        else
        {
            if(pTarget)
            {
                PSPAWNINFO psTarget = (PSPAWNINFO)pTarget;
                sprintf_s(szTemp,"#%s",psTarget->DisplayedName);
                RemoveNameFromSearchList(szTemp);
            }
            else
            {
                WriteChatf("\at%s\ax::\ayYou must have a target or specify a spawn!",PLUGIN_NAME);
                return;
            }
        }
        WriteINI();
    }
    else if (!_stricmp(Arg1,"list"))
    {
        if (SearchStrings.empty())
        {
            WriteChatf("\at%s\ax::\ayNo spawns in watch list",PLUGIN_NAME);
            return;
        }
        WriteChatf("\at%s\ax::\aoCurrently watching for spawns:",PLUGIN_NAME);
        std::list<_SEARCH_STRINGS>::iterator pSearchStrings = SearchStrings.begin();
        while (pSearchStrings!=SearchStrings.end())
        {
            WriteChatf("\ao-%s",pSearchStrings->SearchString);
            pSearchStrings++;
        }
        return;
    }
    else if (!_strnicmp(Arg1,"up",2))
    {
        if (SpawnUpList.empty())
        {
            WriteChatf("\at%s\ax::\ayNothing on list has spawned yet.",PLUGIN_NAME);
            return;
        }
        std::list<SPAWN_DATA>::iterator pSpawnUpList = SpawnUpList.begin();
        while (pSpawnUpList!=SpawnUpList.end())
        {
            if(PSPAWNINFO pSpawn = (PSPAWNINFO)GetSpawnByID(pSpawnUpList->SpawnID)) {
                sprintf_s(szMsg,"\ay*\ax[%s] (%d) %s ", pSpawnUpList->SpawnTime, pSpawnUpList->SpawnID, pSpawnUpList->Name);
                INT Angle = (INT)((atan2f(GetCharInfo()->pSpawn->X - pSpawn->X, GetCharInfo()->pSpawn->Y - pSpawn->Y) * 180.0f / PI + 360.0f) / 22.5f + 0.5f) % 16;
                sprintf_s(szTemp,"(%1.2f %s, %1.2fZ)", GetDistance(GetCharInfo()->pSpawn,pSpawn), szHeadingShort[Angle], pSpawn->Z-GetCharInfo()->pSpawn->Z);
                strcat_s(szMsg,szTemp);
                WriteChatColor(szMsg,USERCOLOR_WHO);
            }
            pSpawnUpList++;
        }
    }
    else if (!_strnicmp(Arg1,"down",4))
    {
        if (SpawnDownList.empty())
        {
            WriteChatf("\at%s\ax::\ayNothing on list has died or despawned yet.",PLUGIN_NAME);
            return;
        }
        std::list<SPAWN_DATA>::iterator pSpawnDownList = SpawnDownList.begin();
        while (pSpawnDownList!=SpawnDownList.end())
        {
            sprintf_s(szMsg,"\ar*\ax[%s] (%d) %s (Loc: %1.2fX/%1.2fY/%1.2fZ)", pSpawnDownList->DeSpawnTime, pSpawnDownList->SpawnID, pSpawnDownList->Name, pSpawnDownList->LastX, pSpawnDownList->LastY, pSpawnDownList->LastZ);
            WriteChatColor(szMsg,USERCOLOR_WHO);

            pSpawnDownList++;
        }
    }
    else if (!_strnicmp(Arg1,"load",4))
    {
        if (gGameState==GAMESTATE_INGAME) {
            SearchStrings.clear();
            ReadSpawnListFromINI();
            WriteChatf("\at%s\ax::\aoSpawns loaded from INI file.",PLUGIN_NAME);
        }
    }
    else
    {
        WriteChatf("/spawnmaster commands:");
        WriteChatf("on|off - Toggles SpawnMaster plugin on or off");
        WriteChatf("add \"spawn name|sound\" - Add spawn name and sound to watch list (or target if no name given)");
        WriteChatf("example: /spawnmaster add \"a moss snake|moss.mp3\" - Note that moss.mp3 must exist in the Voice\\Default directory (its in your everquest dir)");
        WriteChatf("delete \"spawn name\" - Delete spawn name from watch list (or target if no name given)");
        WriteChatf("list - Display watch list for zone");
        WriteChatf("case [off|on] - Control whether to use exact case matching in any compare. Omit on or off to toggle.");
        WriteChatf("uplist - Display any mobs on watch list that are currently up");
        WriteChatf("downlist - Display any watched mobs that have died or despawned");
        WriteChatf("load - Load spawns from INI");
        WriteChatf("vol <0.0-1.0> - Set MasterVolume for sounds, example /spawnmaster vol 0.50");
    }
}

class MQ2SpawnMasterType *pSpawnMasterType = nullptr;

class MQ2SpawnMasterType : public MQ2Type
{
public:
    enum SpawnMasterMembers
    {
        Search,
        UpList,
        DownList,
        Version,
        LastMatch,
        HasTarget
    };

    MQ2SpawnMasterType():MQ2Type("SpawnMaster")
    {
        TypeMember(Search);
        TypeMember(UpList);
        TypeMember(DownList);
        TypeMember(Version);
        TypeMember(LastMatch);
        TypeMember(HasTarget);
    }

    virtual bool GetMember(MQVarPtr VarPtr, const char* Member, char* Index, MQTypeVar& Dest) override
    {
        MQTypeMember* pMember=MQ2SpawnMasterType::FindMember(Member);
        if (!pMember)
            return false;
        switch((SpawnMasterMembers)pMember->ID)
        {
        case Search:
            Dest.Int=SearchStrings.size();
            Dest.Type=mq::datatypes::pIntType;
            return true;
        case UpList:
            Dest.Int=SpawnUpList.size();
            Dest.Type=mq::datatypes::pIntType;
            return true;
        case DownList:
            Dest.Int=SpawnDownList.size();
            Dest.Type=mq::datatypes::pIntType;
            return true;
        case Version:
            Dest.Double=MQ2Version;
            Dest.Type=mq::datatypes::pDoubleType;
            return true;
        case LastMatch:
            if (SpawnUpList.empty()) return false;
			strcpy_s(DataTypeTemp, SpawnUpList.back().Name);
            Dest.Ptr=&DataTypeTemp[0];
            Dest.Type=mq::datatypes::pStringType;
            return true;
        case HasTarget:
            if (SpawnUpList.empty()) return false;
            if (pTarget == nullptr) return false;
            Dest.Type=mq::datatypes::pBoolType;
            // FIXME:  Don't really want to pass a sound here
            Dest.Int=IsWatchedSpawn(pTarget, DataTypeTemp);
            return true;
        }
        return false;
    }

    bool ToString(MQVarPtr VarPtr, char* Destination) override
    {
        if (bSpawnMasterOn)
            strcpy_s(Destination,MAX_STRING,"TRUE");
        else
            strcpy_s(Destination,MAX_STRING,"FALSE");
        return true;
    }
};

bool dataSpawnMaster(const char* szName, MQTypeVar& Dest)
{
    Dest.DWord=1;
    Dest.Type=pSpawnMasterType;
    return true;
}

PLUGIN_API void InitializePlugin()
{
	char szTemp[MAX_STRING] = { 0 };
	AddCommand("/spawnmaster", SpawnMasterCmd);
	AddMQ2Data("SpawnMaster", dataSpawnMaster);
	pSpawnMasterType = new MQ2SpawnMasterType;

	if (gGameState == GAMESTATE_INGAME)
	{
		SpawnUpList.clear();
		SpawnDownList.clear();
		SearchStrings.clear();
		ReadSpawnListFromINI();
		CHAR szProfile[MAX_STRING] = { 0 };
		sprintf_s(szProfile, "%s.%s", EQADDR_SERVERNAME, ((PCHARINFO)pCharData)->Name);
		GetPrivateProfileString(szProfile, "MasterVolume", "1.0", szTemp, MAX_STRING, INIFileName);
		fMasterVolume = GetFloatFromString(szTemp, fMasterVolume);
		// Check for the INI Entry OnSpawnCommand to exist in [Server.CharName] and write a default value if not.
		GetPrivateProfileString(szProfile, "OnSpawnCommand", "notfound", szTemp, MAX_STRING, INIFileName);
		if (!strcmp(szTemp, "notfound"))
		WritePrivateProfileString(szProfile, "OnSpawnCommand", "", INIFileName);

		// Check for the INI Entry Enabled to exist in [Server.CharName] and write a default value if not.
		GetPrivateProfileString(szProfile, "Enabled", "notfound", szTemp, MAX_STRING, INIFileName);
		if (!strcmp(szTemp, "notfound"))
		{
			WritePrivateProfileString(szProfile, "Enabled", "on", INIFileName);
			GetPrivateProfileString(szProfile, "Enabled", "on", szTemp, MAX_STRING, INIFileName);
		}
		if (!_stricmp(szTemp, "on"))
			bSpawnMasterOn = true;
		else
			bSpawnMasterOn = false;

		GetPrivateProfileString(szProfile, "ExactCase", "off", szTemp, MAX_STRING, INIFileName);
		if (!_stricmp(szTemp, "on"))
			bUseExactCase = true;
		else
			bUseExactCase = false;
	}
}


PLUGIN_API void ShutdownPlugin()
{
    RemoveCommand("/spawnmaster");
    RemoveMQ2Data("SpawnMaster");
    delete pSpawnMasterType;
}

PLUGIN_API void OnAddSpawn(PSPAWNINFO pSpawn)
{
    if (!bSpawnMasterOn || gGameState != GAMESTATE_INGAME || !pSpawn->SpawnID)
		return;
	CHAR szSound[MAX_STRING] = { 0 };
	if (IsWatchedSpawn(pSpawn, szSound)) {
		AddSpawnToUpList(pSpawn,szSound);
	}
}

PLUGIN_API void OnRemoveSpawn(PSPAWNINFO pSpawn)
{
    if (!bSpawnMasterOn || gGameState != GAMESTATE_INGAME || !pSpawn->SpawnID)
		return;
    CHAR szMsg[MAX_STRING] = {0};
    CHAR szSound[MAX_STRING] = {0};

    if (IsWatchedSpawn(pSpawn,szSound))
    {
        std::list<SPAWN_DATA>::iterator pSpawnUpList = SpawnUpList.begin();
        while (pSpawnUpList!=SpawnUpList.end())
        {
            if (pSpawnUpList->SpawnID==pSpawn->SpawnID)
            {
                GetLocalTimeHHMMSS(pSpawnUpList->DeSpawnTime);
                pSpawnUpList->DeSpawnGameTime.Minute=pWorldData->Minute;
                pSpawnUpList->DeSpawnGameTime.Day=pWorldData->Day;
                pSpawnUpList->DeSpawnGameTime.Month=pWorldData->Month;
                pSpawnUpList->DeSpawnGameTime.Year=pWorldData->Year;
                pSpawnUpList->LastX=pSpawn->X;
                pSpawnUpList->LastY=pSpawn->Y;
                pSpawnUpList->LastZ=pSpawn->Z;
                sprintf_s(szMsg,"\arDESPAWN\ax[%s] (%d) %s ", pSpawnUpList->DeSpawnTime, pSpawnUpList->SpawnID, pSpawnUpList->Name);

                WriteChatColor(szMsg,USERCOLOR_WHO);
                SpawnDownList.splice(SpawnDownList.end(),SpawnUpList,pSpawnUpList);
                return;
            }
            pSpawnUpList++;
        }

    }
}

PLUGIN_API VOID OnPulse()
{
    static int pulse_counter = 0;
    if (++pulse_counter<=SKIP_PULSES || gGameState != GAMESTATE_INGAME || !bSpawnMasterOn)
		return;
    pulse_counter=0;

    // Check if any watched spawns have become corpses
    CheckForCorpse();
	if (!bMasterVolumeSet) {
		if (gGameState == GAMESTATE_INGAME) {
			CHAR szTemp[MAX_STRING] = { 0 };
			CHAR szProfile[MAX_STRING] = { 0 };
			sprintf_s(szProfile,"%s.%s",EQADDR_SERVERNAME,((PCHARINFO)pCharData)->Name);
			GetPrivateProfileString(szProfile,"MasterVolume","1.0",szTemp,MAX_STRING,INIFileName);
			fMasterVolume = GetFloatFromString(szTemp, fMasterVolume);
			bMasterVolumeSet = true;
		}
	}
}

PLUGIN_API VOID OnBeginZone()
{
    bSpawnMasterOn = false;
}

PLUGIN_API VOID OnEndZone()
{
    char szTemp[MAX_STRING];
    SpawnUpList.clear();
    SpawnDownList.clear();
    SearchStrings.clear();
    ReadSpawnListFromINI();
	CHAR szProfile[MAX_STRING] = { 0 };
	sprintf_s(szProfile,"%s.%s",EQADDR_SERVERNAME,((PCHARINFO)pCharData)->Name);
    GetPrivateProfileString(szProfile,"Enabled","on",szTemp,MAX_STRING,INIFileName);
    if (!_stricmp(szTemp,"on"))
    {
        bSpawnMasterOn = true;
        WalkSpawnList();
    }
    else bSpawnMasterOn = false;
    GetPrivateProfileString(szProfile,"ExactCase","off",szTemp,MAX_STRING,INIFileName);
    if (!_stricmp(szTemp,"on"))
        bUseExactCase = true;
    else
        bUseExactCase = false;
}