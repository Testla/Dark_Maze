#include "Maze.h"
#include "SelectLevelScene.h"
#include "StartScene.h"
#include "SimpleAudioEngine.h"
#pragma execution_character_set("utf-8") 
USING_NS_CC;
using namespace CocosDenshion;

#ifdef _DEBUG   
#include "vld.h"
#endif

using std::pair;

/* 
 * initialize static member
 * mazeSize should be strictly equal or bigger than screenSize
 * both mazeSize and screenSize should be in the form (odd, odd)
 */
const pair<int, int> Maze::screenSize = { 13, 17 };
const Size Maze::gridSize = Size(50.0f, 50.0f);
std::pair<int, int> Maze::mazeSize;

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
	maze = new Matrix<char>(std::make_pair(1, 1));
	mazeLayer = Layer::create();
	addChild(mazeLayer);
	playerLayer = Layer::create();
	addChild(playerLayer);
	layerToSetRightPosition = mazeLayer;
	for (int i = 0; i < 4; ++i)
		keyIsHolding[i] = false;
	holdingCount = 0;
	playerSpeed = 4;
	torchCount = 0;

	initMaze();
	initUpdateEvent();
	initKeyboardEvent();
	SimpleAudioEngine::getInstance()->playBackgroundMusic("bgm.mp3", true);
    return true;
}

Maze::~Maze() {
	delete maze;
}

void Maze::initUpdateEvent() {
	dispatcher->addCustomEventListener("director_after_update", [=](EventCustom* event) {
		static Vec2 endPoint = Vec2((end.second + 0.5) * gridSize.width, (end.first + 0.5) * gridSize.height);
		if ((playerLayer->convertToWorldSpace(player->getPosition())
			- mazeLayer->convertToWorldSpace(monster->getPosition())).length() < 30.0f - 0.1f)
			gameOver(false);
		if (player->getBoundingBox().containsPoint(playerLayer->convertToNodeSpace(endPoint)))
			gameOver(true);
		if (itemSet.find({ (ItemType)0, playerPosition, nullptr }) != itemSet.end()) {
			Item itemTriggered = *itemSet.find({ (ItemType)0, playerPosition, nullptr });
			if ((itemTriggered.itemSprite->getBoundingBox().containsPoint(
				mazeLayer->convertToNodeSpace(playerLayer->convertToWorldSpace(player->getPosition()))))) {
				itemSet.erase(itemSet.find({ (ItemType)0, playerPosition, nullptr }));
				itemTriggered.itemSprite->removeFromParent();
				/* don't know how to use function object ... */
				switch (itemTriggered.itemType) {
				case Tornado:runAction(CallFunc::create(CC_CALLBACK_0(Maze::tornadoCallback, this))); break;
				case SpeedUp:runAction(CallFunc::create(CC_CALLBACK_0(Maze::speedUpCallback, this))); break;
				case SpeedDown:runAction(CallFunc::create(CC_CALLBACK_0(Maze::speedDownCallback, this))); break;
				case Torch:runAction(CallFunc::create(CC_CALLBACK_0(Maze::torchCallback, this))); break;
				}
			}
		}
	});
}

void Maze::initMaze() {
	maze->resize(mazeSize);
	Sprite *newSprite;
	Maze_generate(*maze, playerPosition, end);
	for (int i = 0; i < mazeSize.first; ++i)
		for (int j = 0; j < mazeSize.second; ++j) {
			switch ((*maze)[i][j]) {
				case '#' : newSprite = Sprite::create("wall.png"); break;
				default : newSprite = Sprite::create("floor.jpg"); break;
			}
			newSprite->setAnchorPoint(Vec2::ZERO);
			newSprite->setPosition(gridSize.width * j, gridSize.height * i);
			mazeLayer->addChild(newSprite);
		}
	// add end
	newSprite = Sprite::create("final.png");
	newSprite->setAnchorPoint(Vec2::ZERO);
	newSprite->setPosition(gridSize.width * end.second, gridSize.height * end.first);
	mazeLayer->addChild(newSprite);

	//add monster
	createMonster();

	// add cover
	cover = Sprite::create("normal view.png");
	playerLayer->addChild(cover);

	// add player
	player = Sprite::create("player1_2.png");
	playerLayer->addChild(player);

	// positioning
	calculateRightPosition();
	mazeLayer->setPosition(mazeLayerRightPosition);
	playerLayer->setPosition(playerLayerRightPosition);

	addItemsRandomly();
}

/*
 * will call startMoving whenever there is only one arrow holding
 */
void Maze::initKeyboardEvent() {
	auto listener = EventListenerKeyboard::create();

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
			layerToSetRightPosition->stopAction(loopMove);
			loopMove = nullptr;
		}
		comingDirection = direction;
		layerToSetRightPosition = this;
		//calculateRightPosition();
		loopMove = layerToSetRightPosition->runAction(CallFunc::create(CC_CALLBACK_0(Maze::doMove, this)));
	} else if (currentDirection != -1) {  // orthogonal
		comingDirection = direction;
	} else {  // not moving
		comingDirection = direction;
		layerToSetRightPosition = this;
		//calculateRightPosition();
		loopMove = layerToSetRightPosition->runAction(CallFunc::create(CC_CALLBACK_0(Maze::doMove, this)));
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
			layerToSetRightPosition->stopAction(loopMove);
			loopMove = nullptr;
		}
		return;
	}
	if (maze->at(coordinate_add(playerPosition, directions[currentDirection])) != '#') {
		playerPosition = coordinate_add(playerPosition, directions[currentDirection]);
		loopMove = layerToSetRightPosition->runAction(
			Sequence::create(
				doSetRightPosition(),
				CallFunc::create(CC_CALLBACK_0(Maze::checkItemTrigger, this)),
				CallFunc::create(CC_CALLBACK_0(Maze::doMove, this)),
				NULL
			)
		);
	} else {  // encounters a wall, stop
		if (loopMove) {
			layerToSetRightPosition->stopAction(loopMove);
			loopMove = nullptr;
			player->stopAllActions();
			comingDirection = -1;
			chooseMoveAction();
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

Sprite* Maze::createTornado(std::pair<int, int> position) {
	//add tornado
	Sprite* tornado;
	tornado = Sprite::create("tornado1.png");
	tornado->setAnchorPoint(Vec2::ZERO);
	tornado->setPosition(gridSize.width * position.second, gridSize.height * position.first);
	mazeLayer->addChild(tornado);

	//add tornadoAnimation
	Vector<SpriteFrame*> tornadoFrames;
	tornadoFrames.reserve(3);
	tornadoFrames.pushBack(SpriteFrame::create("tornado1.png", Rect(0, 0, 50, 50)));
	tornadoFrames.pushBack(SpriteFrame::create("tornado2.png", Rect(0, 0, 50, 50)));
	tornadoFrames.pushBack(SpriteFrame::create("tornado3.png", Rect(0, 0, 50, 50)));
	auto tornadoAnimation = Animation::createWithSpriteFrames(tornadoFrames, 0.1f);
	auto tornadoAnimate = Animate::create(tornadoAnimation);
	tornado->runAction(RepeatForever::create(tornadoAnimate));
	return tornado;
}

void Maze::createMonster() {
	//add monster
	monster = Sprite::create("monster/monster1_1.png");
	//monster->setAnchorPoint(Vec2::ZERO);
	monster->setPosition(gridSize.width * end.second, gridSize.height * end.first);
	monsterPosition = end;
	mazeLayer->addChild(monster);
	monster->runAction(CallFunc::create(CC_CALLBACK_0(Maze::monsterDoMove, this)));
	monsterComingDirection = 1;  // init direction
}

Vec2 Maze::monsterposition(std::pair<int, int> position) {
	return Vec2((position.second + 0.5) * gridSize.width
		, (position.first + 0.5) * gridSize.height);
}

void Maze::monsterDoMove() {
	std::vector<int> availableDirection;
	// collect all available directions
	for (int i = 0; i < 4; ++i) {
		if (maze->at(coordinate_add(monsterPosition, directions[i])) != '#') {
			availableDirection.push_back(i);
		}
	}
	srand(time(NULL));
	// reduce the chance for reverse direction
	int assumeDirection = availableDirection[rand() % availableDirection.size()];
	if ((monsterComingDirection^assumeDirection) == 1) {
		monsterComingDirection = availableDirection[rand() % availableDirection.size()];
	} else {
		monsterComingDirection = assumeDirection;
	}
	monster->stopAllActions();
	monsterChooseMoveAction();
	
	monsterPosition = coordinate_add(monsterPosition, directions[monsterComingDirection]);
	monster->runAction(
		Sequence::create(
		MoveTo::create(0.25f, monsterposition(monsterPosition)),
		CallFunc::create(CC_CALLBACK_0(Maze::monsterDoMove, this)),
		NULL
		)
	);
}

void Maze::monsterChooseMoveAction() {
	std::string monsterAnimationSource;
	std::stringstream ss;
	int monsterDirection[] = {2, 3, 4, 1};
	monster->stopAllActions();
	//add monsterAnimation
	Vector<SpriteFrame*> monsterFrames;
	monsterFrames.reserve(4);
	for (int i = 1; i <= 4; ++i) {
		ss.clear();
		ss << "monster/monster" << monsterDirection[monsterComingDirection] << "_" << i << ".png";
		ss >> monsterAnimationSource;
		monsterFrames.pushBack(SpriteFrame::create(monsterAnimationSource, Rect(0, 0, 50, 50)));
	}
	auto monsterAnimation = Animation::createWithSpriteFrames(monsterFrames, 0.1f);
	auto monsterAnimate = Animate::create(monsterAnimation);
	monster->runAction(RepeatForever::create(monsterAnimate));
}

void Maze::calculateRightPosition() {
	Vec2 origin;  // the left-bottom point relative to mazeLayer
	/* x coordinate */
	if (playerPosition.second <= screenSize.second / 2) {  // left over
		origin.x = 0;
	} else if (playerPosition.second > mazeSize.second - screenSize.second / 2) {  // right over
		origin.x = (mazeSize.second - screenSize.second) * gridSize.width;
	} else {  // middle
		origin.x = (playerPosition.second - screenSize.second / 2 - 1) * gridSize.width;
	}
	/* y coordinate */
	if (playerPosition.first <= screenSize.first / 2) {  // bottom over
		origin.y = 0;
	} else if (playerPosition.first > mazeSize.first - screenSize.first / 2) {  // up over
		origin.y = (mazeSize.first - screenSize.first) * gridSize.height;
	} else {  // middle
		origin.y = (playerPosition.first - screenSize.first / 2 - 1) * gridSize.height;
	}
	mazeLayerRightPosition = -origin;
	playerLayerRightPosition = Vec2(
		(playerPosition.second + 0.5) * gridSize.width,
		(playerPosition.first + 0.5) * gridSize.height
	) - origin;
}

FiniteTimeAction* Maze::doSetRightPosition() {
	calculateRightPosition();
	Vec2 currentPosition;
	currentPosition = playerLayer->getPosition();
	if (currentPosition != playerLayerRightPosition) {  // playerLayer
		//log("player");
		layerToSetRightPosition = playerLayer;
		return MoveTo::create(
			(playerLayerRightPosition - currentPosition).length() / 50 / playerSpeed
			, playerLayerRightPosition
		);
	} else {
		// FIXME: Bug. when running back and forth, sometimes it should be player to move
		// but the test passes, used this hack to force some valid action to be returned
		//log("maze");
		currentPosition = mazeLayer->getPosition();
		//if (currentPosition != mazeLayerRightPosition) {  //mazeLayer
			layerToSetRightPosition = mazeLayer;
			return MoveTo::create(
				(mazeLayerRightPosition - currentPosition).length() / 50 / playerSpeed
				, mazeLayerRightPosition
			);
		/*} else {
			log("no!");
		}*/
	}
}

void Maze::gameOver(bool winOrLose) {
	dispatcher->removeAllEventListeners();
	this->stopAllActions();
	monster->stopAllActions();
	player->stopAllActions();
	mazeLayer->stopAllActions();
	playerLayer->stopAllActions();
	cover->setVisible(false);
	auto item = MenuItemLabel::create(Label::createWithSystemFont("·µ»Ø", "Microsoft Yahei", 50.0f), CC_CALLBACK_1(Maze::back, this));
	auto menu = Menu::create(item, NULL);
	menu->setPosition(winSize / 2);
	addChild(menu, 2);
	SimpleAudioEngine::getInstance()->stopBackgroundMusic();
	SimpleAudioEngine::getInstance()->playEffect(winOrLose ? "win.mp3" : "lose.mp3");
}

void Maze::back(Ref *ref) {
	//this->removeAllChildren();
	auto scene = StartScene::createScene();
	Director::getInstance()->replaceScene(scene);
}

void Maze::addItem(const ItemType itemType, const std::pair<int, int> itemPosition) {
	static const char* ItemFilenames[] = {
		nullptr,
		"arrow_up.png",
		"arrow_down.png",
		"torch.png"
	};
	Sprite *itemSprite;
	if (itemType == Tornado) {
		itemSprite = createTornado(itemPosition);
	} else {
		itemSprite = Sprite::create(ItemFilenames[(int)itemType]);
		itemSprite->setAnchorPoint(Vec2::ZERO);
		itemSprite->setPosition(
			gridSize.width * itemPosition.second,
			gridSize.height * itemPosition.first
			);
		mazeLayer->addChild(itemSprite);
	}
	itemSet.insert({ itemType, itemPosition, itemSprite });
}

void Maze::addItemsRandomly() {
	static const int density = 10;  // grids per item
	int expectedNumItems, actualNumItems = 0;
	ItemType itemType;
	pair<int, int> itemPosition;
	expectedNumItems = mazeSize.first * mazeSize.second / density;
	for (int i = 0; i < expectedNumItems * 2; ++i)
		if (rand() & 1)
			++actualNumItems;
	for (int i = 0; i < actualNumItems; ++i) {
		itemType = (ItemType)(rand() % NumberOfItemTypes);
		do {
			itemPosition.first = rand() % mazeSize.first;
			itemPosition.second = rand() % mazeSize.second;
		} while (
			maze->at(itemPosition) == '#'
			|| itemSet.find({ itemType, itemPosition, nullptr }) != itemSet.end()
			|| itemPosition == end
		);
		addItem(itemType, itemPosition);
	}
}

void Maze::checkItemTrigger() {
	return;
	if (itemSet.find({ (ItemType)0, playerPosition, nullptr }) != itemSet.end()) {
		Item itemTriggered = *itemSet.find({ (ItemType)0, playerPosition, nullptr });
		itemSet.erase(itemSet.find({ (ItemType)0, playerPosition, nullptr }));
		itemTriggered.itemSprite->removeFromParent();
		/* don't know how to use function object ... */
		switch (itemTriggered.itemType) {
			case Tornado:runAction(CallFunc::create(CC_CALLBACK_0(Maze::tornadoCallback, this))); break;
			case SpeedUp:runAction(CallFunc::create(CC_CALLBACK_0(Maze::speedUpCallback, this))); break;
			case SpeedDown:runAction(CallFunc::create(CC_CALLBACK_0(Maze::speedDownCallback, this))); break;
			case Torch:runAction(CallFunc::create(CC_CALLBACK_0(Maze::torchCallback, this))); break;
		}
	}
}

void Maze::tornadoCallback() {
	pair<int, int> randomPosition;
	do {
		randomPosition.first = rand() % mazeSize.first;
		randomPosition.second = rand() % mazeSize.second;
	} while (
		maze->at(randomPosition) == '#'
		|| itemSet.find({ (ItemType)0, randomPosition, nullptr }) != itemSet.end()
	);
	if (loopMove)
		layerToSetRightPosition->stopAction(loopMove);
	player->stopAction(walkingAnimation);
	playerPosition = randomPosition;
	calculateRightPosition();
	mazeLayer->setPosition(mazeLayerRightPosition);
	playerLayer->setPosition(playerLayerRightPosition);
	loopMove = layerToSetRightPosition->runAction(CallFunc::create(CC_CALLBACK_0(Maze::doMove, this)));
}

void Maze::speedUpCallback() {
	playerSpeed *= 2;
	runAction(Sequence::create(
		DelayTime::create(5.0f),
		CallFunc::create(CC_CALLBACK_0(Maze::endSpeedUp, this)),
		NULL
	));
}

void Maze::endSpeedUp() {
	playerSpeed /= 2;
}

void Maze::speedDownCallback() {
	playerSpeed /= 2;
	runAction(Sequence::create(
		DelayTime::create(5.0f),
		CallFunc::create(CC_CALLBACK_0(Maze::endSpeedDown, this)),
		NULL
	));
}

void Maze::endSpeedDown() {
	playerSpeed *= 2;
}

void Maze::torchCallback() {
	++torchCount;
	cover->setSpriteFrame(SpriteFrame::create("bigger view.png", Rect(0, 0, 1650, 1250)));
	runAction(Sequence::create(
		DelayTime::create(10.0f),
		CallFunc::create(CC_CALLBACK_0(Maze::endTorch, this)),
		NULL
	));
}

void Maze::endTorch() {
	if (--torchCount == 0)
		cover->setSpriteFrame(SpriteFrame::create("normal view.png", Rect(0, 0, 1650, 1250)));
}
