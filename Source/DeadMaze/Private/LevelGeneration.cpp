#include "LevelGeneration.h"
#include "Engine/World.h"       // For GetWorld()
#include "Engine/StaticMeshActor.h"

ALevelGeneration::ALevelGeneration()
{
	PrimaryActorTick.bCanEverTick = false; // No need to tick unless you want live updates
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called"));

	//fill world grid
	for (int i = 0; i < worldSize; i++)
	{
		for (int j = 0; j < worldSize; j++)
		{
			worldOccupancyGrid[i][j] = "walkable";
		}
	}
}

void ALevelGeneration::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called"));
	GenerateGrid();
	CreateRooms();
	MakeExits();
	CreatePassages();
	FillWalls();
	DecorateRooms();
}

void ALevelGeneration::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelGeneration::GenerateGrid()
{
	srand(time(NULL));
	UE_LOG(LogTemp, Warning, TEXT("Spawning key"));

	// Step 1: Fill with empty
	for (int y = 0; y < MAZESIZE; y++)
	{
		for (int x = 0; x < MAZESIZE; x++)
		{
			MazeGrid[y][x] = "Empty";
		}
	}

	// Step 2: Place entrance & exit
	/*Cell entrance = { 0, rand() % MAZESIZE };
	Cell exit = { MAZESIZE - 1, rand() % MAZESIZE };
	MazeGrid[entrance.y][entrance.x] = "Entrance";
	MazeGrid[exit.y][exit.x] = "Exit";*/

	Cell entrance = {0, 20};
	Cell exit = {MAZESIZE - 1, 0};
	MazeGrid[entrance.y][entrance.x] = "Entrance";
	MazeGrid[exit.y][exit.x] = "Exit";

	// Step 3: Create guaranteed path
	// Simple BFS to carve a straight but meandering path
	Cell current = entrance;
	while (!(current.x == exit.x && current.y == exit.y))
	{
		if (rand() % 2 == 0)
		{
			if (current.x < exit.x) current.x++;
			else if (current.x > exit.x) current.x--;
		}
		else
		{
			if (current.y < exit.y) current.y++;
			else if (current.y > exit.y) current.y--;
		}
		if (MazeGrid[current.y][current.x] == "Empty")
		{
			MazeGrid[current.y][current.x] = "Path";
		}
	}

	////
	/*UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh)
	{
		AStaticMeshActor* SphereActor = GetWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(),
			FVector(20 * 100.0f - 2000.0f, 0 * 100.0f - 1400.0f, 50.0f),
			FRotator::ZeroRotator
		);

		if (SphereActor)
		{
			SphereActor->GetStaticMeshComponent()->SetStaticMesh(SphereMesh);
			SphereActor->SetMobility(EComponentMobility::Movable);
			SphereActor->SetActorEnableCollision(false);
		}
	}*/


	// Step 4: Place keys randomly and carve paths to them
	int keysPlaced = 0;
	int maxAttempts = 1000; // Prevent infinite loops
	int attempts = 0;

	while (keysPlaced < 10 && attempts < maxAttempts)
	{
		attempts++;

		int keyX = rand() % MAZESIZE;
		int keyY = rand() % MAZESIZE;

		// Skip if cell is not empty
		if (MazeGrid[keyY][keyX] != "Empty") continue;

		// Find nearest path cell to carve from
		std::queue<Cell> q;
		bool visited[50][50] = {false};
		q.push({keyX, keyY});
		visited[keyY][keyX] = true;

		Cell nearestPath = {-1, -1};
		bool found = false;

		int dx[4] = {1, -1, 0, 0};
		int dy[4] = {0, 0, 1, -1};

		while (!q.empty() && !found)
		{
			Cell c = q.front();
			q.pop();

			for (int dir = 0; dir < 4; dir++)
			{
				int nx = c.x + dx[dir];
				int ny = c.y + dy[dir];

				if (nx < 0 || nx >= MAZESIZE || ny < 0 || ny >= MAZESIZE) continue;
				if (visited[ny][nx]) continue;

				visited[ny][nx] = true;

				if (MazeGrid[ny][nx] == "Path")
				{
					nearestPath = {nx, ny};
					found = true;
					break;
				}
				else if (MazeGrid[ny][nx] == "Empty")
				{
					q.push({nx, ny});
				}
			}
		}

		if (found)
		{
			// Carve path from nearestPath to key
			Cell c = nearestPath;
			while (!(c.x == keyX && c.y == keyY))
			{
				if (c.x < keyX) c.x++;
				else if (c.x > keyX) c.x--;
				else if (c.y < keyY) c.y++;
				else if (c.y > keyY) c.y--;

				if (MazeGrid[c.y][c.x] == "Empty")
				{
					MazeGrid[c.y][c.x] = "Path";
				}
			}

			MazeGrid[keyY][keyX] = "Key";
			keysPlaced++;
			UE_LOG(LogTemp, Warning, TEXT("Placed a key at (%d, %d)"), keyY, keyX);
		}
	}


	// Step 5: Add enemies far from entrance
	for (int y = 0; y < MAZESIZE; y++)
	{
		for (int x = 0; x < MAZESIZE; x++)
		{
			if (MazeGrid[y][x] == "Empty" && rand() % 20 == 0)
			{
				MazeGrid[y][x] = "Enemy";
			}
		}
	}

	// Step 6: Add random walls without breaking path
	for (int y = 0; y < MAZESIZE; y++)
	{
		for (int x = 0; x < MAZESIZE; x++)
		{
			if (MazeGrid[y][x] == "Empty" && rand() % 4 == 0)
			{
				if (IsCellReachableAfterWall(x, y, entrance, exit))
				{
					MazeGrid[y][x] = "Wall";
				}
			}
		}
	}

	// Optional: Convert "Path" to "Empty" for normal floor
	for (int y = 0; y < MAZESIZE; y++)
	{
		for (int x = 0; x < MAZESIZE; x++)
		{
			if (MazeGrid[y][x] == "Path")
			{
				MazeGrid[y][x] = "Empty";
			}
		}
	}

	//if (WallClass)
	//{
	//    // Example: spawn a wall at (0,0,0)
	//    //GetWorld()->SpawnActor<AActor>(WallClass, FVector(0, 0, 0), FRotator::ZeroRotator);
	//}

	if (WallClass)
	{
		for (size_t i = 0; i < MAZESIZE; i++)
		{
			for (size_t j = 0; j < MAZESIZE; j++)
			{
				if (MazeGrid[i][j] == "Wall")
				{
					float keyPosX = i * 1.0f;
					float keyPosZ = j * 1.0f;
					float TileSize = 100.0f; // 1 meter per tile
					FVector SpawnLocation = FVector(j * TileSize - 2000.0f, i * TileSize - 1400.0f, 50.0f);
					// Z is up in Unreal
					GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, FRotator::ZeroRotator);

					//UE_LOG(LogTemp, Warning, TEXT("Spawning wall at (%d, %d)"), i, j);
					if (rand() % 6 == 0)
					{
						GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation + FVector(0, 0, 100.0f),
						                               FRotator::ZeroRotator);
						GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation + FVector(0, 0, 200.0f),
						                               FRotator::ZeroRotator);
					}
				}
			}
		}
	}
	if (KeyClass)
	{
		for (size_t i = 0; i < MAZESIZE; i++)
		{
			for (size_t j = 0; j < MAZESIZE; j++)
			{
				if (MazeGrid[i][j] == "Key")
				{
					float keyPosX = i * 1.0f;
					float keyPosZ = j * 1.0f;
					float TileSize = 100.0f; // 1 meter per tile
					FVector SpawnLocation = FVector(j * TileSize - 2000.0f, i * TileSize - 1400.0f, 100.0f);
					// Z is up in Unreal
					GetWorld()->SpawnActor<AActor>(KeyClass, SpawnLocation, FRotator::ZeroRotator);

					UE_LOG(LogTemp, Warning, TEXT("Spawning key at (%d, %d)"), i, j);
				}
			}
		}
		// Example: spawn a key at (200,0,0)
		//GetWorld()->SpawnActor<AActor>(KeyClass, FVector(200, 0, 0), FRotator::ZeroRotator);

		//GetWorld()->SpawnActor<AActor>(KeyClass, FVector(400, 200.0f, 200.0f), FRotator::ZeroRotator);
	}

	if (TreasureClass)
	{
		int numVasePlacingAttempts = 0;
		int maxVasePlacingAttempts = 50;
		int numPlacedVases = 0;

		while (numVasePlacingAttempts < maxVasePlacingAttempts && numPlacedVases < 4)
		{
			int xCoordinate = rand() % MAZESIZE;
			int yCoordinate = rand() % MAZESIZE;

			if (MazeGrid[xCoordinate][yCoordinate] == "Empty")
			{
				numPlacedVases++;

				float TileSize = 100.0f; // 1 meter per tile
				FVector SpawnLocation = FVector(yCoordinate * TileSize - 2000.0f, xCoordinate * TileSize - 1400.0f,
				                                100.0f); // Z is up in Unreal
				GetWorld()->SpawnActor<AActor>(TreasureClass, SpawnLocation, FRotator::ZeroRotator);

				//UE_LOG(LogTemp, Warning, TEXT("Spawning treasure at (%d, %d)"), i, j);
			}

			numVasePlacingAttempts++;
		}

		// Example: spawn a key at (200,0,0)
		//GetWorld()->SpawnActor<AActor>(KeyClass, FVector(200, 0, 0), FRotator::ZeroRotator);

		//GetWorld()->SpawnActor<AActor>(KeyClass, FVector(400, 200.0f, 200.0f), FRotator::ZeroRotator);
	}

	//if (PlayerBeginPos)
	//{
	//	FActorSpawnParameters SpawnParams;
	//	FVector Location(100.f, 200.f, 300.f);   // your custom coords
	//	FRotator Rotation(0.f, 90.f, 0.f);

	//	APlayerStart* SpawnedPlayerStart = GetWorld()->SpawnActor<APlayerStart>(
	//		PlayerBeginPos, Location, Rotation, SpawnParams
	//	);
	//}
	//if (TreasureClass)
	//{
	//    // Example: spawn a treasure at (400,0,0)
	//    //GetWorld()->SpawnActor<AActor>(TreasureClass, FVector(400, 0, 0), FRotator::ZeroRotator);
	//}

	//FVector playerLocation = FVector(-2000.0f, 20 * 100.0f - 1400.0f, 100.0f); // Z is up in Unreal   // your custom coords
	//FRotator playerRotation(0.f, 90.f, 0.f);
	//PlayerBeginPos->SetActorLocation()

	/*FVector CustomSpawnLocation(100.f, 200.f, 300.f);
	FRotator CustomSpawnRotation(0.f, 90.f, 0.f);
	FActorSpawnParameters SpawnParams;

	  ACharacter* newCharacter = GetWorld()->SpawnActor<ACharacter>(
		DefaultPawnClass, CustomSpawnLocation, CustomSpawnRotation, SpawnParams
	);*/

	// Pick a location and rotation for spawning
}

bool ALevelGeneration::IsCellReachableAfterWall(int wx, int wy, Cell entrance, Cell exit)
{
	// Temporarily set wall
	FString oldValue = MazeGrid[wy][wx];
	MazeGrid[wy][wx] = "Wall";

	// BFS check
	bool visited[50][50] = {false};
	std::queue<Cell> q;
	q.push(entrance);
	visited[entrance.y][entrance.x] = true;

	int dx[4] = {1, -1, 0, 0};
	int dy[4] = {0, 0, 1, -1};

	bool exitFound = false;

	while (!q.empty())
	{
		Cell c = q.front();
		q.pop();
		if (c.x == exit.x && c.y == exit.y)
		{
			exitFound = true;
			break;
		}
		for (int dir = 0; dir < 4; dir++)
		{
			int nx = c.x + dx[dir];
			int ny = c.y + dy[dir];
			if (nx < 0 || nx >= MAZESIZE || ny < 0 || ny >= MAZESIZE) continue;
			if (!visited[ny][nx] && MazeGrid[ny][nx] != "Wall")
			{
				visited[ny][nx] = true;
				q.push({nx, ny});
			}
		}
	}

	// Restore old value
	MazeGrid[wy][wx] = oldValue;
	return exitFound;
}

bool ALevelGeneration::CheckRoomOverlap(const Room& extraRoom) const
{
	//int roomSize;
	for (int j = 0; j < generatedRooms.size(); j++)
	{
		bool yOverlapping = false;
		bool xOverlapping = false;

		//get locations and room sizes
		FVector generatedRoomLocation = generatedRooms[j].GetRoomWorldLocation();
		FVector extraRoomLocation = extraRoom.GetRoomWorldLocation();

		auto [extraRoomSizeX, extraRoomSizeY] = extraRoom.GetRoomSize();
		auto [generatedRoomSizeX, generatedRoomSizeY] = generatedRooms[j].GetRoomSize();

		//scale to centimeters ??
		extraRoomSizeX *= 100.0f;
		extraRoomSizeY *= 100.0f;
		generatedRoomSizeX *= 100.0f;
		generatedRoomSizeY *= 100.0f;

		//check overlapping for y and x axes

		if (extraRoomLocation.X + extraRoomSizeX / 2.0f > generatedRoomLocation.X - generatedRoomSizeX / 2.0f
			&& extraRoomLocation.X - extraRoomSizeX / 2.0f < generatedRoomLocation.X + generatedRoomSizeX / 2.0f)
		{
			xOverlapping = true;
			UE_LOG(LogTemp, Warning, TEXT("x overlap!"));
		}

		if (extraRoomLocation.Y + extraRoomSizeY / 2.0f > generatedRoomLocation.Y - generatedRoomSizeY / 2.0f
			&& extraRoomLocation.Y - extraRoomSizeX / 2.0f < generatedRoomLocation.Y + generatedRoomSizeY / 2.0f)
		{
			yOverlapping = true;
			UE_LOG(LogTemp, Warning, TEXT("y overlap!"));
		}

		if (yOverlapping && xOverlapping)
		{
			yOverlapping = false;
			xOverlapping = false;
			return true;
		}
	}
	return false;
}

void ALevelGeneration::CreateRooms()
{
	//random initialize
	srand(time(NULL));
	std::random_device rd; // Seed
	std::mt19937 gen(rd()); // Mersenne Twister engine

	//room amount from 1-3
	int numExtraRooms = rand() % 2 + 1;
	//int numExtraRooms = 1;

	// Define float distributions
	std::uniform_real_distribution<float> distribution(-WORLDLOCATIONOFFSET / 2.0f + 2000.0f,
	                                                   WORLDLOCATIONOFFSET / 2.0f - 2000.0f);
	float worldLocationX = distribution(gen);
	float worldLocationY = distribution(gen);

	//snap to even meters
	auto SnapToEvenMeter = [](float valueCm) -> float
	{
		// Convert to meters
		float meters = valueCm / 100.0f;

		// Round to nearest even integer (meters)
		int roundedMeters = FMath::RoundToInt(meters);

		// Convert back to cm
		return roundedMeters * 100.0f;
	};

	worldLocationX = SnapToEvenMeter(worldLocationX);
	worldLocationY = SnapToEvenMeter(worldLocationY);

	//create the entrance room first, it has an entrance, exit to next level and a third exit to another room, sometimes a fourth exit
	Room entranceRoom(numExtraRooms > 1 ? RoomType::FourExit : RoomType::ThreeExit, 34.0f, 34.0f, -300.0f, 200.0f);

	//mesh creation and reference
	AActor* roomRef = GetWorld()->SpawnActor<AActor>(
		entranceRoom.m_roomType == RoomType::FourExit ? RoomFourExit : RoomThreeExit,
		entranceRoom.GetRoomWorldLocation(),
		FRotator::ZeroRotator);
	//scale the room to make rooms of different sizes

	auto [meshScaleX, meshScaleY] = entranceRoom.GetRoomSize();
	roomRef->SetActorScale3D(FVector((meshScaleX * 100.0f) / DEFAULTROOMSIZE, (meshScaleY * 100.0f) / DEFAULTROOMSIZE,
	                                 1.0f));

	//reserve room space
	ReserveRoomSpace(entranceRoom);

	entranceRoom.m_roomMesh = roomRef;

	generatedRooms.emplace_back(entranceRoom);

	//create the extra rooms!
	for (int i = 0; i < numExtraRooms; i++)
	{
	redoPoint:
		int roomSizeX = rand() % 20 + 20;
		int roomSizeY = rand() % 20 + 20;

		// worldlocation snap
		float extraRoomWorldLocationX = distribution(gen);
		float extraRoomWorldLocationY = distribution(gen);

		extraRoomWorldLocationX = SnapToEvenMeter(extraRoomWorldLocationX);
		extraRoomWorldLocationY = SnapToEvenMeter(extraRoomWorldLocationY);

		//number of exits
		int roomTypeIndex = rand() % 2 + 1;

		Room extraRoom(static_cast<RoomType>(roomTypeIndex), static_cast<float>(roomSizeX), static_cast<
			               float>(roomSizeY), extraRoomWorldLocationX, extraRoomWorldLocationY);

		//check overlapping of rooms

		if (CheckRoomOverlap(extraRoom))
		{
			UE_LOG(LogTemp, Warning, TEXT("Going to redo point!"));
			goto redoPoint;
		}

		AActor* extraRoomRef = nullptr;

		switch (roomTypeIndex)
		{
		case 1:
			extraRoomRef = GetWorld()->SpawnActor<AActor>(RoomOneExit, extraRoom.GetRoomWorldLocation(),
			                                              FRotator::ZeroRotator);
			break;
		case 2:
			extraRoomRef = GetWorld()->SpawnActor<AActor>(RoomTwoExit, extraRoom.GetRoomWorldLocation(),
			                                              FRotator::ZeroRotator);
			break;
		default:
			extraRoomRef = GetWorld()->SpawnActor<AActor>(RoomOneExit, extraRoom.GetRoomWorldLocation(),
			                                              FRotator::ZeroRotator);
		}

		//scaling
		auto [extraRoomMeshScaleX, extraRoomMeshScaleY] = extraRoom.GetRoomSize();

		if (roomTypeIndex == 2)
		{
			extraRoomRef->SetActorScale3D(FVector((extraRoomMeshScaleY * 100.0f) / DEFAULTROOMSIZE,
											  (extraRoomMeshScaleX * 100.0f) / DEFAULTROOMSIZE, 1.0f));
		}

		else if (roomTypeIndex == 1)
		{
			extraRoomRef->SetActorScale3D(FVector((extraRoomMeshScaleX * 100.0f) / DEFAULTROOMSIZE,
											  (extraRoomMeshScaleY * 100.0f) / DEFAULTROOMSIZE, 1.0f));
		}

		//reserve room space
		ReserveRoomSpace(extraRoom);

		extraRoom.m_roomMesh = extraRoomRef;

		UE_LOG(LogTemp, Warning, TEXT("The integer value is: %d....%d"), roomSizeX, roomSizeY);
		generatedRooms.emplace_back(extraRoom);
	}
}

void ALevelGeneration::CreatePassages()
{
	Cell startCell = WorldToGrid(generatedRooms[0].GetExitExactLocation(0));
	Cell goalCell = WorldToGrid(generatedRooms[1].GetExitExactLocation(0));

	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh)
	{
		AStaticMeshActor* SphereActor = GetWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), generatedRooms[0].GetExitExactLocation(0),
			FRotator::ZeroRotator
		);

		if (SphereActor)
		{
			SphereActor->GetStaticMeshComponent()->SetStaticMesh(SphereMesh);
			SphereActor->SetMobility(EComponentMobility::Movable);
			SphereActor->SetActorEnableCollision(false);
		}
	}

	auto path = FindCorridorPath(startCell, goalCell);

	for (auto& loc : path)
	{
		//GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, loc, FRotator::ZeroRotator);
	}

	/*if (!PassageBlock1CubicMeter) return;

	for (int i = 0; i < generatedRooms.size() - 1; i++)
	{
		FVector start = generatedRooms[i].GetExitExactLocation(0);
		FVector end   = generatedRooms[i+1].GetExitExactLocation(0);

		FVector current = start;

		float stepSize = 100.0f; // 1 meter
		int numOfTries = 0;
		int numTriesY = 0;

		// Move along X
		while (FMath::Abs(current.X - end.X) > stepSize && numOfTries < 500)
		{
			if (generatedRooms[i + 1].CheckRoomBounds(current.X, current.Y) || generatedRooms[i].CheckRoomBounds(current.X, current.Y))
			{
				current.Y += (end.Y > current.Y) ? stepSize : -stepSize;
				GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, current, FRotator::ZeroRotator);
				numOfTries++;

				if (numOfTries > 450)
				{
					UE_LOG(LogTemp, Warning, TEXT("MANY TRIES!!"));
				}
			}

			else
			{
				current.X += (end.X > current.X) ? stepSize : -stepSize;
				GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, current, FRotator::ZeroRotator);
			}
			
		}

		// Move along Y
		while (FMath::Abs(current.Y - end.Y) > stepSize && numTriesY < 500)
		{
			if (generatedRooms[i + 1].CheckRoomBounds(current.X, current.Y))
			{
				current.X += (end.X > current.X) ? stepSize : -stepSize;
				GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, current, FRotator::ZeroRotator);
				numTriesY++;

				if (numTriesY > 450)
				{
					UE_LOG(LogTemp, Warning, TEXT("MANY TRIES Y!"));
				}
			}

			else
			{
				current.Y += (end.Y > current.Y) ? stepSize : -stepSize;
				GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, current, FRotator::ZeroRotator);
			}
			
		}
	}*/
}

void ALevelGeneration::ReserveRoomSpace(const Room& room)
{
	int centerPointX = (room.GetRoomWorldLocation().X / 100.0f) + 75;
	int centerPointY = (room.GetRoomWorldLocation().Y / 100.0f) + 75;

	FVector center = FVector(centerPointX * 100.0f - 75.0f * 100.0f, centerPointY * 100.0f - 75.0f * 100.0f, 100.0f);
	GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, center, FRotator::ZeroRotator);

	auto [roomSizeX, roomSizeY] = room.GetRoomSize();
	//roomSizeX *= 100.0f;
	//roomSizeY *= 100.0f;
	// float newRoomSizeX = roomSizeX * 100.0f / DEFAULTROOMSIZE;
	// float newRoomSizeY = roomSizeY * 100.0f / DEFAULTROOMSIZE;
	// roomSizeX /= newRoomSizeX;
	// roomSizeY /= newRoomSizeY;

	for (int i = 0; i < worldSize; i++)
	{
		if (i > centerPointY - roomSizeY / 2.0f - 1 && i < centerPointY + roomSizeY / 2.0f + 1)
		{
			for (int j = 0; j < worldSize; j++)
			{
				if (j > centerPointX - roomSizeX / 2.0f - 1 && j < centerPointX + roomSizeX / 2.0f + 1)
				{
					worldOccupancyGrid[i][j] = "Reserved";
					int xPos = i * 100.0f - 75.0f * 100.0f;
					int yPos = j * 100.0f - 75.0f * 100.0f;
					FVector worldPosition = FVector(yPos, xPos, 100.0f);

					
                    
				}
			}
		}
	}
}

std::vector<FVector> ALevelGeneration::FindCorridorPath(Cell start, Cell end)
{
	std::vector<FVector> path;

	//reserve entrance
	worldOccupancyGrid[start.y - 1][start.x] = "exitReserve";
	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh)
	{
		AStaticMeshActor* SphereActor = GetWorld()->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), FVector((start.x) * 100.0f - 75.0f * 100.0f, (start.y - 1) * 100.0f - 75.0f * 100.0f, 100.0f),
			FRotator::ZeroRotator
		);

		if (SphereActor)
		{
			SphereActor->GetStaticMeshComponent()->SetStaticMesh(SphereMesh);
			SphereActor->SetMobility(EComponentMobility::Movable);
			SphereActor->SetActorEnableCollision(false);

			SphereActor->SetActorScale3D(FVector(1.2f, 1.2f, 1.2f));
		}
	}

	// Directions (up, down, left, right)
	const int dx[4] = {1, -1, 0, 0};
	const int dy[4] = {0, 0, 1, -1};

	auto heuristic = [&](int x, int y)
	{
		return FMath::Abs(x - end.x) + FMath::Abs(y - end.y);
	};

	struct Node
	{
		int x, y;
		float g, h;
		Node* parent;
	};

	auto cmp = [](Node* a, Node* b)
	{
		return (a->g + a->h) > (b->g + b->h);
	};

	std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> openSet(cmp);
	std::vector<std::vector<bool>> closed(worldSize, std::vector<bool>(worldSize, false));

	Node* startNode = new Node{start.x, start.y, 0, (float)heuristic(start.x, start.y), nullptr};
	openSet.push(startNode);

	Node* endNode = nullptr;

	while (!openSet.empty())
	{
		Node* current = openSet.top();
		openSet.pop();

		if (current->x == end.x && current->y == end.y)
		{
			endNode = current;
			break;
		}

		if (closed[current->y][current->x])
			continue;

		closed[current->y][current->x] = true;

		for (int dir = 0; dir < 4; dir++)
		{
			int nx = current->x + dx[dir];
			int ny = current->y + dy[dir];

			// Bounds check
			if (nx < 0 || nx >= worldSize || ny < 0 || ny >= worldSize)
				continue;

			// Skip reserved tiles
			if (worldOccupancyGrid[ny][nx] == "Reserved")
				continue;
			if (worldOccupancyGrid[ny][nx] == "ExitBlockReserved")
			{
				UE_LOG(LogTemp, Warning, TEXT("BLOCK RESERVE %u   %u"), ny, nx);
				continue;
			}
				

			if (!closed[ny][nx])
			{
				Node* neighbor = new Node{nx, ny, current->g + 1, (float)heuristic(nx, ny), current};
				openSet.push(neighbor);
			}
		}
	}

	// Reconstruct path
	if (endNode)
	{
		Node* current = endNode;
		while (current != nullptr)
		{
			int wx = current->x * 100.0f - 75.0f * 100.0f;
			int wy = current->y * 100.0f - 75.0f * 100.0f;

			FVector worldPos(wy, wx, 100.0f);
			path.push_back(worldPos);

			// Mark as "path" in world grid
			worldOccupancyGrid[current->y][current->x] = "Path";

			FVector fLoc = FVector(wx, wy, 0.f);
			//GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, fLoc, FRotator::ZeroRotator);

			// Add bordering walls
			for (int dir = 0; dir < 4; dir++)
			{
				int nx = current->x + dx[dir];
				int ny = current->y + dy[dir];

				if (nx < 0 || nx >= worldSize || ny < 0 || ny >= worldSize)
					continue;

				if (worldOccupancyGrid[ny][nx] == "walkable")
				{
					worldOccupancyGrid[ny][nx] = "Wall";

					int wallWx = nx * 100.0f - 75.0f * 100.0f;
					int wallWy = ny * 100.0f - 75.0f * 100.0f;
					FVector wallPos(wallWx, wallWy, 0);
				}
			}

			current = current->parent;
		}

		std::reverse(path.begin(), path.end());
	}

	return path;
}

void ALevelGeneration::FillWalls() const
{
	if (PassageBlock1CubicMeter2)
	{
		for (int y = 0; y < worldSize; y++)
		{
			for (int j = 0; j < worldSize; j++)
			{
				if (worldOccupancyGrid[y][j] == "Wall")
				{
					FVector wallPos = FVector(j * 100.0f - 75.0f * 100.0f, y * 100.0f - 75.0f * 100.0f, 0.f);
					GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter2, wallPos, FRotator::ZeroRotator);
				}
			}
		}
	}
}

void ALevelGeneration::DecorateRooms()
{
	for (int y = 0; y < generatedRooms.size(); y++)
	{
		FVector roomLoc = generatedRooms[y].GetRoomWorldLocation();
		auto [roomSizeX, roomSizeY] = generatedRooms[y].GetRoomSize();

		//north wall
		int numberOfvaultsNorth = roomSizeY / 8.0f;

		for (int x = 0; x < numberOfvaultsNorth; x++)
		{
			FVector vaultLoc = FVector(roomLoc.X + roomSizeX * 100.0f / 2.0f,
			                           roomLoc.Y - roomSizeY * 100.0f / 2.0f + 450.0f + x * 800.0f, 350.0f);
			AActor* vaultMesh = GetWorld()->SpawnActor<AActor>(BankVaultSystem, vaultLoc, FRotator(0, 0, 270.0f));

			vaultMesh->SetActorScale3D(FVector(2.7f, 2.7f, 2.7f));
			vaultMesh->SetActorRotation(FRotator(90.0f, 0, 270.0f));
		}

		//south wall
		int numberOfvaultsSouth = roomSizeY / 8.0f;

		for (int x = 0; x < numberOfvaultsSouth; x++)
		{
			FVector vaultLoc = FVector(roomLoc.X - roomSizeX * 100.0f / 2.0f,
			                           roomLoc.Y - roomSizeY * 100.0f / 2.0f + 450.0f + x * 800.0f, 350.0f);
			AActor* vaultMesh = GetWorld()->SpawnActor<AActor>(BankVaultSystem, vaultLoc, FRotator(0, 0, 270.0f));

			vaultMesh->SetActorScale3D(FVector(2.7f, 2.7f, 2.7f));
			vaultMesh->SetActorRotation(FRotator(90.0f, 180.0f, 270.0f));
		}
	}
}

void ALevelGeneration::MakeExits()
{
	for (int y = 0; y < generatedRooms.size(); y++)
	{
		//exit reserves for blocks
		Cell ExitBlockArea = WorldToGrid(generatedRooms[y].GetExitExactLocation(0));
		ExitBlockArea.x += 2;
		ExitBlockArea.y -= 1;

		worldOccupancyGrid[ExitBlockArea.y][ExitBlockArea.x] = "ExitBlockReserved";
		FVector blockLoc(ExitBlockArea.x * 100.0f - 75.0f * 100.0f, ExitBlockArea.y * 100.0f - 75.0f * 100.0f, 100.0f);
		GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, blockLoc, FRotator::ZeroRotator);

		ExitBlockArea.x -= 4;
		worldOccupancyGrid[ExitBlockArea.y][ExitBlockArea.x] = "ExitBlockReserved";
		FVector blockLoc2(ExitBlockArea.x * 100.0f - 75.0f * 100.0f, ExitBlockArea.y * 100.0f - 75.0f * 100.0f, 100.0f);
		GetWorld()->SpawnActor<AActor>(PassageBlock1CubicMeter, blockLoc2, FRotator::ZeroRotator);
	}
	
}


Cell ALevelGeneration::WorldToGrid(const FVector& worldLocation) const
{
	int xGridPoint = worldLocation.X / 100.0f + 75;
	int yGridPoint = worldLocation.Y / 100.0f + 75;

	Cell gridPointCell;
	gridPointCell.x = xGridPoint;
	gridPointCell.y = yGridPoint;

	return gridPointCell;
}
