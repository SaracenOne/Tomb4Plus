#pragma once
#include "../global/types.h"

int LoadLevel(void* name);
int32_t S_LoadLevelFile(int32_t num);
void FreeLevel();
FILE* T4PFileOpen(const char* name);
void FileClose(FILE* file);
int32_t FileSize(FILE* file);
size_t T4PLoadFileAtRelativePath(const char* name, char** dest);
bool LoadTextures(int32_t RTPages, int32_t OTPages, int32_t BTPages);
bool LoadRooms();
bool LoadObjects();
bool LoadSprites();
bool LoadCameras();
bool LoadSoundEffects();
bool LoadBoxes();
bool LoadAnimatedTextures();
bool LoadTextureInfos();
bool LoadItems();
bool LoadCinematic();
bool LoadAIInfo();
bool LoadSamples();
void S_GetUVRotateTextures();
void AdjustUV(int32_t num);
bool Decompress(char* pDest, char* pCompressed, int32_t compressedSize, int32_t size);

extern TEXTURESTRUCT* textinfo;
extern SPRITESTRUCT* spriteinfo;
extern THREAD LevelLoadingThread;

extern TEXTURESTRUCT* AnimatingWaterfalls[3];
extern int32_t AnimatingWaterfallsV[3];

extern int32_t num_meshes;
extern int32_t num_anims;

extern CHANGE_STRUCT* changes;
extern RANGE_STRUCT* ranges;
extern AIOBJECT* AIObjects;
extern int16_t* aranges;
extern int16_t* frames;
extern int16_t* commands;
extern int16_t* floor_data;
extern int16_t* mesh_base;
extern int32_t nAnimUVRanges;
extern int32_t number_cameras;
extern int16_t nAIObjects;

// T4Plus: Helper table for mapping mesh_ptrs between 64-bit and 32-bit offsets
extern size_t mesh_mapping_table_count;
extern MESH_MAP_TABLE_ENTRY* mesh_mapping_table;