#ifndef _DUNGEONGENERATORCLASS_H_
#define _DUNGEONGENERATORCLASS_H_

//libraries for rand(time(NULL))
#include <stdlib.h>
#include <time.h>
#include <deque>

const int ROOM_MINWIDTH_DIV = 5;
const int ROOM_MINHEIGHT_DIV = 5;
const int BORDER_MINDISTANCE = 20;
const int DUNGEON_ROOMS = 4;


class DungeonGeneratorClass
{
public:
	struct DungeonData
	{
		int* dungeonArray;
		int dungeonWidth;
		int dungeonHeight;
	};


	DungeonGeneratorClass();
	DungeonGeneratorClass(const DungeonGeneratorClass& other);
	~DungeonGeneratorClass();

	bool Initialize(int, int);
	void Shutdown();

	DungeonData* GetDungeonData();
	unsigned int GetDungeonSeed();

	bool GenerateNewDungeon(int);
	

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
	};


	bool GenerateRoom(int, int, int);
	void GetRandomRoom(int, int, int, int, int&, int&, int&);
	bool RoomIsWithinBorders(int*, int*, int);

	void ReleaseRooms();
	void SetRoomExit(RoomData*, int);

	void ConnectTwoRooms(RoomData*, RoomData*);
	bool CreatePathBetweenPoints(int, int, int);
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
	void GetNodeByIndexFromList(int, PathFindingNode*, std::deque<PathFindingNode*>*);
	void RemoveNodeFromList(int, std::deque<PathFindingNode*>*);

	void GetNodeByListIndex(int, PathFindingNode*, std::deque<PathFindingNode*>*);

	void SortList(std::deque<PathFindingNode*>*);
	void HeapSink(std::deque<PathFindingNode*>*, int, int);

	//void TempDrawArea(int, int, int, int);

	unsigned int m_lastUsedSeed;
	DungeonData* m_dungeonData;

	int m_roomAmount;
	RoomData* m_roomData[DUNGEON_ROOMS];

	std::deque<PathFindingNode*> *openList;
	std::deque<PathFindingNode*> *closedList;
};

#endif