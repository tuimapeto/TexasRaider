#include "LevelGeneration.h"
#include "Engine/World.h"       // For GetWorld()
#include "Engine/StaticMeshActor.h"

ALevelGeneration::ALevelGeneration()
{
	PrimaryActorTick.bCanEverTick = false; // No need to tick unless you want live updates
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called"));
}

void ALevelGeneration::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay called"));
	GenerateLevel();
}

void ALevelGeneration::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ALevelGeneration::GenerateLevel()
{

	srand(time(NULL));
	UE_LOG(LogTemp, Warning, TEXT("Spawning key"));

	// Step 1: Fill with empty
	for (int y = 0; y < MAZESIZE; y++) {
		for (int x = 0; x < MAZESIZE; x++) {
			MazeGrid[y][x] = "Empty";
		}
	}

	// Step 2: Place entrance & exit
	/*Cell entrance = { 0, rand() % MAZESIZE };
	Cell exit = { MAZESIZE - 1, rand() % MAZESIZE };
	MazeGrid[entrance.y][entrance.x] = "Entrance";
	MazeGrid[exit.y][exit.x] = "Exit";*/

	Cell entrance = { 0, 20};
	Cell exit = { MAZESIZE - 1, 0 };
	MazeGrid[entrance.y][entrance.x] = "Entrance";
	MazeGrid[exit.y][exit.x] = "Exit";

	// Step 3: Create guaranteed path
	// Simple BFS to carve a straight but meandering path
	Cell current = entrance;
	while (!(current.x == exit.x && current.y == exit.y)) {
		if (rand() % 2 == 0) {
			if (current.x < exit.x) current.x++;
			else if (current.x > exit.x) current.x--;
		}
		else {
			if (current.y < exit.y) current.y++;
			else if (current.y > exit.y) current.y--;
		}
		if (MazeGrid[current.y][current.x] == "Empty") {
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
		bool visited[50][50] = { false };
		q.push({ keyX, keyY });
		visited[keyY][keyX] = true;

		Cell nearestPath = { -1, -1 };
		bool found = false;

		int dx[4] = { 1, -1, 0, 0 };
		int dy[4] = { 0, 0, 1, -1 };

		while (!q.empty() && !found) {
			Cell c = q.front(); q.pop();

			for (int dir = 0; dir < 4; dir++) {
				int nx = c.x + dx[dir];
				int ny = c.y + dy[dir];

				if (nx < 0 || nx >= MAZESIZE || ny < 0 || ny >= MAZESIZE) continue;
				if (visited[ny][nx]) continue;

				visited[ny][nx] = true;

				if (MazeGrid[ny][nx] == "Path") {
					nearestPath = { nx, ny };
					found = true;
					break;
				}
				else if (MazeGrid[ny][nx] == "Empty") {
					q.push({ nx, ny });
				}
			}
		}

		if (found) {
			// Carve path from nearestPath to key
			Cell c = nearestPath;
			while (!(c.x == keyX && c.y == keyY)) {
				if (c.x < keyX) c.x++;
				else if (c.x > keyX) c.x--;
				else if (c.y < keyY) c.y++;
				else if (c.y > keyY) c.y--;

				if (MazeGrid[c.y][c.x] == "Empty") {
					MazeGrid[c.y][c.x] = "Path";
				}
			}

			MazeGrid[keyY][keyX] = "Key";
			keysPlaced++;
			UE_LOG(LogTemp, Warning, TEXT("Placed a key at (%d, %d)"), keyY, keyX);
		}
	}


	// Step 5: Add enemies far from entrance
	for (int y = 0; y < MAZESIZE; y++) {
		for (int x = 0; x < MAZESIZE; x++) {
			if (MazeGrid[y][x] == "Empty" && rand() % 20 == 0) {
				MazeGrid[y][x] = "Enemy";
			}
		}
	}

	// Step 6: Add random walls without breaking path
	for (int y = 0; y < MAZESIZE; y++) {
		for (int x = 0; x < MAZESIZE; x++) {
			if (MazeGrid[y][x] == "Empty" && rand() % 4 == 0) {
				if (IsCellReachableAfterWall(x, y, entrance, exit)) {
					MazeGrid[y][x] = "Wall";
				}
			}
		}
	}

	// Optional: Convert "Path" to "Empty" for normal floor
	for (int y = 0; y < MAZESIZE; y++) {
		for (int x = 0; x < MAZESIZE; x++) {
			if (MazeGrid[y][x] == "Path") {
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
					FVector SpawnLocation = FVector(j * TileSize - 2000.0f, i * TileSize - 1400.0f, 50.0f); // Z is up in Unreal
					GetWorld()->SpawnActor<AActor>(WallClass, SpawnLocation, FRotator::ZeroRotator);

					//UE_LOG(LogTemp, Warning, TEXT("Spawning wall at (%d, %d)"), i, j);

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
					FVector SpawnLocation = FVector(j * TileSize - 2000.0f, i * TileSize - 1400.0f, 100.0f); // Z is up in Unreal
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
				FVector SpawnLocation = FVector(yCoordinate * TileSize - 2000.0f, xCoordinate * TileSize - 1400.0f, 100.0f); // Z is up in Unreal
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

bool ALevelGeneration::IsCellReachableAfterWall(int wx, int wy, Cell entrance, Cell exit) {
	// Temporarily set wall
	FString oldValue = MazeGrid[wy][wx];
	MazeGrid[wy][wx] = "Wall";

	// BFS check
	bool visited[50][50] = { false };
	std::queue<Cell> q;
	q.push(entrance);
	visited[entrance.y][entrance.x] = true;

	int dx[4] = { 1,-1,0,0 };
	int dy[4] = { 0,0,1,-1 };

	bool exitFound = false;

	while (!q.empty()) {
		Cell c = q.front(); q.pop();
		if (c.x == exit.x && c.y == exit.y) {
			exitFound = true;
			break;
		}
		for (int dir = 0; dir < 4; dir++) {
			int nx = c.x + dx[dir];
			int ny = c.y + dy[dir];
			if (nx < 0 || nx >= MAZESIZE || ny < 0 || ny >= MAZESIZE) continue;
			if (!visited[ny][nx] && MazeGrid[ny][nx] != "Wall") {
				visited[ny][nx] = true;
				q.push({ nx, ny });
			}
		}
	}

	// Restore old value
	MazeGrid[wy][wx] = oldValue;
	return exitFound;
}
