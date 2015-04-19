#ifndef _DUNGEONGENERATORCLASS_H_
#define _DUNGEONGENERATORCLASS_H_


//libraries for rand(time(NULL))
#include <stdlib.h>
#include <time.h>
#include <deque>

//classes for creating collectible objects
#include "modelclass.h"
#include "d3dclass.h"

//global constants to create dungeon
const int ROOM_MINWIDTH_DIV = 4;
const int ROOM_MINHEIGHT_DIV = 4;
const int BORDER_MINDISTANCE = 20;
const int DUNGEON_ROOMS = 9; //for now, the algorithm only can produce 9 rooms

const int DUNGEON_MIN_PATHWIDTH = 10;
const int DUNGEON_WALLHEIGHT = 100;
const int COLLECTIBLES_PER_ROOM = 1;
const int COLLECTIBLES_MIN_DISTANCE_TO_WALL = 4;


//the dungen in a nutshell
class DungeonGeneratorClass
{
public:
	
	//structure for the collectible objects
	struct CollectibleData
	{
		ModelClass* model;
		float posX;
		float posY;
		float posZ;

		float collisionRadius;
	};

	//structure for the dungeon itself
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
	bool GenerateNewDungeon(int, D3DClass*);
	void GetSpawningCoord(float&, float&, float&);
	
	bool CollectibleAtPosition(float, float, float);
	int GetTotalPointCount();

private:
	//structure for exits of rooms
	struct ExitIndexes
	{
		int north, south, east, west;
	};

	//structure for rooms (in a nutshell)
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


	//---- private functions

	//functions to create roos and collectibles in the dungeon area
	bool GenerateRoom(int, int, int);
	void GetRandomRoom(int, int, int, int, int&, int&, int&);
	bool RoomIsWithinBorders(int*, int*, int);

	void ReleaseRoomsAndCollectibles();
	void SetRoomExit(RoomData*, int);

	//for pathfinding - node structure and open and close list
	struct PathFindingNode
	{
		int costSoFar;
		int estimatedCosts;
		int index, parentIndex;
	};

	//functions to connect rooms via pathfinding (A Star)
	void ConnectTwoRooms(RoomData*, RoomData*);
	bool CreatePathBetweenPoints(int, int);
	void CalculateHeuristicValue(int, int, int&);

	void ConnectAllAreas(int&, int);
	void ChangeAreaIndexes(int, int, int);

	int GetConnectedIndex(int, int);
	bool IsInList(int, std::deque<PathFindingNode*>*);
	void GetNodeByIndexFromList(int, PathFindingNode*&, std::deque<PathFindingNode*>*);
	void RemoveNodeFromList(int, std::deque<PathFindingNode*>*);

	//heap sort function for the lists of nodes (pathfinding)
	void SortList(std::deque<PathFindingNode*>*);
	void HeapSink(std::deque<PathFindingNode*>*, int, int);

	//dig / carve at a specific point on the map
	void CarvePathPoint(int);

	//spawn collectible object in a specific room
	bool SpawnCollectibles(RoomData*, D3DClass*);


	//---- private variables
	
	//the complete dungeon data
	DungeonData* m_dungeonData;

	//total amount of rooms and all rooms as array
	int m_roomAmount;
	RoomData* m_roomData[DUNGEON_ROOMS];

	//total amunt of points available in dungeon (= total amount of spawned ollectile objects * 1)
	int m_totalPointAmount;
};

#endif