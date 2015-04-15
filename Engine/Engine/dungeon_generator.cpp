#include "dungeon_generator.h"

DungeonGeneratorClass::DungeonGeneratorClass()
	: m_dungeonData(0)
	, m_lastUsedSeed(0)
	, m_roomAmount(0)
{}
DungeonGeneratorClass::DungeonGeneratorClass(const DungeonGeneratorClass& other)
{}
DungeonGeneratorClass::~DungeonGeneratorClass()
{}

//initializes the dungeon and returns the 
bool DungeonGeneratorClass::Initialize(int dungeonWidth, int dungeonHeight)
{
	m_dungeonData = new DungeonData;
	m_dungeonData->dungeonWidth = dungeonWidth;
	m_dungeonData->dungeonHeight = dungeonHeight;

	int lastIndex = 0;
	lastIndex = (dungeonHeight * dungeonWidth) - 1;

	m_dungeonData->dungeonArray = new int[(lastIndex + 1)];
	if(!m_dungeonData->dungeonArray)
		return false;

	for(int i = 0; i < lastIndex; i++)
		m_dungeonData->dungeonArray[i] = 100;

	//init room data
	for(int i = 0; i < DUNGEON_ROOMS; i++)
	{
		m_roomData[i] = 0;
		m_roomData[i] = new RoomData;
		m_roomData[i]->exits = new ExitIndexes;
		m_roomData[i]->exits->north = 0;
		m_roomData[i]->exits->south = 0;
		m_roomData[i]->exits->west = 0;
		m_roomData[i]->exits->east = 0;
	}

	//init pathfinding lists
	openList = new std::deque<PathFindingNode*>;
	closedList = new std::deque<PathFindingNode*>;

	//generate initial dungeon
	bool result = false;
	result = GenerateNewDungeon(DUNGEON_ROOMS);
	
	return result;
}

void DungeonGeneratorClass::Shutdown()
{
	ReleaseRooms();

	if(m_dungeonData)
	{
		delete m_dungeonData;
		m_dungeonData = 0;
	}
}

unsigned int DungeonGeneratorClass::GetDungeonSeed()
{
	return m_lastUsedSeed;
}

bool DungeonGeneratorClass::GenerateNewDungeon(int amountOfRooms)
{
	//create new room array (for pathfinding and spawning object)
	ReleaseRooms();

	//get seed and use it to init the random function
	m_lastUsedSeed = (unsigned int)time(NULL);
	srand(m_lastUsedSeed);

	int startIndex = 0;

	int dungeonWidth = m_dungeonData->dungeonWidth;
	int dungeonHeight = m_dungeonData->dungeonHeight;
	bool result = false;

	//create all rooms (for now, for 4 set dungeons)
	for(int i = 0; i < 4; i++)
	{
		if(i < 1)
			startIndex = 0;
		else if(i < 2)
			startIndex = dungeonWidth / 2;
		else if(i < 3)
			startIndex = dungeonWidth * (dungeonHeight / 2);
		else
			startIndex = (dungeonWidth / 2) + (dungeonWidth * (dungeonHeight / 2));
			
		result = GenerateRoom(dungeonWidth / 2, dungeonHeight / 2, startIndex);
		if(!result)
			return false;
	}

	//connect all rooms together
	for(int i = 0; i < DUNGEON_ROOMS; i++)
	{
		if(i < DUNGEON_ROOMS - 1)
			ConnectTwoRooms(m_roomData[i], m_roomData[i + 1]);
		else
			ConnectTwoRooms(m_roomData[i], m_roomData[0]);
	}

	//temp: set borders to 0
	int tmpInt = 0;
	for(int i = 0; i < dungeonHeight; i++)
	{
		for(int j = 0; j < dungeonWidth; j++)
		{
			tmpInt = j + (i * dungeonWidth);
			if(tmpInt % dungeonWidth == dungeonHeight / 2)
				m_dungeonData->dungeonArray[tmpInt] = 0;

			else if(tmpInt >= (dungeonHeight / 2 * dungeonWidth) && tmpInt < ((dungeonHeight / 2 * dungeonWidth) + dungeonWidth))
				m_dungeonData->dungeonArray[tmpInt] = 0;
		}
	}

	return true;
}

bool DungeonGeneratorClass::GenerateRoom(int areaWidth, int areaHeight, int startIndex)
{
	//set input parameters for the room
	int input[3] = {0, 0, 0};
	input[0] = startIndex;
	input[1] = areaWidth;
	input[2] = areaHeight;

	//set random room point until a valid one appears
	int roomData[3] = {0, 0, 0};
	
	int roomTopLeftIndex = 0;
	int roomWidth = 0;
	int roomHeight = 0;

	bool validRoomFound = false;
	int counter = 0;

	while(!validRoomFound && counter < 999999)
	{
		GetRandomRoom(startIndex, areaWidth, areaHeight, BORDER_MINDISTANCE, roomTopLeftIndex, roomWidth, roomHeight);
		roomData[0] = roomTopLeftIndex;
		roomData[1] = roomWidth;
		roomData[2] = roomHeight;

		if(RoomIsWithinBorders(roomData, input, BORDER_MINDISTANCE))
			validRoomFound = true;

		counter++;
	}

	//check if room is valid
	if(counter > 999998)
		return false;

	//carve room into array
	for(int i = 0; i < roomHeight; i++)
	{
		for(int j = 0; j < roomWidth; j++)
		{
			if((roomTopLeftIndex + j) + (i * m_dungeonData->dungeonWidth) > (m_dungeonData->dungeonWidth * m_dungeonData->dungeonHeight) - 1)
				return false;

			m_dungeonData->dungeonArray[(roomTopLeftIndex + j) + (i * m_dungeonData->dungeonWidth)] = 0;
		}
	}

	//save room data
	m_roomData[m_roomAmount] = new RoomData;
	if(!m_roomData[m_roomAmount])
		return false;

	//set the data
	m_roomData[m_roomAmount]->topLeftIndex = roomTopLeftIndex;
	m_roomData[m_roomAmount]->width = roomWidth;
	m_roomData[m_roomAmount]->length = roomHeight;
	m_roomData[m_roomAmount]->areaTopLeftIndex = startIndex;

	m_roomData[m_roomAmount]->exits = new ExitIndexes;
	m_roomData[m_roomAmount]->exits->north = 0;
	m_roomData[m_roomAmount]->exits->south = 0;
	m_roomData[m_roomAmount]->exits->west = 0;
	m_roomData[m_roomAmount]->exits->east = 0;

	//calculate the exits
	for(int i = 0; i < 4; i++)
	{
		SetRoomExit(m_roomData[m_roomAmount], i);
	}

	//temp show exit positions
	m_dungeonData->dungeonArray[m_roomData[m_roomAmount]->exits->north] = 200;
	m_dungeonData->dungeonArray[m_roomData[m_roomAmount]->exits->south] = 200;
	m_dungeonData->dungeonArray[m_roomData[m_roomAmount]->exits->west] = 200;
	m_dungeonData->dungeonArray[m_roomData[m_roomAmount]->exits->east] = 200;

	m_roomAmount++;
	
	return true;
}

void DungeonGeneratorClass::ReleaseRooms()
{
	for(int i = 0; i < DUNGEON_ROOMS; i++)
	{
		if(m_roomData[i])
		{
			//delete m_roomData[i];
			delete m_roomData[i]->exits;
			m_roomData[i]->exits = 0;
			m_roomData[i] = 0;
		}
	}
}


void DungeonGeneratorClass::GetRandomRoom(int areaStartIndex, int areaWidth, int areaHeight, int minBorderSpace,
	int& roomStartIndex, int& roomWidth, int& roomHeight)
{
	//calc new width with min width
	int minRoomWidth = areaWidth / ROOM_MINWIDTH_DIV;
	roomWidth = areaWidth - ((minBorderSpace * 2) + minRoomWidth);
	roomWidth = minRoomWidth + (rand() % roomWidth);

	//calc new height with min height
	int minRoomHeight = areaHeight / ROOM_MINHEIGHT_DIV;
	roomHeight = areaHeight - ((minBorderSpace * 2) + minRoomHeight);
	roomHeight = minRoomHeight + (rand() % roomHeight);

	//calculate new top left coordinate (within the area space)
	int xCoord = areaWidth - ((minBorderSpace * 2) + roomWidth);
	xCoord = minBorderSpace + (rand() % xCoord);

	int zCoord = areaHeight - ((minBorderSpace * 2) + roomHeight);
	zCoord = minBorderSpace + (rand() % zCoord);

	//set start index to top left coordinate in world space
	roomStartIndex = areaStartIndex + xCoord + (zCoord * m_dungeonData->dungeonWidth);
}

bool DungeonGeneratorClass::RoomIsWithinBorders(int* roomData, int* areaData, int minBorderDistance)
{
	//get the rows and colums of the area borders
	int areaBorderRows[2];
	areaBorderRows[0] = areaData[0] / m_dungeonData->dungeonWidth; //north
	areaBorderRows[1] = ((areaData[0] + areaData[1]) + (areaData[2] * m_dungeonData->dungeonWidth))  / m_dungeonData->dungeonWidth; //south

	int areaBorderColumns[2];
	areaBorderColumns[0] = areaData[0] % m_dungeonData->dungeonWidth; //west
	areaBorderColumns[1] = (areaData[0] % m_dungeonData->dungeonWidth) + areaData[1]; //east

	//get rows and columns of the room borders (calculations are similar to previous ones)
	int roomBorderRows[2];
	roomBorderRows[0] = roomData[0] / m_dungeonData->dungeonWidth; //north
	roomBorderRows[1] = ((roomData[0] + roomData[1]) + (roomData[2] * m_dungeonData->dungeonWidth))  / m_dungeonData->dungeonWidth; //south

	int roomBorderColumns[2];
	roomBorderColumns[0] = roomData[0] % m_dungeonData->dungeonWidth; //west
	roomBorderColumns[1] = roomBorderColumns[0] + roomData[1]; //east


	//check north border
	if(roomBorderRows[0] - minBorderDistance < areaBorderRows[0])
		return false;
	//check south border
	if(roomBorderRows[1] + minBorderDistance > areaBorderRows[1])
		return false;

	//check west border
	if(roomBorderColumns[0] - minBorderDistance < areaBorderColumns[0])
		return false;
	//check east border
	if(roomBorderColumns[1] + minBorderDistance > areaBorderColumns[1])
		return false;
		
	return true;
}

void DungeonGeneratorClass::SetRoomExit(RoomData* room, int exitIndex)
{
	//set north exit
	if(exitIndex == 0)
		room->exits->north = room->topLeftIndex + (room->width / 2);

	//set south exit
	else if(exitIndex == 2)
		room->exits->south = room->topLeftIndex + (room->width / 2) + (room->length * m_dungeonData->dungeonWidth);

	//set west exit
	else if(exitIndex == 3)
		room->exits->west = room->topLeftIndex + ((room->length / 2) * m_dungeonData->dungeonWidth);

	//set east exit
	else if(exitIndex == 1)
		room->exits->east = room->topLeftIndex + ((room->length / 2) * m_dungeonData->dungeonWidth) + (room->width - 1);
}

DungeonGeneratorClass::DungeonData* DungeonGeneratorClass::GetDungeonData()
{
	return m_dungeonData;
}


void DungeonGeneratorClass::ConnectTwoRooms(RoomData* room1, RoomData* room2)
{
	int roomRowNumber[2];
	roomRowNumber[0] = room1->areaTopLeftIndex / m_dungeonData->dungeonWidth;
	roomRowNumber[1] = room2->areaTopLeftIndex / m_dungeonData->dungeonWidth;

	int roomColNumber[2];
	roomColNumber[0] = room1->areaTopLeftIndex % m_dungeonData->dungeonWidth;
	roomColNumber[1] = room2->areaTopLeftIndex % m_dungeonData->dungeonWidth;

	//case: rooms are horizontally connected
	if(roomRowNumber[0] == roomRowNumber[1] && roomColNumber[0] != roomColNumber[1])
	{
		if(room1->topLeftIndex < room2->topLeftIndex)
			CreatePathBetweenPoints(room1->exits->east, room2->exits->west, 2);
		else
			CreatePathBetweenPoints(room1->exits->west, room2->exits->east, 6);
	}

	//case: rooms are vertically connected
	else if(roomRowNumber[0] != roomRowNumber[1] && roomColNumber[0] == roomColNumber[1])
	{
		if(room1->topLeftIndex < room2->topLeftIndex)
			CreatePathBetweenPoints(room1->exits->south, room2->exits->north, 4);
		else
			CreatePathBetweenPoints(room1->exits->north, room2->exits->south, 0);
	}

	//last case, they are slightly diagonal - no solution so far
	
	return;
}

//Dijkstra algorithm to calculate path to entry
bool DungeonGeneratorClass::CreatePathBetweenPoints(int startIndex, int endIndex, int preferredDirection)
{
	PathFindingNode* entryNode = new PathFindingNode;
	entryNode->costSoFar = 0;
	CalculateHeuristicValue(startIndex, endIndex, entryNode->estimatedCosts);
	entryNode->index = startIndex;
	entryNode->parentIndex = -1;

	if(!openList->empty()){
		openList->clear();
		delete openList;
		openList = 0;
	}

	if(!closedList->empty())
	{
		closedList->clear();
		delete closedList;
		closedList = 0;
	}

	openList = new std::deque<PathFindingNode*>;
	openList->push_back(entryNode);
	closedList = new std::deque<PathFindingNode*>;

	PathFindingNode* currentNode = new PathFindingNode;
	int pathFindCounter = 0;
	int newCostSoFar = 0;
	int heuristicEstimate = 1000;

	while(!openList->empty() && pathFindCounter < 999999999)
	{
		currentNode = openList->front();
		if(currentNode->index == endIndex)
			break;

		int connectIndex = -1;
		//get new index, depending on direction
		for(int i = 0; i < 8; i++)
		{
			PathFindingNode* endNode = new PathFindingNode;
			connectIndex = GetConnectedIndex(currentNode->index, i);
			if(connectIndex != -1)
			{
				newCostSoFar = currentNode->costSoFar + 1;

				if(IsInList(connectIndex, closedList))
				{
					GetNodeByIndexFromList(connectIndex, endNode, closedList);

					if(endNode->costSoFar <= newCostSoFar)
						continue;

					RemoveNodeFromList(connectIndex, closedList);
					heuristicEstimate = endNode->estimatedCosts;
				}

				else if(IsInList(connectIndex, openList))
				{
					GetNodeByIndexFromList(connectIndex, endNode, openList);
					if(endNode->costSoFar <= newCostSoFar)
						continue;

					heuristicEstimate = endNode->estimatedCosts;
				}

				//unvisited node
				else
				{
					endNode->index = connectIndex;
					CalculateHeuristicValue(connectIndex, endIndex, endNode->estimatedCosts);
				}

				endNode->costSoFar = newCostSoFar;
				endNode->parentIndex = currentNode->index;

				if(!IsInList(connectIndex, openList))
					openList->push_back(endNode);
			}
		}

		openList->pop_front();
		closedList->push_back(currentNode);

		//sort the open list
		SortList(openList);

		pathFindCounter++;
	}

	if(currentNode->index != endIndex)
		return false;

	//carve path
	else
	{
		int counter = 0;
		while(currentNode->index != startIndex && counter < 999999999)
		{
			m_dungeonData->dungeonArray[currentNode->index] = 0;
			if(currentNode->parentIndex == -1)
				return false;
			
			GetNodeByIndexFromList(currentNode->parentIndex, currentNode, closedList);
			counter++;
		}

		if(counter > 999999998)
			return false;
	}

	delete entryNode;
	entryNode = 0;

	return true;
}

bool DungeonGeneratorClass::IsInList(int index, std::deque<PathFindingNode*> *list)
{
	//push null as pointer for the end
	list->push_back(0);

	PathFindingNode* tmp = list->front();
	list->pop_front();

	bool result = false;
	while(tmp)
	{
		list->push_back(tmp);
		if(tmp->index == index)
			result = true;

		tmp = list->front();
		list->pop_front();
	}

	delete tmp;
	tmp = 0;

	return result;
}
void DungeonGeneratorClass::GetNodeByIndexFromList(int index, PathFindingNode* node, std::deque<PathFindingNode*>* list)
{
	//push null as pointer for the end
	list->push_back(0);

	//check each node
	PathFindingNode* tmp = list->front();
	list->pop_front();

	while(tmp)
	{
		list->push_back(tmp);
		if(node->index == index)
			node = tmp;

		tmp = list->front();
		list->pop_front();
	}
	
	delete tmp;
	tmp = 0;
}
void DungeonGeneratorClass::RemoveNodeFromList(int index, std::deque<PathFindingNode*>* list)
{
	list->push_back(0);

	PathFindingNode* tmp = new PathFindingNode;
	tmp = list->front();
	list->pop_front();

	while(tmp)
	{
		if(tmp->index != index)
			list->push_back(tmp);

		tmp = list->front();
		list->pop_front();
	}

	delete tmp;
	tmp = 0;
}
void DungeonGeneratorClass::CalculateHeuristicValue(int startIndex, int endIndex, int& value)
{
	value = 1000;
	int rowNumberStart = startIndex / m_dungeonData->dungeonWidth;
	int colNumberStart = startIndex % m_dungeonData->dungeonWidth;

	int rowNumberEnd = endIndex / m_dungeonData->dungeonWidth;
	int colNumberEnd = endIndex % m_dungeonData->dungeonWidth;

	value = abs(rowNumberEnd - rowNumberStart) * abs(colNumberEnd - colNumberStart);
}

void DungeonGeneratorClass::GetNodeByListIndex(int index, PathFindingNode* node, std::deque<PathFindingNode*>* list)
{
	int i = 0;
	list->push_back(0);

	PathFindingNode* tmp = new PathFindingNode;
	tmp = list->front();
	list->pop_front();

	while(tmp)
	{
		list->push_back(tmp);
		if(i == index)
			node = tmp;

		tmp = list->front();
		list->pop_front();

		i++;
	}
}

//heap sort a vector list
void DungeonGeneratorClass::SortList(std::deque<PathFindingNode*>* list)
{
	for (int i = (int)(list->size() / 2); i >= 0; i--)
	{
		HeapSink (list, i, list->size() - 1);
	}

	for (int i = 0; i < (int)list->size(); i++) {

		if(i == 0){
			PathFindingNode* tmp = list->at(0);
			list->at(0) = list->at(list->size()-1-i);
			list->at(list->size()-1-i) = tmp;
		}else{
			PathFindingNode* tmp = list->at(0);
			list->at(0) = list->at(list->size()-i);
			list->at(list->size()-i) = tmp;
		}

		HeapSink(list, 0, list->size()-1-i);
	}
}
void DungeonGeneratorClass::HeapSink(std::deque<PathFindingNode*>* list, int i, int n)
{
	PathFindingNode* tmp1 = new PathFindingNode;
	PathFindingNode* tmp2 = new PathFindingNode;

	int lc = 2 * i;
	if (lc > n)
		return;

	int rc = lc + 1;
	int mc = rc;

	if (rc > n)
		mc = lc;
	else
	{
		tmp1 = list->at(lc);
		tmp2 = list->at(rc);
		if ((tmp1->costSoFar + tmp1->estimatedCosts) > (tmp2->costSoFar + tmp2->estimatedCosts))
			mc = lc;
	}

	tmp1 = list->at(i);
	tmp2 = list->at(mc);
	if ((tmp1->costSoFar + tmp1->estimatedCosts) >= (tmp2->costSoFar + tmp2->estimatedCosts))
		return;

	//swap
	list->at(i) = list->at(mc);
	list->at(mc) = tmp1;

	HeapSink (list, mc, n);
}

int DungeonGeneratorClass::GetConnectedIndex(int startIndex, int direction)
{
	int result = -1;

	int rowNumber = startIndex / m_dungeonData->dungeonWidth;
	int colNumber = startIndex % m_dungeonData->dungeonWidth;

	//north
	if(direction == 0 && rowNumber != 0)
		result = startIndex - m_dungeonData->dungeonWidth;
	
	//north east
	else if(direction == 1 && rowNumber != 0 && colNumber != (m_dungeonData->dungeonWidth - 1))
		result = startIndex - m_dungeonData->dungeonWidth + 1;

	//east
	else if(direction == 2 && colNumber != (m_dungeonData->dungeonWidth - 1))
		result = startIndex + 1;

	//south east
	else if(direction == 3 && rowNumber != (m_dungeonData->dungeonHeight - 1) && colNumber != (m_dungeonData->dungeonWidth - 1))
		result = startIndex + m_dungeonData->dungeonWidth + 1;

	//south
	else if(direction == 4 && rowNumber != (m_dungeonData->dungeonHeight - 1))
		result = startIndex + m_dungeonData->dungeonWidth;

	//south west
	else if(direction == 5 && rowNumber != (m_dungeonData->dungeonHeight - 1) && colNumber != 0)
		result = startIndex + m_dungeonData->dungeonWidth - 1;

	//west
	else if(direction == 6 && colNumber != 0)
		result = startIndex - 1;

	//north west
	else if(direction == 7 && rowNumber != 0 && colNumber != 0)
		result = startIndex - m_dungeonData->dungeonWidth - 1;


	return result;
}

/*void DungeonGeneratorClass::TempDrawArea(int areaWidth, int areaHeight, int startIndex, int quarterIndex)
{
	if(quarterIndex == 0 || quarterIndex == 1){
		for(int i = 0; i < areaHeight; i++)
		{
			for(int j = 0; j < areaWidth; j++)
			{
				m_dungeonData->dungeonArray[(startIndex + j) + (i * m_dungeonData->dungeonWidth)] = 0;
			}
		}
	}
}*/