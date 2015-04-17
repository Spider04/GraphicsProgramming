#ifndef _DUNGEONGENERATORCLASS_H_
#define _DUNGEONGENERATORCLASS_H_

//libraries for rand(time(NULL))
#include <stdlib.h>
#include <time.h>
#include <deque>
#include "modelclass.h"
#include "d3dclass.h"

const int ROOM_MINWIDTH_DIV = 4;
const int ROOM_MINHEIGHT_DIV = 4;
const int BORDER_MINDISTANCE = 20;
const int DUNGEON_ROOMS = 9; //for now, the algorithm only can produce 9 rooms

const int DUNGEON_MIN_PATHWIDTH = 10;
const int DUNGEON_WALLHEIGHT = 150;
const int COLLECTIBLES_PER_ROOM = 1;

class DungeonGeneratorClass
{
public:
	
	struct CollectibleData
	{
		ModelClass* model;
		float posX;
		float posY;
		float posZ;

		float collisionRadius;
	};

	struct DungeonData
	{
		int* dungeonArray;
		int dungeonWidth;
		int dungeonHeight;

		std::deque<CollectibleData*> *collectibles;
	};


	DungeonGeneratorClass();
	DungeonGeneratorClass(const DungeonGeneratorClass& other);
	~DungeonGeneratorClass();

	bool Initialize(int, int);
	void Shutdown();

	DungeonData* GetDungeonData();
	unsigned int GetDungeonSeed();

	bool GenerateNewDungeon(int, D3DClass*);
	void GetSpawningCoord(float&, float&, float&);
	
	bool CollectibleAtPosition(float, float, float);
	int GetTotalPointCount();

private:
	
	struct ExitIndexes
	{
		int north, south, east, west;
	};

	struct RoomData
	{
		int topLeftIndex;
		int width;
		int length;

		ExitIndexes* exits;
		int areaTopLeftIndex;

		int areaBlock;
		int* allowedConnections;
		int connectionCount;
	};


	bool GenerateRoom(int, int, int);
	void GetRandomRoom(int, int, int, int, int&, int&, int&);
	bool RoomIsWithinBorders(int*, int*, int);

	void ReleaseRoomsAndCollectibles();
	void SetRoomExit(RoomData*, int);

	void ConnectTwoRooms(RoomData*, RoomData*);
	bool CreatePathBetweenPoints(int, int);
	void CalculateHeuristicValue(int, int, int&);

	//for pathfinding - node structure and open and close list
	struct PathFindingNode
	{
		int costSoFar;
		int estimatedCosts;
		int index, parentIndex;
	};

	int GetConnectedIndex(int, int);
	bool IsInList(int, std::deque<PathFindingNode*>*);
	void GetNodeByIndexFromList(int, PathFindingNode*&, std::deque<PathFindingNode*>*);
	void RemoveNodeFromList(int, std::deque<PathFindingNode*>*);

	void SortList(std::deque<PathFindingNode*>*);
	void HeapSink(std::deque<PathFindingNode*>*, int, int);


	void CarvePathPoint(int);
	bool SpawnCollectibles(RoomData*, D3DClass*);

	void ConnectAllAreas(int&, int);
	void ChangeAreaIndexes(int, int, int);

	//void TempDrawArea(int, int, int, int);

	unsigned int m_lastUsedSeed;
	DungeonData* m_dungeonData;

	int m_roomAmount;
	int m_totalPointAmount;
	RoomData* m_roomData[DUNGEON_ROOMS];

	std::deque<PathFindingNode*> *openList;
	std::deque<PathFindingNode*> *closedList;
};

#endif