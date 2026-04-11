// Fill out your copyright notice in the Description page of Project Settings.


#include "GridGenerator.h"

// Sets default values
AGridGenerator::AGridGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGridGenerator::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGridGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AGridGenerator::GenerateGrid(FVector gridOffset)
{
	srand(time(NULL));

	// Step 1: Fill with empty
	for (int y = 0; y < MAZESIZE; y++)
	{
		for (int x = 0; x < MAZESIZE; x++)
		{
			MazeGrid[y][x] = "Empty";
		}
	}

	// Step 2: Place entrance & exit

	Cell entrance = {7, 17};
	Cell exit = {7, 0};
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
	
	// Step 4: Place keys randomly and carve paths to them
	// Step 4: Place keys randomly and carve paths to them
	int keysPlaced = 0;
	int maxAttempts = 1000; // Prevent infinite loops
	int attempts = 0;

	const int MinKeyDistance = 6;

	TArray<Cell> PlacedKeys;

	while (keysPlaced < 8 && attempts < maxAttempts)
	{
		attempts++;

		int keyX = rand() % MAZESIZE;
		int keyY = rand() % MAZESIZE;

		// Skip if cell is not empty
		if (MazeGrid[keyY][keyX] != "Empty") continue;

		// Ensure minimum distance from other keys
		bool tooClose = false;
		for (const Cell& existingKey : PlacedKeys)
		{
			int dist = abs(existingKey.x - keyX) + abs(existingKey.y - keyY); // Manhattan distance
			if (dist < MinKeyDistance)
			{
				tooClose = true;
				break;
			}
		}

		if (tooClose) continue;

		// Find nearest path cell to carve from
		std::queue<Cell> q;
		bool visited[50][50] = { false };

		q.push({ keyX, keyY });
		visited[keyY][keyX] = true;

		Cell nearestPath = { -1, -1 };
		bool found = false;

		int dx[4] = { 1, -1, 0, 0 };
		int dy[4] = { 0, 0, 1, -1 };

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
					nearestPath = { nx, ny };
					found = true;
					break;
				}
				else if (MazeGrid[ny][nx] == "Empty")
				{
					q.push({ nx, ny });
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

			PlacedKeys.Add({ keyX, keyY }); // Store key position
			keysPlaced++;

			UE_LOG(LogTemp, Warning, TEXT("Placed a key at (%d, %d)"), keyY, keyX);
		}
	}
	
	// Step 5: Place enemies with spacing rules

	int maxEnemies = 4;
	int minDistanceFromEntrance = 7;   // tweak
	int minDistanceBetweenEnemies = 5; // tweak

	TArray<FIntPoint> placedEnemies;

	for (int y = 0; y < MAZESIZE; y++)
	{
		for (int x = 0; x < MAZESIZE; x++)
		{
			if (placedEnemies.Num() >= maxEnemies)
				break;

			if (MazeGrid[y][x] != "Empty")
				continue;

			// Distance from entrance
			int distFromEntrance = abs(x - entrance.x) + abs(y - entrance.y);
			if (distFromEntrance < minDistanceFromEntrance)
				continue;

			// Distance from other enemies
			bool tooClose = false;
			for (const FIntPoint& enemyPos : placedEnemies)
			{
				int dist = abs(x - enemyPos.X) + abs(y - enemyPos.Y);
				if (dist < minDistanceBetweenEnemies)
				{
					tooClose = true;
					break;
				}
			}

			if (tooClose)
				continue;

			// Small random chance so we don't fill everything valid
			if (rand() % 4 != 0)
				continue;

			MazeGrid[y][x] = "Enemy";
			placedEnemies.Add(FIntPoint(x, y));
		}
	}

	// Step 6: Add wall segments (corridor style)
	int numWallSegments = 7;  // tweak this
	int minLength = 4;
	int maxLength = 14;

	int regionCount = 5; // 4x4 regions
	int regionSize = MAZESIZE / regionCount;

	// 4 possible directions
	FIntPoint directions[4] =
	{
		FIntPoint(1, 0),   // right
		FIntPoint(-1, 0),  // left
		FIntPoint(0, 1),   // down
		FIntPoint(0, -1)   // up
	};

	for (int h = 0; h < numWallSegments; h++)
	{
		for (int ry = 0; ry < regionCount; ry++)
		{
			for (int rx = 0; rx < regionCount; rx++)
			{
				int regionStartX = rx * regionSize;
				int regionStartY = ry * regionSize;

				// Pick a random position INSIDE this region
				int startX = regionStartX + rand() % regionSize;
				int startY = regionStartY + rand() % regionSize;

				if (MazeGrid[startY][startX] != "Empty")
					continue;
				if (HasNearbyWall(startX, startY, 1))
					continue;

				int x = startX;
				int y = startY;

				FIntPoint dir = directions[rand() % 4];
				int length = minLength + rand() % (maxLength - minLength + 1);

				// Chance to turn (adjust this)
				float turnChance = 0.40f; // 25%
				bool hasTurned = false;

				for (int i = 0; i < length; i++)
				{
					if (x < 0 || x >= MAZESIZE || y < 0 || y >= MAZESIZE)
						break;

					if (MazeGrid[y][x] != "Empty")
						break;

					MazeGrid[y][x] = "Wall";

					// Random 90-degree turn
					if ((float)rand() / RAND_MAX < turnChance)
					{
						// Perpendicular directions
						if (dir.X != 0) // currently horizontal
						{
							dir = (rand() % 2 == 0)
								? FIntPoint(0, 1)
								: FIntPoint(0, -1);
						}
						else // currently vertical
						{
							dir = (rand() % 2 == 0)
								? FIntPoint(1, 0)
								: FIntPoint(-1, 0);
						}

						hasTurned = true;
					}

					x += dir.X;
					y += dir.Y;
				}
			}
		}
	}

	/*for (int s = 0; s < numWallSegments; s++)
	{
		int startX = rand() % MAZESIZE;
		int startY = rand() % MAZESIZE;

		if (MazeGrid[startY][startX] != "Empty")
			continue;

		// Pick direction: 0=horizontal, 1=vertical
		bool horizontal = rand() % 2 == 0;

		int length = minLength + rand() % (maxLength - minLength + 1);

		for (int i = 0; i < length; i++)
		{
			int x = startX + (horizontal ? i : 0);
			int y = startY + (horizontal ? 0 : i);

			if (x < 0 || x >= MAZESIZE || y < 0 || y >= MAZESIZE)
				break;

			if (MazeGrid[y][x] != "Empty")
				break;

			/*if (!IsCellReachableAfterWall(x, y, entrance, exit))
				break;#1#

			MazeGrid[y][x] = "Wall";
		}
	}*/

	// Optional: Convert "Path" to "Empty" for normal floor
	// for (int y = 0; y < MAZESIZE; y++)
	// {
	// 	for (int x = 0; x < MAZESIZE; x++)
	// 	{
	// 		if (MazeGrid[y][x] == "Path")
	// 		{
	// 			MazeGrid[y][x] = "Empty";
	// 		}
	// 	}
	// }

	//-----Place Actors-----//
	

	if (WallClass)
	{
		PlaceActorsInGrid(gridOffset, "Wall", WallClass, 50.0f);
	}
	
	if (KeyClass)
	{
		PlaceActorsInGrid(gridOffset, "Key", KeyClass, 150.0f);
		
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
				FVector SpawnLocation = FVector(yCoordinate * TileSize - 1700.0f, xCoordinate * TileSize - 1300.0f,
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

	if (PlantClass)
	{
		int numPlantPlacingAttempts = 0;
		int maxPlantPlacingAttempts = 50;
		int numPlacedPlants = 0;

		while (numPlantPlacingAttempts < maxPlantPlacingAttempts && numPlacedPlants < 4)
		{
			int xCoordinate = rand() % MAZESIZE;
			int yCoordinate = rand() % MAZESIZE;

			if (MazeGrid[xCoordinate][yCoordinate] == "Empty")
			{
				numPlacedPlants++;

				float TileSize = 100.0f; // 1 meter per tile
				FVector SpawnLocation = FVector(yCoordinate * TileSize - 1700.0f, xCoordinate * TileSize - 1300.0f,
												30.0f); // Z is up in Unreal
				GetWorld()->SpawnActor<AActor>(PlantClass, SpawnLocation, FRotator::ZeroRotator);

				//UE_LOG(LogTemp, Warning, TEXT("Spawning treasure at (%d, %d)"), i, j);
			}

			numPlantPlacingAttempts++;
		}
	}

	if (EnemyClass)
	{
		PlaceActorsInGrid(gridOffset, "Enemy", EnemyClass, 50.0f);
	}
	
}

bool AGridGenerator::IsCellReachableAfterWall(int wx, int wy, Cell entrance, Cell exit)
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

bool AGridGenerator::HasNearbyWall(int x, int y, int radius) const
{
	for (int dy = -radius; dy <= radius; dy++)
	{
		for (int dx = -radius; dx <= radius; dx++)
		{
			int nx = x + dx;
			int ny = y + dy;

			if (nx >= 0 && nx < MAZESIZE &&
				ny >= 0 && ny < MAZESIZE)
			{
				if (MazeGrid[ny][nx] == "Wall")
					return true;
			}
		}
	}
	return false;
}

void AGridGenerator::PlaceActorsInGrid(const FVector& gridOffset, const FString& objectClass, UClass* placedActor, float zOffset) const
{
	for (size_t i = 0; i < MAZESIZE; i++)
	{
		for (size_t j = 0; j < MAZESIZE; j++)
		{
			if (MazeGrid[i][j] == objectClass)
			{
				float TileSize = 100.0f; // 1 meter per tile

				FActorSpawnParameters SpawnParams;
				SpawnParams.SpawnCollisionHandlingOverride =
					ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				
				FVector SpawnLocation = FVector(j * TileSize + gridOffset.X, i * TileSize + gridOffset.Y, zOffset);
				// Z is up in Unreal
				GetWorld()->SpawnActor<AActor>(placedActor, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
				
			}
		}
	}
}

