#include "Maze.h"
#pragma execution_character_set("utf-8") 
USING_NS_CC;

#ifdef _DEBUG   
#include "vld.h"
#endif

using std::pair;

void Maze_generate(
	Matrix<char> &matrix,
	pair<int, int> &start,
	pair<int, int> &end
);

static pair<int, int> coordinate_add(const pair<int, int> a, const pair<int, int> b) {
	return{ a.first + b.first, a.second + b.second };
}

Scene* Maze::createScene()
{
    // 'scene' is an autorelease object
    auto scene = Scene::create();
    
    // 'layer' is an autorelease object
	auto layer = Maze::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

bool Maze::init() {
    if ( !Layer::init() )
    {
        return false;
    }

	dispatcher = Director::getInstance()->getEventDispatcher();
	winSize = Director::getInstance()->getWinSize();
	directions[0] = { 0, -1 };
	directions[1] = { 0, 1 };
	directions[2] = { 1, 0 };
	directions[3] = { -1, 0 };
	currentDirection = -1;
	loopMove = nullptr;
	maze = new Matrix<char>(std::make_pair(13, 17));

	initMaze({ 13, 17 });
	initKeyboardEvent();
    return true;
}

Maze::~Maze() {
	delete maze;
}

void Maze::initMaze(pair<int, int> Maze_Size) {
	maze->resize(Maze_Size);
	Sprite *newSprite;
	Maze_generate(*maze, playerPosition, end);
	for (int i = 0; i < Maze_Size.first; ++i)
		for (int j = 0; j < Maze_Size.second; ++j) {
			switch ((*maze)[i][j]) {
				case '#' : newSprite = Sprite::create("wall.png"); break;
				default : newSprite = Sprite::create("floor.jpg"); break;
			}
			newSprite->setAnchorPoint(Vec2::ZERO);
			newSprite->setPosition(gridSize.width * j, gridSize.height * i);
			addChild(newSprite);
		}
	// add end
	newSprite = Sprite::create("final.png");
	newSprite->setAnchorPoint(Vec2::ZERO);
	newSprite->setPosition(gridSize.width * end.second, gridSize.height * end.first);
	addChild(newSprite);

	// add view layer
	viewLayer = Layer::create();
	addChild(viewLayer);
	auto bgsprite = Sprite::create("bigger view.png");
	viewLayer->addChild(bgsprite);

	// add player and positioning
	player = Sprite::create("player1_2.png");
	viewLayer->addChild(player);
	viewLayer->setPosition(playerLayerPosition(playerPosition));
}

/*
 * will call startMoving whenever there is only one arrow holding
 */
void Maze::initKeyboardEvent() {
	auto listener = EventListenerKeyboard::create();
	static bool keyIsHolding[4] = { false };
	static int holdingCount = 0;

	listener->onKeyPressed = [&](EventKeyboard::KeyCode code, Event* event) {
		int key = (int)code - (int)EventKeyboard::KeyCode::KEY_LEFT_ARROW;
		if (key < 0 || key >= 4)
			return;
		++holdingCount;
		keyIsHolding[key] = true;
		if (holdingCount == 1) {
			startMoving(key);
		}
	};

	listener->onKeyReleased = [&](EventKeyboard::KeyCode code, Event* event) {
		int key = (int)code - (int)EventKeyboard::KeyCode::KEY_LEFT_ARROW;
		if (key < 0 || key >= 4)
			return;
		--holdingCount;
		keyIsHolding[key] = false;
		if (holdingCount == 1) {
			// find the only key holding
			for (key = 0; key < 4; ++key)
				if (keyIsHolding[key])
					break;
			startMoving(key);
		} else if (holdingCount == 0) {
			stopMoving();
		}
	};
	
	dispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

Vec2 Maze::playerLayerPosition(std::pair<int, int> position) {
	return Vec2((position.second + 0.5) * gridSize.width
				, (position.first + 0.5) * gridSize.height);
}

/*
 * try to change direction
 * if it was not moving, great
 * if it was moving in the same direction, do nothing
 * if it was moving in the opposite direction, stop and return
 * if it was moving in the orthogonal direction, wait for it to end
 */
void Maze::startMoving(int direction) {
	if (direction == currentDirection) {  // same
		comingDirection = direction;
	} else if ((direction ^ currentDirection) == 1) {  // opposite
		// if the player is moving, stop it
		if (loopMove) {
			viewLayer->stopAction(loopMove);
			loopMove = nullptr;
			//walkingAnimation->stop();
		}
		comingDirection = direction;
		loopMove = viewLayer->runAction(CallFunc::create(CC_CALLBACK_0(Maze::doMove, this)));
	} else if (currentDirection != -1) {  // orthogonal
		comingDirection = direction;
	} else {  // not moving
		comingDirection = direction;
		if (!loopMove)
			loopMove = viewLayer->runAction(CallFunc::create(CC_CALLBACK_0(Maze::doMove, this)));
	}
}

/* stop moving until the running action ends */
void Maze::stopMoving() {
	comingDirection = -1;
}

/*
 * move towards the comingDirection
 * force refresh the walking animation
 */
void Maze::doMove() {
	chooseMoveAction();
	currentDirection = comingDirection;
	if (currentDirection == -1) {
		if (loopMove) {
			viewLayer->stopAction(loopMove);
			loopMove = nullptr;
		}
		return;
	}
	if (maze->at(coordinate_add(playerPosition, directions[currentDirection])) != '#') {
		playerPosition = coordinate_add(playerPosition, directions[currentDirection]);
		if (playerPosition == end)
			;// win
		loopMove = viewLayer->runAction(
			Sequence::create(
				MoveTo::create((playerLayerPosition(playerPosition) - viewLayer->getPosition()).length() / 50 / 4  // 4 grid pre second
				, playerLayerPosition(playerPosition)),
				CallFunc::create(CC_CALLBACK_0(Maze::doMove, this)),
				NULL
			)
		);
	} else {  // encounters a wall, stop
		if (loopMove) {
			viewLayer->stopAction(loopMove);
			loopMove = nullptr;
		}
		currentDirection = -1;
	}
}

/* player's animation*/
void Maze::chooseMoveAction() {
	/*logic: left: 0, right: 1, up: 2, down: 3*/
	/*source: left: 2, right: 3, up: 4, down: 1*/
	int frameLabel[] = { 2, 3, 4, 1 };
	Vector<SpriteFrame*> animFrames;
	std::string framesSource = "";
	std::stringstream ss;
	if (comingDirection == -1) {
		player->stopAllActions();
		animFrames.reserve(1);
		ss << "player" << frameLabel[currentDirection] << "_2.png";
		ss >> framesSource;
		animFrames.pushBack(SpriteFrame::create(framesSource, Rect(0, 0, 50, 50)));
		ss.clear();
		auto animation = Animation::createWithSpriteFrames(animFrames, 0.5f);
		auto animate = Animate::create(animation);
		walkingAnimation = player->runAction(RepeatForever::create(animate));
	}
	else if (currentDirection != comingDirection) {
		player->stopAllActions();
		animFrames.reserve(2);
		ss << "player" << frameLabel[comingDirection] << "_1.png";
		ss >> framesSource;
		animFrames.pushBack(SpriteFrame::create(framesSource, Rect(0, 0, 50, 50)));
		ss.clear();
		ss << "player" << frameLabel[comingDirection] << "_3.png";
		ss >> framesSource;
		animFrames.pushBack(SpriteFrame::create(framesSource, Rect(0, 0, 50, 50)));
		ss.clear();
		auto animation = Animation::createWithSpriteFrames(animFrames, 0.2f);
		auto animate = Animate::create(animation);
		walkingAnimation = player->runAction(RepeatForever::create(animate));
	}
}
