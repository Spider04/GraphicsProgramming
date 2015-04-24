#include "dungeon_generator.h"

DungeonGeneratorClass::DungeonGeneratorClass()
	: m_dungeonData(0)
	, m_roomAmount(0)
	, m_totalPointAmount(0)
{
	//init all pointers for the rooms
	for (int i = 0; i < DUNGEON_ROOMS; i++)
		m_roomData[i] = 0;
}
DungeonGeneratorClass::DungeonGeneratorClass(const DungeonGeneratorClass& other)
{}
DungeonGeneratorClass::~DungeonGeneratorClass()
{}


bool DungeonGeneratorClass::Initialize(int dungeonWidth, int dungeonHeight)
{
	//init dungeon data
	m_dungeonData = new DungeonData;
	m_dungeonData->dungeonWidth = dungeonWidth;
	m_dungeonData->dungeonHeight = dungeonHeight;
	m_dungeonData->collectibles = 0;

	//init dungeon array
	int lastIndex = 0;
	lastIndex = (dungeonHeight * dungeonWidth) - 1;
	m_dungeonData->dungeonArray = new int[(lastIndex + 1)];
	if(!m_dungeonData->dungeonArray)
		return false;

	//init room data with 0 pointers
	for (int i = 0; i < DUNGEON_ROOMS; i++)
	{
		m_roomData[i] = 0;
	}

	return true;
}

void DungeonGeneratorClass::Shutdown()
{
	//release and delete room data and collectibles (part of dungeon data)
	ReleaseRoomsAndCollectibles();

	//delete and release the rest of the dungeon data
	if(m_dungeonData)
	{	
		delete m_dungeonData;
		m_dungeonData = 0;
	}
}

void DungeonGeneratorClass::ReleaseRoomsAndCollectibles()
{
	//delete and release all room data
	for (int i = 0; i < DUNGEON_ROOMS; i++)
	{
		if (m_roomData[i])
		{
			//delete exit data
			if (m_roomData[i]->exits)
			{
				delete m_roomData[i]->exits;
				m_roomData[i]->exits = 0;
			}
			delete m_roomData[i];
			m_roomData[i] = 0;
		}
	}

	//delete and release all collectibles
	if (m_dungeonData->collectibles)
	{
		for (int j = 0; j < (int)m_dungeonData->collectibles->size(); j++)
		{
			//shutdownn and delete model data
			m_dungeonData->collectibles->at(j)->model->Shutdown();
			delete m_dungeonData->collectibles->at(j);
			m_dungeonData->collectibles->at(j) = 0;
		}
		m_dungeonData->collectibles->clear();
		m_dungeonData->collectibles = 0;
	}
}


//generate a complete dungeon
bool DungeonGeneratorClass::GenerateNewDungeon(int amountOfRooms, D3DClass* d3d)
{
	//release all previous room, pathfinding and collectibles data (in case they are already initialized)
	ReleaseRoomsAndCollectibles();

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
	//srand(time(NULL));
	
	//pseudo-random function by using a set seed
	srand((unsigned int)500);


	//init all room data
	m_roomAmount = 0;
	for (int i = 0; i < DUNGEON_ROOMS; i++)
	{
		m_roomData[i] = new RoomData;
		m_roomData[i]->exits = new ExitIndexes;
		m_roomData[i]->exits->north = 0;
		m_roomData[i]->exits->south = 0;
		m_roomData[i]->exits->west = 0;
		m_roomData[i]->exits->east = 0;
		m_roomData[i]->allowedConnections = new int;
	}

	//create all rooms in the dungeon area (for now, hardcoded indeces for exact 9 rooms)
	//no connections made yet
	int startIndex = 0;
	bool result = false;
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
			//get new room (from allowed connections) and create path
			nextRoomIndex = rand() % m_roomData[i]->connectionCount;
			nextRoomIndex = m_roomData[i]->allowedConnections[nextRoomIndex];
			ConnectTwoRooms(m_roomData[i], m_roomData[nextRoomIndex]);

			//combine both to a new block if they are both not in a block
			if(m_roomData[nextRoomIndex]->areaBlock == -1)
			{
				m_roomData[i]->areaBlock = totalAmountOfBlocks;
				m_roomData[nextRoomIndex]->areaBlock = totalAmountOfBlocks;
				totalAmountOfBlocks++;

			//or add the new room to the existing block
			}else
				m_roomData[i]->areaBlock = m_roomData[nextRoomIndex]->areaBlock;
			
		}
	}

	//connect all blocks (groups of rooms) with each other
	result = false;

	//counter makes sure that algorithm terminates (in case blocks cannot be connected at all for some reason)
	int counter = 0;
	while(!result && counter < 999999)
	{
		ConnectAllAreas(totalAmountOfBlocks, 9);

		//if all rooms are in one block, all of them are connected
		if(totalAmountOfBlocks == 1)
			result = true;

		counter++;
	}

	if(!result)
		return false;

	
	//reset total amount of points
	m_totalPointAmount = 0;

	//spawn collectibles in all rooms (fyi, takes a considerable amount of time to generate; probably more than generating and connecting the rooms)
	m_dungeonData->collectibles = new std::deque<DungeonGeneratorClass::CollectibleData*>;
	for(int i = 0; i < 9; i++)
	{
		SpawnCollectibles(m_roomData[i], d3d);
	}
	return result;
}

//generates one room and adds it to the room data array
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
		//create a random start index, width and length for a room within the given parameters
		//returns start index, width and length (not imported into the scene at this point)
		GetRandomRoom(startIndex, areaWidth, areaHeight, BORDER_MINDISTANCE, roomTopLeftIndex, roomWidth, roomHeight);
		roomData[0] = roomTopLeftIndex;
		roomData[1] = roomWidth;
		roomData[2] = roomHeight;

		//check if the generated room is within the defined area
		if(RoomIsWithinBorders(roomData, input, BORDER_MINDISTANCE))
			validRoomFound = true;

		counter++;
	}

	//check if loop was terminated because the room was valid
	if (!validRoomFound)
		return false;

	//import room into array by setting all cells to 0 (floor height)
	for(int i = 0; i < roomHeight; i++)
	{
		for(int j = 0; j < roomWidth; j++)
		{
			if((roomTopLeftIndex + j) + (i * m_dungeonData->dungeonWidth) > (m_dungeonData->dungeonWidth * m_dungeonData->dungeonHeight) - 1)
				return false;

			m_dungeonData->dungeonArray[(roomTopLeftIndex + j) + (i * m_dungeonData->dungeonWidth)] = 0;
		}
	}


	//------- save room data

	//set the basic room data
	m_roomData[m_roomAmount]->topLeftIndex = roomTopLeftIndex;
	m_roomData[m_roomAmount]->width = roomWidth;
	m_roomData[m_roomAmount]->length = roomHeight;
	m_roomData[m_roomAmount]->areaTopLeftIndex = startIndex;
	m_roomData[m_roomAmount]->areaBlock = -1; //not assigned to a specific block at this ppoint

	//calculate all exit positions
	for(int i = 0; i < 4; i++)
	{
		SetRoomExit(m_roomData[m_roomAmount], i);
	}

	//set allowed connections by hand for now - necessary to create valid connections
	//expanding the dungeon to more rooms would mean to automate this process
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

//get random parameters for a new room within the given parameters
void DungeonGeneratorClass::GetRandomRoom(int areaStartIndex, int areaWidth, int areaHeight, int minBorderSpace,
	int& roomStartIndex, int& roomWidth, int& roomLength)
{
	//calc new width (at least a specific fraction of total area width)
	int minRoomWidth = areaWidth / ROOM_MINWIDTH_DIV;
	roomWidth = areaWidth - ((minBorderSpace * 2) + minRoomWidth);
	roomWidth = minRoomWidth + (rand() % roomWidth);

	//calc new height (at least a specific fraction of total area length)
	int minRoomLength = areaHeight / ROOM_MINHEIGHT_DIV;
	roomLength = areaHeight - ((minBorderSpace * 2) + minRoomLength);
	roomLength = minRoomLength + (rand() % roomLength);


	//calculate new top left coordinate (within the area space), take the necessary space for width and length into account
	int xCoord = areaWidth - ((minBorderSpace * 2) + roomWidth);
	xCoord = minBorderSpace + (rand() % xCoord);

	int zCoord = areaHeight - ((minBorderSpace * 2) + roomLength);
	zCoord = minBorderSpace + (rand() % zCoord);

	//set start index at top left coordinate in world space
	roomStartIndex = areaStartIndex + xCoord + (zCoord * m_dungeonData->dungeonWidth);
}

//check if a room is within given borders (double check to make sure a random room generation was valid)
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

//calculate the positions for the exists of a given room and record them in the given data structure
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


//connect two rooms from suitable exits
void DungeonGeneratorClass::ConnectTwoRooms(RoomData* room1, RoomData* room2)
{
	//get the row number of both rooms
	int roomRowNumber[2];
	roomRowNumber[0] = room1->areaTopLeftIndex / m_dungeonData->dungeonWidth;
	roomRowNumber[1] = room2->areaTopLeftIndex / m_dungeonData->dungeonWidth;

	//get the column number of both rooms
	int roomColNumber[2];
	roomColNumber[0] = room1->areaTopLeftIndex % m_dungeonData->dungeonWidth;
	roomColNumber[1] = room2->areaTopLeftIndex % m_dungeonData->dungeonWidth;


	//1st case: rooms are horizontally connected
	if(roomRowNumber[0] == roomRowNumber[1] && roomColNumber[0] != roomColNumber[1])
	{
		if(roomColNumber[0] > roomColNumber[1])
			CreatePathBetweenPoints(room1->exits->east, room2->exits->west);
		else
			CreatePathBetweenPoints(room1->exits->west, room2->exits->east);
	}

	//2nd case: rooms are vertically connected
	else if(roomRowNumber[0] != roomRowNumber[1] && roomColNumber[0] == roomColNumber[1])
	{
		if(roomRowNumber[0] < roomRowNumber[1])
			CreatePathBetweenPoints(room1->exits->south, room2->exits->north);
		else
			CreatePathBetweenPoints(room1->exits->north, room2->exits->south);
	}

	//last case, they are slightly diagonal - this case is being avoided so far
	
	return;
}

/* AStar algorithm to calculate path to entry
 * if successfull, the path is imported / carved into the dungeon array
 */
bool DungeonGeneratorClass::CreatePathBetweenPoints(int startIndex, int endIndex)
{
	//create first node from start index
	PathFindingNode* entryNode = new PathFindingNode;
	entryNode->costSoFar = 0;
	CalculateHeuristicValue(startIndex, endIndex, entryNode->estimatedCosts);
	entryNode->index = startIndex;
	entryNode->parentIndex = -1;

	//create open and insert the first node
	std::deque<PathFindingNode*> *openList = new std::deque<PathFindingNode*>;
	openList->push_back(entryNode);

	//reset pointer to data
	entryNode = 0;

	//create closed list
	std::deque<PathFindingNode*> *closedList = new std::deque<PathFindingNode*>;

	//create reference to current node
	PathFindingNode* currentNode = 0;
	PathFindingNode* endNode = 0;

	//init various variable for pathfinding
	int pathFindCounter = 0;
	int newCostSoFar = 0;
	int heuristicEstimate = 1000;
	int connectIndex = -1;

	//while loop for complete astar algorithm (always terminates because it depends on a counter)
	while(!openList->empty() && pathFindCounter < 999999999)
	{
		//set current node reference to first element in the open list
		currentNode = openList->front();

		//end pathfinding, if the current node is at the position we are looking for
		if(currentNode->index == endIndex)
			break;

		//init index for next node
		connectIndex = -1;

		//check all 4 directions for a new node
		for(int i = 0; i < 4; i++)
		{
			//create reference to next node and calculate the index of the next node
			endNode = new PathFindingNode;
			connectIndex = GetConnectedIndex(currentNode->index, i);

			//only consider this node if the index is valid (= next node in direction exist)
			if(connectIndex != -1)
			{

				//calculate the cost so far from the start node to this node
				newCostSoFar = currentNode->costSoFar + 1;

				//1st case: node is already in closed list
				if(IsInList(connectIndex, closedList))
				{
					//get node from closed list and only consider it if the previous cost are higher than the new ones
					GetNodeByIndexFromList(connectIndex, endNode, closedList);
					if(endNode->costSoFar <= newCostSoFar)
						continue;

					//remove node from closed list and get the estimated costs from the node
					RemoveNodeFromList(connectIndex, closedList);
					heuristicEstimate = endNode->estimatedCosts;
				}

				//2nd case: node is already in the open list
				else if(IsInList(connectIndex, openList))
				{
					//get node from open list and only consider it if the previous cost are higher than the new ones
					GetNodeByIndexFromList(connectIndex, endNode, openList);
					if(endNode->costSoFar <= newCostSoFar)
						continue;

					//get the estimated costs from the node
					heuristicEstimate = endNode->estimatedCosts;
				}

				//last case: unvisited (/new) node
				else
				{
					//record the new index and calculate the estimated costs for this new node
					endNode->index = connectIndex;
					CalculateHeuristicValue(connectIndex, endIndex, endNode->estimatedCosts);
				}

				//record the cost so far and the parent (current) node index 
				endNode->costSoFar = newCostSoFar;
				endNode->parentIndex = currentNode->index;

				//if node is not already in the open list, insert it at the back of the list
				if(!IsInList(connectIndex, openList))
					openList->push_back(endNode);
			}

			//reset pointer to next node
			endNode = 0;
		}

		//remove the current node from the open list and insert it in the closed list
		openList->pop_front();
		closedList->push_back(currentNode);

		//(heap) sort the open list
		SortList(openList);

		//increment the counter for thee while loop
		pathFindCounter++;
	}

	//if the current node isn't at the index we are looking for, the algorithm was unsuccessfull
	if(currentNode->index != endIndex)
		return false;

	//if it was successfull, carve the path in the dungeon area
	else
	{
		//go through all nods from the path we found
		int counter = 0;
		while(currentNode->index != startIndex && counter < 999999999)
		{
			//carve the index we found and some from its surroundings in the dungeon array
			CarvePathPoint(currentNode->index);

			//if the current node has no parennt, exit the loop (usuccessfull)
			if(currentNode->parentIndex == -1)
				return false;
			
			//get the parent node by index from the closed list and overwrite the pointer of the current node with it
			GetNodeByIndexFromList(currentNode->parentIndex, currentNode, closedList);
			counter++;
		}

		//end function unsuccessfull if the current node is not at the start index (algorithm terminated somewhere else)
		if (currentNode->index != startIndex)
			return false;

		//carve the start point too -> algorithm terminates before, so sometimes the entrace stays sealed
		CarvePathPoint(startIndex);
	}

	currentNode = 0;

	//release open list (pathfinding)
	if (openList)
	{
		for (int j = 0; j < (int)openList->size(); j++)
		{
			delete openList->at(j);
			openList->at(j) = 0;
		}
		openList->clear();
		openList = 0;
	}

	//release closed list (pathfinding)
	if (closedList)
	{
		for (int j = 0; j < (int)closedList->size(); j++)
		{
			delete closedList->at(j);
			closedList->at(j) = 0;
		}
		closedList->clear();
		closedList = 0;
	}

	return true;
}

//returns the next index in one of the 4 directions
int DungeonGeneratorClass::GetConnectedIndex(int startIndex, int direction)
{
	//init result with -1 (= invalid)
	int result = -1;

	//get row and column number of start index on the grid
	int rowNumber = startIndex / m_dungeonData->dungeonWidth;
	int colNumber = startIndex % m_dungeonData->dungeonWidth;

	//check in north direction
	if (direction == 0 && rowNumber > 1)
		result = startIndex - m_dungeonData->dungeonWidth;

	//check in east direction
	else if (direction == 1 && colNumber < (m_dungeonData->dungeonWidth - 2))
		result = startIndex + 1;

	//check in south direction
	else if (direction == 2 && rowNumber < (m_dungeonData->dungeonHeight - 2))
		result = startIndex + m_dungeonData->dungeonWidth;

	//check in west direction
	else if (direction == 3 && colNumber > 1)
		result = startIndex - 1;

	return result;
}

//check if a node with the given index exist in the list you are searching for
bool DungeonGeneratorClass::IsInList(int index, std::deque<PathFindingNode*> *list)
{
	//push 0 as pointer for the end
	list->push_back(0);

	//create temporary reference, set it to the first element and delete it from the list
	PathFindingNode* tmp = list->front();
	list->pop_front();

	//go through the list
	bool result = false;
	while(tmp)
	{
		//check if the current index is equal to the one you search for
		if(tmp->index == index)
			result = true;

		//push pointer back in the list, take the next one and delete it from the list
		list->push_back(tmp);
		tmp = list->front();
		list->pop_front();
	}

	//return search result
	return result;
}

//returns the reference to the node with the index you search for in the given list
void DungeonGeneratorClass::GetNodeByIndexFromList(int index, PathFindingNode* &node, std::deque<PathFindingNode*>* list)
{
	//push 0 as pointer for the end
	list->push_back(0);

	//create temporary reference, set it to the first element and delete it from the list
	PathFindingNode* tmp = list->front();
	list->pop_front();

	//go through the list
	while(tmp)
	{
		//check if the current index is equal to the one you search for
		if(tmp->index == index)
			node = tmp;

		//push pointer back in the list, take the next one and delete it from the list
		list->push_back(tmp);
		tmp = list->front();
		list->pop_front();
	}

	return;
}

//remove node with the given index from the index in the parameter list
void DungeonGeneratorClass::RemoveNodeFromList(int index, std::deque<PathFindingNode*>* list)
{
	//push 0 as pointer for the end
	list->push_back(0);

	//create temporary reference, set it to the first element and delete it from the list
	PathFindingNode* tmp = list->front();
	list->pop_front();

	//go through the list
	while(tmp)
	{
		//only push the element back if it's not the index you are searching for
		if(tmp->index != index)
			list->push_back(tmp);

		//take the next one and delete it from the list
		tmp = list->front();
		list->pop_front();
	}

	return;
}

//returns the estimated costs / heuristic value of a given index to the one you want to do your pathfinding for
void DungeonGeneratorClass::CalculateHeuristicValue(int startIndex, int endIndex, int& value)
{
	//init value with a high value
	value = 100000;

	//calculate the row and column number on the grid for the start index
	int rowNumberStart = startIndex / m_dungeonData->dungeonWidth;
	int colNumberStart = startIndex % m_dungeonData->dungeonWidth;

	//calculate the row and column number on the grid for the index you want to do pathinding for
	int rowNumberEnd = endIndex / m_dungeonData->dungeonWidth;
	int colNumberEnd = endIndex % m_dungeonData->dungeonWidth;

	//calculate the estimated costs with the difference in row and column on the grid
	//does not need to be correct, just the relative value between different nodes nneed tto be correct
	value = (abs(rowNumberEnd - rowNumberStart) * abs(rowNumberEnd - rowNumberStart)) + (abs(colNumberEnd - colNumberStart) * abs(colNumberEnd - colNumberStart));
}


//heap sort a deque list
void DungeonGeneratorClass::SortList(std::deque<PathFindingNode*>* list)
{
	//do first heap sort to sort the list roughly
	for (int i = (int)(list->size() / 2); i >= 0; i--)
	{
		HeapSink (list, i, list->size() - 1);
	}

	//do the heap sort precisely
	for (int i = 0; i < (int)list->size(); i++) {
		PathFindingNode* tmp;

		if(i == 0)
		{
			tmp = list->at(0);
			list->at(0) = list->at(list->size()-1-i);
			list->at(list->size()-1-i) = tmp;
		}
		else
		{
			tmp = list->at(0);
			list->at(0) = list->at(list->size()-i);
			list->at(list->size()-i) = tmp;
		}

		tmp = 0;

		HeapSink(list, 0, list->size()-1-i);
	}
	
		
}

//heap sink function for the heap sort algorithm
void DungeonGeneratorClass::HeapSink(std::deque<PathFindingNode*>* list, int i, int n)
{
	//create references to both nodes
	PathFindingNode* tmp1;
	PathFindingNode* tmp2;

	//heap sink algorithm
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

	//termination condition
	if ((tmp1->costSoFar + tmp1->estimatedCosts) >= (tmp2->costSoFar + tmp2->estimatedCosts))
		return;

	//swap necessary nodes
	list->at(i) = list->at(mc);
	list->at(mc) = tmp1;

	//reset the pointers
	tmp1 = 0;
	tmp2 = 0;

	//do the same algorithm recursively
	HeapSink (list, mc, n);
}

//carve points from the surroundings of a given index into the dungeon array
void DungeonGeneratorClass::CarvePathPoint(int index)
{
	//start at the index with is half of the min width away from the index
	int startColIndex = index - (DUNGEON_MIN_PATHWIDTH / 2);

	//if the index was too close to the beginning of the grid,set it back to 0
	if (startColIndex < 0)
		startColIndex = 0;

	//temp int to go through all cells
	int cellIndex = 0;
	for(int i = 0; i < DUNGEON_MIN_PATHWIDTH; i++)
	{
		//set the height of the current cell to floor height (0)
		cellIndex = startColIndex + (i * m_dungeonData->dungeonWidth);
		for(int j = 0; j < DUNGEON_MIN_PATHWIDTH; j++)
		{
			m_dungeonData->dungeonArray[cellIndex] = 0;
			cellIndex++;
		}
	}
}


/* connects all blocks (groups of rooms) within the room data with each other
 * connects them from low to high index
 */
void DungeonGeneratorClass::ConnectAllAreas(int& totalAreaCount, int totalAmountOfRooms)
{
	//init boolean value with false
	bool connectionMade = false;

	//go through all blocks (except the last)
	for (int i = (totalAreaCount - 1); i > 0; i--)
	{
		//init boolean value with false (again)
		connectionMade = false;

		//go through all rooms to search for a room of this block
		for (int j = 0; j < totalAmountOfRooms; j++)
		{
			//get the next room from this block (if a connection is not already made
			if (!connectionMade && m_roomData[j]->areaBlock == i)
			{


				//check all other rooms if they are from the next block
				for (int k = 0; k < totalAmountOfRooms; k++)
				{
					//get the next room of the net block
					if (!connectionMade && k != j && m_roomData[k]->areaBlock == i - 1)
					{


						//check if a connection can be made between the rooms we found
						for (int m = 0; m < m_roomData[j]->connectionCount; m++)
						{
							if (m_roomData[j]->allowedConnections[m] == k)
							{
								//connect both rooms
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

//change the area (block) indexes of all rooms with the given block index into the new one
void DungeonGeneratorClass::ChangeAreaIndexes(int from, int to, int totalAmountOfRooms)
{
	for (int j = 0; j < totalAmountOfRooms; j++)
	{
		if (m_roomData[j]->areaBlock == from)
			m_roomData[j]->areaBlock = to;
	}
}


//spawn a collectible or multiple ones in a given room
bool DungeonGeneratorClass::SpawnCollectibles(RoomData* room, D3DClass* d3d)
{
	float posX = 0.0f, posZ = 0.0f;
	float widthPartition = (float)(room->width / COLLECTIBLES_PER_ROOM);
	bool result = false;

	for(int i = 0; i < COLLECTIBLES_PER_ROOM; i++)
	{
		//get random position in room (x: start column + minDistanceToWall + random (width - minDistanceToWall * 2)
		posX = (room->topLeftIndex % m_dungeonData->dungeonWidth) + COLLECTIBLES_MIN_DISTANCE_TO_WALL + ((rand() % (int)(((widthPartition * i) - (COLLECTIBLES_MIN_DISTANCE_TO_WALL * 2)) * 100)) * 0.01f);
		
		//(z: start row + minDistanceToWall + random (length - minDistanceToWall * 2)
		posZ = (room->topLeftIndex / m_dungeonData->dungeonWidth) + COLLECTIBLES_MIN_DISTANCE_TO_WALL + ((rand() % (int)((room->length - (COLLECTIBLES_MIN_DISTANCE_TO_WALL * 2)) * 100)) * 0.01f);

		//create new model for collectible (no instance, since I want to delete them one by one when collected)
		ModelClass* newModel = new ModelClass;
		result = newModel->Initialize(d3d->GetDevice(), "data/sphere.txt", L"data/red.dds");
		if(!result)
			return false;
		newModel->SetPosition(posX, 1.0f, posZ);

		//create new collectible object, push it into the list and increment the counter of the total points
		DungeonGeneratorClass::CollectibleData* newCollectible = new DungeonGeneratorClass::CollectibleData;
		newCollectible->model = newModel;
		newCollectible->collisionRadius = 2.0f;

		newCollectible->posX = posX;
		newCollectible->posY = 0.0f;
		newCollectible->posZ = posZ;

		m_dungeonData->collectibles->push_back(newCollectible);
		m_totalPointAmount++;
	}

	return true;
}

//returns if collectible is at a specific position
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

DungeonGeneratorClass::DungeonData* DungeonGeneratorClass::GetDungeonData()
{
	return m_dungeonData;
}

//return the spawning position in the dungeon
void DungeonGeneratorClass::GetSpawningCoord(float& xPos, float& yPos, float& zPos)
{
	//get the center position at the first room
	xPos = (m_roomData[0]->topLeftIndex % m_dungeonData->dungeonWidth) + (m_roomData[0]->width * 0.5f);
	zPos = (m_roomData[0]->topLeftIndex / m_dungeonData->dungeonWidth) + (m_roomData[0]->length * 0.5f);
	yPos = 0.0f;
}