#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
//using namespace CSC8503;

#include <chrono>
#include <thread>
#include <sstream>

/*
void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();

	int someData = 0;

	NewState A = [](void* st) {
		int* real = (int*)st;
		(*real)++;
		std::cout << "I'm in State A" << std::endl;
	};

	NewState B = [](void* data) {
		int* realData = (int*)data;
		(*realData)--;
		std::cout << "I'm in State B" << std::endl;
	};

	NormalState* stateA = new NormalState(A, (void*)&someData);
	NormalState* stateB = new NormalState(B, (void*)&someData);
	testMachine->AddState(stateA);
	testMachine->AddState(stateB);

	// if greater than 10, A to B
	GenericTransition<int&, int>* transA = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::GreaterThanTransition,
		someData, 10, stateA, stateB);

	// if equals 0, B to A
	GenericTransition<int&, int>* transB = new GenericTransition<int&, int>(
		GenericTransition<int&, int>::EqualsTransition,
		someData, 0, stateB, stateA);

	testMachine->AddTransition(transA);
	testMachine->AddTransition(transB);

	for (int i = 0; i < 100; ++i) {
		// run state machine
		testMachine->Update(1.0);
	}
	delete testMachine;
}

class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(string name) {
		this->name = name;
	}

	void ReceivePacket(int type, GamePacket* payload, int source) {
		if (type == String_Message) {
			StringPacket* realPacket = (StringPacket*)payload;

			string msg = realPacket->GetStringFromData();

			std::cout << name << " received message: " << msg << std::endl;
		}
	}

protected:
	string name;
};


void TestNetworking() {
	/*
	NetworkBase::Initialise();

	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");

	int port = NetworkBase::GetDefaultPort();

	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();

	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);

	bool canConnect = client->Connect(127, 0, 0, 1, port);

	for (int i = 0; i < 100; ++i) {
		server->SendGlobalPacket(StringPacket("Server says hello " + std::to_string(i)));

		client->SendPacket(StringPacket("Client says hello " + std::to_string(i)));

		server->UpdateServer();
		client->UpdateClient();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	NetworkBase::Destroy();
	*/


vector<Vector3> testNodes;

void TestPathfinding() {
	NavigationGrid grid("TestGrid1.txt");
	//NavigationGrid grid("CourseworkMap.txt");

	NavigationPath outPath;

	//Vector3 startPos(310, 0, 240);
	//Vector3 endPos(10, 0, 340);

	Vector3 startPos(80, 0, 10);
	Vector3 endPos(80, 0, 80);

	bool found = grid.FindPath(startPos, endPos, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos))
		testNodes.push_back(pos);
}


void DisplayPathfinding() {
	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];

		Debug::DrawLine(a, b, Vector4(1, 0, 0, 1));
	}
}

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);

	if (!w->HasInitialised()) {
		return -1;
	}	

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	TutorialGame* g = new TutorialGame();
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);
	}
	Window::DestroyGameWindow();
}