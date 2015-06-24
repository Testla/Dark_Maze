#include "SelectLevelScene.h"
#include "Maze.h"
#pragma execution_character_set("utf-8")
USING_NS_CC;

Scene* SelectLevel::createScene()
{
	// 'scene' is an autorelease object
	auto scene = Scene::create();

	// 'layer' is an autorelease object
	auto layer = SelectLevel::create();

	// add layer as a child to scene
	scene->addChild(layer);

	// return the scene
	return scene;
}

bool SelectLevel::init() {
	if (!Layer::init())
	{
		return false;
	}

	dispatcher = Director::getInstance()->getEventDispatcher();
	winSize = Director::getInstance()->getWinSize();

	Sprite* background = Sprite::create("StartScene.png");
	background->setPosition(winSize / 2);
	addChild(background);

	auto item1 = MenuItemLabel::create(Label::createWithSystemFont("初级", "Microsoft Yahei", 30.0f), CC_CALLBACK_1(SelectLevel::start, this, 0));
	auto item2 = MenuItemLabel::create(Label::createWithSystemFont("中级", "Microsoft Yahei", 30.0f), CC_CALLBACK_1(SelectLevel::start, this, 1));
	auto item3 = MenuItemLabel::create(Label::createWithSystemFont("高级", "Microsoft Yahei", 30.0f), CC_CALLBACK_1(SelectLevel::start, this, 2));

	// create menu, it's an autorelease object
	auto menu = Menu::create(item1, item2, item3, NULL);
	menu->setPosition(winSize.width / 2, winSize.height / 2);
	menu->alignItemsVerticallyWithPadding(item1->getContentSize().height / 2);
	this->addChild(menu, 1);

	return true;
}

void SelectLevel::start(Ref* ref, const int difficulty) {
	static const std::pair<int, int> mazeSizes[] = {
		{13, 17},
		{23, 27},
		{33, 33}
	};
	Maze::mazeSize = mazeSizes[difficulty];
	this->stopAllActions();
	auto scene = Maze::createScene();
	Director::getInstance()->replaceScene(scene);
}
