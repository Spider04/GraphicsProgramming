#include "dungeon_generator.h"

DungeonGeneratorClass::DungeonGeneratorClass()
	: m_dungeonData(0)
	, m_lastUsedSeed(0)
	, m_roomAmount(0)
	, m_totalPointAmount(0)
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
	m_dungeonData->collectibles = new std::deque<DungeonGeneratorClass::CollectibleData*>;

	int lastIndex = 0;
	lastIndex = (dungeonHeight * dungeonWidth) - 1;

	m_dungeonData->dungeonArray = new int[(lastIndex + 1)];
	if(!m_dungeonData->dungeonArray)
		return false;

	for(int i = 0; i < lastIndex; i++)
		m_dungeonData->dungeonArray[i] = DUNGEON_WALLHEIGHT;

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

	//generate initial dungeon
	/*bool result = false;
	result = GenerateNewDungeon(DUNGEON_ROOMS);
	*/
	return true;
}

void DungeonGeneratorClass::Shutdown()
{
	ReleaseRoomsAndCollectibles();

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

bool DungeonGeneratorClass::GenerateNewDungeon(int amountOfRooms, D3DClass* d3d)
{
	//create new room array (for pathfinding and spawning object)
	ReleaseRoomsAndCollectibles();

	//init room data
	m_roomAmount = 0;
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
	m_dungeonData->collectibles = new std::deque<DungeonGeneratorClass::CollectibleData*>;
	
	//init pathfinding lists
	openList = new std::deque<PathFindingNode*>;
	closedList = new std::deque<PathFindingNode*>;

	//reset terrain
	int dungeonWidth = m_dungeonData->dungeonWidth;
	int dungeonHeight = m_dungeonData->dungeonHeight;

	for(int i = 0; i < dungeonHeight; i++)
	{
		for(int j = 0; j < dungeonWidth; j++)
		{
			m_dungeonData->dungeonArray[(j + (dungeonWidth * i))] = DUNGEON_WALLHEIGHT;
		}
	}

	//get seed and use it to init the random function
	//m_lastUsedSeed = (unsigned int)time(NULL);
	
	//semi-random function by using a set seed
	m_lastUsedSeed = (unsigned int)500;

	srand(m_lastUsedSeed);


	int startIndex = 0;
	bool result = false;

	//create all rooms
	for(int i = 0; i < 9; i++)
	{
		if(i < 1)
			startIndex = 0;
		else if(i < 2)
			startIndex = dungeonWidth / 3;
		else if(i < 3)
			startIndex = (dungeonWidth / 3) * 2;

		else if(i < 4)
			startIndex = dungeonWidth * (dungeonHeight / 3);
		else if(i < 5)
			startIndex = (dungeonWidth * (dungeonHeight / 3)) + (dungeonWidth / 3);
		else if(i < 6)
			startIndex = (dungeonWidth * (dungeonHeight / 3)) + ((dungeonWidth / 3) * 2);

		else if(i < 7)
			startIndex = dungeonWidth * ((dungeonHeight / 3) * 2);
		else if(i < 8)
			startIndex = (dungeonWidth * ((dungeonHeight / 3) * 2)) + (dungeonWidth / 3);
		else
			startIndex = (dungeonWidth * ((dungeonHeight / 3) * 2)) + ((dungeonWidth / 3) * 2);

		result = GenerateRoom(dungeonWidth / 3, dungeonHeight / 3, startIndex);

		if(!result)
			return false;
	}
	

	//connect all rooms with at least one other room to create a block (used later to ensure that all rooms are connected)
	int totalAmountOfBlocks = 0;
	int nextRoomIndex = 0;
	for(int i = 0; i < 9; i++)
	{
		//find room for connection, if not already assigned to a block
		if(m_roomData[i]->areaBlock == -1)
		{
			nextRoomIndex = rand() % m_roomData[i]->connectionCount;
			nextRoomIndex = m_roomData[i]->allowedConnections[nextRoomIndex];
			ConnectTwoRooms(m_roomData[i], m_roomData[nextRoomIndex]);

			if(m_roomData[nextRoomIndex]->areaBlock == -1)
			{
				m_roomData[i]->areaBlock = totalAmountOfBlocks;
				m_roomData[nextRoomIndex]->areaBlock = totalAmountOfBlocks;
				totalAmountOfBlocks++;

			}else
				m_roomData[i]->areaBlock = m_roomData[nextRoomIndex]->areaBlock;
			
		}
	}

	//connect all blocks with each other
	result = false;
	int counter = 0;
	while(!result && counter < 999999)
	{
		ConnectAllAreas(totalAmountOfBlocks, 9);
		if(totalAmountOfBlocks == 1)
			result = true;

		counter++;
	}

	if(!result)
		return false;

	
	//reset total amount of points
	m_totalPointAmount = 0;

	//spawn collectibles in all rooms
	for(int i = 0; i < 9; i++)
	{
		SpawnCollectibles(m_roomData[i], d3d);
	}
	return result;
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
	//m_roomData[m_roomAmount] = new RoomData;
	if(!m_roomData[m_roomAmount])
		return false;

	//set the data
	m_roomData[m_roomAmount]->topLeftIndex = roomTopLeftIndex;
	m_roomData[m_roomAmount]->width = roomWidth;
	m_roomData[m_roomAmount]->length = roomHeight;
	m_roomData[m_roomAmount]->areaTopLeftIndex = startIndex;
	m_roomData[m_roomAmount]->areaBlock = -1; //not assigned to a specific block

	//m_roomData[m_roomAmount]->exits = new ExitIndexes;
	m_roomData[m_roomAmount]->exits->north = 0;
	m_roomData[m_roomAmount]->exits->south = 0;
	m_roomData[m_roomAmount]->exits->west = 0;
	m_roomData[m_roomAmount]->exits->east = 0;

	//calculate the exits
	for(int i = 0; i < 4; i++)
	{
		SetRoomExit(m_roomData[m_roomAmount], i);
	}

	//set allowed connections by hand for now
	m_roomData[m_roomAmount]->allowedConnections = new int;
	if(m_roomAmount == 0){
		m_roomData[m_roomAmount]->allowedConnections[0] = 1;
		m_roomData[m_roomAmount]->allowedConnections[1] = 3;
		m_roomData[m_roomAmount]->connectionCount = 2;

	}else if(m_roomAmount == 1){
		m_roomData[m_roomAmount]->allowedConnections[0] = 0;
		m_roomData[m_roomAmount]->allowedConnections[1] = 2;
		m_roomData[m_roomAmount]->allowedConnections[2] = 4;
		m_roomData[m_roomAmount]->connectionCount = 3;

	}else if(m_roomAmount == 2){
		m_roomData[m_roomAmount]->allowedConnections[0] = 1;
		m_roomData[m_roomAmount]->allowedConnections[1] = 5;
		m_roomData[m_roomAmount]->connectionCount = 2;

	}else if(m_roomAmount == 3){
		m_roomData[m_roomAmount]->allowedConnections[0] = 0;
		m_roomData[m_roomAmount]->allowedConnections[1] = 4;
		m_roomData[m_roomAmount]->allowedConnections[2] = 6;
		m_roomData[m_roomAmount]->connectionCount = 2;

	}else if(m_roomAmount == 4){
		m_roomData[m_roomAmount]->allowedConnections[0] = 1;
		m_roomData[m_roomAmount]->allowedConnections[1] = 3;
		m_roomData[m_roomAmount]->allowedConnections[2] = 5;
		m_roomData[m_roomAmount]->allowedConnections[3] = 7;
		m_roomData[m_roomAmount]->connectionCount = 4;
	}
	
	else if(m_roomAmount == 5){
		m_roomData[m_roomAmount]->allowedConnections[0] = 2;
		m_roomData[m_roomAmount]->allowedConnections[1] = 4;
		m_roomData[m_roomAmount]->allowedConnections[2] = 8;
		m_roomData[m_roomAmount]->connectionCount = 3;

	}else if(m_roomAmount == 6){
		m_roomData[m_roomAmount]->allowedConnections[0] = 3;
		m_roomData[m_roomAmount]->allowedConnections[1] = 7;
		m_roomData[m_roomAmount]->connectionCount = 2;

	}else if(m_roomAmount == 7){
		m_roomData[m_roomAmount]->allowedConnections[0] = 6;
		m_roomData[m_roomAmount]->allowedConnections[1] = 4;
		m_roomData[m_roomAmount]->allowedConnections[2] = 8;
		m_roomData[m_roomAmount]->connectionCount = 3;

	}else if(m_roomAmount == 8){
		m_roomData[m_roomAmount]->allowedConnections[0] = 5;
		m_roomData[m_roomAmount]->allowedConnections[1] = 7;
		m_roomData[m_roomAmount]->connectionCount = 2;
	}

	m_roomAmount++;
	
	return true;
}

void DungeonGeneratorClass::ReleaseRoomsAndCollectibles()
{
	for(int i = 0; i < DUNGEON_ROOMS; i++)
	{
		if(m_roomData[i])
		{
			if(m_roomData[i]->exits)
			{
				delete m_roomData[i]->exits;
				m_roomData[i]->exits = 0;
			}
			m_roomData[i] = 0;
		}
	}

	//release collectibles
	if(m_dungeonData->collectibles)
	{
		for(int j = 0; j < (int)m_dungeonData->collectibles->size(); j++)
		{
			m_dungeonData->collectibles->at(j)->model->Shutdown();
			delete m_dungeonData->collectibles->at(j);
			m_dungeonData->collectibles->at(j) = 0;
		}
		m_dungeonData->collectibles->clear();
		m_dungeonData->collectibles = 0;
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
		room->exits->west = room->topLeftIndex + ((room->length / 2) * m_dungeonData->dungeonWidth) + (room->width - 1);

	//set east exit
	else if(exitIndex == 1)
		room->exits->east = room->topLeftIndex + ((room->length / 2) * m_dungeonData->dungeonWidth);
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
		if(roomColNumber[0] > roomColNumber[1])
			CreatePathBetweenPoints(room1->exits->east, room2->exits->west);
		else
			CreatePathBetweenPoints(room1->exits->west, room2->exits->east);
	}

	//case: rooms are vertically connected
	else if(roomRowNumber[0] != roomRowNumber[1] && roomColNumber[0] == roomColNumber[1])
	{
		if(roomRowNumber[0] < roomRowNumber[1])
			CreatePathBetweenPoints(room1->exits->south, room2->exits->north);
		else
			CreatePathBetweenPoints(room1->exits->north, room2->exits->south);
	}

	//last case, they are slightly diagonal - no solution so far
	
	return;
}

//AStar algorithm to calculate path to entry
bool DungeonGeneratorClass::CreatePathBetweenPoints(int startIndex, int endIndex)
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
		for(int i = 0; i < 4; i++)
		{
			PathFindingNode* endNode = new PathFindingNode;
			connectIndex = GetConnectedIndex(currentNode->index, i);
			if(connectIndex != -1)
			{
				newCostSoFar = currentNode->costSoFar + 1;

				//node from closed list
				if(IsInList(connectIndex, closedList))
				{
					GetNodeByIndexFromList(connectIndex, endNode, closedList);

					if(endNode->costSoFar <= newCostSoFar)
						continue;

					RemoveNodeFromList(connectIndex, closedList);
					heuristicEstimate = endNode->estimatedCosts;
				}

				//node from open list
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

		if(pathFindCounter == 100)
		{
			PathFindingNode* tmp = new PathFindingNode;
			tmp = openList->front();

			int hValue = 0;
			CalculateHeuristicValue(tmp->index, endIndex, hValue);
		}
			
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
			CarvePathPoint(currentNode->index);

			if(currentNode->parentIndex == -1)
				return false;
			
			GetNodeByIndexFromList(currentNode->parentIndex, currentNode, closedList);
			counter++;
		}

		if(counter > 999999998)
			return false;

		
		//carve the start point too -> led sometimes to thye entry being sealed by one triangle
		m_dungeonData->dungeonArray[startIndex] = 0;
		CarvePathPoint(startIndex);
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

	return result;
}
void DungeonGeneratorClass::GetNodeByIndexFromList(int index, PathFindingNode* &node, std::deque<PathFindingNode*>* list)
{
	//push null as pointer for the end
	list->push_back(0);

	//check each node
	PathFindingNode* tmp = list->front();
	list->pop_front();

	while(tmp)
	{
		list->push_back(tmp);
		if(tmp->index == index){
			node = tmp;
		}

		tmp = list->front();
		list->pop_front();
	}

	return;
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

	return;
}

void DungeonGeneratorClass::CalculateHeuristicValue(int startIndex, int endIndex, int& value)
{
	value = 1000;
	int rowNumberStart = startIndex / m_dungeonData->dungeonWidth;
	int colNumberStart = startIndex % m_dungeonData->dungeonWidth;

	int rowNumberEnd = endIndex / m_dungeonData->dungeonWidth;
	int colNumberEnd = endIndex % m_dungeonData->dungeonWidth;

	value = (abs(rowNumberEnd - rowNumberStart) * abs(rowNumberEnd - rowNumberStart)) + (abs(colNumberEnd - colNumberStart) * abs(colNumberEnd - colNumberStart));
}

//heap sort a vector list
void DungeonGeneratorClass::SortList(std::deque<PathFindingNode*>* list)
{
	for (int i = (int)(list->size() / 2); i >= 0; i--)
	{
		HeapSink (list, i, list->size() - 1);
	}

	for (int i = 0; i < (int)list->size(); i++) {
		PathFindingNode* tmp;

		if(i == 0){
			tmp = list->at(0);
			list->at(0) = list->at(list->size()-1-i);
			list->at(list->size()-1-i) = tmp;
		}else{
			tmp = list->at(0);
			list->at(0) = list->at(list->size()-i);
			list->at(list->size()-i) = tmp;
		}

		tmp = 0;

		HeapSink(list, 0, list->size()-1-i);
	}
	
		
}
void DungeonGeneratorClass::HeapSink(std::deque<PathFindingNode*>* list, int i, int n)
{
	PathFindingNode* tmp1;
	PathFindingNode* tmp2;

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

	tmp1 = 0;
	tmp2 = 0;

	HeapSink (list, mc, n);
}

int DungeonGeneratorClass::GetConnectedIndex(int startIndex, int direction)
{
	int result = -1;

	int rowNumber = startIndex / m_dungeonData->dungeonWidth;
	int colNumber = startIndex % m_dungeonData->dungeonWidth;

	//north
	if(direction == 0 && rowNumber > 1)
		result = startIndex - m_dungeonData->dungeonWidth;
	
	//east
	else if(direction == 1 && colNumber < (m_dungeonData->dungeonWidth - 2))
		result = startIndex + 1;

	//south
	else if(direction == 2 && rowNumber < (m_dungeonData->dungeonHeight - 2))
		result = startIndex + m_dungeonData->dungeonWidth;

	//west
	else if(direction == 3 && colNumber > 1)
		result = startIndex - 1;

	//tiles who are already floor are not valid
	/*if(result != -1)
		if(m_dungeonData->dungeonArray[result] < DUNGEON_WALLHEIGHT - 1)
			result = -1;
			*/
	return result;
}

void DungeonGeneratorClass::CarvePathPoint(int index)
{
	m_dungeonData->dungeonArray[index] = 0;
	
	int startColIndex = index - (DUNGEON_MIN_PATHWIDTH / 2);
	int cellIndex = 0;
	for(int i = 0; i < DUNGEON_MIN_PATHWIDTH; i++)
	{
		cellIndex = startColIndex + (i * m_dungeonData->dungeonWidth);
		for(int j = 0; j < DUNGEON_MIN_PATHWIDTH; j++)
		{
			m_dungeonData->dungeonArray[cellIndex] = 0;
			cellIndex++;
		}
	}
}

void DungeonGeneratorClass::GetSpawningCoord(float& xPos, float& yPos, float& zPos)
{
	xPos = (m_roomData[0]->topLeftIndex % m_dungeonData->dungeonWidth) + (m_roomData[0]->width * 0.5f);
	zPos = (m_roomData[0]->topLeftIndex / m_dungeonData->dungeonWidth) + (m_roomData[0]->length * 0.5f);
	yPos = 0.0f;
}


bool DungeonGeneratorClass::SpawnCollectibles(RoomData* room, D3DClass* d3d)
{
	float posX = 0.0f, posZ = 0.0f;
	float widthPartition = (float)(room->width / COLLECTIBLES_PER_ROOM);
	bool result = false;

	for(int i = 0; i < COLLECTIBLES_PER_ROOM; i++)
	{
		//get random position in room (x: start column + random (width - minDistanceToWall * 2)
		posX = (room->topLeftIndex % m_dungeonData->dungeonWidth) + (rand() % (int)(((widthPartition * i) - 2) * 100)) * 0.01f;
		//(z:
		posZ = (room->topLeftIndex / m_dungeonData->dungeonWidth) + (rand() % (int)((room->length - 2) * 100)) * 0.01f;

		//create new model for collectible (no instance, since I want to delete them one by one when collected)
		ModelClass* testModel = new ModelClass;
		result = testModel->Initialize(d3d->GetDevice(), "../Engine/data/sphere.txt", L"../Engine/data/stone01.dds");
		if(!result)
			return false;
		testModel->SetPosition(posX, 1.0f, posZ);

		//create new collectible object and increment the counter of the total points
		DungeonGeneratorClass::CollectibleData* testObject = new DungeonGeneratorClass::CollectibleData;
		testObject->model = testModel;
		testObject->collisionRadius = 2.0f;

		testObject->posX = posX;
		testObject->posY = 0.0f;
		testObject->posZ = posZ;

		m_dungeonData->collectibles->push_back(testObject);
		m_totalPointAmount++;
	}

	return true;
}

bool DungeonGeneratorClass::CollectibleAtPosition(float xPos, float yPos, float zPos)
{
	bool returnValue = false;

	//get pointer to collectibles list and set the object ptr to the first object in the list
	std::deque<CollectibleData*> *listPtr = m_dungeonData->collectibles;
	if(!m_dungeonData->collectibles->empty())
	{
		CollectibleData* currentObject = listPtr->front();

		//push 0 as end marker in the back and pop the first element out
		listPtr->push_back(0);
		listPtr->pop_front();

		//go through all objects until the 0 marker
		while(currentObject)
		{
			bool positionsMatch = false;
			//check if the player is not above the object (with a bit tolerance, since we have float values)
			//check also if another collectible was already detected (only one per frame for now)
			if(currentObject->posY + 0.1f > yPos && !returnValue)
			{
				//check if x and z position ar eclose enough
				if((fabs(xPos - currentObject->posX) < currentObject->collisionRadius) 
					&& (fabs(zPos - currentObject->posZ) < currentObject->collisionRadius))
				{
					//delete object and set boolean var to true
					currentObject->model->Shutdown();
					delete currentObject;
					currentObject = 0;

					positionsMatch = true;
				}
			}

			//push element back, get next element and pop it out from the list
			if(!positionsMatch)
				listPtr->push_back(currentObject);
			else
				returnValue = true;

			currentObject = listPtr->front();
			listPtr->pop_front();
		}
	}

	return returnValue;
}

int DungeonGeneratorClass::GetTotalPointCount()
{
	return m_totalPointAmount;
}


void DungeonGeneratorClass::ConnectAllAreas(int& totalAreaCount, int totalAmountOfRooms)
{
	bool connectionMade = false;
	for (int i = (totalAreaCount - 1); i > 0; i--)
	{
		connectionMade = false;

		for(int j = 0; j < totalAmountOfRooms; j++)
		{
			//get next room from this area
			if(!connectionMade && m_roomData[j]->areaBlock == i)
			{

				//check all other rooms if they are from the next area
				for(int k = 0; k < totalAmountOfRooms; k++)
				{
					if(!connectionMade && k != j && m_roomData[k]->areaBlock == i-1)
					{

						//check if connection can be made
						for(int m = 0; m < m_roomData[j]->connectionCount; m++)
						{
							if(m_roomData[j]->allowedConnections[m] == k)
							{
								ConnectTwoRooms(m_roomData[j], m_roomData[k]);
								connectionMade = true;

								//reset all indexes from the connected areas to the smaller one
								ChangeAreaIndexes(k, j, totalAmountOfRooms);
								totalAreaCount--;

								break;
							}
						
						}

					}
				}
			}
		}
	}
}

void DungeonGeneratorClass::ChangeAreaIndexes(int from, int to, int totalAmountOfRooms)
{
	for(int j = 0; j < totalAmountOfRooms; j++)
	{
		if(m_roomData[j]->areaBlock == from)
			m_roomData[j]->areaBlock = to;
	}
}